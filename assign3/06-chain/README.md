# 06-chain

## Goal
Understand return-oriented-programming (ROP)

## Mitigations
```
    Arch:     amd64-64-little
    RELRO:    Partial RELRO
    Stack:    No canary found
    NX:       NX enabled
    PIE:      No PIE (0x400000)
```

## Question
- Let's assume that we prevent from calling `read` using `seccomp` (using blacklist). Can you still read a flag? How?

## Hint
- This binary only allows you to use `open`, `read`, `write`. Thus, you need to make ROP chain for directly reading a flag.
