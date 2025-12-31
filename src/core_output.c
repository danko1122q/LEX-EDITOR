#include "core_output.h"

#include "core_config.h"
#include "core_editor.h"
#include "core_highlight.h"
#include "core_os.h"
#include "core_select.h"
#include "core_terminal.h"
#include "core_unicode.h"

#include <ctype.h>

/**
 * editorDrawTopStatusBar - Draw the top status bar with file tabs
 * @ab: Append buffer to write to
 * 
 * Draws the top bar showing:
 * - Navigation arrows (< >) if there are more tabs
 * - File tabs with names and dirty indicators (*)
 * - Editor name and version on the right
 * - Loading message when in loading state
 */
static void editorDrawTopStatusBar(abuf *ab)
{
  const char *right_buf      = "  " EDITOR_NAME " v" EDITOR_VERSION " ";
  bool        has_more_files = false;
  int         rlen           = strlen(right_buf);
  int         len            = gEditor.explorer.width;

  // Move to the position after explorer panel
  gotoXY(ab, 1, gEditor.explorer.width + 1);

  // Set colors for top status bar
  setColor(ab, gEditor.color_cfg.top_status[0], 0);
  setColor(ab, gEditor.color_cfg.top_status[1], 1);

  // Draw left arrow if there are tabs scrolled off-screen to the left
  if (gEditor.tab_offset != 0)
  {
    abufAppendN(ab, "<", 1);
    len++;
  }

  gEditor.tab_displayed = 0;
  
  // Show loading message when loading files
  if (gEditor.state == LOADING_MODE)
  {
    const char *loading_text     = "Loading...";
    int         loading_text_len = strlen(loading_text);
    abufAppendN(ab, loading_text, loading_text_len);
    len = loading_text_len;
  }
  else
  {
    // Draw each file tab
    for (int i = 0; i < gEditor.file_count; i++)
    {
      // Skip tabs that are scrolled off to the left
      if (i < gEditor.tab_offset)
        continue;

      const EditorFile *file = &gEditor.files[i];

      // Highlight current tab differently
      bool is_current = (file == gCurFile);
      if (is_current)
      {
        setColor(ab, gEditor.color_cfg.top_status[4], 0);
        setColor(ab, gEditor.color_cfg.top_status[5], 1);
      }
      else
      {
        setColor(ab, gEditor.color_cfg.top_status[2], 0);
        setColor(ab, gEditor.color_cfg.top_status[3], 1);
      }

      // Format tab text with filename and dirty indicator
      int  buf_len;
      char buf[EDITOR_PATH_MAX] = {0};
      if (file->filename)
      {
        buf_len = snprintf(buf, sizeof(buf), " %s%s ", file->dirty ? "*" : "",
                           getBaseName(file->filename));
      }
      else
      {
        // Untitled files get a number
        buf_len =
            snprintf(buf, sizeof(buf), " Untitled-%d%s ", file->new_id + 1, file->dirty ? "*" : "");
      }

      int tab_width = strUTF8Width(buf);

      // Check if tab fits on screen
      if (gEditor.screen_cols - len < tab_width ||
          (i != gEditor.file_count - 1 && gEditor.screen_cols - len == tab_width))
      {
        has_more_files = true;
        if (gEditor.tab_displayed == 0)
        {
          // Display at least one tab (even if truncated)
          // TODO: This is wrong
          tab_width = gEditor.screen_cols - len - 1;
          buf_len   = gEditor.screen_cols - len - 1;
        }
        else
        {
          break;
        }
      }

      // Not enough space to even show one tab
      if (tab_width < 0)
        break;

      abufAppendN(ab, buf, buf_len);
      len += tab_width;
      gEditor.tab_displayed++;
    }
  }

  // Reset to default status bar colors
  setColor(ab, gEditor.color_cfg.top_status[0], 0);
  setColor(ab, gEditor.color_cfg.top_status[1], 1);

  // Draw right arrow if there are more tabs off-screen to the right
  if (has_more_files)
  {
    abufAppendN(ab, ">", 1);
    len++;
  }

  // Fill remaining space and draw editor name/version on right
  while (len < gEditor.screen_cols)
  {
    if (gEditor.screen_cols - len == rlen)
    {
      abufAppendN(ab, right_buf, rlen);
      break;
    }
    else
    {
      abufAppendN(ab, " ", 1);
      len++;
    }
  }
}

