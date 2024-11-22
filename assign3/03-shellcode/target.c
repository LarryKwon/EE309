#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/personality.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>

#define MAP_ADDR 0x5fff00000000

void copy_envp(char **envp) {
  size_t len = 0;

  // Compute the length of the environment variables
  for (char **env = envp; *env != NULL; env++) {
    len += strlen(*env) + 1;
  }

  // Round up to the nearest page
  len = (len/0x1000 + 1) * 0x1000;

  // Allocate a buffer to hold the environment variables
  char *buf = mmap((void *)MAP_ADDR, len, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (buf == MAP_FAILED) {
    printf("mmap failed: %s\n", strerror(errno));
    exit(1);
  }

  // Copy the environment variables into the buffer
  char *ptr = buf;
  for (char **env = envp; *env != NULL; env++) {
    strcpy(ptr, *env);
    ptr += strlen(*env) + 1;
  }
}

int main(int argc, char *argv[], char *envp[]) {
  setregid(getegid(), getegid());
  setvbuf(stdout, NULL, _IONBF, 0);
  setvbuf(stdin, NULL, _IONBF, 0);

  char buf[100];

  copy_envp(envp);

  printf("Give me something...\n");
  read(0, buf, 0x100);
}
