#ifndef OUTPUT_H
#define OUTPUT_H

/**
 * LICORE_WIDTH - Get the width of line number column
 *
 * Returns the width occupied by line numbers in the editor if
 * line numbers are enabled (lilx config variable), otherwise returns 0.
 *
 * This macro is used to calculate column offsets when drawing text,
 * since line numbers take up space on the left side of the editor.
 *
 * Returns: Width in characters of the line number column, or 0 if disabled
 *
 * Example:
 *   If lilx is enabled and file has 1000 lines:
 *     - Line number column needs 6 chars: " 1000 "
 *     - LICORE_WIDTH() returns 6
 *   If lilx is disabled:
 *     - LICORE_WIDTH() returns 0
 */
#define LICORE_WIDTH() (CONVAR_GETINT(lilx) ? gCurFile->licore_width : 0)

/**
 * editorRefreshScreen - Refresh and redraw the entire screen
 *
 * This is the main rendering function that redraws all UI elements
 * on the screen. It should be called whenever something changes that
 * requires the display to be updated.
 *
 * The function draws (in order):
 * 1. Top status bar with file tabs
 * 2. Text editor content area with syntax highlighting
 * 3. File explorer sidebar
 * 4. Console messages (if any)
 * 5. Command prompt (if in prompt mode)
 * 6. Bottom status bar with file info
 * 7. Positions the cursor appropriately
 *
 * All drawing is done to a buffer first, then written to the
 * terminal in one operation for smooth rendering without flicker.
 *
 * This function is called after:
 * - User input (keypress, mouse action)
 * - File operations (open, save, close)
 * - Mode changes (edit, explorer, prompt)
 * - Window resize events
 * - Any state change that affects the display
 */
void editorRefreshScreen(void);

#endif