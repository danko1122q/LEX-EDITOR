#include "core_prompt.h"

#include "core_editor.h"
#include "core_input.h"
#include "core_output.h"
#include "core_prompt.h"
#include "core_terminal.h"
#include "core_unicode.h"

#include <ctype.h>
#include <stdarg.h>

/**
 * editorMsg - Display a message in the console area
 * @fmt: Format string (printf-style)
 * @...: Variable arguments for format string
 * 
 * Adds a message to the console message circular buffer.
 * Messages are displayed at the bottom of the screen above
 * the status bar. The buffer maintains a fixed number of
 * recent messages.
 */
void editorMsg(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(gEditor.con_msg[gEditor.con_rear], sizeof(gEditor.con_msg[0]), fmt, ap);
  va_end(ap);

  // Handle circular buffer wraparound
  if (gEditor.con_front == gEditor.con_rear)
  {
    // Buffer is full, move front pointer (oldest message gets overwritten)
    gEditor.con_front = (gEditor.con_front + 1) % EDITOR_CON_COUNT;
    gEditor.con_size--;
  }
  else if (gEditor.con_front == -1)
  {
    // First message, initialize front pointer
    gEditor.con_front = 0;
  }
  
  gEditor.con_size++;
  gEditor.con_rear = (gEditor.con_rear + 1) % EDITOR_CON_COUNT;
}

/**
 * editorMsgClear - Clear all console messages
 * 
 * Resets the console message buffer, removing all displayed messages.
 */
void editorMsgClear(void)
{
  gEditor.con_front = -1;
  gEditor.con_rear  = 0;
  gEditor.con_size  = 0;
}

/**
 * editorSetPrompt - Set the prompt text (left side)
 * @fmt: Format string (printf-style)
 * @...: Variable arguments for format string
 * 
 * Sets the text displayed on the left side of the prompt line.
 * Supports printf-style formatting.
 */
static void editorSetPrompt(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(gEditor.prompt, sizeof(gEditor.prompt), fmt, ap);
  va_end(ap);
}

/**
 * editorSetRightPrompt - Set the right prompt text
 * @fmt: Format string (printf-style)
 * @...: Variable arguments for format string
 * 
 * Sets the text displayed on the right side of the prompt line.
 * Useful for showing additional information like search results count.
 */
static void editorSetRightPrompt(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(gEditor.prompt_right, sizeof(gEditor.prompt_right), fmt, ap);
  va_end(ap);
}

// Initial buffer size for prompt input
#define PROMPT_BUF_INIT_SIZE 64
// Growth rate when buffer needs to expand
#define PROMPT_BUF_GROWTH_RATE 2.0f

/**
 * editorPrompt - Display a prompt and get user input
 * @prompt: Prompt text to display (can contain %s for user input)
 * @state: Editor state to enter while prompting
 * @callback: Function called on each input event (can be NULL)
 * 
 * Enters a prompt mode where the user can type text. The prompt string
 * should contain %s where the user's input will be displayed.
 * 
 * The callback function is called after each keypress, allowing for
 * real-time processing of the input (e.g., incremental search).
 * 
 * Returns: The entered string if confirmed with Enter, NULL if cancelled
 */
