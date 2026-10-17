# Minix2.0 Source Code Archive

## Historical Overview
- **Release Date**: October 1996
- **Authors**: Andrew S. Tanenbaum, Kees J. Bot, and Philip Homburg
- **Publisher**: Prentice-Hall, Inc.
- **Target Architecture**: Intel 80386, 80486, and Pentium (32-bit protected mode native, with legacy 16-bit real-mode compatibility preserved)
- **Historical Milestone**: The companion operating system to the 2nd Edition of Tanenbaum and Woodhull's *Operating Systems: Design and Implementation* (1997). Minix 2.0 fully modernized the microkernel, achieving POSIX.1 conformance, native 32-bit flat memory addressing, integrated TCP/IP networking, and an advanced boot environment.

### Key Architectural Advances
- **Full POSIX.1 Conformance**: System call definitions, process semantics, job control, signals, and header files were overhauled to adhere strictly to IEEE POSIX 1003.1 standards.
- **Native 32-bit Flat Paging Memory**: Completely left behind the 64 KB segment limitations of the 16-bit 8086 architecture. Applications in Minix 2.0 could access full 32-bit linear address spaces with paging.
- **Integrated TCP/IP Networking (`src/inet/`)**: Engineered by Philip Homburg. Implemented as an independent network server process running alongside the Memory Manager and File System, providing IP, TCP, UDP, ICMP, and Ethernet drivers (`mnx_eth.c`).
- **Modular Boot Monitor (`src/boot/`)**: Created by Kees J. Bot. Provided a sophisticated interactive boot environment with scripting capabilities, kernel parameter overrides, multi-boot loading, and dynamic disk partitioning (`installboot`).
- **Sound & Multimedia**: Introduced audio driver support for the Sound Blaster 16 (`sb16_mixer.c`).
- **Complete Unix Manual Suite (`man/`)**: Distributed with a complete 9-section troff/nroff manual page suite covering commands, system calls, library functions, file formats, and architecture.

---

## Directory Tree & Navigation Map

```
Minix2.0/
├── include/                     # System header files
│   ├── minix/                   # Minix-specific kernel communication headers
│   │   ├── callnr.h             # POSIX and Minix system call numbers
│   │   ├── com.h                # Inter-process communication message definitions
│   │   ├── const.h              # Buffer sizes, process limits, memory layout
│   │   └── type.h               # Process table structures and message unions
│   ├── sys/                     # Standard POSIX headers (stat.h, wait.h, ioctl.h)
│   ├── net/                     # Networking headers (gen/in.h, gen/tcp.h, hton.h)
│   └── ibm/                     # PC architecture hardware headers (diskparm.h, partition.h)
├── man/                         # Comprehensive 9-Section Unix Manual Pages
│   ├── man1/                    # User commands (ls.1, sh.1, cc.1, mined.1)
│   ├── man2/                    # System calls (fork.2, read.2, write.2, time.2)
│   ├── man3/                    # C library routines (malloc.3, stdio.3)
│   ├── man4/                    # Device drivers (tty.4, sd.4, fd.4, ip.4)
│   ├── man5/                    # File formats (termcap.5, utmp.5, hosts.5)
│   ├── man6/                    # Games and amusements
│   ├── man7/                    # Miscellaneous conventions (environ.7, ascii.7)
│   ├── man8/                    # System administration tools (fsck.8, mkfs.8)
│   └── man9/                    # Commands and tools internal to Minix
└── src/                         # Complete Operating System Source Code
    ├── .ashrc                   # [AUX] Almquist shell initialization script
    ├── .ellepro.e               # [AUX] Elle editor initialization script
    ├── boot/                    # Kees J. Bot's Advanced Boot Monitor
    │   ├── bootimage.c          # Image unpacker and execution transfer
    │   ├── installboot.c        # Bootsector installer on floppy and hard disks
    │   ├── masterboot.s         # Master Boot Record (MBR) partition selector
    │   ├── extboot.s            # Extended partition bootloader
    │   └── boothead.s           # 16-bit to 32-bit transitional assembly entry
    ├── kernel/                  # Layer 1 & 2: 32-bit Microkernel Core & Drivers
    │   ├── mpx386.s             # 32-bit assembly interrupt dispatcher and context switch
    │   ├── main.c               # Kernel bootstrap and process initialization
    │   ├── proc.c               # Message passing IPC (mini_send, mini_rec) & scheduler
    │   ├── system.c             # System task service loop (memory copy, I/O ports)
    │   ├── protect.c            # 32-bit GDT/IDT protected mode segment setup
    │   ├── driver.c             # Unified driver support library (message handling)
    │   ├── clock.c              # Programmable timer and software timers
    │   ├── tty.c                # Terminal driver and line discipline
    │   ├── floppy.c             # Floppy disk driver
    │   ├── at_wini.c            # AT / IDE hard disk driver
    │   ├── bios_wini.c          # BIOS-assisted hard disk driver
    │   ├── sb16_mixer.c         # Sound Blaster 16 mixer and audio driver
    │   └── i8259.c              # PIC interrupt controller initialization
    ├── mm/                      # Layer 3: POSIX Memory Manager Server
    │   ├── main.c               # MM request loop
    │   ├── alloc.c              # Free memory hole list manager
    │   ├── forkexit.c           # POSIX fork, exit, waitpid implementation
    │   ├── break.c              # brk/sbrk dynamic heap allocation
    │   ├── exec.c               # 32-bit executable binary loader
    │   ├── signal.c             # POSIX sigaction, sigprocmask, and kill
    │   └── profile.c            # Kernel execution profiling hooks
    ├── fs/                      # Layer 3: POSIX File System Server
    │   ├── main.c               # FS request dispatch loop
    │   ├── cache.c              # Dynamic block buffer cache
    │   ├── inode.c              # Inode allocation and disk persistence
    │   ├── super.c              # Superblock management and mount points
    │   ├── open.c               # POSIX open, creat, close file descriptors
    │   ├── read.c               # File reading and direct/indirect block retrieval
    │   ├── write.c              # File writing and dynamic block allocation
    │   ├── select.c             # POSIX select() asynchronous I/O multiplexing
    │   ├── pipe.c               # FIFO pipe implementation
    │   └── path.c               # Path lookup and name resolution
    ├── inet/                    # Layer 3: TCP/IP Network Server (Philip Homburg)
    │   ├── inet.c               # Network server request loop
    │   ├── ip.c                 # Internet Protocol (IPv4) routing and fragmentation
    │   ├── tcp.c                # Transmission Control Protocol state machine
    │   ├── udp.c                # User Datagram Protocol handling
    │   ├── eth.c                # Ethernet generic frame abstraction
    │   ├── mnx_eth.c            # Minix native Ethernet device driver interface
    │   └── clock.c              # TCP retransmission timers
    ├── commands/                # Hundreds of POSIX userland commands
    ├── lib/                     # Standard C Library (ansi, posix, stdio, math, ip)
    └── etc/                     # System startup scripts and password files
```