/**
 * editorDrawConMsg - Draw console messages
 * @ab: Append buffer to write to
 * 
 * Draws console/status messages at the bottom of the screen,
 * just above the prompt line. Messages are displayed in a
 * circular buffer queue.
 */
static void editorDrawConMsg(abuf *ab)
{
  // Return early if no messages to display
  if (gEditor.con_size == 0)
  {
    return;
  }

  // Set prompt colors for console messages
  setColor(ab, gEditor.color_cfg.prompt[0], 0);
  setColor(ab, gEditor.color_cfg.prompt[1], 1);

  // Calculate starting row for console messages
  bool should_draw_prompt = (gEditor.state != EDIT_MODE && gEditor.state != EXPLORER_MODE);
  int  draw_x             = gEditor.screen_rows - gEditor.con_size;
  if (should_draw_prompt)
  {
    draw_x--;  // Leave space for prompt line
  }

  // Draw each console message from the circular buffer
  int index = gEditor.con_front;
  for (int i = 0; i < gEditor.con_size; i++)
  {
    gotoXY(ab, draw_x, 0);
    draw_x++;

    const char *buf = gEditor.con_msg[index];
    index           = (index + 1) % EDITOR_CON_COUNT;

    // Truncate message if longer than screen width
    int len = strlen(buf);
    if (len > gEditor.screen_cols)
    {
      len = gEditor.screen_cols;
    }

    abufAppendN(ab, buf, len);

    // Fill rest of line with spaces
    while (len < gEditor.screen_cols)
    {
      abufAppendN(ab, " ", 1);
      len++;
    }
  }
}

/**
 * editorDrawPrompt - Draw the prompt line
 * @ab: Append buffer to write to
 * 
 * Draws the command prompt at the bottom of the screen when
 * not in edit or explorer mode. Shows prompt text on left
 * and additional info on right.
 */
static void editorDrawPrompt(abuf *ab)
{
  // Only draw prompt in non-edit modes
  bool should_draw_prompt = (gEditor.state != EDIT_MODE && gEditor.state != EXPLORER_MODE);
  if (!should_draw_prompt)
  {
    return;
  }

  // Set prompt colors
  setColor(ab, gEditor.color_cfg.prompt[0], 0);
  setColor(ab, gEditor.color_cfg.prompt[1], 1);

  // Move to bottom line
  gotoXY(ab, gEditor.screen_rows - 1, 0);

  // Get left and right prompt text
  const char *left = gEditor.prompt;
  int         len  = strlen(left);

  const char *right = gEditor.prompt_right;
  int         rlen  = strlen(right);

  // Truncate right text if too long
  if (rlen > gEditor.screen_cols)
  {
    rlen = 0;
  }

  // Truncate left text if combined length exceeds screen width
  if (len + rlen > gEditor.screen_cols)
  {
    len = gEditor.screen_cols - rlen;
  }

  abufAppendN(ab, left, len);

  // Fill middle with spaces and draw right text at end
  while (len < gEditor.screen_cols)
  {
    if (gEditor.screen_cols - len == rlen)
    {
      abufAppendN(ab, right, rlen);
      break;
    }
    else
    {
      abufAppendN(ab, " ", 1);
      len++;
    }
  }
}

/**
 * editorDrawStatusBar - Draw the bottom status bar
 * @ab: Append buffer to write to
 * 
 * Draws the status bar at the very bottom showing:
 * - Help text with keyboard shortcuts (left)
 * - File type/language (middle-right)
 * - Cursor position and line info (right)
 */
