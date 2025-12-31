#include "core_highlight.h"

#include "../resources/bundle.h"
#include "core_config.h"
#include "core_editor.h"
#include "core_os.h"

#include <ctype.h>

// JSON parser configuration
#define JSON_IMPLEMENTATION
#define JSON_MALLOC malloc_s
#include "core_json.h"

/**
 * editorUpdateSyntax - Update syntax highlighting for a single row
 * @file: The file containing the row
 * @row: The row to update highlighting for
 * 
 * Performs syntax highlighting on a single line based on the file's
 * syntax definition. Handles:
 * - Single-line comments
 * - Multi-line comments (with state tracking across lines)
 * - String literals (with escape sequences)
 * - Numbers (decimal, hex, octal, float)
 * - Keywords (3 categories)
 * - Trailing whitespace
 * 
 * The function uses a state machine approach and may recursively
 * update the next line if multi-line comment state changes.
 */
void editorUpdateSyntax(EditorFile *file, EditorRow *row)
{
  if (row->hl)
  {
    // Reset all highlighting to normal
    // Note: realloc might return NULL when row->size == 0
    memset(row->hl, HL_NORMAL, row->size);
  }

  EditorSyntax *s = file->syntax;

  // Skip if syntax highlighting is disabled or no syntax defined
  if (!CONVAR_GETINT(syntax) || !s)
    goto update_trailing;

  // Get comment delimiters from syntax definition
  const char *scs = s->singleline_comment_start;
  const char *mcs = s->multiline_comment_start;
  const char *mce = s->multiline_comment_end;

  int scs_len = scs ? strlen(scs) : 0;
  int mcs_len = mcs ? strlen(mcs) : 0;
  int mce_len = mce ? strlen(mce) : 0;

  // State variables for syntax highlighting
  int prev_sep   = 1;  // Previous character was a separator
  int in_string  = 0;  // Currently inside a string (stores opening quote char)
  int row_index  = (int) (row - file->row);
  int in_comment = (row_index > 0 && file->row[row_index - 1].hl_open_comment);

  int i = 0;
  while (i < row->size)
  {
    char c = row->data[i];

    // Handle single-line comments
    if (scs_len && !in_string && !in_comment)
    {
      if (i + scs_len <= row->size && strncmp(&row->data[i], scs, scs_len) == 0)
      {
        // Rest of line is a comment
        memset(&row->hl[i], HL_COMMENT, row->size - i);
        break;
      }
    }

    // Handle multi-line comments
    if (mcs_len && mce_len && !in_string)
    {
      if (in_comment)
      {
        // Currently inside a multi-line comment
        row->hl[i] = HL_COMMENT;
        if (i + mce_len <= row->size && strncmp(&row->data[i], mce, mce_len) == 0)
        {
          // Found comment end delimiter
          memset(&row->hl[i], HL_COMMENT, mce_len);
          i += mce_len;
          in_comment = 0;
          prev_sep   = 1;
        }
        i++;
        continue;
      }
      else if (i + mcs_len <= row->size && strncmp(&row->data[i], mcs, mcs_len) == 0)
      {
        // Found comment start delimiter
        memset(&row->hl[i], HL_COMMENT, mcs_len);
        i += mcs_len;
        in_comment = 1;
        continue;
      }
    }

    // Handle string literals
    if (s->flags & HL_HIGHLIGHT_STRINGS)
    {
      if (in_string)
      {
        row->hl[i] = HL_STRING;
        
        // Handle escape sequences
        if (c == '\\' && i + 1 < row->size)
        {
          row->hl[i + 1] = HL_STRING;
          i += 2;
          continue;
        }
        
        // Check for closing quote
        if (c == in_string)
          in_string = 0;
        i++;
        prev_sep = 1;
        continue;
      }
      else if (c == '"' || c == '\'')
      {
        // Start of string
        in_string  = c;
        row->hl[i] = HL_STRING;
        i++;
        continue;
      }
    }

    // Handle number literals
    if (s->flags & HL_HIGHLIGHT_NUMBERS)
    {
      if ((isdigit((uint8_t) c) || c == '.') && prev_sep)
      {
        int start = i;
        i++;
        
        // Special handling for numbers starting with 0
        if (c == '0')
        {
          if (i < row->size)
          {
            if (row->data[i] == 'x' || row->data[i] == 'X')
            {
              // Hexadecimal number (0x...)
              i++;
              while (i < row->size && (isdigit((uint8_t) row->data[i]) ||
                                       (row->data[i] >= 'a' && row->data[i] <= 'f') ||
                                       (row->data[i] >= 'A' && row->data[i] <= 'F')))
              {
                i++;
              }
            }
            else if (row->data[i] >= '0' && row->data[i] <= '7')
            {
              // Octal number (0...)
              i++;
              while (i < row->size && row->data[i] >= '0' && row->data[i] <= '7')
              {
                i++;
              }
            }
            else if (row->data[i] == '.')
            {
              // Floating point starting with 0. (0.123)
              i++;
              while (i < row->size && isdigit((uint8_t) row->data[i]))
              {
                i++;
              }
            }
          }
        }
        else
        {
          // Regular decimal or floating point number
          while (i < row->size && isdigit((uint8_t) row->data[i]))
          {
            i++;
          }
          
          // Check for decimal point
          if (c != '.' && i < row->size && row->data[i] == '.')
          {
            i++;
            while (i < row->size && isdigit((uint8_t) row->data[i]))
            {
              i++;
            }
          }
        }
        
        // Don't highlight lone '.' as a number
        if (c == '.' && i - start == 1)
          continue;

        // Handle float suffix (f or F)
        if (i < row->size && (row->data[i] == 'f' || row->data[i] == 'F'))
          i++;
          
        // Only highlight if followed by separator or whitespace
        if (i == row->size || isSeparator(row->data[i]) || isSpace(row->data[i]))
          memset(&row->hl[start], HL_NUMBER, i - start);
        prev_sep = 0;
        continue;
      }
    }

    // Handle keywords (only after separators)
    if (prev_sep)
    {
      bool found_keyword = false;
      
      // Check all three keyword categories
      for (int kw = 0; kw < 3; kw++)
      {
        for (size_t j = 0; j < s->keywords[kw].size; j++)
        {
          int klen         = strlen(s->keywords[kw].data[j]);
          int keyword_type = HL_KEYWORD1 + kw;
          
          // Match keyword and ensure it's followed by non-identifier char
          if (klen <= row->size - i && strncmp(&row->data[i], s->keywords[kw].data[j], klen) == 0 &&
              (i + klen == row->size || isNonIdentifierChar(row->data[i + klen])))
          {
            found_keyword = true;
            memset(&row->hl[i], keyword_type, klen);
            i += klen;
            break;
          }
        }
        if (found_keyword)
        {
          break;
        }
      }

      if (found_keyword)
      {
        prev_sep = 0;
        continue;
      }
    }
    
    // Update separator state
    prev_sep = isNonIdentifierChar(c);
    i++;
  }
  
  // Update multi-line comment state
  int changed          = (row->hl_open_comment != in_comment);
  row->hl_open_comment = in_comment;
  
  // Recursively update next line if comment state changed
  if (changed && row_index + 1 < file->num_rows)
    editorUpdateSyntax(file, &file->row[row_index + 1]);

  // Highlight trailing whitespace
update_trailing:
  for (i = row->size - 1; i >= 0; i--)
  {
    if (row->data[i] == ' ' || row->data[i] == '\t')
    {
      row->hl[i] = HL_BG_TRAILING << HL_FG_BITS;
    }
    else
    {
      break;
    }
  }
}

