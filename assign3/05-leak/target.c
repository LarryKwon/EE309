#include <unistd.h>
#include <stdio.h>

int main(int argc) {
  setregid(getegid(), getegid());
  setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stdin, NULL, _IONBF, 0);

  char buf[100];
  printf("Give me something...\n");
  read(0, buf, 0x100);
}
