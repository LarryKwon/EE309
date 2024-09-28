#include "dir.h"
#include "dnode.h"

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int is_all;
extern int is_print_dir;
extern int is_recursive;

// parse_dir : parse the directory into dnode linked list
// @param path : path of directory that will be parsed
// @param cnt : size_t pointer which count of dnode entries stored
// @return : head pointer of dnode linked list
struct dnode *parse_dir(const char *path, size_t *cnt)
{
  struct dnode *head = NULL;
  *cnt = 0;

  // TODO: Task 2
  DIR *dir = opendir(path);
  if (dir == NULL)
  {
    fprintf(stderr, "cannot open directory : %s\n", strerror(errno));
    return NULL;
  }
  struct dirent *entry;
  struct dnode *current;
  while ((entry = readdir(dir)) != NULL)
  {
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
    {
        continue;
    }
    if(!is_all && entry->d_name[0] == '.'){
      continue;
    }
    (*cnt)++;
    struct dnode *ptr;
    if(!(ptr = (struct dnode *)malloc(sizeof(struct dnode)))){
      fprintf(stderr,"cannot allocate dnode : %s\n", strerror(errno));
    };
    ptr->name = strdup(entry->d_name);
    ptr->fullname = strdup(concat_path(path, entry->d_name));
    ptr->dn_next = head;
    head = ptr;
  }
  current = head;
  // printf("%zd\n", *cnt);
  // for (int i = 0; i < *cnt; i++)
  // {
  //   printf("print files in dir: %s %d\n", current->name, current->dn_mode);
  //   current = current -> dn_next;
  // }
  return head;
}

// print_dir : print the entires of given directory with proper flags
// @param path : path of directory that will be printed
void print_dir(const char *path)
{
  // TODO for assign
  size_t dnode_cnt;
  struct dnode *dnode_head, **dnode_arr;

  dnode_head = parse_dir(path, &dnode_cnt);
  dnode_arr = sort_dnode_entries(dnode_head, dnode_cnt);

  if (is_print_dir)
    printf("%s:\n", path);
  display_dnode_arr(dnode_arr, dnode_cnt);

  // TODO: Task 4
  //
  //
  //

  free(dnode_arr);
}

// concat_path: concatenate two pathes with path seperator
// @param path1 : string pointer of first pathes (will be prefix of path)
// @param path2 : string pointer of second pathes (will be postfix of path)
// @return : string pointer of concatenated path
char *concat_path(const char *path1, const char *path2)
{
  size_t path1_len = strlen(path1), path2_len = strlen(path2);
  char *res;

  // if one of path1 is empty path, return others
  if (!path1_len)
    return strdup(path2);
  if (!path2_len)
    return strdup(path1);

  // now we can assume two non-empthy paths
  // remove trailing path seperator
  if (path1[path1_len - 1] == '/')
    path1_len--;
  // we ignore leading path seperator from path2
  // ...since concat absolute path is not intended

  if (!(res = malloc(path1_len + path2_len + 2)))
    return NULL;

  memcpy(res, path1, path1_len);
  res[path1_len] = '/';
  memcpy(res + path1_len + 1, path2, path2_len);
  res[path1_len + path2_len + 1] = '\0';

  return res;
}