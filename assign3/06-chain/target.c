#include <unistd.h>
#include <stdio.h>
#include <err.h>
#include <seccomp.h>
#include <stdlib.h>
#include <string.h>

void setup_rules()
{
    // Init the filter
    scmp_filter_ctx ctx;
    ctx = seccomp_init(SCMP_ACT_KILL); // default action: kill
    if (ctx == NULL)
        exit(-1);

    // setup basic whitelist
    if (seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(rt_sigreturn), 0) < 0)
        exit(-1);
    if (seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(exit), 0) < 0)
        exit(-1);
    if (seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(exit_group), 0) < 0)
        exit(-1);
    if (seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(write), 0) < 0)
        exit(-1);
    if (seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(read), 0) < 0)
        exit(-1);
    if (seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(open), 0) < 0)
        exit(-1);
    if (seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(openat), 0) < 0)
        exit(-1);
    if (seccomp_load(ctx) < 0)
        exit(-1);
}

void vuln() {
  char buf[100];
  printf("Give me something...\n");
  read(0, buf, 0x100);
}

int main(int argc) {
  setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stdin, NULL, _IONBF, 0);
  setup_rules();

  vuln();
}
