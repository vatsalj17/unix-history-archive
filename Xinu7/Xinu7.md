# Xinu7 Source Code Archive

## Historical Overview
- **Release Date**: January 1987 (Xinu Version 7)
- **Author**: Professor Douglas Comer (Purdue University, West Lafayette, Indiana)
- **Target Architecture**: Digital Equipment Corporation (DEC) PDP-11 and LSI-11 microcomputers (cross-compiled from a host VAX running Unix)
- **Historical Milestone**: The release accompanying Douglas Comer's celebrated textbook, *Operating System Design Vol II: Internetworking with Xinu* (Prentice-Hall, 1987). "Xinu Is Not Unix" (a recursive acronym) was engineered to strip away the obfuscating complexity of commercial Unix kernels, offering students and systems programmers an elegant, completely transparent, layered operating system that demonstrated the inner workings of multitasking, device independence, and a full TCP/IP Internet protocol suite.

### Key Architectural Advances
- **Layered Elegance**: Constructed in strict hierarchical software layers: Process Management -> Memory Management -> Inter-Process Communication -> Device Independent I/O -> Network Protocol Suite -> Distributed Remote File System.
- **Complete In-Tree TCP/IP Stack**: One of the earliest and cleanest open implementations of internetworking protocols in C:
  - **Network Layer**: Internet Protocol (IPv4) with fragmentation/reassembly (`ip_in.c`, `ip_out.c`), ICMP error/echo handling, and ARP table resolution (`arp_in.c`).
  - **Transport Layer**: User Datagram Protocol (UDP) and a full Transmission Control Protocol (TCP) finite state machine (`tcp_in.c`, `tcp_out.c`).
  - **Routing & Resolution**: Routing Information Protocol (RIP) daemon and Domain Name System (DNS) name resolver.
- **Device-Independent I/O Switch**: Centralized device configuration table (`src/sys/sys/conf.c`) routing `read()`, `write()`, `open()`, and `control()` through unified function pointers.
- **Distributed Remote Filesystem (RFS)**: Because LSI-11 lab computers frequently lacked hard drives, Xinu implemented a network client (`src/sys/sys/rfopen.c`, `rfread.c`) communicating over Ethernet with a background Unix server daemon (`src/serve11/`) providing virtual disk access.

---

## Directory Tree & Navigation Map

```
Xinu7/
├── README                       # Douglas Comer's original distribution release notes
├── bin/                         # Host-compiled cross-development binaries
│   ├── lorder11                 # PDP-11 object file ordering utility
│   ├── cprint                   # C source code pretty-printer for textbook listing
│   └── cvt                      # A.out binary format converter
├── include/                     # Host cross-compiler header files
│   └── pdp11/                   # PDP-11 memory geometry (a.out.h, pagsiz.h)
├── install/                     # Cross-compilation environment setup
│   ├── Install                  # Shell installer script
│   ├── .install                 # [AUX] Hidden installer helper script
│   ├── sample.output            # Reference installation log
│   └── README                   # Installation guide
├── lib/                         # Host macro formatting libraries
│   ├── tmac.an, tmac.ad         # Troff macro definitions for manual printing
│   └── machines                 # Target machine definitions table
├── storage/                     # Sample execution captures
│   └── output/                  # Captured terminal output logs (netstat, routes, who)
├── man/                         # Comprehensive Troff Manual Pages
│   ├── man0/                    # Introductory notes and manual index
│   ├── man1/                    # Cross commands (cc11, as11, download, odt)
│   ├── man2/                    # Xinu system calls (create, resume, send, receive, wait)
│   ├── man3/                    # Standalone C library routines
│   └── man4/                    # Device drivers (ether.4, tty.4, rfilsys.4)
├── contrib/                     # Contributed network monitoring and debugging tools
│   ├── distodt/                 # Distributed Optical Diagnostic Terminal (remote console)
│   └── ether.monitor/           # Standalone Ethernet packet analyzer / sniffer
└── src/                         # Complete Operating System Source Code
    ├── sys/                     # The Xinu Kernel & Network Subsystem
    │   ├── con/                 # Device configuration compiler
    │   │   ├── config.c         # Parser that reads Configuration file
    │   │   └── Configuration    # Device definitions (TTY, Ethernet, Disk, Remote FS)
    │   ├── h/                   # Kernel header files
    │   │   ├── kernel.h         # Core types, process state constants, semaphore structs
    │   │   ├── proc.h           # Process table entry definition (pentry)
    │   │   ├── sem.h            # Semaphore table entry definition (sentry)
    │   │   ├── ip.h, tcp.h      # IP, TCP, UDP packet header structures
    │   │   ├── ether.h          # Ethernet frame headers and DEQNA controller registers
    │   │   └── conf.h           # Generated device switch table macros
    │   └── sys/                 # Core Kernel & TCP/IP Engine (231 files)
    │       ├── initialize.c     # sysinit() - hardware initialization and null process
    │       ├── resched.c        # Preemptive priority scheduler
    │       ├── ctxsw.s          # Assembly context switch (saves R0-R5, SP, PC, PS)
    │       ├── clkint.s         # Clock interrupt service routine
    │       ├── create.c         # Process creation and stack initialization
    │       ├── ready.c          # Places process into ready list (readylist)
    │       ├── kill.c           # Process termination and resource release
    │       ├── wait.c, signal.c # Counting semaphore synchronization
    │       ├── ip_in.c, ip_out.c # IP packet reception, forwarding, and transmission
    │       ├── tcp_in.c         # TCP packet input and state machine processing
    │       ├── udp_in.c         # UDP packet demultiplexing
    │       ├── arp_in.c         # ARP packet handling and IP-to-Ethernet cache
    │       ├── rfs.c            # Remote filesystem client protocol
    │       └── ...              # Scratch scripts (,lint, ,sizes, Checkup, etc.)
    ├── serve11/                 # Host Unix Server Software (Runs on VAX/Unix)
    │   ├── sys/                 # Server daemon listening for LSI-11 Ethernet requests
    │   ├── con/                 # Host configuration tools
    │   └── h/                   # Host server headers
    ├── cmd/                     # Cross-Development Toolchain (Runs on VAX)
    │   ├── cc11/                # PDP-11 C compiler front-end
    │   ├── as11/                # PDP-11 cross-assembler
    │   │   └── orig/            # Patch backup files (.orig)
    │   ├── cpp11/               # PDP-11 C preprocessor
    │   ├── download/            # LSI-11 serial line bootstrap downloader
    │   ├── fs/                  # Host filesystem daemon
    │   └── pm/                  # Post-mortem memory crash analyzer
    └── lib/                     # Standalone PDP-11 Runtime Libraries
        ├── libxc/               # Standalone C runtime library
        ├── libeis/              # PDP-11 Extended Instruction Set emulation
        └── standalone/          # Standalone boot helpers
```

