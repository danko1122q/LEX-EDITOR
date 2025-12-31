#ifndef HIGHLIGHT_H
#define HIGHLIGHT_H

#include "core_utils.h"

// Forward declarations to avoid circular dependencies
typedef struct EditorFile EditorFile;
typedef struct EditorRow  EditorRow;

/**
 * Highlight color encoding masks and shifts
 *
 * Each character's highlight is stored as a single byte:
 * - Lower 4 bits (0x0F): Foreground color (text color)
 * - Upper 4 bits (0xF0): Background color (highlight/selection)
 *
 * HL_FG_BITS is used to shift background values into the upper 4 bits.
 */
#define HL_FG_MASK 0x0F  // Mask for foreground color (bits 0-3)
#define HL_BG_MASK 0xF0  // Mask for background color (bits 4-7)
#define HL_FG_BITS 4     // Number of bits for foreground (shift amount for background)

/**
 * Syntax highlighting feature flags
 *
 * Flags that can be combined (bitwise OR) to enable different
 * syntax highlighting features for a language.
 */
#define HL_HIGHLIGHT_NUMBERS (1 << 0)  // Enable number literal highlighting
#define HL_HIGHLIGHT_STRINGS (1 << 1)  // Enable string literal highlighting

/**
 * enum EditorHighlightFg - Foreground (text) color types
 *
 * Defines the different syntax element types for foreground coloring.
 * These values occupy the lower 4 bits of the highlight byte.
 *
 * @HL_NORMAL: Default text color
 * @HL_COMMENT: Comments (single-line and multi-line)
 * @HL_KEYWORD1: Primary keywords (e.g., control flow, declarations)
 * @HL_KEYWORD2: Secondary keywords (e.g., types, modifiers)
 * @HL_KEYWORD3: Tertiary keywords (e.g., built-in functions, constants)
 * @HL_STRING: String literals
 * @HL_NUMBER: Numeric literals (decimal, hex, octal, float)
 * @HL_SPACE: Whitespace characters (spaces, tabs) when visualized
 * @HL_FG_COUNT: Total number of foreground colors (not a color itself)
 */
enum EditorHighlightFg
{
  HL_NORMAL = 0,
  HL_COMMENT,
  HL_KEYWORD1,
  HL_KEYWORD2,
  HL_KEYWORD3,
  HL_STRING,
  HL_NUMBER,
  HL_SPACE,

  HL_FG_COUNT,  // Count of foreground colors
};

/**
 * enum EditorHighlightBg - Background highlight types
 *
 * Defines the different background highlighting states.
 * These values are shifted left by HL_FG_BITS (4) to occupy
 * the upper 4 bits of the highlight byte.
 *
 * @HL_BG_NORMAL: Default background (or current line highlight)
 * @HL_BG_MATCH: Search match highlighting
 * @HL_BG_SELECT: Text selection highlighting
 * @HL_BG_TRAILING: Trailing whitespace highlighting
 * @HL_BG_COUNT: Total number of background types (not a type itself)
 */
enum EditorHighlightBg
{
  HL_BG_NORMAL = 0,
  HL_BG_MATCH,
  HL_BG_SELECT,
  HL_BG_TRAILING,

  HL_BG_COUNT,  // Count of background types
};

/**
 * struct EditorSyntax - Syntax highlighting definition for a language
 * @next: Pointer to next syntax in HLDB linked list
 * @file_type: Display name of the language (e.g., "C", "Python", "JSON")
 * @singleline_comment_start: String that starts a single-line comment (e.g., "//", "#")
 * @multiline_comment_start: String that starts a multi-line comment (e.g., slash-star)
 * @multiline_comment_end: String that ends a multi-line comment (e.g., star-slash)
 * @file_exts: Vector of file extensions/patterns that use this syntax
 * @keywords: Three categories of keywords for different highlight colors
 *            - keywords[0]: Primary keywords (control flow, declarations)
 *            - keywords[1]: Secondary keywords (types, modifiers)
 *            - keywords[2]: Tertiary keywords (built-ins, constants)
 * @flags: Feature flags (HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS)
 * @value: Pointer to parsed JSON value (owned by JSON arena)
 *
 * This structure defines how a particular programming language should
 * be syntax highlighted. Multiple EditorSyntax structures form a
 * linked list (HLDB) that the editor searches through when opening files.
 *
 * File extensions in file_exts can be either:
 * - Extensions starting with '.' (e.g., ".c", ".py")
 * - Filename patterns (e.g., "Makefile", "README")
 */
typedef struct EditorSyntax
{
  struct EditorSyntax *next;

  const char *file_type;
  const char *singleline_comment_start;
  const char *multiline_comment_start;
  const char *multiline_comment_end;
  VECTOR(const char *) file_exts;
  VECTOR(const char *) keywords[3];
  int flags;

  struct JsonValue *value;
} EditorSyntax;

/**
 * editorUpdateSyntax - Update syntax highlighting for a single row
 * @file: The file containing the row
 * @row: The row to update
 *
 * Performs syntax highlighting on a single line based on the file's
 * syntax definition. This function is called:
 * - When a line is modified
 * - When syntax is changed
 * - Recursively when multi-line comment state changes
 */
void editorUpdateSyntax(EditorFile *file, EditorRow *row);

/**
 * editorSetSyntaxHighlight - Set syntax highlighting for a file
 * @file: The file to set syntax for
 * @syntax: The syntax definition to use
 *
 * Assigns a specific syntax definition to a file and updates
 * highlighting for all rows. Use NULL to disable syntax highlighting.
 */
void editorSetSyntaxHighlight(EditorFile *file, EditorSyntax *syntax);

/**
 * editorSelectSyntaxHighlight - Auto-detect and set syntax for a file
 * @file: The file to detect syntax for
 *
 * Automatically selects appropriate syntax highlighting based on
 * the file's extension or filename pattern by searching through
 * all registered syntax definitions in HLDB.
 */
void editorSelectSyntaxHighlight(EditorFile *file);

/**
 * editorInitHLDB - Initialize the syntax highlighting database
 *
 * Loads all syntax definitions from multiple sources:
 * 1. Built-in editor config syntax
 * 2. Bundled syntax files (compiled into binary)
 * 3. User syntax files from ~/.config/lx/syntax/ directory
 *
 * This function should be called once during editor initialization.
 */
void editorInitHLDB(void);

/**
 * editorLoadHLDB - Load a syntax definition from a JSON file
 * @json_file: Path to the JSON syntax definition file
 *
 * Loads and parses a syntax definition from a JSON file and adds
 * it to the HLDB. Used for loading user-defined syntax files.
 *
 * JSON format example:
 * {
 *   "name": "C",
 *   "extensions": [".c", ".h"],
 *   "comment": "//",
 *   "multiline-comment": ["slash-star", "star-slash"],
 *   "keywords1": ["if", "else", "while", ...],
 *   "keywords2": ["int", "char", "void", ...],
 *   "keywords3": ["NULL", "true", "false", ...]
 * }
 *
 * Returns: true if loading succeeded, false on error
 */
bool editorLoadHLDB(const char *json_file);

/**
 * editorFreeHLDB - Free all syntax highlighting definitions
 *
 * Frees all memory used by the syntax highlighting database.
 * This includes all EditorSyntax structures and the JSON parsing arena.
 * Should be called during editor shutdown.
 */
void editorFreeHLDB(void);

#endif