static void editorDrawStatusBar(abuf *ab)
{
  // Move to last row
  gotoXY(ab, gEditor.screen_rows, 0);

  // Set status bar colors
  setColor(ab, gEditor.color_cfg.status[0], 0);
  setColor(ab, gEditor.color_cfg.status[1], 1);

  const char *help_str = "";
  
  // TOTAL MODIFICATION: Changed ALL help strings to match new shortcuts
  const char *help_info[] = {
      // EDIT mode (0)
      " ^X: Quit  ^S: Open  ^P: Prompt  ^O: Save  ^F: Find  ^G: Goto",
      // EXPLORER mode (1)
      " ^X: Quit  ^S: Open  ^P: Prompt",
      // PROMPT mode (2)
      " ^X: Cancel  Up: Back  Down: Next",
      // Other PROMPT modes (3-6)
      " ^X: Cancel",
      " ^X: Cancel",
      " ^X: Cancel",
      " ^X: Cancel",
  };
  // END MODIFICATION
  
  // Show help info if enabled
  if (CONVAR_GETINT(helpinfo))
    help_str = help_info[gEditor.state];

  char lang[16];
  char pos[64];
  int  len = strlen(help_str);
  int  lang_len, pos_len;
  int  rlen;
  
  // Don't show file info if no files open
  if (gEditor.file_count == 0)
  {
    lang_len = 0;
    pos_len  = 0;
  }
  else
  {
    // Get file type/language
    const char *file_type = gCurFile->syntax ? gCurFile->syntax->file_type : "Plain Text";
    
    // Calculate cursor row and column (1-indexed for display)
    int         row       = gCurFile->cursor.y + 1;
    int         col = editorRowCxToRx(&gCurFile->row[gCurFile->cursor.y], gCurFile->cursor.x) + 1;
    
    // Calculate line percentage for scroll position
    float       line_percent = 0.0f;
    const char *nl_type      = (gCurFile->newline == NL_UNIX) ? "LF" : "CRLF";
    if (gCurFile->num_rows - 1 > 0)
    {
      line_percent = (float) gCurFile->row_offset / (gCurFile->num_rows - 1) * 100.0f;
    }

    // Format language and position strings
    lang_len = snprintf(lang, sizeof(lang), "  %s  ", file_type);
    pos_len  = snprintf(pos, sizeof(pos), " %d:%d [%.f%%] <%s> ", row, col, line_percent, nl_type);
  }

  rlen = lang_len + pos_len;

  // Truncate if texts don't fit
  if (rlen > gEditor.screen_cols)
    rlen = 0;
  if (len + rlen > gEditor.screen_cols)
    len = gEditor.screen_cols - rlen;

  // Draw help text
  abufAppendN(ab, help_str, len);

  // Fill middle and draw file info on right
  while (len < gEditor.screen_cols)
  {
    if (gEditor.screen_cols - len == rlen)
    {
      // Draw language/file type
      setColor(ab, gEditor.color_cfg.status[2], 0);
      setColor(ab, gEditor.color_cfg.status[3], 1);
      abufAppendN(ab, lang, lang_len);
      
      // Draw position info
      setColor(ab, gEditor.color_cfg.status[4], 0);
      setColor(ab, gEditor.color_cfg.status[5], 1);
      abufAppendN(ab, pos, pos_len);
      break;
    }
    else
    {
      abufAppendN(ab, " ", 1);
      len++;
    }
  }
}

/**
 * editorDrawRows - Draw the text editor content area
 * @ab: Append buffer to write to
 * 
 * Draws all visible text rows with:
 * - Line numbers (if enabled)
 * - Syntax highlighting
 * - Selection highlighting
 * - Special character visualization (tabs, spaces, control chars)
 * - Current line highlighting
 */
