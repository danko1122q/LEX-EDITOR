/*
 * EDITOR.H - Text Editor Core Data Structures and API
 * 
 * This header defines the main data structures for a terminal-based text editor
 * with features like multiple file support, syntax highlighting, undo/redo,
 * and a file explorer.
 */

#ifndef EDITOR_H
#define EDITOR_H

#include "core_action.h"   // Undo/redo action structures
#include "core_config.h"   // Configuration settings
#include "core_file_io.h"  // File I/O operations
#include "core_os.h"       // Operating system abstraction layer
#include "core_row.h"      // Text row/line structures
#include "core_select.h"   // Text selection structures

/*
 * Maximum File Slots
 * The editor can have up to 32 files open simultaneously (like browser tabs)
 * This creates a fixed-size array to avoid dynamic allocation complxity
 */
#define EDITOR_FILE_MAX_SLOT 32

/*
 * Console Message Buffer Configuration
 * EDITOR_CON_COUNT: Number of messages stored in circular buffer (16 messages)
 * EDITOR_CON_LENGTH: Maximum length of each message (255 characters + null terminator)
 * This creates a ring buffer for status/error messages shown at editor bottom
 */
#define EDITOR_CON_COUNT 16
#define EDITOR_CON_LENGTH 255

/*
 * Prompt Buffer Sizes
 * EDITOR_PROMPT_LENGTH: Main prompt text (e.g., "Save as: filename.txt")
 * EDITOR_RIGHT_PROMPT_LENGTH: Right-aligned info (e.g., "Line 45, Col 12")
 */
#define EDITOR_PROMPT_LENGTH 255
#define EDITOR_RIGHT_PROMPT_LENGTH 32

/*
 * Editor State Machine
 * Represents different modes the editor can be in
 * Each mode changes how keyboard input is interpreted
 */
enum EditorState
{
  LOADING_MODE,      // Initial state while loading files/config
  EDIT_MODE,         // Normal text editing mode
  EXPLORER_MODE,     // File browser/explorer mode
  FIND_MODE,         // Text search mode (like Ctrl+F)
  GOTO_LINE_MODE,    // Jump to line number mode
  OPEN_FILE_MODE,    // File open dialog mode
  CONFIG_MODE,       // Settings/configuration mode
  SAVE_AS_MODE,      // Save file with new name dialog
};

/*
 * Forward Declaration
 * EditorSyntax is defined elsewhere but referenced here
 * Used for syntax highlighting rules (keywords, colors, etc.)
 */
typedef struct EditorSyntax EditorSyntax;

/*
 * EditorFile Structure
 * Represents a single open file/buffer in the editor
 * Contains all state needed for that file: content, cursor, undo history, etc.
 */
