# 02-jump

## Goal
Understand how buffer overflow can change the control flow of the program.

## Mitigations
```
    Arch:     amd64-64-little
    RELRO:    Partial RELRO
    Stack:    No canary found
    NX:       NX enabled
    PIE:      No PIE (0x400000)
```

## Question
- Why `setregid` is required for solving this challenge?
