//Author: Ryan - David Reyes

#include <stdio.h>
#include <stdlib.h>   //malloc
#include <ncurses.h>  //for drawing to screen
#include <unistd.h>   //for getcwd
#include <linux/limits.h>   //for PATH_MAX
#include <sys/stat.h> //for determining if directory
#include <signal.h>   //signal handling
#include <string.h>   //strlen
#include <dirent.h>   //to get directory entries
#include <stdbool.h>  //true or false

#define DEFAULT_CONFIG_FILE ".rdirrc"
#define ENTER 0x10
#define NUM_DIR_LISTS 3

/**********************************/
/*        ENUM DEFINITIONS        */
/**********************************/
typedef enum {RUNNING, EXITING} program_states_t; //overall state of program
//movement of folder up the tree or down the tree
//typedef enum {STAY, UP, DOWN, NEXT, PREV} dir_movement_t;

/**********************************/
/*       STRUCT DEFINITIONS       */
/**********************************/
typedef struct {
  size_t max_rows;
  size_t max_columns;
} screen_t;

//represents a single directory entry
typedef struct {
  char *basename;
  bool is_dir;
} directory_entry_t;

//represents a list of above entries
typedef struct {
  size_t capacity;
  directory_entry_t *entries;
  char *path;
} directory_entry_list_t;

typedef struct {
  program_states_t state;
  screen_t screen;

  char   *config_file_name;
  char input;

  size_t parent_column_width;
  size_t current_column_width;
  size_t child_column_width;

  size_t selected_dir_index; //zero based index
  size_t begin_list_offset;
  size_t cursor_index;

  union {
    struct {
      directory_entry_list_t dir_parent;    //directory entry list for parent dir
      directory_entry_list_t dir_current;   //direntry list for current working dir
      directory_entry_list_t dir_selected;  //directory entry list for highlighted dir
    };
    directory_entry_list_t dir_lists[NUM_DIR_LISTS];
  };
} rdir_t;

/**********************************/
/*       FUNCTION PROTOTYPES      */
/**********************************/
void initTerminal(screen_t *screen);
void initSettings();
int readConfig(char *config_file_name);
void handleKeys(rdir_t *rdir);
int getDirList(directory_entry_list_t *dir_list, char *path);
void clearDirList(directory_entry_list_t *dir_list);
void initDirList(directory_entry_list_t *dir_list);
void signalHandler(int param);


