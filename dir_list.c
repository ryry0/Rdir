#include <dir_list.h>
#include <stdlib.h>   //malloc
#include <unistd.h>   //for getcwd
#include <linux/limits.h>   //for PATH_MAX
#include <sys/stat.h> //for determining if directory
#include <string.h>   //strlen
#include <dirent.h>   //to get directory entries

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
