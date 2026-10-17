# Linux0.96c Source Code Archive

## Historical Overview
- **Release Date**: July 1992 (Version 0.96 patchlevel c)
- **Author / Community**: Linus Torvalds with an emerging global collective of early kernel contributors (Rémy Card, Ross Biro, Drew Eckhardt, Dirk Hohndel, Peter MacDonald, etc.)
- **Target Architecture**: Intel 80386 and 80486 (32-bit protected mode)
- **Historical Milestone**: The critical turning point transitioning Linux from a toy educational experiment into a capable, multi-subsystem production operating system. Between Linux 0.01 (September 1991) and the upcoming 1.0 (March 1994), 0.96c laid the foundations for modern Linux storage, networking, and virtual memory.

### Key Architectural Revolutions
- **The First Native Linux Filesystem (`fs/ext/`)**: Linux 0.01 was handcuffed to Andrew Tanenbaum's Minix filesystem (14-character filename limit, 64 MB partition limit). In 0.96c, French software engineer Rémy Card introduced the Extended Filesystem (`ext`), expanding filename limits to 255 characters and partition limits to 2 gigabytes.
- **The Birth of Linux Networking (`net/`)**: Introduced the primordial BSD socket abstraction layer written by Ross Biro, laying the bedrock for TCP/IP and Unix domain IPC (`AF_UNIX`).
- **Paging to Disk & Swapping (`mm/swap.c`, `mm/mmap.c`)**: Surpassed the physical memory bottleneck by introducing disk-backed swapping with bitmap allocation and early memory-mapped files (`mmap`).
- **SCSI Subsystem (`kernel/blk_drv/scsi/`)**: Hardware support expanded from standard IDE/AT disks to enterprise SCSI host adapters (Adaptec AHA-1542, Seagate ST01/ST02).
- **FPU Math Emulation (`kernel/math/`)**: Allowed Linux to run on cost-effective Intel 386SX/DX chips lacking an 80387 hardware math coprocessor.
- **Virtual Consoles (`kernel/chr_drv/vt.c`)**: Introduced Alt+Fn keyboard switching across multiple virtual teletype consoles.

---

## Directory Tree & Navigation Map

```
Linux0.96c/
├── Makefile                 # Top-level build configuration and compiler flags
├── makever.sh               # Shell script generating version definitions (.version)
├── boot/                    # System bootstrap and hardware probing
│   ├── bootsect.S           # Floppy bootsector loading setup and kernel
│   ├── setup.S              # BIOS query routines (display, memory, hard drive params)
│   └── head.s               # 32-bit protected mode setup and paging initialization
├── init/                    # System bootstrap sequence
│   └── main.c               # Kernel startup, subsystem init, root mounting, init spawn
├── kernel/                  # Monolithic kernel subsystems
│   ├── Makefile             # Subsystem build definition
│   ├── exit.c               # Process exit, wait4, zombie reclamation
│   ├── fork.c               # sys_fork, copy_process, VM copy
│   ├── irq.c                # Programmable Interrupt Controller (8259A PIC) dispatch
│   ├── itimer.c             # Interval timers (ITIMER_REAL, ITIMER_VIRTUAL, ITIMER_PROF)
│   ├── panic.c              # Kernel halt and emergency message printing
│   ├── printk.c             # Formatted console logger
│   ├── ptrace.c             # Process tracing and debugging support (for gdb)
│   ├── sched.c              # Priority round-robin scheduler and sleep queues
│   ├── signal.c             # POSIX signal delivery and stack frame restoration
│   ├── sys.c                # Core POSIX system calls
│   ├── sys_call.s           # int 0x80 assembly entry, register save/restore
│   ├── traps.c              # CPU fault traps and exception vectors
│   ├── vsprintf.c           # In-kernel string formatting
│   ├── blk_drv/             # Block device driver subsystem
│   │   ├── Makefile         # Block driver makefile
│   │   ├── blk.h            # Block driver macros and request queue structures
│   │   ├── floppy.c         # Floppy disk driver
│   │   ├── hd.c             # IDE / ST-506 hard disk controller driver
│   │   ├── ll_rw_blk.c      # Low-level request scheduling and sorting (elevator)
│   │   ├── ramdisk.c        # RAM disk driver
│   │   └── scsi/            # Primordial SCSI subsystem
│   │       ├── aha1542.c    # Adaptec AHA-1542 SCSI host adapter driver
│   │       ├── hosts.c      # SCSI host adapter registration table
│   │       ├── scsi.c       # Generic SCSI command midlayer
│   │       ├── sd.c         # SCSI hard disk driver
│   │       ├── seagate.c    # Seagate ST01/ST02 SCSI controller driver
│   │       ├── sr.c         # SCSI CD-ROM driver
│   │       └── st.c         # SCSI tape drive driver
│   ├── chr_drv/             # Character device driver subsystem
│   │   ├── Makefile         # Character driver makefile
│   │   ├── console.c        # VGA console rendering and escape sequences
│   │   ├── keyboard.c       # Keyboard translation and scancode processing
│   │   ├── mem.c            # /dev/null, /dev/mem, /dev/kmem, /dev/zero
│   │   ├── pty.c            # Pseudo-terminal master/slave pairs
│   │   ├── serial.c         # 8250/16450/16550 UART serial driver
│   │   ├── tty_io.c         # TTY queuing and line discipline dispatch
│   │   ├── tty_ioctl.c      # Terminal parameter manipulation
│   │   └── vt.c             # Virtual terminal multiplexer
│   └── math/                # 80387 Floating Point Unit software emulator
│       ├── Makefile         # Math emulator makefile
│       ├── add.c            # Floating point addition and subtraction
│       ├── convert.c        # Float/integer and IEEE-754 format conversions
│       ├── ea.c             # Effective address decoder for FPU opcodes
│       ├── emulate.c        # Trap 7 (device not available) dispatcher
│       └── mul.c            # Floating point multiplication and division
├── mm/                      # Virtual Memory subsystem
│   ├── Makefile             # Memory subsystem makefile
│   ├── memory.c             # Page table allocation, demand paging, COW faults
│   ├── mmap.c               # sys_mmap memory mapping implementation
│   └── swap.c               # Swap device management, swap_in, swap_out, page aging
├── fs/                      # Virtual Filesystem & filesystem drivers
│   ├── Makefile             # Filesystem makefile
│   ├── buffer.c             # Block buffer cache hash queues and async writing
│   ├── exec.c               # Binary loader (a.out executable support)
│   ├── fcntl.c              # File control system call
│   ├── file_table.c         # System-wide file structures
│   ├── inode.c              # Generic inode cache
│   ├── ioctl.c              # Device ioctl routing
│   ├── namei.c              # VFS pathname lookup and translation
│   ├── open.c               # sys_open, sys_close, descriptor table allocation
│   ├── pipe.c               # Named and anonymous FIFO pipes
│   ├── read_write.c         # Generic sys_read and sys_write entry points
│   ├── select.c             # sys_select synchronous I/O multiplexing
│   ├── stat.c               # Metadata query calls
│   ├── super.c              # Superblock registration and filesystem mounting
│   ├── ext/                 # The original Extended Filesystem (by Rémy Card)
│   │   ├── Makefile         # Ext build rules
│   │   ├── dir.c            # Ext directory scanning and entry creation
│   │   ├── file.c           # Regular file operations
│   │   ├── freespace.c      # Ext free block bitmap allocation
│   │   ├── inode.c          # Ext inode reading, writing, and block indexing
│   │   └── namei.c          # Ext directory entry lookup and unlinking
│   └── minix/               # Minix V1 Filesystem driver
│       ├── Makefile         # Minix build rules
│       ├── bitmap.c         # Minix inode/zone bitmaps
│       ├── dir.c            # Minix directory entry processing
│       ├── inode.c          # Minix inode manipulation
│       └── namei.c          # Minix name lookup
├── net/                     # Early BSD Socket networking subsystem
│   ├── Makefile             # Net build rules
│   ├── kern_sock.h          # Internal socket structure definitions
│   ├── socket.c             # BSD socket system call switchboard (sys_socketcall)
│   ├── socketcall.h         # Socket call sub-function numbers
│   └── unix.c               # AF_UNIX local IPC domain sockets
├── lib/                     # Minimal kernel helper and libc routines
│   ├── Makefile             # Library makefile
│   ├── malloc.c             # Dynamic kernel heap allocator
│   ├── string.c             # Optimized assembly string functions
│   └── itimer.c             # Library timer helpers
├── include/                 # Kernel header definitions
│   ├── asm/                 # Architecture specific headers (io.h, system.h, page.h)
│   ├── linux/               # Subsystem internal headers (fs.h, mm.h, sched.h, ext_fs.h)
│   └── sys/                 # POSIX compatible definitions
└── tools/                   # Host disk generation utility
    └── build.c              # Assembles raw bootable kernel floppy images
```