/**
 * editorSetSyntaxHighlight - Set syntax highlighting for a file
 * @file: The file to set syntax for
 * @syntax: The syntax definition to use
 * 
 * Sets the syntax definition for a file and updates highlighting
 * for all rows in the file.
 */
void editorSetSyntaxHighlight(EditorFile *file, EditorSyntax *syntax)
{
  file->syntax = syntax;
  for (int i = 0; i < file->num_rows; i++)
  {
    editorUpdateSyntax(file, &file->row[i]);
  }
}

/**
 * editorSelectSyntaxHighlight - Auto-detect and set syntax for a file
 * @file: The file to detect syntax for
 * 
 * Automatically selects appropriate syntax highlighting based on
 * the file's extension or filename pattern. Searches through all
 * registered syntax definitions in HLDB.
 */
void editorSelectSyntaxHighlight(EditorFile *file)
{
  file->syntax = NULL;
  if (file->filename == NULL)
    return;

  // Get file extension
  char *ext = strrchr(file->filename, '.');

  // Search through all syntax definitions
  for (EditorSyntax *s = gEditor.HLDB; s; s = s->next)
  {
    for (size_t i = 0; i < s->file_exts.size; i++)
    {
      int is_ext = (s->file_exts.data[i][0] == '.');
      
      // Match by extension or filename pattern
      if ((is_ext && ext && strCaseCmp(ext, s->file_exts.data[i]) == 0) ||
          (!is_ext && strCaseStr(file->filename, s->file_exts.data[i])))
      {
        editorSetSyntaxHighlight(file, s);
        return;
      }
    }
  }
}

// Arena size for JSON parsing (4KB)
#define ARENA_SIZE (1 << 12)

// Memory arena for syntax definition parsing
static JsonArena hldb_arena;

static void loadEditorConfigHLDB(void);
static void editorLoadBundledHLDB(void);