static void editorDrawRows(abuf *ab)
{
  // Set background color
  setColor(ab, gEditor.color_cfg.bg, 1);

  // Get selection range if text is selected
  EditorSelectRange range = {0};
  if (gCurFile->cursor.is_selected)
    getSelectStartEnd(&range);

  // Draw each visible row
  for (int i = gCurFile->row_offset, s_row = 2; i < gCurFile->row_offset + gEditor.display_rows;
       i++, s_row++)
  {
    int  len;
    bool is_row_full = false;

    // Move cursor to the beginning of the row
    gotoXY(ab, s_row, 1 + gEditor.explorer.width);

    // Set default background for normal text
    gEditor.color_cfg.highlightBg[HL_BG_NORMAL] = gEditor.color_cfg.bg;
    
    // Only draw if row exists in file
    if (i < gCurFile->num_rows)
    {
      // Draw line numbers if enabled
      if (CONVAR_GETINT(lilx))
      {
        char line_number[16];
        
        // Highlight current line number differently
        if (i == gCurFile->cursor.y)
        {
          // Only highlight line if no selection active
          if (!gCurFile->cursor.is_selected)
          {
            gEditor.color_cfg.highlightBg[HL_BG_NORMAL] = gEditor.color_cfg.cursor_line;
          }
          setColor(ab, gEditor.color_cfg.line_number[1], 0);
          setColor(ab, gEditor.color_cfg.line_number[0], 1);
        }
        else
        {
          setColor(ab, gEditor.color_cfg.line_number[0], 0);
          setColor(ab, gEditor.color_cfg.line_number[1], 1);
        }

        // Format and draw line number (1-indexed)
        len = snprintf(line_number, sizeof(line_number), " %*d ", gCurFile->licore_width - 2,
                       i + 1);

        abufAppendN(ab, line_number, len);
      }

      // Clear to end of line and reset colors
      abufAppendStr(ab, ANSI_CLEAR);
      setColor(ab, gEditor.color_cfg.bg, 1);

      // Calculate visible columns and starting position
      int cols       = gEditor.screen_cols - gEditor.explorer.width - LICORE_WIDTH();
      int col_offset = editorRowRxToCx(&gCurFile->row[i], gCurFile->col_offset);
      len            = gCurFile->row[i].size - col_offset;
      len            = (len < 0) ? 0 : len;

      // Calculate rendered line length
      int rlen    = gCurFile->row[i].rsize - gCurFile->col_offset;
      is_row_full = (rlen > cols);
      rlen        = is_row_full ? cols : rlen;
      rlen += gCurFile->col_offset;

      // Get pointers to character data and highlight info
      char    *c       = &gCurFile->row[i].data[col_offset];
      uint8_t *hl      = &(gCurFile->row[i].hl[col_offset]);
      uint8_t  curr_fg = HL_BG_NORMAL;
      uint8_t  curr_bg = HL_NORMAL;

      // Set initial colors
      setColor(ab, gEditor.color_cfg.highlightFg[curr_fg], 0);
      setColor(ab, gEditor.color_cfg.highlightBg[curr_bg], 1);

      // Draw each character in the row
      int j  = 0;
      int rx = gCurFile->col_offset;
      while (rx < rlen)
      {
        // Handle control characters (except tab)
        if (iscntrl((uint8_t) c[j]) && c[j] != '\t')
        {
          // Display as caret notation (e.g., ^A for Ctrl-A)
          char sym = (c[j] <= 26) ? '@' + c[j] : '?';
          abufAppendStr(ab, ANSI_INVERT);
          abufAppendN(ab, &sym, 1);
          abufAppendStr(ab, ANSI_CLEAR);
          setColor(ab, gEditor.color_cfg.highlightFg[curr_fg], 0);
          setColor(ab, gEditor.color_cfg.highlightBg[curr_bg], 1);

          rx++;
          j++;
        }
        else
        {
          // Get syntax highlighting colors
          uint8_t fg = hl[j] & HL_FG_MASK;
          uint8_t bg = hl[j] >> HL_FG_BITS;

          // Apply selection highlighting if character is selected
          if (gCurFile->cursor.is_selected && isPosSelected(i, j + col_offset, range))
          {
            bg = HL_BG_SELECT;
          }
          
          // Highlight spaces/tabs if drawspace is enabled
          if (CONVAR_GETINT(drawspace) && (c[j] == ' ' || c[j] == '\t'))
          {
            fg = HL_SPACE;
          }
          
          // Don't show trailing whitespace highlight if disabled
          if (bg == HL_BG_TRAILING && !CONVAR_GETINT(trailing))
          {
            bg = HL_BG_NORMAL;
          }

          // Update foreground color if changed
          if (fg != curr_fg)
          {
            curr_fg = fg;
            setColor(ab, gEditor.color_cfg.highlightFg[fg], 0);
          }
          
          // Update background color if changed
          if (bg != curr_bg)
          {
            curr_bg = bg;
            setColor(ab, gEditor.color_cfg.highlightBg[bg], 1);
          }

          // Handle tab characters
          if (c[j] == '\t')
          {
            // Show tab indicator if drawspace enabled
            if (CONVAR_GETINT(drawspace))
            {
              abufAppendN(ab, "|", 1);
            }
            else
            {
              abufAppendN(ab, " ", 1);
            }

            rx++;
            
            // Fill to next tab stop
            while (rx % CONVAR_GETINT(tabsize) != 0 && rx < rlen)
            {
              abufAppendN(ab, " ", 1);
              rx++;
            }
            j++;
          }
          // Handle space characters
          else if (c[j] == ' ')
          {
            // Show dot if drawspace enabled
            if (CONVAR_GETINT(drawspace))
            {
              abufAppendN(ab, ".", 1);
            }
            else
            {
              abufAppendN(ab, " ", 1);
            }
            rx++;
            j++;
          }
          // Handle regular UTF-8 characters
          else
          {
            size_t   byte_size;
            uint32_t unicode = decodeUTF8(&c[j], len - j, &byte_size);
            int      width   = unicodeWidth(unicode);
            if (width >= 0)
            {
              rx += width;
              // Make sure double-width chars don't exceed screen
              if (rx <= rlen)
                abufAppendN(ab, &c[j], byte_size);
            }
            j += byte_size;
          }
        }
      }

      // Add newline character highlighting when line is selected
      if (gCurFile->cursor.is_selected && range.end_y > i && i >= range.start_y &&
          gCurFile->row[i].rsize - gCurFile->col_offset < cols)
      {
        setColor(ab, gEditor.color_cfg.highlightBg[HL_BG_SELECT], 1);
        abufAppendN(ab, " ", 1);
      }
      setColor(ab, gEditor.color_cfg.highlightBg[HL_BG_NORMAL], 1);
    }
    
    // Erase rest of line if row isn't full width
    if (!is_row_full)
      abufAppendStr(ab, ANSI_ERASE_LINE);
    setColor(ab, gEditor.color_cfg.bg, 1);
  }
}

