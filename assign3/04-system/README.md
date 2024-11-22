# 04-system

## Goal
Understand ret2libc

## Mitigations
```
    Arch:     amd64-64-little
    RELRO:    Partial RELRO
    Stack:    No canary found
    NX:       NX enabled
    PIE:      No PIE (0x400000)
```

## Hint
- You can compute `libc_base` from a return address of main.
- NOTE: `__libc_start_main` is changed to `__libc_start_main_call`: https://sourceware.org/git/?p=glibc.git;a=commit;f=sysdeps/generic/libc_start_call_main.h;h=130fca173f323a24b41873b6656ab77c7cff86e1
- Use `pop rdi` in `__libc_csu_init` for putting shellcode.
- Remember that we need to make the stack 16-byte aligned for calling a function in x86_64.