typedef struct EditorFile
{
  /*
   * Cursor Position
   * cursor: Logical cursor position (row/column in the text)
   * Defined in select.h, likely contains x, y coordinates
   */
  EditorCursor cursor;

  /*
   * Hidden Cursor X Position
   * sx: Stores the desired column when moving up/down through lines
   * Example: Moving from "hello world" (col 8) to "hi" (only 2 chars)
   *          sx remembers we want column 8, so next down movement goes to col 8
   */
  int sx;

  /*
   * Bracket Auto-complete Level
   * bracket_autocomplete: Controls automatic bracket/quote closing behavior
   * Values might be: 0=off, 1=basic (), 2=includes [], {}, "", etc.
   */
  int bracket_autocomplete;

  /*
   * Editor Viewport Offsets
   * row_offset: Number of lines scrolled down (hidden above viewport)
   * col_offset: Number of columns scrolled right (hidden to left of viewport)
   * These enable scrolling through files larger than the terminal window
   */
  int row_offset;
  int col_offset;

  /*
   * File Dimensions
   * num_rows: Total number of lines in the file
   * licore_width: Width of line number column (e.g., 5 chars for line "99999")
   *              Name suggests "Line Index Width"
   */
  int num_rows;
  int licore_width;

  /*
   * Line Ending Type
   * newline: Encoding for line endings
   * Common values: '\n' (LF/Unix), '\r\n' (CRLF/Windows), '\r' (CR/old Mac)
   * Stored as uint8_t to save space (only need 0-255)
   */
  uint8_t newline;

  /*
   * File Identity
   * filename: Full path to file (NULL if this is an unsaved "untitled" buffer)
   * new_id: Unique ID for untitled files (e.g., "Untitled-1", "Untitled-2")
   * file_info: Metadata like permissions, timestamps (defined in file_io.h)
   */
  char    *filename;  // NULL if untitled
  int      new_id;
  FileInfo file_info;

  /*
   * Text Content Storage
   * row_capacity: Allocated size of row array (can be > num_rows for growth)
   * row: Dynamic array of EditorRow structures, each representing one line
   * This allows efficient insertion/deletion without constant reallocation
   */
  size_t     row_capacity;
  EditorRow *row;

  /*
   * Syntax Highlighting
   * syntax: Pointer to syntax rules for this file type (C, Python, etc.)
   * NULL if no syntax highlighting (plain text)
   * Points to entry in global HLDB (syntax highlight database)
   */
  EditorSyntax *syntax;

  /*
   * Undo/Redo System
   * dirty: Change counter - increments with edits, decrements with undo
   *        Zero means file matches saved version on disk
   * action_head: Start of doubly-linked list of all edit actions
   * action_current: Current position in undo/redo list
   * 
   * Example: [Create]<->[Type "hi"]<->[Delete char]<-current
   *          Undo moves current left, Redo moves current right
   */
  int               dirty;
  EditorActionList *action_head;
  EditorActionList *action_current;
} EditorFile;

/*
 * Editor Structure
 * The global editor state - contains all files, UI state, settings, etc.
 * This is the "god object" that holds everything about the editor session
 */
