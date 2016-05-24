#include <keyboard.h>
#include <ncurses.h>
#include <stdio.h>
#include <dirent.h>
#include <hashtable.h>
#include <unistd.h>

int getMapping(hash_table *map_table, char *key) {
  hash_node *map_node = retrieve(map_table, key);
  if (map_node != NULL)
    return map_node->data;
  else
    return 0;
}

/*
 * HANDLEKEYS takes the input and executes the corresponding bound action
 */
void handleKeys(rdir_t *rdir) {
  char input_buffer[2] = {0};
  input_buffer[0] = rdir->input;
  char f_input;

  command_actions_t command_action;
  search_actions_t search_action;

  size_t *selected_dir_index = &rdir->selected_dir_index; //zero based index
  size_t *begin_list_offset = &rdir->begin_list_offset;
  size_t *cursor_index = &rdir->cursor_index;
  directory_entry_list_t *dir_current = &rdir->dir_current;
  screen_t screen = rdir->screen;
  program_states_t *state = &rdir->state;

  switch(rdir->mode) {
    case COMMAND:
      command_action = getMapping(&rdir->command_key_mappings, input_buffer);
      switch(command_action) {
        case MOVE_SEL_DOWN: //move selection down
          *selected_dir_index = *selected_dir_index + 1;
          *cursor_index = *cursor_index + 1;

          if (*selected_dir_index >= dir_current->capacity) //bounds checking
            *selected_dir_index = dir_current->capacity - 1;

          //if the selection is off screen scroll down
          if (*cursor_index > screen.max_rows - 1)
            (*begin_list_offset)++;

          //bounds checking for cursor
          if (*cursor_index >= screen.max_rows - 1)
            *cursor_index = screen.max_rows - 1;

          if (*cursor_index >= dir_current->capacity)
            *cursor_index = dir_current->capacity - 1;

          //bounds checking for list offset
          //should only apply if list goes off screen
          if(screen.max_rows < dir_current->capacity) {
            if (*begin_list_offset + screen.max_rows > dir_current->capacity)
              (*begin_list_offset)--;
          }
          break;

        case MOVE_SEL_UP: //move selection up
          if (*selected_dir_index > 0)
            *selected_dir_index = *selected_dir_index - 1;

          if (*cursor_index > 0)
            *cursor_index = *cursor_index - 1;

          //if the selection is off screen scroll up
          if (*selected_dir_index < *begin_list_offset)
            (*begin_list_offset)--;
          break;

        case UP_DIR: //change current dir up
          chdir("..");
          //reset scroll
          *begin_list_offset = 0;
          break;

        case CH_DIR: //change current directory to selected dir
          chdir(dir_current->entries[*selected_dir_index].basename);
          *begin_list_offset = 0;
          break;

        case FORWARD_MODE:
          rdir->mode = FORWARD;
          break;

        case SEARCH_MODE:
          rdir->mode = SEARCH;
          break;

        case QUIT: //quit
          *state = EXITING;
          break;

        case PRINT_CURRENT_DIR:
          fprintf(stderr, "%s", dir_current->path);
          *state = EXITING;
          break;

        case PRINT_SEL_DIR: //print selected dir to stderr
          fprintf(stderr, "%s/%s", dir_current->path,
              dir_current->entries[*selected_dir_index].basename);
          *state = EXITING;
          break;
      } //end switch key
      break;

    case FORWARD:
      f_input = rdir->input;
      *begin_list_offset = 0;
      *cursor_index = 0;
      *selected_dir_index = 0;
      for (int i = 0; i < dir_current->capacity; ++i) {
        if(dir_current->entries[*selected_dir_index].basename[0] == f_input)
          break;

        *selected_dir_index = *selected_dir_index + 1;
        *cursor_index = *cursor_index + 1;

        if (*selected_dir_index >= dir_current->capacity) //bounds checking
          *selected_dir_index = dir_current->capacity - 1;

        //if the selection is off screen scroll down
        if (*cursor_index > screen.max_rows - 1)
          (*begin_list_offset)++;

        //bounds checking for cursor
        if (*cursor_index >= screen.max_rows - 1)
          *cursor_index = screen.max_rows - 1;

        if (*cursor_index >= dir_current->capacity)
          *cursor_index = dir_current->capacity - 1;

        //bounds checking for list offset
        //should only apply if list goes off screen
        if(screen.max_rows < dir_current->capacity) {
          if (*begin_list_offset + screen.max_rows > dir_current->capacity)
            (*begin_list_offset)--;
        }
      }
      rdir->mode = COMMAND;
      break;

    case SEARCH:
      f_input = rdir->input;
      *begin_list_offset = 0;
      *cursor_index = 0;
      *selected_dir_index = 0;
      for (int i = 0; i < dir_current->capacity; ++i) {
        if(dir_current->entries[*selected_dir_index].basename[0] == f_input)
          break;

        *selected_dir_index = *selected_dir_index + 1;
        *cursor_index = *cursor_index + 1;

        if (*selected_dir_index >= dir_current->capacity) //bounds checking
          *selected_dir_index = dir_current->capacity - 1;

        //if the selection is off screen scroll down
        if (*cursor_index > screen.max_rows - 1)
          (*begin_list_offset)++;

        //bounds checking for cursor
        if (*cursor_index >= screen.max_rows - 1)
          *cursor_index = screen.max_rows - 1;

        if (*cursor_index >= dir_current->capacity)
          *cursor_index = dir_current->capacity - 1;

        //bounds checking for list offset
        //should only apply if list goes off screen
        if(screen.max_rows < dir_current->capacity) {
          if (*begin_list_offset + screen.max_rows > dir_current->capacity)
            (*begin_list_offset)--;
        }
      }
      rdir->mode = COMMAND;
      break;
  } //end switch mode
} //end handleKeys
