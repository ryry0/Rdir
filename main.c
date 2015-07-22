//Author: Ryan - David Reyes

#include <stdio.h>
#include <stdlib.h>   //malloc
#include <ncurses.h>  //for drawing to screen
#include <unistd.h>   //for getcwd
#include <linux/limits.h>   //for PATH_MAX
#include <sys/stat.h> //for determining if directory
#include <signal.h>   //signal handling
#include <string.h>   //strlen
#include <dirent.h>
#include <stdbool.h>

#define DEFAULT_CONFIG_FILE ".rdirrc"
#define ENTER 0x10

/**********************************/
/*        ENUM DEFINITIONS        */
/**********************************/
typedef enum {RUNNING, EXITING} program_states_t; //overall state of program
//movement of folder up the tree or down the tree
typedef enum {STAY, UP, DOWN, NEXT, PREV} dir_movement_t;

/**********************************/
/*       STRUCT DEFINITIONS       */
/**********************************/
typedef struct {
  size_t max_rows;
  size_t max_columns;
} screen_t;

//represents a single directory entry
typedef struct {
  char * basename;
  bool is_dir;
} directory_entry_t;

//represents a list of above entries
typedef struct {
  size_t capacity;
  directory_entry_t * entries;
  char * path;
} directory_entry_list_t;


/**********************************/
/*       FUNCTION PROTOTYPES      */
/**********************************/
void initTerminal(screen_t *screen);
void initSettings();
int readConfig(char * config_file_name);
void handleKeys(char input, directory_entry_list_t *dir_list,
    size_t *selected_dir_index);
int listDirs(directory_entry_list_t *dir_list, char * path);
void clearDirList(directory_entry_list_t *dir_list);
void initDirList(directory_entry_list_t *dir_list);
void signalHandler(int param);


/**********************************/
/*          GLOBAL STATE          */
/**********************************/
static struct Rdir {
  program_states_t state;
  size_t parent_column_width;
  size_t current_column_width;
  size_t child_column_width;
} rdir;

/**********************************/
/*             MAIN               */
/**********************************/
int main(int argc, char ** argv) {
  screen_t screen;
  char * config_file_name = DEFAULT_CONFIG_FILE;
  size_t selected_dir_index = 0; //zero based index

  directory_entry_list_t dir_parent;    //directory entry list for parent dir
  directory_entry_list_t dir_child;     //directory entry list for highlighted dir
  directory_entry_list_t dir_current;   //direntry list for current working dir

  char input;

  //ncurses initialization
  initTerminal(&screen);

  initSettings();
  //read configuration file
  if (readConfig(config_file_name) == -1) {
    fprintf(stderr, "using defaults");
  }

  //initialize the lists
  initDirList(&dir_parent);
  initDirList(&dir_child);
  initDirList(&dir_current);

  //main loop
  while (rdir.state == RUNNING) {
    //get directories
    listDirs(&dir_current, ".");
    listDirs(&dir_parent, "..");

    //force index to be current dir max
    if (selected_dir_index >= dir_current.capacity) {
      if (dir_current.capacity == 0) //if it's actually zero force zero
        selected_dir_index = 0;
      else
        selected_dir_index = dir_current.capacity -1;
    }

    //make sure something exists in dir before trying to get child
    if (dir_current.capacity > 0) {
      if (dir_current.entries[selected_dir_index].is_dir)
        listDirs(&dir_child, dir_current.entries[selected_dir_index].basename);
    }

    //clear screen
    clear();
    //draw screen
    //print parent if current directory is not root
    if (strcmp(dir_current.path, "/") != 0) {
      for(int i = 0; i < dir_parent.capacity; ++i) {
        mvprintw(i, 0, "%s", dir_parent.entries[i].basename);
      }
    }
    for(int i = 0; i < dir_current.capacity; ++i) {
      if (i == selected_dir_index)
        attron(A_STANDOUT);
      mvprintw(i, rdir.parent_column_width, "%s", dir_current.entries[i].basename);
      if (i == selected_dir_index)
        attroff(A_STANDOUT);
    }
    if (dir_current.entries[selected_dir_index].is_dir) {
      for(int i = 0; i < dir_child.capacity; ++i) {
        mvprintw(i, rdir.current_column_width + rdir.parent_column_width,
            "%s", dir_child.entries[i].basename);
      }
    }

    input = getch();
    handleKeys(input, &dir_current, &selected_dir_index); //handle keys and exec key action

    if (dir_current.entries[selected_dir_index].is_dir)
      clearDirList(&dir_child);
    clearDirList(&dir_current);
    clearDirList(&dir_parent);
  } //end while (state)

  clearDirList(&dir_current);
  clearDirList(&dir_parent);
  clearDirList(&dir_child);
  endwin();
  return 0;
} //end main