typedef struct Editor
{
  /*
   * Terminal Dimensions
   * screen_rows: Total terminal height in characters
   * screen_cols: Total terminal width in characters
   * display_rows: Actual rows available for text (screen_rows - status bars)
   * 
   * Example: 80x24 terminal → screen_cols=80, screen_rows=24, display_rows=22
   *          (reserving 2 rows for status line and command line)
   */
  int screen_rows;
  int screen_cols;
  int display_rows;

  /*
   * Editor Mode and Settings
   * state: Current mode from EditorState enum (EDIT_MODE, FIND_MODE, etc.)
   * mouse_mode: Enable/disable mouse support (clicking, selecting, scrolling)
   */
  int  state;
  bool mouse_mode;

  /*
   * Prompt Cursor Position
   * px: Cursor position within the prompt input field
   * Example: In "Save as: file█.txt", px=15 (cursor after "file")
   */
  int px;

  /*
   * Clipboard System
   * clipboard: Stores copied/cut text (structure defined in select.h)
   * copy_line: Flag indicating if entire line was copied (affects paste behavior)
   *            true: pasting inserts new lines, false: pasting inserts inline
   */
  EditorClipboard clipboard;
  bool            copy_line;

  /*
   * Color Theme
   * color_cfg: Current color scheme (syntax colors, UI colors, background)
   * Structure defined in config.h, allows switching between themes
   */
  EditorColorScheme color_cfg;

  /*
   * Console Commands (ConCmd)
   * cvars: Linked list of console variables/commands
   * Similar to game engines: user can type commands like "set tabsize 4"
   * Each ConCmd node has name, value, and callback function
   */
  EditorConCmd *cvars;

  /*
   * Multiple File Management
   * files: Array of open files (tabs), fixed size EDITOR_FILE_MAX_SLOT
   * file_count: Number of actually open files (0 to EDITOR_FILE_MAX_SLOT)
   * file_index: Index of currently active/visible file (0-based)
   * tab_offset: First visible tab index (for horizontal scrolling of tab bar)
   * tab_displayed: Number of tabs currently visible on screen
   * 
   * Example: 10 files open, terminal shows 5 tabs at a time
   *          tab_offset=3 means tabs 3,4,5,6,7 are visible
   *          file_index=5 means file 5 is active
   */
  EditorFile files[EDITOR_FILE_MAX_SLOT];
  int        file_count;
  int        file_index;
  int        tab_offset;
  int        tab_displayed;

  /*
   * Syntax Highlight Database
   * HLDB: Array of syntax definitions for different file types
   * Each EditorSyntax contains: file extensions, keywords, comment patterns, etc.
   * Loaded from config at startup, shared by all EditorFile instances
   */
  EditorSyntax *HLDB;

  /*
   * File Explorer State
   * explorer: State for file browser mode (current directory, selected file, etc.)
   * Used when state == EXPLORER_MODE
   */
  EditorExplorer explorer;

  /*
   * Console Message Ring Buffer
   * Circular buffer for status messages displayed at editor bottom
   * con_front: Index of oldest message (read position)
   * con_rear: Index where next message will be written (write position)
   * con_size: Current number of messages in buffer (0 to EDITOR_CON_COUNT)
   * con_msg: 2D array storing the actual message strings
   * 
   * Ring Buffer Example:
   *   Initial: front=0, rear=0, size=0
   *   Add "File saved": front=0, rear=1, size=1
   *   Add 15 more: front=0, rear=0 (wrapped), size=16 (full)
   *   Add another: front=1, rear=1, size=16 (oldest message overwritten)
   */
  int  con_front;
  int  con_rear;
  int  con_size;
  char con_msg[EDITOR_CON_COUNT][EDITOR_CON_LENGTH];

  /*
   * Prompt Display Buffers
   * prompt: Main prompt text (left-aligned)
   *         Example: "Find: search_term" or "Save as: newfile.txt"
   * prompt_right: Right-aligned status info
   *               Example: "Ln 45, Col 12" or "UTF-8"
   */
  char prompt[EDITOR_PROMPT_LENGTH];
  char prompt_right[EDITOR_RIGHT_PROMPT_LENGTH];
} Editor;

/*
 * Global Variables
 * These are defined in editor.c and accessible throughout the codebase
 * 
 * gEditor: The single global editor instance (singleton pattern)
 * gCurFile: Convenience pointer to currently active file
 *           Equivalent to gEditor.files[gEditor.file_index]
 *           Avoids repeated array indexing in hot code paths
 */
extern Editor gEditor;
extern EditorFile *gCurFile;

/*
 * Lifecycle Functions
 * editorInit(): Initialize editor state, load config, setup terminal
 * editorFree(): Cleanup all resources before program exit
 * editorInitFile(): Initialize a single file structure to default state
 * editorFreeFile(): Free all memory associated with a file (rows, filename, etc.)
 */
void editorInit(void);
void editorFree(void);
void editorInitFile(EditorFile *file);
void editorFreeFile(EditorFile *file);

/*
 * Multiple File Management API
 * 
 * editorAddFile(): Add a new file to the editor
 *   - Returns: file index (0 to EDITOR_FILE_MAX_SLOT-1) on success, -1 if full
 *   - Copies file structure and initializes undo/redo system
 * 
 * editorRemoveFile(): Close a file and remove from file list
 *   - Frees all resources and shifts remaining files to fill gap
 *   - Does not switch active file - caller must handle that
 * 
 * editorChangeToFile(): Switch to a different open file
 *   - Updates gCurFile pointer and adjusts tab scrolling if needed
 *   - Bounds-checked: ignores invalid indices
 * 
 * editorNewUntitledFile(): Create a new empty buffer
 *   - Assigns unique "Untitled-N" identifier
 *   - Creates single empty row
 *   - File is marked as unsaved (dirty flag set)
 */
int  editorAddFile(const EditorFile *file);
void editorRemoveFile(int index);
void editorChangeToFile(int index);
void editorNewUntitledFile(EditorFile *file);

#endif