char *editorPrompt(const char *prompt, int state, void (*callback)(char *, int))
{
  int old_state = gEditor.state;
  gEditor.state = state;

  // Allocate initial buffer for user input
  size_t bufsize = PROMPT_BUF_INIT_SIZE;
  char  *buf     = malloc_s(bufsize);

  size_t buflen = 0;
  buf[0]        = '\0';

  // Find the position of %s in prompt to calculate cursor start position
  int start = 0;
  while (prompt[start] != '\0' && prompt[start] != '%')
  {
    start++;
  }
  gEditor.px = start;
  
  while (true)
  {
    // Update prompt with current input
    editorSetPrompt(prompt, buf);
    editorRefreshScreen();

    // Read user input
    EditorInput input = editorReadKey();
    int         x     = input.data.cursor.x;
    int         y     = input.data.cursor.y;

    // Calculate cursor index in buffer
    size_t idx = gEditor.px - start;
    
    switch (input.type)
    {
      case DEL_KEY:
        // Delete key deletes character after cursor
        if (idx != buflen)
          idx++;
        else
          break;
        // Fall through to backspace logic
        __attribute__((fallthrough));
        
      case CTRL_KEY('h'):
      case BACKSPACE:
        // Delete character before cursor
        if (idx != 0)
        {
          memmove(&buf[idx - 1], &buf[idx], buflen - idx + 1);
          buflen--;
          idx--;
          if (callback)
            callback(buf, input.type);
        }
        break;

      case PASTE_INPUT:
      case CTRL_KEY('v'):
      {
        // Handle paste operation
        EditorClipboard *clipboard =
            (input.type == PASTE_INPUT) ? &input.data.paste : &gEditor.clipboard;
        if (!clipboard->size)
          break;
          
        // Only paste the first line of clipboard content
        const char *paste_buf = clipboard->lines[0].data;
        size_t      paste_len = clipboard->lines[0].size;
        if (paste_len == 0)
          break;

        // Expand buffer if needed
        if (buflen + paste_len >= bufsize)
        {
          bufsize = buflen + paste_len + 1;
          bufsize *= PROMPT_BUF_GROWTH_RATE;
          buf = realloc_s(buf, bufsize);
        }
        
        // Insert pasted text at cursor position
        memmove(&buf[idx + paste_len], &buf[idx], buflen - idx + 1);
        memcpy(&buf[idx], paste_buf, paste_len);
        buflen += paste_len;
        idx += paste_len;

        if (callback)
        {
          // Send ctrl-v in case callback didn't handle PASTE_INPUT
          callback(buf, CTRL_KEY('v'));
        }

        break;
      }

      case HOME_KEY:
        // Move cursor to start of input
        idx = 0;
        break;

      case END_KEY:
        // Move cursor to end of input
        idx = buflen;
        break;

      case ARROW_LEFT:
        // Move cursor left one character
        if (idx != 0)
          idx--;
        break;

      case ARROW_RIGHT:
        // Move cursor right one character
        if (idx < buflen)
          idx++;
        break;

      case WHEEL_UP:
        // Scroll editor content up
        editorScroll(-3);
        break;

      case WHEEL_DOWN:
        // Scroll editor content down
        editorScroll(3);
        break;

      case MOUSE_PRESSED:
      {
        int field = getMousePosField(x, y);
        if (field == FIELD_PROMPT)
        {
          // Click in prompt area - move cursor to clicked position
          if (x >= start)
          {
            size_t cx = x - start;
            if (cx < buflen)
              idx = cx;
            else
              idx = buflen;
          }
          break;
        }
        else if (field == FIELD_TEXT)
        {
          // Click in text area - move editor cursor
          mousePosToEditorPos(&x, &y);
          gCurFile->cursor.y = y;
          gCurFile->cursor.x = editorRowRxToCx(&gCurFile->row[y], x);
          gCurFile->sx       = x;
        }
      }
      // Fall through to cancel
      __attribute__((fallthrough));
      
      // MODIFICATION: Changed cancel shortcut from Ctrl+Q to Ctrl+X
      case CTRL_KEY('x'):
      case ESC:
        // Cancel prompt and return NULL
        editorSetPrompt("");
        gEditor.state = old_state;
        if (callback)
          callback(buf, input.type);
        free(buf);
        return NULL;
        // END MODIFICATION

      case '\r':
        // Enter key - confirm input
        if (buflen != 0)
        {
          editorSetPrompt("");
          gEditor.state = old_state;
          if (callback)
            callback(buf, input.type);
          return buf;
        }
        break;

      case CHAR_INPUT:
      {
        // Handle regular character input
        char output[4];
        int  len = encodeUTF8(input.data.unicode, output);
        if (len == -1)
          return buf;

        // Expand buffer if needed
        if (buflen + len >= bufsize)
        {
          bufsize += len;
          bufsize *= PROMPT_BUF_GROWTH_RATE;
          buf = realloc_s(buf, bufsize);
        }
        
        // Insert character at cursor position
        memmove(&buf[idx + len], &buf[idx], buflen - idx + 1);
        memcpy(&buf[idx], output, len);
        buflen += len;
        idx += len;

        // TODO: Support Unicode characters in prompt

        if (callback)
          callback(buf, input.data.unicode);
      }
      break;

      default:
        // Pass other input types to callback
        if (callback)
          callback(buf, input.type);
    }
    
    editorFreeInput(&input);
    gEditor.px = start + idx;
  }
}