---

## Key Source Code Files

| Source File | Functionality / Significance |
| :--- | :--- |
| `src/kernel/mpx386.s` | 32-bit protected-mode assembly backbone. Configures 32-bit interrupt handlers, intercepts hardware interrupts, saves general registers into `proc` structs, and switches CPU context. |
| `src/kernel/protect.c` | Manages 32-bit Intel GDT and IDT tables. Sets up flat 4 GB code and data segments for user processes and isolates the microkernel in ring 0. |
| `src/kernel/driver.c` | Standardized driver library written to allow Minix device drivers to be implemented uniformly as task loops responding to standard driver message opcodes. |
| `src/inet/inet.c` & `tcp.c` | Philip Homburg's network server. Demonstrates how a complete TCP/IP protocol stack operates as a clean user-level server in a microkernel architecture without compromising kernel stability. |
| `src/fs/select.c` | Implementation of POSIX `select()`. Bridges the File System server with device drivers (TTY, network) to support non-blocking asynchronous I/O multiplexing. |
| `src/boot/bootimage.c` | The core of Kees J. Bot's Boot Monitor. Reads Minix boot images, uncompresses binary segments, parses boot flags, and switches the CPU from 16-bit real mode to 32-bit protected mode before jumping to `mpx386.s`. |

---

## Recommended Exploration Path

1. **The 32-Bit Microkernel Architecture**:
   - Begin at `src/kernel/protect.c` to see how 32-bit flat memory segments are configured.
   - Inspect `src/kernel/mpx386.s` and `src/kernel/proc.c` to trace interrupt entry, message dispatch, and context switching.
2. **Network Server as an Isolated Process**:
   - Read `src/inet/inet.c` and `src/inet/tcp.c` to study how a full TCP/IP stack runs cleanly as a microkernel user-level daemon.
3. **Storage & POSIX VFS**:
   - Trace a file read request from `src/fs/main.c` -> `src/fs/open.c` -> `src/fs/read.c` -> `src/fs/cache.c`.
4. **Bootstrapping**:
   - Examine `src/boot/boothead.s` and `src/boot/bootimage.c` to observe how the boot monitor transitions from real mode to protected mode.
