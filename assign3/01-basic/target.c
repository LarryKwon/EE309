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

  char buf[100];
  int secret = 0xBAADF00D;
  printf("Give me somthing...\n");
  read(0, buf, 0x100);

  if (secret == 0xDEADBEEF) {
    win();
  }
}