// ========== Goto Line Feature ==========

/**
 * editorGotoCallback - Callback for goto line prompt
 * @query: Current input string
 * @key: Key that was pressed
 * 
 * Processes the goto line command. Accepts line numbers (positive or negative).
 * Negative numbers count from the end of the file (-1 = last line).
 */
static void editorGotoCallback(char *query, int key)
{
  // MODIFICATION: Also handle Ctrl+X here
  if (key == ESC || key == CTRL_KEY('x'))
  {
    return;
  }

  editorMsgClear();

  // No input yet, nothing to do
  if (query == NULL || query[0] == '\0')
  {
    return;
  }

  // Convert input to line number
  int line = strToInt(query);

  // Negative numbers count from end of file
  if (line < 0)
  {
    line = gCurFile->num_rows + 1 + line;
  }

  // Jump to line if valid
  if (line > 0 && line <= gCurFile->num_rows)
  {
    gCurFile->cursor.x = 0;
    gCurFile->sx       = 0;
    gCurFile->cursor.y = line - 1;  // Convert to 0-indexed
    editorScrollToCursorCenter();
  }
  else
  {
    editorMsg("Type a line number between 1 to %d (negative too).", gCurFile->num_rows);
  }
}

/**
 * editorGotoLine - Show goto line prompt
 * 
 * Prompts the user to enter a line number and jumps to that line.
 * Supports both positive (1-based) and negative (from end) line numbers.
 */
void editorGotoLine(void)
{
  char *query = editorPrompt("Goto line: %s", GOTO_LINE_MODE, editorGotoCallback);
  if (query)
  {
    free(query);
  }
}

// ========== Find/Search Feature ==========

/**
 * struct FindList - Linked list node for search results
 * @prev: Previous match in list
 * @next: Next match in list
 * @row: Row number of match
 * @col: Column number of match
 * 
 * Used to store all matches for the current search query,
 * allowing navigation between results.
 */
typedef struct FindList
{
  struct FindList *prev;
  struct FindList *next;
  int              row;
  int              col;
} FindList;

/**
 * findListFree - Free a find list
 * @thisptr: Pointer to first node to free
 * 
 * Frees all nodes in the find list.
 */
static void findListFree(FindList *thisptr)
{
  FindList *temp;
  while (thisptr)
  {
    temp    = thisptr;
    thisptr = thisptr->next;
    free(temp);
  }
}

/**
 * editorFindCallback - Callback for find/search prompt
 * @query: Current search query
 * @key: Key that was pressed
 * 
 * Handles incremental search with the following features:
 * - Real-time search as you type
 * - Navigate results with Up/Down arrows
 * - Highlight current match
 * - Show match count (e.g., "3 of 10")
 * - Case-sensitive or case-insensitive search (configurable)
 * - Smart case: case-insensitive if query is all lowercase
 */
