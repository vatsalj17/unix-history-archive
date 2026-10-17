# Minix1.5 Source Code Archive

## Historical Overview
- **Release Date**: November 1989
- **Author / Community**: Andrew S. Tanenbaum with contributions from a worldwide academic community
- **Target Architectures**: IBM PC/XT/AT (Intel 8088, 80286, 80386 protected mode) and non-Intel architectures: Motorola 68000 (Atari ST, Commodore Amiga, Apple Macintosh) and SPARC (Sun SPARCstation)
- **Historical Milestone**: The transition of Minix from a 16-bit IBM PC-only educational project into a portable, multi-architecture operating system. Minix 1.5 added protected-mode support for Intel 286/386 CPUs, introduced network-oriented distributed systems code via Amoeba, adopted modern ANSI C library standards, and expanded its userland tools.

### Key Architectural Advances
- **Protected Mode & Memory Segmentation (`kernel/protect1.c`)**: Introduced Intel 80286/80386 Global Descriptor Table (GDT), Local Descriptor Table (LDT), and Interrupt Descriptor Table (IDT) configuration, enabling Minix to access extended memory beyond the 640 KB real-mode ceiling.
- **Amoeba Distributed System Support (`amoeba/`)**: Integrated experimental protocols and transactions from Tanenbaum's Amoeba distributed microkernel project (`amoeba/kernel/trans.c`), establishing distributed RPC client/server capabilities.
- **Elle Text Editor (`elle/`)**: Included "Elle Looks Like Emacs", a lightweight text editor engineered by Michael Plass, providing an Emacs-style editing environment on microcomputers.
- **Standardized C Libraries (`lib/`)**: Reorganized the C runtime library into standard compliance domains: `lib/ansi/` (ANSI C compliance), `lib/posix/` (POSIX.1 system call interface), `lib/ibm/` (PC hardware assembly stubs), and `lib/string/`.

---

## Directory Tree & Navigation Map

