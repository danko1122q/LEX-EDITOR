#ifndef ACTION_H
#define ACTION_H

#include "core_select.h"

/**
 * struct EditorCursor - Represents the cursor position and selection state
 * @x: Current cursor column position
 * @y: Current cursor row position
 * @is_selected: Flag indicating if text is currently selected
 * @select_x: Selection starting column position
 * @select_y: Selection starting row position
 *
 * This structure stores the complete state of the editor cursor,
 * including its position and any active text selection.
 */
typedef struct EditorCursor
{
  int  x, y;
  bool is_selected;
  int  select_x;
  int  select_y;
} EditorCursor;

/**
 * struct EditAction - Represents a text editing action
 * @deleted_range: Range of text that was deleted
 * @deleted_text: Content of the deleted text
 * @added_range: Range where new text was added
 * @added_text: Content of the added text
 * @old_cursor: Cursor state before the edit
 * @new_cursor: Cursor state after the edit
 *
 * This structure captures all information needed to undo/redo
 * a text editing operation, including what was deleted, what was
 * added, and cursor positions before and after.
 */
typedef struct EditAction
{
  EditorSelectRange deleted_range;
  EditorClipboard   deleted_text;

  EditorSelectRange added_range;
  EditorClipboard   added_text;

  EditorCursor old_cursor;
  EditorCursor new_cursor;
} EditAction;

/**
 * struct AttributeAction - Represents a file attribute change action
 * @old_newline: Previous newline character setting
 * @new_newline: New newline character setting
 *
 * This structure stores changes to file attributes such as
 * the newline character format (LF, CRLF, etc.).
 */
typedef struct AttributeAction
{
  int old_newline;
  int new_newline;
} AttributeAction;

/**
 * enum EditorActionType - Types of actions that can be performed
 * @ACTION_EDIT: Text editing action (insert, delete, paste, etc.)
 * @ACTION_ATTRI: File attribute modification action
 *
 * Defines the different categories of actions that can be
 * tracked in the undo/redo history.
 */
typedef enum EditorActionType
{
  ACTION_EDIT,
  ACTION_ATTRI,
} EditorActionType;

/**
 * struct EditorAction - Container for any type of editor action
 * @type: The type of action (edit or attribute)
 * @edit: Edit action data (valid when type is ACTION_EDIT)
 * @attri: Attribute action data (valid when type is ACTION_ATTRI)
 *
 * This is a tagged union that can hold either an edit action
 * or an attribute action. The type field determines which
 * member of the union is valid.
 */
typedef struct EditorAction
{
  EditorActionType type;
  union
  {
    EditAction      edit;
    AttributeAction attri;
  };
} EditorAction;

/**
 * struct EditorActionList - Doubly-linked list node for action history
 * @prev: Pointer to previous action in history
 * @next: Pointer to next action in history
 * @action: Pointer to the actual action data
 *
 * Forms a doubly-linked list to maintain the undo/redo history.
 * This allows traversing both forward (redo) and backward (undo)
 * through the action history.
 */
typedef struct EditorActionList
{
  struct EditorActionList *prev;
  struct EditorActionList *next;
  EditorAction            *action;
} EditorActionList;

/**
 * editorUndo - Undo the last action performed
 *
 * Returns: true if undo was successful, false if nothing to undo
 */
bool editorUndo(void);

/**
 * editorRedo - Redo the previously undone action
 *
 * Returns: true if redo was successful, false if nothing to redo
 */
bool editorRedo(void);

/**
 * editorAppendAction - Add a new action to the history
 * @action: Pointer to the action to append
 *
 * Adds the action to the end of the history and clears any
 * redo history after the current position.
 */
void editorAppendAction(EditorAction *action);

/**
 * editorFreeActionList - Free a list of actions
 * @thisptr: Pointer to the first node to free
 *
 * Recursively frees all nodes in the action list starting
 * from the given pointer.
 */
void editorFreeActionList(EditorActionList *thisptr);

/**
 * editorFreeAction - Free a single action
 * @action: Pointer to the action to free
 *
 * Frees all memory associated with the action, including
 * any clipboard content for edit actions.
 */
void editorFreeAction(EditorAction *action);

#endif