/**
 * editorDrawFileExplorer - Draw the file explorer sidebar
 * @ab: Append buffer to write to
 * 
 * Draws the file explorer panel showing:
 * - Explorer header
 * - Directory tree with expand/collapse indicators
 * - Current selection highlight
 * - File/folder icons and colors
 */
static void editorDrawFileExplorer(abuf *ab)
{
  char *explorer_buf = malloc_s(gEditor.explorer.width + 1);
  gotoXY(ab, 1, 1);

  // Draw explorer header
  setColor(ab, gEditor.color_cfg.explorer[3], 0);
  if (gEditor.state == EXPLORER_MODE)
    setColor(ab, gEditor.color_cfg.explorer[4], 1);  // Highlight if in explorer mode
  else
    setColor(ab, gEditor.color_cfg.explorer[0], 1);

  snprintf(explorer_buf, gEditor.explorer.width + 1, " EXPLORER%*s", gEditor.explorer.width, "");
  abufAppendN(ab, explorer_buf, gEditor.explorer.width);

  // Calculate how many lines to display
  int lines = gEditor.explorer.flatten.size - gEditor.explorer.offset;
  if (lines < 0)
  {
    lines = 0;
  }
  else if (lines > gEditor.display_rows)
  {
    lines = gEditor.display_rows;
  }

  // Draw each visible explorer entry
  for (int i = 0; i < lines; i++)
  {
    gotoXY(ab, i + 2, 1);

    int                 index = gEditor.explorer.offset + i;
    EditorExplorerNode *node  = gEditor.explorer.flatten.data[index];
    
    // Highlight selected entry
    if (index == gEditor.explorer.selected_index)
      setColor(ab, gEditor.color_cfg.explorer[1], 1);
    else
      setColor(ab, gEditor.color_cfg.explorer[0], 1);

    // Set icon based on file type and state
    const char *icon = "";
    if (node->is_directory)
    {
      setColor(ab, gEditor.color_cfg.explorer[2], 0);
      icon = node->is_open ? "v " : "> ";  // Expanded vs collapsed
    }
    else
    {
      setColor(ab, gEditor.color_cfg.explorer[3], 0);
    }
    
    const char *filename = getBaseName(node->filename);

    // Format with indentation based on depth
    snprintf(explorer_buf, gEditor.explorer.width + 1, "%*s%s%s%*s", node->depth * 2, "", icon,
             filename, gEditor.explorer.width, "");
    abufAppendN(ab, explorer_buf, gEditor.explorer.width);
  }

  // Draw blank lines to fill rest of explorer panel
  setColor(ab, gEditor.color_cfg.explorer[0], 1);
  setColor(ab, gEditor.color_cfg.explorer[3], 0);

  memset(explorer_buf, ' ', gEditor.explorer.width);

  for (int i = 0; i < gEditor.display_rows - lines; i++)
  {
    gotoXY(ab, lines + i + 2, 1);
    abufAppendN(ab, explorer_buf, gEditor.explorer.width);
  }

  free(explorer_buf);
}

