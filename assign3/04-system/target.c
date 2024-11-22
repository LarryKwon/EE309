#include <unistd.h>
#include <stdio.h>

int main(int argc) {
  setregid(getegid(), getegid());
  setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stdin, NULL, _IONBF, 0);

  long long local;
  char buf[100];

  puts("Find the address of system(), here is your stack:");
  for (int i = 0; i < 0x10; i ++)
    printf("[%p] 0x%016llx\n", &local + i, *(&local + i));

  printf("Give me something...");
  read(0, buf, 0x100);
}
