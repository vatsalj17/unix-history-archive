# Minix1.1 Source Code Archive

## Historical Overview
- **Release Date**: January 10, 1987
- **Author**: Andrew S. Tanenbaum (Vrije Universiteit Amsterdam, Netherlands)
- **Publisher**: Prentice-Hall, Inc.
- **Target Architecture**: IBM PC, XT, and compatibles (Intel 8088 / 8086 processor, 640 KB RAM, real mode)
- **Historical Milestone**: The original, world-changing educational microkernel accompanying Andrew Tanenbaum's seminal textbook, *Operating Systems: Design and Implementation*. Because AT&T strictly prohibited the educational disclosure of Unix V6/V7 source code in universities, Tanenbaum undertook a total clean-room reimplementation of the Unix Seventh Edition (V7) system call interface from scratch—without looking at a single line of AT&T source code.

### The 4-Layer Microkernel Architecture
Minix 1.1 pioneered modular microkernel design on personal computers, dividing the operating system into four distinct isolation layers:
1. **Layer 1 (The Kernel)**: Executes in supervisor/hardware mode. Handles interrupts, CPU traps, low-level process scheduling, and pure message-passing inter-process communication (IPC) via `mini_send`, `mini_receive`, and `mini_both`.
2. **Layer 2 (I/O Tasks / Device Drivers)**: Device drivers running as separate, isolated processes with their own state: floppy disk (`floppy.c`), hard disk (`wini.c`), console/terminal (`tty.c`), system task (`system.c`), and clock (`clock.c`).
3. **Layer 3 (Server Processes)**: Core operating system intelligence running entirely as userspace server daemons:
   - **Memory Manager (`mm/`)**: Manages process memory allocation, segment limits, `fork()`, `exec()`, and signals.
   - **File System (`fs/`)**: Manages buffer caching, inodes, directory trees, pipes, and mounting.
4. **Layer 4 (User Applications)**: Standard userland utilities, the Bourne shell (`sh`), compilers, and editor (`mined`).

---

## Directory Tree & Navigation Map

```
Minix1.1/
├── etc/                         # System configuration files
│   ├── rc                       # System boot initialization script
│   ├── passwd                   # User password database
│   ├── ttys                     # Terminal line configuration
│   └── message                  # Message of the day login banner
├── user/                        # Userland documentation, extra utilities, and test suites
│   ├── doc/                     # Minix installation and operational documentation
│   ├── commands/                # Auxiliary user commands (mined editor, at, compress, de)
│   └── test/                    # Minix POSIX verification test suite (test1.c to test19.c)
└── usr/                         # Primary system tree
    ├── ast/                     # Secondary user profile for Andrew Tanenbaum
    ├── include/                 # Standard C library header files (stdio.h, signal.h, sgtty.h)
    ├── lib/                     # Low-level CRT runtime stubs (head.s, crtso.s, end.s)
    └── src/                     # Complete operating system source code
        ├── h/                   # Global operating system header definitions
        │   ├── callnr.h         # System call identification numbers
        │   ├── com.h            # Inter-task communication message structures
        │   ├── const.h          # Hardware constants (memory limits, NR_TASKS, NR_PROCS)
        │   ├── error.h          # POSIX / Minix error constants (EPERM, ENOENT, etc.)
        │   ├── stat.h           # Inode stat structure definition
        │   └── type.h           # Message formats, proc struct, and fundamental typedefs
        ├── include/             # System header files
        ├── kernel/              # Layer 1 & Layer 2: Microkernel Core and I/O Tasks
        │   ├── mpx88.s          # 8088 assembly interrupt vectors, task switching, save/restore
        │   ├── main.c           # System startup, task table setup, kernel process spawning
        │   ├── proc.c           # Message-passing IPC (mini_send, mini_rec) & scheduler
        │   ├── system.c         # System task (kernel memory copy, I/O port reading)
        │   ├── clock.c          # Programmable Interval Timer (8253 PIT) interrupt handler
        │   ├── memory.c         # RAM disk driver and physical memory task
        │   ├── tty.c            # Terminal keyboard input and display task
        │   ├── floppy.c         # NEC 765 floppy disk controller driver
        │   ├── wini.c           # Winchester hard disk driver (WD1002/WD1003 controllers)
        │   ├── printer.c        # Centronics parallel printer driver
        │   ├── C86/             # [AUX] Computer Innovations C86 build scripts & batch files
        │   ├── PCIX/            # [AUX] IBM PC/IX cross-development build scripts
        │   └── MINIX/           # Minix native build definitions
        ├── mm/                  # Layer 3: Memory Manager Server
        │   ├── main.c           # MM message dispatch loop (receives from Layer 4 procs)
        │   ├── alloc.c          # Physical memory management (hole list first-fit allocator)
        │   ├── forkexit.c       # Process creation (fork) and destruction (exit, wait)
        │   ├── break.c          # Process heap resizing (brk, sbrk)
        │   ├── exec.c           # Binary loader for Minix executable format
        │   ├── signal.c         # Signal delivery, sigaction, and alarm handling
        │   ├── table.c          # MM system call dispatch jump table
        │   ├── C86/, PCIX/, MINIX/ # Compiler configuration scripts
        ├── fs/                  # Layer 3: File System Server
        │   ├── main.c           # FS message loop (receives POSIX open, read, write)
        │   ├── buffer.c         # Block buffer cache (hash chains, LRU free list)
        │   ├── cache.c          # In-core inode table and block buffer caching
        │   ├── inode.c          # Inode allocation, deallocation, and disk synching
        │   ├── super.c          # Superblock loading, filesystem mounting (mount, umount)
        │   ├── open.c           # File creation, opening, and closing (open, creat, close)
        │   ├── read.c           # Block reading and zone indirection (read)
        │   ├── write.c          # Block writing, dynamic disk zone allocation (write)
        │   ├── link.c           # Hard link and directory unlinking (link, unlink)
        │   ├── mount.c          # Filesystem mounting mechanisms
        │   ├── path.c           # Path lookup and directory traversal (eat_path)
        │   ├── pipe.c           # Inter-process UNIX pipes
        │   ├── protect.c        # Permission checking (chmod, chown, access)
        │   └── C86/, PCIX/, MINIX/ # Compiler configuration scripts
        ├── commands/            # Layer 4: Standard Userland Command Suite
        │   ├── sh/              # The Minix Bourne Shell
        │   └── ...              # 60 standard commands (cat, cp, ls, grep, make, ed, etc.)
        ├── lib/                 # Standard C Runtime Library implementation
        │   ├── call.s           # Assembly system call stubs (send_rec to MM or FS)
        │   └── ...              # Standard C library functions
        └── tools/               # Installation and media formatting tools
            ├── build.c          # Bootsystem packager (merges kernel, MM, FS, init into boot image)
            ├── fsck.c           # Filesystem consistency check and repair
            └── mkfs.c           # Minix filesystem creator
```