/**
 * editorInitHLDB - Initialize the syntax highlighting database
 * 
 * Loads all syntax definitions from:
 * 1. Built-in editor config syntax
 * 2. Bundled syntax files (compiled into binary)
 * 3. User syntax files from ~/.config/lx/syntax/ (*.json files)
 */
void editorInitHLDB(void)
{
  // Initialize JSON parsing arena
  json_arena_init(&hldb_arena, ARENA_SIZE);

  // Load built-in and bundled syntax definitions
  loadEditorConfigHLDB();
  editorLoadBundledHLDB();

  // Load user-defined syntax files from config directory
  char path[EDITOR_PATH_MAX];
  snprintf(path, sizeof(path), PATH_CAT("%s", CONF_DIR, "syntax"), getenv(ENV_HOME));

  DirIter iter = dirFindFirst(path);
  if (iter.error)
    return;

  do
  {
    const char *filename = dirGetName(&iter);
    char        file_path[EDITOR_PATH_MAX];
    int         len = snprintf(file_path, sizeof(file_path), PATH_CAT("%s", "%s"), path, filename);

    // Suppress Wformat-truncation warning
    if (len < 0)
      continue;

    // Only load .json files
    if (getFileType(file_path) == FT_REG)
    {
      const char *ext = strrchr(filename, '.');
      if (ext && strcmp(ext, ".json") == 0)
      {
        editorLoadHLDB(file_path);
      }
    }
  } while (dirNext(&iter));
  dirClose(&iter);
}

/**
 * loadEditorConfigHLDB - Load built-in syntax for editor config files
 * 
 * Creates a syntax definition for the editor's own configuration files
 * (.lxrc and .lxconfig). Keywords include all config variables and
 * color settings.
 */
static void loadEditorConfigHLDB(void)
{
  EditorSyntax *syntax = calloc_s(1, sizeof(EditorSyntax));

  syntax->file_type = EDITOR_NAME;
  vector_push(syntax->file_exts, EDITOR_RC_FILE);
  vector_push(syntax->file_exts, EDITOR_CONFIG_EXT);
  syntax->singleline_comment_start = "#";
  syntax->multiline_comment_start  = NULL;
  syntax->multiline_comment_end    = NULL;

  // Add all config variables as keywords
  EditorConCmd *curr = gEditor.cvars;
  while (curr)
  {
    // Commands go in keywords1, variables in keywords2
    vector_push(syntax->keywords[curr->has_callback ? 0 : 1], curr->name);
    curr = curr->next;
  }

  // Add all color element names as keywords3
  for (int i = 0; i < EDITOR_COLOR_COUNT; i++)
  {
    vector_push(syntax->keywords[2], color_element_map[i].label);
  }

  syntax->flags = HL_HIGHLIGHT_STRINGS;

  // Add to beginning of HLDB linked list
  syntax->next = gEditor.HLDB;
  gEditor.HLDB = syntax;
}

/**
 * editorLoadJsonHLDB - Parse and load a syntax definition from JSON
 * @json: JSON string containing syntax definition
 * @syntax: Syntax structure to populate
 * 
 * Parses a JSON syntax definition file and populates the EditorSyntax
 * structure. The JSON format includes:
 * - name: Display name for the language
 * - extensions: Array of file extensions/patterns
 * - comment: Single-line comment delimiter
 * - multiline-comment: Array of [start, end] delimiters
 * - keywords1, keywords2, keywords3: Keyword arrays
 * 
 * Returns: true if parsing succeeded, false otherwise
 */