/**********************************/
/*             MAIN               */
/**********************************/
int main(int argc, char ** argv) {
  //ncurses initialization
  rdir_t rdir;
  initTerminal(&rdir.screen);

  initSettings(&rdir);
  //read configuration file
  if (readConfig(rdir.config_file_name) == -1) {
    //fprintf(stderr, "using default settings");
  }

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

      for(int i = 0; i < rdir.dir_parent.capacity; ++i) {

        if (rdir.dir_parent.entries[i].is_dir) { //if dir, color it
          attron(COLOR_PAIR(1));
          attron(A_BOLD);
        }
        mvprintw(i, 0, "%s", rdir.dir_parent.entries[i].basename);

        if (rdir.dir_parent.entries[i].is_dir) { //if dir, color it
          attroff(COLOR_PAIR(1));
          attroff(A_BOLD);
        }
      } //end for(int

    } //end if (strcmp

    //print current directory and highlights
    for(int i = 0; (i < rdir.dir_current.capacity && i < rdir.screen.max_rows); ++i) {
      if (i == rdir.cursor_index) //if its selected, highlight it
        attron(A_STANDOUT);

      if (rdir.dir_current.entries[i + rdir.begin_list_offset].is_dir) {//if dir, color it
        attron(COLOR_PAIR(1));
        attron(A_BOLD);
      }

      mvprintw(i, rdir.parent_column_width, "%s",
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

      for(int i = 0; i < rdir.dir_selected.capacity; ++i) {

        if (rdir.dir_selected.entries[i].is_dir) { //if dir, color it
          attron(COLOR_PAIR(1));
          attron(A_BOLD);
        }

        mvprintw(i, rdir.current_column_width + rdir.parent_column_width,
            "%s", rdir.dir_selected.entries[i].basename);

        if (rdir.dir_selected.entries[i].is_dir) { //if dir, color it
          attroff(COLOR_PAIR(1));
          attroff(A_BOLD);
        }
      } //end for
    } //end if

    mvprintw(rdir.screen.max_rows -1, 0, "sdi:%d ci:%d lo:%d mr:%d",
        rdir.selected_dir_index, rdir.cursor_index, rdir.begin_list_offset, rdir.screen.max_rows);

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

/*
 * HANDLEKEYS takes the input and executes the corresponding bound action
 */
void handleKeys(rdir_t *rdir) {
  char f_input;
  size_t *selected_dir_index = &rdir->selected_dir_index; //zero based index
  size_t *begin_list_offset = &rdir->begin_list_offset;
  size_t *cursor_index = &rdir->cursor_index;
  directory_entry_list_t *dir_current = &rdir->dir_current;
  screen_t screen = rdir->screen;
  program_states_t *state = &rdir->state;

  switch(rdir->input) {
    case 'j': //move selection down
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

    case 'k': //move selection up
      if (*selected_dir_index > 0)
        *selected_dir_index = *selected_dir_index - 1;

      if (*cursor_index > 0)
        *cursor_index = *cursor_index - 1;

      //if the selection is off screen scroll up
      if (*selected_dir_index < *begin_list_offset)
        (*begin_list_offset)--;
      break;

    case 'h': //change current dir up
      chdir("..");
      //reset scroll
      *begin_list_offset = 0;
      break;

    case 'l': //change current directory to selected dir
      chdir(dir_current->entries[*selected_dir_index].basename);
      *begin_list_offset = 0;
      break;

    case 'f':
      f_input = getch();
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
      break;

    case 'q': //quit
      *state = EXITING;
      break;

    case 'c':
      fprintf(stderr, "%s", dir_current->path);
      *state = EXITING;
      break;

    case '\n': //print selected dir to stderr
      fprintf(stderr, "%s/%s", dir_current->path,
          dir_current->entries[*selected_dir_index].basename);
      *state = EXITING;
      break;
  }
} //end handleKeys

/*
 * getDirList populates the directory lists with the folders in path
 */
int getDirList(directory_entry_list_t *dir_list, char * path) {
  struct dirent **namelist;
  size_t num_entries = 0;
  size_t counter = 0;
  struct stat stat_buff;
  char cwd[PATH_MAX];
  char new_wd[PATH_MAX];

  //get cwd
  getcwd(cwd, PATH_MAX); //save working directory

  chdir(path); //change to new directory
  getcwd(new_wd, PATH_MAX);
  dir_list->path = malloc(strlen(new_wd)+1);
  strcpy(dir_list->path, new_wd);

  //grab number of dirs
  num_entries = scandir(new_wd, &namelist, NULL, alphasort);

  //recount dir capacity
  dir_list->capacity = 0;
  for (int i = 0; i < num_entries; i++) { //get num entries without preceding dots
    if (strncmp(namelist[i]->d_name, ".", 1) != 0) {
      dir_list->capacity++;
    }
  }
  //allocate required number of entries
  dir_list->entries = malloc(dir_list->capacity * sizeof (directory_entry_t));

  for (int i = 0; i < num_entries; i++) {
    //if it doesn't start with period, put in dirlist
    if (strncmp(namelist[i]->d_name, ".", 1) != 0) {
      dir_list->entries[counter].basename =
        malloc(strlen(namelist[i]->d_name)+1);
      strcpy(dir_list->entries[counter].basename, namelist[i]->d_name);

      //check if entry is directory
      stat(namelist[i]->d_name, &stat_buff);
      if (S_ISDIR(stat_buff.st_mode))
        dir_list->entries[counter].is_dir = true;
      else
        dir_list->entries[counter].is_dir = false;
      counter ++;
    }
    free(namelist[i]);
  } //end while (entry_p...

  free(namelist);
  chdir(cwd); //return to previous working directory

  return 0;
} //end getDirList

/*
 * CLEARDIRLIST deletes the directory lists with the folders in path
 */
void clearDirList(directory_entry_list_t *dir_list) {
  if (dir_list->path != NULL) { //clear the path
    free(dir_list->path);
    dir_list->path = NULL;
  }
  if (dir_list->entries != NULL) {
    //clear all names
    for (int i = 0; i < dir_list->capacity; ++i) {
      if (dir_list->entries[i].basename != NULL) {
        free(dir_list->entries[i].basename);
        dir_list->entries[i].basename = NULL;
      }
    }

    //free directory list itself
    free(dir_list->entries);
    dir_list->entries = NULL;
    dir_list->capacity = 0;
  }
} //end clearDirList

/*
 * INITDIRLIST initializes directory lists to nothing
 */
void initDirList(directory_entry_list_t *dir_list) {
  dir_list->entries = NULL;
  dir_list->path = NULL;
  dir_list->capacity = 0;
} //end initDirList

/*
 * SIGNALHANDLER sets state var to exit
 */
void signalHandler(int param) {
  //rdir.state = EXITING;
}