/**
 * editorRefreshScreen - Refresh the entire screen display
 * 
 * Main rendering function that draws all UI elements:
 * - Top status bar with tabs
 * - Text editor content area
 * - File explorer sidebar
 * - Console messages
 * - Command prompt
 * - Bottom status bar
 * - Cursor positioning
 * 
 * Called whenever the screen needs to be redrawn.
 */
void editorRefreshScreen(void)
{
  abuf ab = ABUF_INIT;

  // Hide cursor and reset position during drawing
  abufAppendStr(&ab, ANSI_CURSOR_HIDE ANSI_CURSOR_RESET_POS);

  // Draw all UI components
  editorDrawTopStatusBar(&ab);
  editorDrawRows(&ab);
  editorDrawFileExplorer(&ab);

  editorDrawConMsg(&ab);
  editorDrawPrompt(&ab);

  editorDrawStatusBar(&ab);

  // Calculate cursor position
  bool should_show_cursor = true;
  if (gEditor.state == EDIT_MODE)
  {
    // Calculate screen row (offset from top, accounting for status bar)
    int row = (gCurFile->cursor.y - gCurFile->row_offset) + 2;
    
    // Calculate screen column (accounting for tabs, explorer, line numbers)
    int col = (editorRowCxToRx(&gCurFile->row[gCurFile->cursor.y], gCurFile->cursor.x) -
               gCurFile->col_offset) +
              1 + LICORE_WIDTH();
    
    // Hide cursor if it's outside visible area
    if (row <= 1 || row > gEditor.screen_rows - 1 || col <= 0 ||
        col > gEditor.screen_cols - gEditor.explorer.width ||
        row >= gEditor.screen_rows - gEditor.con_size)
    {
      should_show_cursor = false;
    }
    else
    {
      gotoXY(&ab, row, col + gEditor.explorer.width);
    }
  }
  else
  {
    // In prompt mode, position cursor in prompt area
    gotoXY(&ab, gEditor.screen_rows - 1, gEditor.px + 1);
  }

  // Hide cursor in explorer mode
  if (gEditor.state == EXPLORER_MODE)
  {
    should_show_cursor = false;
  }

  // Show or hide cursor based on calculated state
  if (should_show_cursor)
  {
    abufAppendStr(&ab, ANSI_CURSOR_SHOW);
  }
  else
  {
    abufAppendStr(&ab, ANSI_CURSOR_HIDE);
  }

  // Clear any remaining formatting
  abufAppendStr(&ab, ANSI_CLEAR);

  // Write everything to console at once
  writeConsoleAll(ab.buf, ab.len);
  abufFree(&ab);
}