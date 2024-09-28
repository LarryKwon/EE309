#ifndef _DIR_H
#define _DIR_H

#include <stddef.h>

struct dnode *parse_dir(const char *, size_t *);
void print_dir(const char *);
char *concat_path(const char *, const char *);
#endif
