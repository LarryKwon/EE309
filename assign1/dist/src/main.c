#include "dir.h"
#include "dnode.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

extern int optind;

// global values and flags
time_t current_time;
int is_all;
int is_long;
int is_print_dir;
int is_recursive;

void do_ls(char **paths)
{
  char **curr;
  struct dnode *file_head = NULL, *dir_head = NULL;
  struct dnode *d, **file_arr, **dir_arr;
  size_t file_cnt = 0, dir_cnt = 0;

  // parse the given arguments and seperate them by file/directory
  for (curr = paths; *curr; curr++)
  {
    // printf("curr: %s\n", *curr);
    if (!(d = parse_dnode(*curr,
                          !is_long))) // don't follow the link if -l specified
      exit(EXIT_FAILURE);

    d->name = strdup(*curr);
    if (S_ISDIR(d->dn_mode))
    {
      // printf("dir: %s\n", d->name);
      d->dn_next = dir_head;
      dir_head = d;
      dir_cnt++;
    }
    else
    {
      // printf("file: %s\n", d->name);
      d->dn_next = file_head;
      file_head = d;
      file_cnt++;
    }
  }

  // sort each dnodes
  // printf("file_cnt: %ld\n", file_cnt);
  if (!(file_arr = sort_dnode_entries(file_head, file_cnt)))
    exit(EXIT_FAILURE);
  // printf("dir_cnt: %ld\n", dir_cnt);
  if (!(dir_arr = sort_dnode_entries(dir_head, dir_cnt)))
    exit(EXIT_FAILURE);

  // print the non-dir args first
  for (size_t i = 0; i < file_cnt; i++)
  {
    display_dnode(file_arr[i]);
  }

  if (file_cnt && dir_cnt)
    putchar('\n');

  // ...and print the dir args
  for (size_t i = 0; i < dir_cnt; i++)
  {
    print_dir(dir_arr[i]->name);
    if (i != dir_cnt - 1)
      putchar('\n');
  }

  for (size_t i = 0; i < file_cnt; i++)
    free_dnode(file_arr[i]);
  for (size_t i = 0; i < dir_cnt; i++)
    free_dnode(dir_arr[i]);
  free(file_arr);
  free(dir_arr);
}

int main(int argc, char *argv[])
{
  int opt;

  // parse given options and set proper flags
  while ((opt = getopt(argc, argv, "alR")) != -1)
  {
    // char** arg = argv;
    // // while(*arg != NULL){
    // //   printf("%s\n", *arg);
    // //   arg++;
    // // }

    switch (opt)
    {
    // TODO: Task 1
    case 'a':
      is_all = 1;
      break;

    case 'l':
      is_long = 1;
      break;

    case 'R':
      is_recursive = 1;
      break;

    default: /* unknown options */
      fprintf(stderr, "Usage: %s [-alR] [file path]\n", argv[0]);
      exit(EXIT_FAILURE);
    }
  }

  argv += optind;

  // if there's no given args, use cwd
  if (!argv[0])
    *--argv = (char *)".";
  // if multiple arg are given or recursive option is specified,
  // print directory name before printing entries
  if (argv[1] || is_recursive)
    is_print_dir = 1;

  // get time for last-modified time printing
  current_time = time(NULL);

  do_ls(argv);

  return 0;
}
