# 01-basic

## Goal
Understand buffer overflow and endianness.

## Mitigations
```
    Arch:     amd64-64-little
    RELRO:    Partial RELRO
    Stack:    No canary found
    NX:       NX enabled
    PIE:      No PIE (0x400000)
```

## Hint
- Please refer to the `template.py`.
- https://docs.pwntools.com/en/stable/util/packing.html#pwnlib.util.packing.p64