#ifndef DIR_LIST_H_
#define DIR_LIST_H_

#include <stdlib.h>
#include <stdbool.h>  //true or false

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

int getDirList(directory_entry_list_t *dir_list, char *path);
void clearDirList(directory_entry_list_t *dir_list);
void initDirList(directory_entry_list_t *dir_list);

#endif
