//Author: Ryan - David Reyes

/**********************************/
/*        SYSTEM INCLUDES         */
/**********************************/
#include <ncurses.h>  //for drawing to screen
#include <signal.h>   //signal handling
#include <string.h>

/**********************************/
/*         OTHER INCLUDES         */
/**********************************/
#include <rdir.h>
#include <hashtable.h>
#include <dir_list.h>
#include <keyboard.h>

#define DEFAULT_CONFIG_FILE ".rdirrc"
#define ENTER 0x10

/**********************************/
/*       FUNCTION PROTOTYPES      */
/**********************************/
void initTerminal(screen_t *screen);
void initSettings();
int readConfig(char *config_file_name);
void signalHandler(int param);
void printColumn(directory_entry_list_t *dir_list, size_t offset,
    char *format_string);

/**********************************/
/*             MAIN               */
/**********************************/
int main(int argc, char ** argv) {
  rdir_t rdir;
  char format_string[10];
  //ncurses initialization
  initTerminal(&rdir.screen);

  initTable(&rdir.command_key_mappings, 20); //initialize hash table of key mappings
  initTable(&rdir.search_key_mappings, 20); //initialize hash table of key mappings
  initSettings(&rdir);

  //read configuration file
  if (readConfig(rdir.config_file_name) == -1) {
    //fprintf(stderr, "using default settings");
  }

  sprintf(format_string, "%%.%lds", rdir.current_column_width - 1);

  //initialize the lists
  for (int i = 0; i < NUM_DIR_LISTS; ++i) {
    initDirList(&rdir.dir_lists[i]);
  }

  //main loop
  while (rdir.state == RUNNING) {
    //GRAB DIRECTORIES
    //populate current and parent directory lists
    getDirList(&rdir.dir_current, ".");
    getDirList(&rdir.dir_parent, "..");

    //bounds checking for the selected folder index
    if (rdir.selected_dir_index >= rdir.dir_current.capacity) {
      if (rdir.dir_current.capacity == 0) {//if capacity zero force index to zero
        rdir.selected_dir_index = 0;
        rdir.cursor_index = 0;
      }
      else {//else force index to capacity - 1
        rdir.selected_dir_index = rdir.dir_current.capacity -1;
        rdir.cursor_index = rdir.selected_dir_index;
      }
    }

    //make sure something exists in current dir before
    //trying to get selected dir's contents
    if (rdir.dir_current.capacity > 0) {
      if (rdir.dir_current.entries[rdir.selected_dir_index].is_dir)
        getDirList(&rdir.dir_selected,
            rdir.dir_current.entries[rdir.selected_dir_index].basename);
    }

    //DRAW DISPLAY
    //clear screen
    clear();
    //print parent dir list if current directory is not root
    if (strcmp(rdir.dir_current.path, "/") != 0) {
      printColumn(&rdir.dir_parent, 0, format_string);
    }

    //print current directory and highlights
    for(int i = 0; (i < rdir.dir_current.capacity && i < rdir.screen.max_rows); ++i) {
      if (i == rdir.cursor_index) //if its selected, highlight it
        attron(A_STANDOUT);

      if (rdir.dir_current.entries[i + rdir.begin_list_offset].is_dir) {//if dir, color it
        attron(COLOR_PAIR(1));
        attron(A_BOLD);
      }

      mvprintw(i, rdir.parent_column_width, format_string,
          rdir.dir_current.entries[i + rdir.begin_list_offset].basename);

      if (rdir.dir_current.entries[i + rdir.begin_list_offset].is_dir) { //if dir, color it
        attroff(COLOR_PAIR(1));
        attroff(A_BOLD);
      }

      if (i == rdir.cursor_index) //unhighlight
        attroff(A_STANDOUT);
    } //end for

    //print the contents of the selected directory
    if (rdir.dir_current.entries[rdir.selected_dir_index].is_dir) {
      printColumn(&rdir.dir_selected, rdir.current_column_width +
          rdir.parent_column_width, format_string);
    }

    //debug

    mvprintw(rdir.screen.max_rows -2, 0, "sdi:%d ci:%d lo:%d mr:%d",
        rdir.selected_dir_index, rdir.cursor_index, rdir.begin_list_offset, rdir.screen.max_rows);

    if (rdir.mode == SEARCH)
      mvprintw(rdir.screen.max_rows -1, 0, "/%s", rdir.search_buffer);

    mvprintw(rdir.screen.max_rows -3, 0, "inputc:%c inputd: %d", rdir.input,
        rdir.input);

    //HANDLE INPUT
    rdir.input = getch();


    //handle keys and exec key action
    handleKeys(&rdir);

    //CLEAR LISTS
    if (rdir.dir_current.entries[rdir.selected_dir_index].is_dir)
      clearDirList(&rdir.dir_selected);
    clearDirList(&rdir.dir_current);
    clearDirList(&rdir.dir_parent);
  } //end while (state)

  clearDirList(&rdir.dir_current);
  clearDirList(&rdir.dir_parent);
  clearDirList(&rdir.dir_selected);
  destroyTable(&rdir.command_key_mappings); //destroy hash table of key mappings
  destroyTable(&rdir.search_key_mappings); //initialize hash table of key mappings
  endwin();
  return 0;
} //end main

