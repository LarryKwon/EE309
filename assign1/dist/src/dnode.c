#include "dnode.h"

#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_MODE_BUF_SIZE 16
#define MAX_PATH_SIZE 0x1000

static const char type_chars[16] = "?pc?d?b?-?l?s???";
static const char mode_chars[7] = "rwxSTst";
static const mode_t mode_flags[] = {S_IRUSR, S_IWUSR, S_IXUSR, S_ISUID,
                                    S_IRGRP, S_IWGRP, S_IXGRP, S_ISGID,
                                    S_IROTH, S_IWOTH, S_IXOTH, S_ISVTX};

extern time_t current_time;
extern int is_all;
extern int is_long;

// mode_string : convert mode flag into mode string
// @param mode : mode flag that will be converted
// @param buf : buf for mode string
// @return  : string pointer of mode string (same with param buf)
const char *mode_string(mode_t mode, char *buf)
{
  char *p = buf;

  int i = 0, j, k;

  *p = type_chars[TYPEINDEX(mode)];
  do
  {
    j = k = 0;
    do
    {
      *++p = '-';
      if (mode & mode_flags[i + j])
      {
        *p = mode_chars[j];
        k = j;
      }
    } while (++j < 3);
    if (mode & mode_flags[i + j])
    {
      *p = mode_chars[3 + (k & 2) + ((i & 8) >> 3)];
    }
    i += 4;
  } while (i < 12);

  return buf;
}

// display_dnode_simple : print given dnode info simply (only name)
// @param d : dnode that will be printed
void display_dnode_simple(struct dnode *d) { printf("%s\n", d->name); }

// display_dnode_long : print detailed info about given dnode (long format)
// @param d : dnode that will be printed
void display_dnode_long(struct dnode *d)
{
  char mode_buf[MAX_MODE_BUF_SIZE], *filetime, *lpath;
  ssize_t ret = 0;

  memset(mode_buf, '\0', sizeof(mode_buf));

  printf("%-10s ", mode_string(d->dn_mode, mode_buf));
  printf("%4lu ", (long)d->dn_nlink);
  printf("%-8.8s %-8.8s ", getpwuid(d->dn_uid)->pw_name,
         getgrgid(d->dn_gid)->gr_name);
  printf("%9lu ", d->dn_size);

  filetime = ctime(&d->dn_time);
  /* filetime's format: "Wed Jun 30 21:49:08 1993\n" */
  time_t age = current_time - d->dn_time;
  if (age < 3600L * 24 * 365 / 2 && age > -15 * 60)
  {
    /* less than 6 months old */
    /* "mmm dd hh:mm " */
    printf("%.12s ", filetime + 4);
  }
  else
  {
    /* "mmm dd  yyyy " */
    /* "mmm dd yyyyy " after year 9999 :) */
    strchr(filetime + 20, '\n')[0] = ' ';
    printf("%.7s%6s", filetime + 4, filetime + 20);
  }

  printf("%s", d->name);
  if (S_ISLNK(d->dn_mode))
  {
    if (!(lpath = malloc(MAX_PATH_SIZE + 1)))
    {
      fprintf(stderr, "cannot allocate lpath : %s\n", strerror(errno));
      return; /* could not allocate lpath */
    }

    // TODO: Task 3
    //
    //
    //

    lpath[ret] = '\0';

    printf(" -> ");
    printf("%s", lpath);
    free(lpath);
  }
  putchar('\n');
}

// parse_dnode : parse the info about given file into dnode
// @param fullname : (full) file path for parsing
// @param follow_link : determine wheter follows symlink or not
// @return : dnode pointer that stores info about given file
struct dnode *parse_dnode(const char *fullname, int follow_link)
{
  struct dnode *res;

  if (!(res = malloc(sizeof(struct dnode))))
  {
    fprintf(stderr, "cannot allocate dnode : %s\n", strerror(errno));
    return NULL; /* could not allocate dnode */
  }

  struct stat fileStat;

  // TODO: Task 3
  if (follow_link)
  {
    if (stat(fullname, &fileStat) == -1)
    {
      perror("stat");
      free(res);
      return NULL;
    }
  }
  else
  {
    if (lstat(fullname, &fileStat) == -1)
    {
      perror("lstat");
      free(res);
      return NULL;
    }
  }

