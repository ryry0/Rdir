#ifndef RDIR_H_
#define RDIR_H_
#include <dir_list.h>
#include <hashtable.h>
#define NUM_DIR_LISTS 3
#define SEARCH_BUFF_SIZE 100

/**********************************/
/*        ENUM DEFINITIONS        */
/**********************************/
typedef enum {RUNNING, EXITING} program_states_t; //overall state of program
typedef enum {COMMAND, FORWARD, SEARCH} modes_t;
typedef enum {
  MOVE_SEL_DOWN, MOVE_SEL_UP, //move selections up and down
  UP_DIR, //move into parent directory
  CH_DIR, //move into selected directory
  SET_FORWARD_MODE, //activate forward mode: 'f' key in vim
  SET_SEARCH_MODE,  //activate search mode
  PRINT_CURRENT_DIR, //print current dir to stderr
  PRINT_SEL_DIR, //print highlighted dir to stderr
  QUIT,
  NOP           //no map
} command_actions_t;

typedef enum {
  SELECT, //print matching dir to stdout
  APPEND, //keep adding to search string
  END_SEARCH,  //change directory into matching dir
  TRAVEL, //change dir and re invoke search mode
} search_actions_t;

/**********************************/
/*       STRUCT DEFINITIONS       */
/**********************************/
typedef struct {
  size_t max_rows;
  size_t max_columns;
} screen_t;

typedef struct {
  program_states_t state;
  screen_t screen;

  char   *config_file_name;
  char input;
  char search_buffer[SEARCH_BUFF_SIZE];

  size_t parent_column_width;
  size_t current_column_width;
  size_t child_column_width;

  size_t selected_dir_index; //zero based index
  size_t begin_list_offset;
  size_t cursor_index;

  hash_table command_key_mappings;
  hash_table search_key_mappings;
  modes_t mode;

  union {
    struct {
      directory_entry_list_t dir_parent;    //directory entry list for parent dir
      directory_entry_list_t dir_current;   //direntry list for current working dir
      directory_entry_list_t dir_selected;  //directory entry list for highlighted dir
    };
    directory_entry_list_t dir_lists[NUM_DIR_LISTS];
  };
} rdir_t;

#endif