static void editorFindCallback(char *query, int key)
{
  // Static variables maintain state between callback invocations
  static char     *prev_query = NULL;  // Previous query to detect changes
  static FindList  head       = {.prev = NULL, .next = NULL};  // List head
  static FindList *match_node = NULL;  // Current match

  // Saved highlight state for restoring after search
  static uint8_t *saved_hl_pos = NULL;
  static uint8_t *saved_hl     = NULL;
  static size_t   saved_hl_len = 0;

  static int total   = 0;  // Total matches found
  static int current = 0;  // Current match index (1-based)

  // Restore previous highlight before applying new one
  if (saved_hl && saved_hl_pos)
  {
    memcpy(saved_hl_pos, saved_hl, saved_hl_len);
    free(saved_hl);
    saved_hl     = NULL;
    saved_hl_pos = NULL;
    saved_hl_len = 0;
  }

  // Quit find mode
  // MODIFICATION: Changed cancel shortcut from Ctrl+Q to Ctrl+X
  if (key == ESC || key == CTRL_KEY('x') || key == '\r' || key == MOUSE_PRESSED)
  {
    // END MODIFICATION
    // Clean up all allocated resources
    if (prev_query)
    {
      free(prev_query);
      prev_query = NULL;
    }
    if (saved_hl)
    {
      free(saved_hl);
      saved_hl = NULL;
    }
    findListFree(head.next);
    head.next = NULL;
    editorSetRightPrompt("");
    return;
  }

  size_t len = strlen(query);
  if (len == 0)
  {
    editorSetRightPrompt("");
    return;
  }

  FindList *tail_node = NULL;
  
  // Recompute search results if query changed
  if (!head.next || !prev_query || strcmp(prev_query, query) != 0)
  {
    // Recompute find list

    total   = 0;
    current = 0;

    match_node = NULL;
    if (prev_query)
      free(prev_query);
    findListFree(head.next);
    head.next = NULL;

    // Save new query
    prev_query = malloc_s(len + 1);
    memcpy(prev_query, query, len + 1);
    prev_query[len] = '\0';

    // Determine case sensitivity mode
    int  ignorecase_mode = CONVAR_GETINT(ignorecase);
    bool ignore_case     = false;
    if (ignorecase_mode == 1)
    {
      // Always case-insensitive
      ignore_case = true;
    }
    else if (ignorecase_mode == 2)
    {
      // Smart case: case-insensitive if query is all lowercase
      bool has_upper = false;
      for (size_t j = 0; j < len; j++)
      {
        if (isupper((unsigned char) query[j]))
        {
          has_upper = true;
          break;
        }
      }
      ignore_case = !has_upper;
    }

    // Search through all rows
    FindList *cur = &head;
    for (int i = 0; i < gCurFile->num_rows; i++)
    {
      size_t col     = 0;
      size_t row_len = (size_t) gCurFile->row[i].size;

      // Find all matches in current row
      while (col < row_len)
      {
        int match_idx = findSubstring(gCurFile->row[i].data, row_len, query, len, col, ignore_case);
        if (match_idx < 0)
          break;

        col = (size_t) match_idx;

        // Add match to list
        FindList *node = malloc_s(sizeof(FindList));
        node->prev     = cur;
        node->next     = NULL;
        node->row      = i;
        node->col      = (int) col;
        cur->next      = node;
        cur            = cur->next;
        tail_node      = cur;

        total++;
        
        // Find first match after cursor position
        if (!match_node)
        {
          current++;
          if (((i == gCurFile->cursor.y && col >= (size_t) gCurFile->cursor.x) ||
               i > gCurFile->cursor.y))
          {
            match_node = cur;
          }
        }
        col += len;
      }
    }

    // No matches found
    if (!head.next)
    {
      editorSetRightPrompt("  No results");
      return;
    }

    // If no match after cursor, wrap to first match
    if (!match_node)
      match_node = head.next;

    // Make list circular (don't go back to dummy head)
    head.next->prev = tail_node;
  }

  // Navigate between matches
  if (key == ARROW_DOWN)
  {
    // Next match
    if (match_node->next)
    {
      match_node = match_node->next;
      current++;
    }
    else
    {
      // Wrap to first match
      match_node = head.next;
      current    = 1;
    }
  }
  else if (key == ARROW_UP)
  {
    // Previous match
    match_node = match_node->prev;
    if (current == 1)
      current = total;
    else
      current--;
  }
  
  // Show match count
  editorSetRightPrompt("  %d of %d", current, total);

  // Move cursor to current match
  gCurFile->cursor.x = match_node->col;
  gCurFile->cursor.y = match_node->row;

  editorScrollToCursorCenter();

  // Highlight current match
  uint8_t *match_pos = &gCurFile->row[match_node->row].hl[match_node->col];
  saved_hl_len       = len;
  saved_hl_pos       = match_pos;
  saved_hl           = malloc_s(len + 1);
  memcpy(saved_hl, match_pos, len);
  
  // Apply match highlight to all characters in match
  for (size_t i = 0; i < len; i++)
  {
    match_pos[i] &= ~HL_BG_MASK;
    match_pos[i] |= HL_BG_MATCH << HL_FG_BITS;
  }
}

/**
 * editorFind - Show find/search prompt
 * 
 * Enters incremental search mode where the user can:
 * - Type to search for text
 * - Use Up/Down arrows to navigate between matches
 * - See real-time results as they type
 * - See match count and current position
 */
void editorFind(void)
{
  char *query = editorPrompt("Find: %s", FIND_MODE, editorFindCallback);
  if (query)
  {
    free(query);
  }
}