/**********************************/
/*        FUNCTION DEFINITIONS        */
/**********************************/
/*
 * INITSETTINGS initializes the values in the global struct
 */
void initSettings(rdir_t *rdir) {
  rdir->state = RUNNING;
  rdir->config_file_name = DEFAULT_CONFIG_FILE;

  rdir->parent_column_width = 30;
  rdir->child_column_width = 30;
  rdir->current_column_width = 30;

  rdir->selected_dir_index = 0;
  rdir->begin_list_offset = 0;
  rdir->cursor_index = 0;
  rdir->mode = COMMAND;

  //set up default key bindings
  insert(&rdir->command_key_mappings, "j", MOVE_SEL_DOWN);
  insert(&rdir->command_key_mappings, "k", MOVE_SEL_UP);
  insert(&rdir->command_key_mappings, "h", UP_DIR);
  insert(&rdir->command_key_mappings, "l", CH_DIR);
  insert(&rdir->command_key_mappings, "f", SET_FORWARD_MODE);
  insert(&rdir->command_key_mappings, "/", SET_SEARCH_MODE);
  insert(&rdir->command_key_mappings, "c", PRINT_CURRENT_DIR);
  insert(&rdir->command_key_mappings, "\n", PRINT_SEL_DIR);
  insert(&rdir->command_key_mappings, "q", QUIT);

  insert(&rdir->search_key_mappings, "/", CHDIR);
  insert(&rdir->search_key_mappings, "\n", SELECT);
  insert(&rdir->search_key_mappings, "\t", TRAVEL);
}

/*
 * INITTERMINAL perform all initialization regarding the terminal and ncurses
 */
void initTerminal(screen_t *screen) {
  initscr();  //start curses mode
  getmaxyx(stdscr, screen->max_rows, screen->max_columns);
  raw();      //disable line buffering (no need to press enter with getch());
  keypad(stdscr, TRUE); //allows the user to read F1..arrow keys
  noecho();             //no echo with getch
  curs_set(FALSE);      //no blinking cursor
  start_color();
  use_default_colors();
  init_pair(1, COLOR_BLUE, -1); //allow transparency support
} //end void initTerminal

/*
 * READCONFIG reads the configuration file (.rdirrc by default)
 * and sets the corresponding structs.
 */
int readConfig(char * config_file_name) {
  FILE * file_p;

  file_p = fopen(config_file_name, "r");

  if (file_p == NULL)
    return -1;

  fclose(file_p);
  return 0;
} //end readConfig

void printColumn(directory_entry_list_t *dir_list, size_t offset, char *format_string) {
  for(int i = 0; i < dir_list->capacity; ++i) {

    if (dir_list->entries[i].is_dir) { //if dir, color it
      attron(COLOR_PAIR(1));
      attron(A_BOLD);
    }
    mvprintw(i, offset, format_string, dir_list->entries[i].basename);

    if (dir_list->entries[i].is_dir) { //if dir, color it
      attroff(COLOR_PAIR(1));
      attroff(A_BOLD);
    }
  } //end for(int
}

/*
 * SIGNALHANDLER sets state var to exit
 */
void signalHandler(int param) {
  //rdir.state = EXITING;
}
