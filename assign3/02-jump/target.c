#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

void win() {
  execve("/bin/sh", NULL, NULL);
}

int main() {
  setregid(getegid(), getegid());
  setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stdin, NULL, _IONBF, 0);
  win; // Suppress optimization

  char buf[100];
  printf("Give me somthing...\n");
  read(0, buf, 0x100);
}
