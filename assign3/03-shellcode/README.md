# 03-shellcode

## Goal
Understand shellcode and how to use it.

## Mitigations
```
    Arch:     amd64-64-little
    RELRO:    Full RELRO
    Stack:    No canary found
    NX:       NX disabled
    PIE:      PIE enabled
    RWX:      Has RWX segments
```

## Question
- As you can see, if the binary is PIE enabled, it also enables RELRO by default. Why do developers make this choice?

## Hint
- In pwntools, shellcode could be achieved like this

```python
context.update(arch='amd64', os='linux')
shellcode = asm(shellcraft.sh())
```

- Use NOP sled to make the exploit more reliable.
- We disabled ASLR using personality syscall, so you can use the same address every time.