/**********************************/
/*        FUNC DEFINITIONS        */
/**********************************/
/*
 * INITSETTINGS initializes the values in the global struct
 */
void initSettings() {
  rdir.state = RUNNING;
  rdir.parent_column_width = 30;
  rdir.child_column_width = 30;
  rdir.current_column_width = 30;
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
 *
 */
void handleKeys(char input, directory_entry_list_t *dir_list, size_t
    *selected_dir_index) {
  switch(input) {
    case 'j':
      *selected_dir_index = *selected_dir_index + 1;
      if (*selected_dir_index >= dir_list->capacity)
        *selected_dir_index = dir_list->capacity - 1;
      break;
    case 'k':
      if (*selected_dir_index > 0)
        *selected_dir_index = *selected_dir_index - 1;
      break;
    case 'h':
      chdir("..");
      break;
    case 'l':
      chdir(dir_list->entries[*selected_dir_index].basename);
      break;
    case 'q':
      rdir.state = EXITING;
      break;
    case '\n':
      fprintf(stderr, "%s/%s", dir_list->path,
          dir_list->entries[*selected_dir_index].basename);
      rdir.state = EXITING;
      break;
  }
} //end handleKeys

/*
 * LISTDIRS populates the directory lists with the folders in path
 */
int listDirs(directory_entry_list_t *dir_list, char * path) {
  DIR * dir_p;
  struct dirent *entry_p;
  size_t num_dirs = 0;
  size_t counter = 0;
  struct stat stat_buff;
  char cwd[PATH_MAX];

  //get cwd
  getcwd(cwd, PATH_MAX);
  dir_list->path = malloc(strlen(cwd)+1);
  strcpy(dir_list->path, cwd);

  dir_p = opendir(path);
  if (dir_p == NULL) {
    closedir(dir_p);
    return -1;
  }

  //grab number of dirs and allocate corresponding amount
  while ((entry_p = readdir(dir_p))) {
    if (strncmp(entry_p->d_name, ".", 1) != 0) {
      num_dirs++;
    }
  }

  dir_list->capacity = num_dirs;
  dir_list->entries = malloc(num_dirs * sizeof (directory_entry_t));
  rewinddir(dir_p);

  while ((entry_p = readdir(dir_p))) {
    //if it doesn't start with period
    if (strncmp(entry_p->d_name, ".", 1) != 0) {
      dir_list->entries[counter].basename = malloc(strlen(entry_p->d_name)+1);
      strcpy(dir_list->entries[counter].basename, entry_p->d_name);

      //check if directory
      stat(entry_p->d_name, &stat_buff);
      if (S_ISDIR(stat_buff.st_mode))
        dir_list->entries[counter].is_dir = true;
      else
        dir_list->entries[counter].is_dir = false;

      counter++;
    }
  } //end while (entry_p...
  closedir(dir_p);

  return 0;
} //end listDirs

/*
 * CLEARDIRLIST deletes the directory lists with the folders in path
 */
void clearDirList(directory_entry_list_t *dir_list) {
  if (dir_list->path != NULL) {
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
  rdir.state = EXITING;
}
