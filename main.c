//Author: Ryan - David Reyes

#include <stdio.h>
#include <stdlib.h>   //malloc
#include <ncurses.h>  //for drawing to screen
#include <unistd.h>   //for getcwd
#include <limits.h>   //for PATH_MAX
#include <signal.h>   //signal handling
#include <string.h>   //strlen
#include <dirent.h>
#include <stdbool.h>

#define DEFAULT_CONFIG_FILE ".rdirrc"
#define ENTER 0x10

typedef enum {RUNNING, EXITING} program_states_t; //overall state of program
//movement of folder up the tree or down the tree
typedef enum {STAY, UP, DOWN, NEXT, PREV} dir_movement_t;

typedef struct {
  size_t max_rows;
  size_t max_columns;
} screen_t;

//command set
typedef struct {
  dir_movement_t dir_movement;
  bool dir_select;
} dir_command_t;

//represents a single directory entry
typedef struct {
  char * basename;
  char * path;
} directory_entry_t;

//represents a list of above entries
typedef struct {
  int capacity;
  directory_entry_t * entries;
} directory_entry_list_t;

void init_terminal(screen_t *screen);
int readConfig(char * config_file_name);
void handleKeys(char input, dir_command_t *dir_command);
int listDirs(directory_entry_list_t *dir_list, char * path);
void clearDirList(directory_entry_list_t *dir_list);

int main(int argc, char ** argv) {
  screen_t screen;
  program_states_t state = RUNNING;
  //dir_command_t dir_command;
  char * config_file_name = DEFAULT_CONFIG_FILE;

  //directory_entry_list_t dir_parent;    //directory entry list for parent dir
  //directory_entry_list_t dir_child;     //directory entry list for highlighted dir
  directory_entry_list_t dir_current;   //direntry list for current working dir

  char input;

  //ncurses initialization
  init_terminal(&screen);

  //read configuration file
  if (readConfig(config_file_name) == -1) {
    endwin();
    fprintf(stderr, "Config file error");
  }

  //main loop
  while (state == RUNNING) {
    //get directories
    listDirs(&dir_current, ".");
    //listDirs(&dir_parent, "..");
    //listDirs(&dir_child, ".");
    //draw screen
    for(int i = 0; i < dir_current.capacity; ++i) {
      mvprintw(i, 0, "%s", dir_current.entries[i].basename);
    }

    input = getch();
    if (input == 'q')
      state = EXITING;
    //handleKeys(input, &dir_command); //handle keys and exec key action

    clearDirList(&dir_current);
  } //end while (state)

  clearDirList(&dir_current);
  endwin();
  return 0;
} //end main

/*
 * init_terminal perform all initialization regarding the terminal and ncurses
 */
void init_terminal(screen_t *screen) {
  initscr();  //start curses mode
  getmaxyx(stdscr, screen->max_rows, screen->max_columns);
  raw();      //disable line buffering (no need to press enter with getch());
  keypad(stdscr, TRUE); //allows the user to read F1..arrow keys
  noecho();             //no echo with getch
} //end void init_terminal

/*
 * Read config reads the configuration file (.rdirrc by default)
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
 * handleKeys takes the input and executes the corresponding bound action
 *
 */
void handleKeys(char input, dir_command_t *dir_command) {
  switch(input) {
    case 'j':
      break;
    case 'k':
      break;
    case 'h':
      break;
    case 'l':
      break;
    case ENTER:
      break;
  }
} //end handleKeys

/*
 * list_dirs populates the directory lists with the folders in path
 */
int listDirs(directory_entry_list_t *dir_list, char * path) {
  DIR * dir_p;
  struct dirent *entry_p;
  size_t num_dirs = 0;
  size_t counter = 0;

  dir_p = opendir(path);
  if (dir_p == NULL) {
    closedir(dir_p);
    return -1;
  }

  //grab number of dirs and allocate corresponding amount
  while (readdir(dir_p))
    num_dirs++;

  dir_list->capacity = num_dirs;
  dir_list->entries = malloc(num_dirs * sizeof (directory_entry_t));
  rewinddir(dir_p);

  while ((entry_p = readdir(dir_p))) {
    dir_list->entries[counter].basename = malloc(strlen(entry_p->d_name)+1);
    strcpy(dir_list->entries[counter].basename, entry_p->d_name);
    counter++;
  } //end while (entry_p...
  closedir(dir_p);

  return 0;
} //end listDirs

void clearDirList(directory_entry_list_t *dir_list) {
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
