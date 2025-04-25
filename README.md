# EE309: Advanced Programming Techniques for EE – Assignments

This repository contains my solutions for the EE309 course assignments at KAIST. Each assignment focuses on different aspects of system-level programming in C, emphasizing Unix/Linux internals and network programming.

## Assignments Overview

### Assignment 1: Directory Listing Program (`ls309`)
- **Description**: Implemented a simplified version of the Unix `ls` command supporting options `-a`, `-l`, and `-R`.
- **Key Features**:
  - Handles multiple file and directory inputs.
  - Supports hidden files display with `-a`.
  - Provides detailed file information with `-l`.
  - Recursively lists directories with `-R`.
- **Assignment Details**: [Assignment 1 Description](https://teemo.kaist.ac.kr/ee309/2024/assignments/assignment1/)

### Assignment 2: Memory Management
- **Description**: Developed a custom dynamic memory allocator mimicking `malloc` and `free`.
- **Key Features**:
  - Implemented explicit free lists.
  - Utilized segregated free lists for efficient allocation.
  - Ensured coalescing of free blocks to minimize fragmentation.
- **Assignment Details**: [Assignment 2 Description](https://teemo.kaist.ac.kr/ee309/2024/assignments/assignment2/)

### Assignment 3: Buffer Overflow
- **Description**: Explored buffer overflow vulnerabilities and implemented protections.
- **Key Features**:
  - Demonstrated exploitation techniques.
  - Applied stack canaries and address space layout randomization (ASLR).
  - Analyzed the impact of buffer overflows on program control flow.
- **Assignment Details**: [Assignment 3 Description](https://teemo.kaist.ac.kr/ee309/2024/assignments/assignment3/)

### Assignment 4: HTTP Proxy
- **Description**: Built a concurrent HTTP proxy server handling multiple client requests.
- **Key Features**:
  - Parsed and forwarded HTTP/1.0 requests.
  - Managed concurrent connections using multithreading.
  - Implemented caching mechanisms for repeated requests.
- **Assignment Details**: [Assignment 4 Description](https://teemo.kaist.ac.kr/ee309/2024/assignments/assignment4/)