---

## Key Source Code Files

| Source File | Functionality / Significance |
| :--- | :--- |
| `src/sys/sys/initialize.c` | Entry point `sysinit()`. Sets up the interrupt vector table, initializes process tables, semaphores, memory free lists, and device drivers, spawning the idle null process (`pid 0`). |
| `src/sys/sys/resched.c` | Core preemptive scheduler. Checks the highest-priority runnable process in the delta list `readylist`. If different from the currently running process, calls assembly context switch `ctxsw()`. |
| `src/sys/sys/ctxsw.s` | PDP-11 assembly context switch. Saves registers `r0` through `r5`, the processor status word (`PS`), and stack pointer (`SP`), swaps stack pointers to the new process, and restores registers. |
| `src/sys/sys/create.c` | Process creation routine. Allocates a stack frame from heap memory, mimics an interrupt stack frame, initializes saved registers, and inserts the process into `proctab[]`. |
| `src/sys/sys/wait.c` & `signal.c` | Classic Dijkstra counting semaphore primitives. Decrements/increments semaphore counts, placing waiting processes into suspension queues or waking blocked processes. |
| `src/sys/sys/ip_in.c` | Internet Protocol input processor. Validates IP header checksums, handles fragmentation reassembly, checks TTL expiration, and demultiplexes packets to UDP, TCP, or ICMP. |
| `src/sys/sys/tcp_in.c` | TCP finite state machine. Processes SYN, ACK, FIN, and RST flags, enforces window flow control, manages sequence numbers, and triggers connection state transitions. |
| `src/sys/con/config.c` | The device configuration generator. Compiles the declarative `Configuration` file into static C device switch tables (`conf.c`, `conf.h`), implementing device independence. |
| `src/serve11/sys/serve11.c` | Host Unix daemon. Listens on UDP/Ethernet sockets for requests from remote Xinu microcomputers, translating remote file accesses into local Unix file system system calls. |

---

## Recommended Exploration Path

1. **Process Management & Scheduling**:
   - Start with `src/sys/h/kernel.h` and `src/sys/h/proc.h` to examine the process table structure.
   - Trace `src/sys/sys/initialize.c` to understand how Xinu boots.
   - Follow into `src/sys/sys/resched.c` and `src/sys/sys/ctxsw.s` to see the complete, minimal context switch.
2. **Synchronization & Communication**:
   - Read `src/sys/sys/wait.c` and `src/sys/sys/signal.c` for semaphore synchronization.
   - Study `src/sys/sys/send.c` and `src/sys/sys/receive.c` for message passing.
3. **Device Independence**:
   - Examine `src/sys/con/Configuration` and see how `src/sys/con/config.c` converts declarative driver lists into `conf.c`.
4. **Internetworking (TCP/IP)**:
   - Follow an incoming Ethernet packet: `src/sys/sys/ethintr.c` -> `src/sys/sys/arp_in.c` -> `src/sys/sys/ip_in.c` -> `src/sys/sys/tcp_in.c`.
5. **Distributed Remote Filesystem**:
   - Inspect `src/sys/sys/rfs.c` and contrast it with host server `src/serve11/sys/serve11.c`.