```
Minix1.5/
├── kernel/                      # Microkernel core & hardware drivers
│   ├── main.c                   # Microkernel initialization sequence
│   ├── proc.c                   # IPC message passing (mini_send, mini_rec) & task scheduler
│   ├── system.c                 # System task (kernel memory copying and privilege control)
│   ├── protect1.c               # 80286 / 80386 GDT, LDT, IDT descriptor tables & protected mode
│   ├── protect.h                # Segment descriptor register structures
│   ├── start.x                  # Low-level CPU startup and vector configuration
│   ├── clock.c                  # Programmable interval timer handling
│   ├── tty.c                    # Terminal console driver
│   ├── floppy.c                 # Floppy disk driver
│   ├── wini.c                   # Winchester hard disk driver
│   ├── i8259.c                  # Dual 8259 PIC cascaded interrupt controller driver
│   ├── dmp.c                    # Debugging routines (dumps process table, memory map to console)
│   └── table.c                  # Process table slot allocation
├── mm/                          # Memory Manager Server
│   ├── main.c                   # MM message processing loop
│   ├── alloc.c                  # Hole-list memory allocator for processes
│   ├── forkexit.c               # fork, exit, and wait system calls
│   ├── break.c                  # brk and sbrk dynamic heap management
│   ├── exec.c                   # Binary program loader
│   ├── signal.c                 # Signal dispatching, sigaction, and kill
│   └── getset.c                 # getuid, setuid, getgid, setgid management
├── fs/                          # File System Server
│   ├── main.c                   # FS request message dispatch loop
│   ├── cache.c                  # Buffer cache and block hashing
│   ├── inode.c                  # Inode allocation and disk persistence
│   ├── super.c                  # Superblock parsing and mount point binding
│   ├── open.c                   # open, creat, close file descriptors
│   ├── read.c                   # File read operations and indirect block traversal
│   ├── write.c                  # File write operations and zone allocation
│   ├── link.c                   # Link, unlink, rename directory entries
│   ├── mount.c                  # Filesystem mounting logic
│   └── pipe.c                   # IPC anonymous and named pipes
├── amoeba/                      # Amoeba Distributed Operating System Subsystem
│   ├── kernel/                  # Amoeba transaction and network message layer
│   │   ├── trans.c              # Transaction layer RPC implementation
│   │   ├── etherplus.c          # Ethernet packet driver for Amoeba
│   │   └── mpx.H                # Amoeba multiplexer definitions
│   ├── fs/                      # Amoeba distributed filesystem glue
│   ├── mm/                      # Amoeba distributed memory management extensions
│   ├── util/                    # Distributed utilities (rcp, from, sherver)
│   └── examples/                # Amoeba client/server demonstration code
├── commands/                    # 156 Userland commands and utilities
│   ├── mined/                   # Andy Tanenbaum's Minix text editor
│   ├── make/                    # Minix Make build tool
│   ├── patch/                   # Larry Wall's patch utility ported to Minix
│   ├── kermit/                  # Kermit serial file transfer program
│   ├── dis88/                   # 8088/8086 machine code disassembler
│   ├── bawk/                    # Minimal AWK pattern scanning interpreter
│   └── ...                      # Comprehensive POSIX command collection
├── lib/                         # Standard C Library implementation
│   ├── ansi/                    # ANSI C standard functions (stdio, stdlib, string)
│   ├── posix/                   # POSIX.1 system call wrappers
│   ├── ibm/                     # x86 PC architecture assembly stubs (portio.c, catchsig.s)
│   └── string/                  # Optimized string manipulation routines
├── elle/                        # Elle text editor
│   └── help.dat                 # Help and command reference documentation for Elle
├── tools/                       # System building and installation tools
│   ├── build.c                  # Microkernel binary image packager
│   ├── bootblok.s               # Primary bootsector assembly
│   ├── menu.c                   # Interactive bootloader menu
│   └── init.c                   # PID 1 init daemon source
├── test/                        # 29 system call regression tests (test1.c to test29.c)
└── include/                     # System header files (minix/, sys/, ANSI headers)
```

---

## Key Source Code Files

| Source File | Functionality / Significance |
| :--- | :--- |
| `kernel/protect1.c` | First protected mode implementation in Minix. Sets up the 80286/80386 GDT and LDT segment descriptors, switching the processor into protected mode and expanding addressing beyond real-mode 1 MB limits. |
| `kernel/proc.c` | Microkernel scheduler and message passing core. Implements synchronous message rendezvous via `mini_send()` and `mini_rec()` with task preemption. |
| `amoeba/kernel/trans.c` | Implements the Amoeba transaction layer protocol. Enables transparent network RPC across network nodes using capability-based ports and client/server transaction primitives. |
| `commands/mined/mined1.c` | Andrew Tanenbaum's "Mined" (Minix Editor). A full-screen visual text editor designed to be simple, fast, and educational for undergraduate students. |
| `commands/dis88/dismain.c` | An in-tree 8088/8086 disassembler written for Minix, allowing programmers to inspect compiled system binaries directly on the platform. |
| `tools/menu.c` | The first interactive boot menu for Minix, allowing users to select root devices, RAM disk sizes, and processor modes during startup. |

---

## Recommended Exploration Path

1. **Protected Mode Evolution**:
   - Compare `kernel/protect1.c` with the 8086 real-mode code from Minix 1.1 (`kernel/mpx88.s`) to see how Minix adopted descriptor-based 286/386 segmentation.
2. **Distributed Microkernel Experimentation**:
   - Inspect `amoeba/kernel/trans.c` to see how Tanenbaum integrated network RPC primitives directly into the microkernel message model.
3. **Userland Tools**:
   - Study `commands/mined/` to see how an elegant screen editor is implemented in standard C on a microkernel.
   - Trace the bootstrap flow through `tools/bootblok.s` -> `tools/menu.c` -> `kernel/main.c`.
