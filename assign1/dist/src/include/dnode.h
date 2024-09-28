#ifndef _DNODE_H
#define _DNODE_H

#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

struct dnode {
  char *name;
  char *fullname;
  struct dnode *dn_next;

  off_t dn_size;
  blksize_t dn_blocks;
  time_t dn_time;
  ino_t dn_ino;
  mode_t dn_mode;
  nlink_t dn_nlink;
  uid_t dn_uid;
  gid_t dn_gid;
};

/* FYI type values: 1:fifo 2:char 4:dir 6:blk 8:file 10:link 12:socket
 * (various wacky OSes: 13:Sun door 14:BSD whiteout 5:XENIX named file
 *  3/7:multiplexed char/block device)
 * and we use 0 for unknown and 15 for executables (see below) */
#define TYPEINDEX(mode) (((mode) >> 12) & 0x0f)

const char *mode_string(mode_t, char *);
void display_dnode_simple(struct dnode *);
void display_dnode_long(struct dnode *);
struct dnode *parse_dnode(const char *, int);
int compare_dnode_name(const void *, const void *);
struct dnode **sort_dnode_entries(struct dnode *, size_t);
void display_dnode(struct dnode *);
void display_dnode_arr(struct dnode **, size_t);
void free_dnode(struct dnode *);
#endif