  res->fullname = strdup(fullname);
  res->dn_next = NULL;
  printf("parse-dnode file: %s %d %d\n", fullname, fileStat.st_blocks, fileStat.st_mode);
  // TODO: Task 3
  res->dn_size = fileStat.st_size;
  res->dn_blocks = fileStat.st_blocks;
  res->dn_time = fileStat.st_mtime;
  res->dn_ino = fileStat.st_ino;
  res->dn_mode = fileStat.st_mode;
  res->dn_nlink = fileStat.st_nlink;
  res->dn_uid = fileStat.st_uid;
  res->dn_gid = fileStat.st_gid;

  return res;
}

// compare_dnode_name : compare two dnode pointer using lexicographical order of
// their name
// @param a : the first dnode double pointer
// @param b : the second dnode second pointer
// @return : < 0 if a comes first, 0 for same name, > 0 for a comes later
int compare_dnode_name(const void *a, const void *b)
{
  struct dnode *d1 = *(struct dnode **)a;
  struct dnode *d2 = *(struct dnode **)b;
  // printf("%s %s\n", d1->name, d2->name);
  // printf("strcmp result: %d\n", strcmp(d1->name, d2->name));
  return strcmp(d1->name, d2->name);
}

// sort_dnode_entries : sort given dnode linked list into dnode array
// @param head : head pointer of dnode linked list
// @param cnt : count of dnodes
// @return : dnode double pointer that points sorted dnode array
struct dnode **sort_dnode_entries(struct dnode *head, size_t cnt)
{
  struct dnode **res, **curr;

  if (!(res = calloc(sizeof(struct dnode *), cnt)))
  {
    fprintf(stderr, "cannot allocate dnode array : %s\n", strerror(errno));
    return NULL; /* could not allocate dnode array */
  }
  curr = res;
  while (head)
  {
    printf("while-loop file: %s %d %d\n", head->name, head->dn_blocks, head->dn_mode);
    *curr++ = head;
    head = head->dn_next;
  }

  struct dnode **head_sort = res;
  struct dnode **target = res;
  int i = 1;
  // insetion sort
  while (i < cnt)
  {
    // printf("filename: %s\n", (*target)->name);
    int walk_index = i - 1;
    int target_index = i;
    struct dnode **walk = head_sort + walk_index;
    struct dnode **target = head_sort + target_index;
    // printf("walk: %p\n", walk);
    // printf("head_sort: %p\n", head_sort);
    while (compare_dnode_name(target, walk) < 0 && (walk_index >= 0))
    {
      // printf("%s\n", "should swap");
      struct dnode *tmp = *target;
      *target = *walk;
      *walk = tmp;
      walk_index--;
      target_index--;
      walk = head_sort + walk_index;
      target = head_sort + target_index;
      if (walk_index < 0)
        break;
    }
    // printf("target: %s\n", (*target)->name);
    // printf("target: %s swap_target: %s\n", (*target)->name,(*walk)->name);
    i++;
  }

  for (int i = 0; i < cnt; i++)
  {
    printf("file: %s %d %d\n", (*(res + i))->name, (*(res + i))->dn_blocks, (*(res + i))->dn_mode);
  }
  return res;
}

// display_dnode : display given dnode based on proper flags
// @param d : dnode that will be displayed
void display_dnode(struct dnode *d)
{
  if (is_long)
  {
    display_dnode_long(d);
  }
  else
    display_dnode_simple(d);
}

// display_dnode_arr : display given dnode arrays
// @param arr : dnode double pointer that points dnode array
// @param cnt : count of dnodes
void display_dnode_arr(struct dnode **arr, size_t cnt)
{
  size_t total_blocks = 1;
  if (is_long)
  {
    for (size_t i = 0; i < cnt; i++)
      total_blocks += arr[i]->dn_blocks;
    /* Even though standard says use 512 byte blocks, coreutils use 1k */
    /* Actually, we round up by calculating (blocks + 1) / 2, */
    /* "+ 1" was done when we initialized blocks to 1 */
    printf("total %lu\n", total_blocks >> 1);
  }
  for (size_t i = 0; i < cnt; i++)
    display_dnode(arr[i]);
}

// free_dnode : free given dnode
// @param d : dnode pointer that will be freed
void free_dnode(struct dnode *d)
{
  free(d->name);
  free(d->fullname);
  free(d);
}