---

## Key Source Code Files

| Source File | Functionality / Significance |
| :--- | :--- |
| `fs/ext/inode.c` & `namei.c` | Rémy Card's first Extended Filesystem (`ext`). Provides the historical missing link between the primitive Minix filesystem and `ext2`/`ext3`/`ext4`. Overcame 14-char filename restrictions by utilizing variable-length directory entries. |
| `net/socket.c` | Written by Ross Biro. Implements `sys_socketcall()`, multiplexing `socket()`, `bind()`, `connect()`, `listen()`, `accept()`, `send()`, and `recv()`. The origin point of all Linux network computing. |
| `mm/swap.c` | The birth of swapping in Linux. Manages swap partitions and swap files using a page allocation bitmask, scanning page tables to evict dormant physical pages when memory pressure rises. |
| `mm/mmap.c` | Implements `sys_mmap()`, allowing user processes to map files directly into their virtual memory address space. |
| `kernel/blk_drv/scsi/hosts.c` | The original SCSI host adapter registry. Scans hardware ports for supported SCSI host bus adapters and initializes the SCSI request pipeline. |
| `kernel/chr_drv/vt.c` | Virtual terminal driver. Manages screen buffers for multiple independent virtual consoles, handling switching via keyboard hotkeys (`Alt+F1` through `Alt+F8`). |
| `kernel/math/emulate.c` | Software 80387 FPU emulator. Catches CPU trap 7 (coprocessor not present), parses math instructions, and executes software floating point routines. |
| `kernel/blk_drv/ll_rw_blk.c` | Low-level block request scheduler. Sorts disk read/write requests using a one-way elevator sorting algorithm to minimize disk head seek latency. |

---

## Recommended Exploration Path

1. **Evolution from 0.01 to 0.96c**:
   - Compare `fs/ext/inode.c` against `fs/minix/inode.c` to see how the Extended Filesystem restructured Unix disk layouts.
2. **Virtual Memory Expansion**:
   - Study `mm/swap.c` and `mm/mmap.c` to observe how demand paging transitioned into full virtual memory with secondary storage backing.
3. **Networking Foundations**:
   - Trace `net/socket.c` and follow how `sys_socketcall` handles `AF_UNIX` IPC in `net/unix.c`.
4. **Hardware Driver Modularization**:
   - Inspect `kernel/blk_drv/ll_rw_blk.c` to see how the block I/O layer decoupled filesystems from physical IDE and SCSI disk drivers.
   - Look inside `kernel/blk_drv/scsi/` to understand early enterprise storage adapters.
