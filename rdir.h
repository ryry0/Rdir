#ifndef RDIR_H_
#define RDIR_H_
#include <dir_list.h>
#define NUM_DIR_LISTS 3

/**********************************/
/*        ENUM DEFINITIONS        */
/**********************************/
typedef enum {RUNNING, EXITING} program_states_t; //overall state of program

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

#endif
