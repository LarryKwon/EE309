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
extern int is_long;

// parse_dir : parse the directory into dnode linked list
// @param path : path of directory that will be parsed
// @param cnt : size_t pointer which count of dnode entries stored
// @return : head pointer of dnode linked list

void free_dnode_list(struct dnode *head) {
    struct dnode *current = head;
    while (current != NULL) {
        struct dnode *next = current->dn_next;
        free(current->fullname);
        free(current->name); // strdup로 할당된 메모리 해제
        free(current);       // dnode 구조체 해제
        current = next;
    }
}

int is_cur_dir(const char *name)
{
  return strcmp(name, ".") == 0;
}

int is_parent_dir(const char *name)
{
  return strcmp(name, "..") == 0;
}

int is_hidden(const char *name)
{
  return name[0] == '.';
}

struct dnode *parse_dir(const char *path, size_t *cnt)
{
  struct dnode *head = NULL;
  *cnt = 0;

  // TODO: Task 2
  DIR *dir = opendir(path);
  if (dir == NULL)
  {
    fprintf(stderr, "cannot open directory %s : %s\n", path, strerror(errno));
    closedir(dir);
    return NULL;
  }
  struct dirent *entry;
  // struct dnode *current;
  while ((entry = readdir(dir)) != NULL)
  {

    if (!is_all)
    {
      if (is_hidden(entry->d_name) || is_cur_dir(entry->d_name) || is_parent_dir(entry->d_name))
      {
        continue;
      }
    }
    (*cnt)++;
    // struct dnode *ptr;
    // if(!(ptr = (struct dnode *)malloc(sizeof(struct dnode)))){
    //   fprintf(stderr,"cannot allocate dnode : %s\n", strerror(errno));
    // };
    // printf("%s\n",path);
    // printf("%s\n",entry->d_name);
    char* concated_path = concat_path(path, entry->d_name);
    struct dnode *ptr = parse_dnode(concated_path, 0);
    if (ptr == NULL) {
      free(concated_path);
      continue;
    }
    free(concated_path);
    ptr->name = strdup(entry->d_name);
    ptr->dn_next = head;
    head = ptr;
  }
  // current = head;
  // printf("%zd\n", *cnt);
  // for (int i = 0; i < *cnt; i++)
  // {
  //   printf("print files in dir: %s %d\n", current->name, current->dn_mode);
  //   current = current -> dn_next;
  // }
  closedir(dir);
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

  // TODO: Task 4: Recursive call
  if (is_recursive)
  {
    struct dnode **current = dnode_arr;
    for (size_t i = 0; i < dnode_cnt; i++)
    {
      current = dnode_arr + i;
      if (S_ISDIR((*current)->dn_mode) &&  !is_cur_dir((*current)->name) && !is_parent_dir((*current)->name))
      {
        // printf("%s\n", (*current)->name);
        printf("\n");
        char *new_path = concat_path(path, (*current)->name);
        // printf("%s\n", new_path);
        print_dir(new_path);
        free(new_path);
      }
    }
    // free(dnode_arr);
  }
    free(dnode_arr);         
    free_dnode_list(dnode_head); 
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