---

## Key Source Code Files

| Source File | Functionality / Significance |
| :--- | :--- |
| `usr/src/h/com.h` | Defines the fundamental Minix message structures (`mess_1` through `mess_6`). All IPC in Minix relies on passing these 24-byte message unions across address spaces. |
| `usr/src/kernel/proc.c` | Implements `sys_call()`, `mini_send()`, and `mini_rec()`. Handles rendezvous-style synchronous message delivery between processes and schedules tasks across 3 priority queues (Tasks, Servers, User processes). |
| `usr/src/kernel/mpx88.s` | Low-level 8088 assembly code. Manages the 8259 PIC interrupt vectors, saves CPU registers onto the process stack, and performs context switches when transitioning between tasks and servers. |
| `usr/src/kernel/main.c` | Microkernel initialization. Constructs the process table (`proc[]`), spawns the kernel tasks (Clock, Disk, TTY), and starts the initial server processes (Memory Manager and File System). |
| `usr/src/mm/main.c` | Memory Manager server loop. Blocks in `receive()`, unmarshals requests from user processes, dispatches them through `call_vector[]` (handling `fork`, `exec`, `wait`, `brk`), and replies to callers. |
| `usr/src/mm/alloc.c` | 8086 real-mode memory manager. Maintains a linked list of free memory holes and allocates contiguous base/limit segment chunks using a first-fit algorithm. |
| `usr/src/fs/main.c` | File System server loop. Receives file requests from user programs, routes them to filesystem routines (`do_open`, `do_read`, `do_write`), and sends replies. |
| `usr/src/fs/buffer.c` | Minix block buffer cache. Manages disk block buffers using hash buckets and an LRU (Least Recently Used) list to minimize mechanical floppy and hard disk seeks. |
| `usr/src/fs/path.c` | Implements `eat_path()`, the Minix equivalent to Unix `namei()`. Parses slash-delimited directory paths component by component to locate the target inode. |
| `usr/src/tools/build.c` | Host utility that reads the separately compiled binary executable files for the Kernel, Memory Manager, File System, and Init, and stitches them into a contiguous bootable floppy disk image. |

---

## Recommended Exploration Path

1. **The Microkernel Message Passing Core**:
   - Begin at `usr/src/h/com.h` to inspect the fundamental 24-byte message structures.
   - Read `usr/src/kernel/proc.c` to see how `mini_send()` and `mini_rec()` implement synchronous message exchange.
   - Trace `usr/src/kernel/mpx88.s` to observe register saving and real-mode interrupt handling.
2. **Server Architecture (Layer 3)**:
   - Study `usr/src/mm/main.c` and `usr/src/mm/alloc.c` to see how a memory server operates completely independent of the kernel.
   - Study `usr/src/fs/main.c` and `usr/src/fs/buffer.c` to understand how the file system acts as an isolated RPC server receiving read/write requests.
3. **Device Drivers as Processes (Layer 2)**:
   - Inspect `usr/src/kernel/floppy.c` and `usr/src/kernel/tty.c` to see how hardware drivers run as distinct task loops waiting for IPC messages.
4. **System Bootstrap**:
   - Trace `usr/src/kernel/main.c` followed by `usr/src/tools/build.c` to see how the multi-binary microkernel image is assembled and launched.
