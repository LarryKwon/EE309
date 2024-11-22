#include <unistd.h>
#include <stdio.h>

int main(int argc) {
  setregid(getegid(), getegid());
  setvbuf(stdout, NULL, _IONBF, 0);
  setvbuf(stdin, NULL, _IONBF, 0);

  char buf[100];
  while (1) {
    printf("Give me something...\n");
    memset(buf, 0, sizeof(buf));
    read(0, buf, 0x100);
    printf("Your input: %s\n", buf);
    if (buf[0] == 'q')
      break;
  }
}