static bool editorLoadJsonHLDB(const char *json, EditorSyntax *syntax)
{
  // Parse JSON string
  JsonValue *value = json_parse(json, &hldb_arena);
  if (value->type != JSON_OBJECT)
  {
    return false;
  }

  // Helper macro for validation
#define CHECK(boolean)                                                                             \
  do                                                                                               \
  {                                                                                                \
    if (!(boolean))                                                                                \
      return false;                                                                                \
  } while (0)

  JsonObject *object = value->object;

  // Parse language name
  JsonValue *name = json_object_find(object, "name");
  CHECK(name && name->type == JSON_STRING);
  syntax->file_type = name->string;

  // Parse file extensions
  JsonValue *extensions = json_object_find(object, "extensions");
  CHECK(extensions && extensions->type == JSON_ARRAY);
  for (size_t i = 0; i < extensions->array->size; i++)
  {
    JsonValue *item = extensions->array->data[i];
    CHECK(item->type == JSON_STRING);
    vector_push(syntax->file_exts, item->string);
  }
  vector_shrink(syntax->file_exts);

  // Parse single-line comment delimiter (optional)
  JsonValue *comment = json_object_find(object, "comment");
  if (comment && comment->type != JSON_NULL)
  {
    CHECK(comment->type == JSON_STRING);
    syntax->singleline_comment_start = comment->string;
  }
  else
  {
    syntax->singleline_comment_start = NULL;
  }

  // Parse multi-line comment delimiters (optional)
  JsonValue *multi_comment = json_object_find(object, "multiline-comment");
  if (multi_comment && multi_comment->type != JSON_NULL)
  {
    CHECK(multi_comment->type == JSON_ARRAY);
    CHECK(multi_comment->array->size == 2);
    
    // Get start delimiter
    JsonValue *mcs = multi_comment->array->data[0];
    CHECK(mcs && mcs->type == JSON_STRING);
    syntax->multiline_comment_start = mcs->string;
    
    // Get end delimiter
    JsonValue *mce = multi_comment->array->data[1];
    CHECK(mce && mce->type == JSON_STRING);
    syntax->multiline_comment_end = mce->string;
  }
  else
  {
    syntax->multiline_comment_start = NULL;
    syntax->multiline_comment_end   = NULL;
  }
  
  // Parse keyword arrays (3 categories)
  const char *kw_fields[] = {"keywords1", "keywords2", "keywords3"};

  for (int i = 0; i < 3; i++)
  {
    JsonValue *keywords = json_object_find(object, kw_fields[i]);
    if (keywords && keywords->type != JSON_NULL)
    {
      CHECK(keywords->type == JSON_ARRAY);
      for (size_t j = 0; j < keywords->array->size; j++)
      {
        JsonValue *item = keywords->array->data[j];
        CHECK(item->type == JSON_STRING);
        vector_push(syntax->keywords[i], item->string);
      }
    }
    vector_shrink(syntax->keywords[i]);
  }

  // TODO: Add flags option in json file
  syntax->flags = HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS;

  return true;
}

/**
 * editorLoadBundledHLDB - Load all bundled syntax definitions
 * 
 * Loads syntax definitions that are compiled into the binary
 * from the resources/bundle.h file. These are the default
 * syntax definitions for common languages.
 */
static void editorLoadBundledHLDB(void)
{
  for (size_t i = 0; i < sizeof(bundle) / sizeof(bundle[0]); i++)
  {
    EditorSyntax *syntax = calloc_s(1, sizeof(EditorSyntax));
    if (editorLoadJsonHLDB(bundle[i], syntax))
    {
      // Add to HLDB linked list
      syntax->next = gEditor.HLDB;
      gEditor.HLDB = syntax;
    }
    else
    {
      // Failed to parse, free the allocation
      free(syntax);
    }
  }
}

/**
 * editorLoadHLDB - Load a syntax definition from a file
 * @path: Path to JSON syntax definition file
 * 
 * Loads and parses a syntax definition from a JSON file on disk.
 * Used to load user-defined syntax files from ~/.config/lx/syntax/
 * 
 * Returns: true if loading succeeded, false otherwise
 */
bool editorLoadHLDB(const char *path)
{
  FILE  *fp;
  size_t size;
  char  *buffer;

  // Open and read file
  fp = openFile(path, "rb");
  if (!fp)
    return false;

  // Get file size
  fseek(fp, 0, SEEK_END);
  size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  // Allocate buffer and read file
  buffer = calloc_s(1, size + 1);

  if (fread(buffer, size, 1, fp) != 1)
  {
    fclose(fp);
    free(buffer);
    return false;
  }
  fclose(fp);

  // Parse JSON and add to HLDB
  EditorSyntax *syntax = calloc_s(1, sizeof(EditorSyntax));
  if (editorLoadJsonHLDB(buffer, syntax))
  {
    // Add to HLDB linked list
    syntax->next = gEditor.HLDB;
    gEditor.HLDB = syntax;
  }
  else
  {
    // Failed to parse
    free(syntax);
  }

  free(buffer);
  return true;
}

/**
 * editorFreeHLDB - Free all syntax highlighting definitions
 * 
 * Frees all memory used by the syntax highlighting database,
 * including all syntax definitions and the JSON parsing arena.
 * Called on editor shutdown.
 */
void editorFreeHLDB(void)
{
  EditorSyntax *HLDB = gEditor.HLDB;
  
  // Free all syntax definitions in linked list
  while (HLDB)
  {
    EditorSyntax *temp = HLDB;
    HLDB               = HLDB->next;
    
    // Free file extensions array
    free(temp->file_exts.data);
    
    // Free all keyword arrays
    for (size_t i = 0; i < sizeof(temp->keywords) / sizeof(temp->keywords[0]); i++)
    {
      free(temp->keywords[i].data);
    }
    
    free(temp);
  }
  
  // Free JSON parsing arena
  json_arena_deinit(&hldb_arena);
  gEditor.HLDB = NULL;
}