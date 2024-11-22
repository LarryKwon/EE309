# 05-leak

## Goal
Understand leak & exploit

## Mitigations
```
    Arch:     amd64-64-little
    RELRO:    Partial RELRO
    Stack:    No canary found
    NX:       NX enabled
    PIE:      No PIE (0x400000)
```

## Hint
- Use puts to leak libc address, then call `system()`.