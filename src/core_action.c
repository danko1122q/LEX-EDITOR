#include "core_action.h"

#include "core_editor.h"

/**
 * editorUndo - Undo the last action performed in the editor
 * 
 * Returns: true if undo was successful, false if there's nothing to undo
 */
bool editorUndo(void)
{
  // Check if we're at the beginning of action history (nothing to undo)
  if (gCurFile->action_current == gCurFile->action_head)
    return false;

  // Handle different action types
  switch (gCurFile->action_current->action->type)
  {
    case ACTION_EDIT:
    {
      // Get the edit action details
      EditAction *edit = &gCurFile->action_current->action->edit;
      
      // Delete the text that was added
      editorDeleteText(edit->added_range);
      
      // Restore the text that was deleted
      editorPasteText(&edit->deleted_text, edit->deleted_range.start_x,
                      edit->deleted_range.start_y);
      
      // Restore the old cursor position
      gCurFile->cursor = edit->old_cursor;
    }
    break;

    case ACTION_ATTRI:
    {
      // Get the attribute action details
      AttributeAction *attri = &gCurFile->action_current->action->attri;
      
      // Restore the old newline setting
      gCurFile->newline      = attri->old_newline;
    }
    break;
  }

  // Move current action pointer to previous action
  gCurFile->action_current = gCurFile->action_current->prev;
  
  // Decrement dirty flag (file modification counter)
  gCurFile->dirty--;
  
  return true;
}

/**
 * editorRedo - Redo the previously undone action
 * 
 * Returns: true if redo was successful, false if there's nothing to redo
 */
bool editorRedo(void)
{
  // Check if there's a next action to redo
  if (!gCurFile->action_current->next)
    return false;

  // Move current action pointer to next action
  gCurFile->action_current = gCurFile->action_current->next;

  // Handle different action types
  switch (gCurFile->action_current->action->type)
  {
    case ACTION_EDIT:
    {
      // Get the edit action details
      EditAction *edit = &gCurFile->action_current->action->edit;
      
      // Delete the text that was previously there
      editorDeleteText(edit->deleted_range);
      
      // Re-add the text that was added in this action
      editorPasteText(&edit->added_text, edit->added_range.start_x, edit->added_range.start_y);
      
      // Restore the new cursor position
      gCurFile->cursor = edit->new_cursor;
    }
    break;

    case ACTION_ATTRI:
    {
      // Get the attribute action details
      AttributeAction *attri = &gCurFile->action_current->action->attri;
      
      // Restore the new newline setting
      gCurFile->newline      = attri->new_newline;
    }
    break;
  }

  // Increment dirty flag (file modification counter)
  gCurFile->dirty++;
  
  return true;
}

/**
 * editorAppendAction - Add a new action to the action history
 * @action: The action to append to the history
 * 
 * This function adds a new action to the undo/redo history and handles
 * clearing any redo history after the current position.
 */
void editorAppendAction(EditorAction *action)
{
  // Return early if action is NULL
  if (!action)
    return;

  // Allocate memory for new action list node
  EditorActionList *node = malloc_s(sizeof(EditorActionList));
  node->action           = action;
  node->next             = NULL;

  // Increment dirty flag (file has been modified)
  gCurFile->dirty++;

  // Free any actions after current position (clear redo history)
  editorFreeActionList(gCurFile->action_current->next);

  // Special case: if we're at the head (dummy node)
  if (gCurFile->action_current == gCurFile->action_head)
  {
    // Link the new node after the head
    gCurFile->action_head->next = node;
    node->prev                  = gCurFile->action_head;
    gCurFile->action_current    = node;
    return;
  }

  // Link the new node after current action
  node->prev                     = gCurFile->action_current;
  gCurFile->action_current->next = node;
  
  // Move current pointer to the newly added action
  gCurFile->action_current       = gCurFile->action_current->next;
}

/**
 * editorFreeAction - Free memory allocated for an editor action
 * @action: The action to free
 * 
 * Properly deallocates all memory associated with an action,
 * including any clipboard content for edit actions.
 */
void editorFreeAction(EditorAction *action)
{
  // Return early if action is NULL
  if (!action)
    return;

  // For edit actions, free the clipboard content
  if (action->type == ACTION_EDIT)
  {
    // Free the deleted text clipboard
    editorFreeClipboardContent(&action->edit.deleted_text);
    
    // Free the added text clipboard
    editorFreeClipboardContent(&action->edit.added_text);
  }

  // Free the action structure itself
  free(action);
}

/**
 * editorFreeActionList - Free an entire linked list of actions
 * @thisptr: Pointer to the first node in the list to free
 * 
 * Recursively frees all nodes in the action list starting from
 * the given pointer.
 */
void editorFreeActionList(EditorActionList *thisptr)
{
  EditorActionList *temp;
  
  // Iterate through the list
  while (thisptr)
  {
    // Save current node
    temp    = thisptr;
    
    // Move to next node
    thisptr = thisptr->next;
    
    // Free the action in current node
    editorFreeAction(temp->action);
    
    // Free the node itself
    free(temp);
  }
}