# Linux-0.01 Source Code Archive

## Historical Overview
- **Release Date**: September 17, 1991
- **Author**: Linus Benedict Torvalds (University of Helsinki, Finland)
- **Target Architecture**: Intel 80386 (strictly 32-bit x86 protected mode)
- **Historical Milestone**: The primordial first public release of the Linux kernel. Comprising only 88 files and ~10,239 lines of code, Linux 0.01 represents Linus Torvalds' initial experimental monolithic Unix kernel. Written out of a desire to understand the hardware capabilities of the Intel 80386 (specifically task switching and paging) and motivated by dissatisfaction with Minix's educational licensing and terminal emulation limitations, Linux 0.01 could compile itself under Minix using GCC and execute GNU Bash.

### Architectural Innovations & Limitations
- **TSS Hardware Task Switching**: Unlike modern Linux, 0.01 directly leveraged the 80386 hardware Task State Segment (TSS) for context switching via far jumps (`ljmp`).
- **Memory Management Ceiling**: Memory management was hard-coded to a maximum of 16 MB RAM, configured using demand paging without virtual memory swap.
- **Filesystem**: Implemented a clean-room driver for Andrew Tanenbaum's Minix V1 filesystem (14-character filename limit, 32-bit blocks).
- **Process Model**: Primitive implementation of `fork()`, `execve()`, and `exit()` with process 0 hardcoded as the idle task and process 1 executing `/bin/sh`.

---

## Directory Tree & Navigation Map

```
Linux-0.01/
├── Makefile             # Root kernel build orchestration file
├── boot/                # 80386 bootstrap assembly code
│   ├── boot.s           # 512-byte Master Boot Record sector (loaded by BIOS at 0x7C00)
│   └── head.s           # 32-bit entry point; establishes GDT, IDT, paging, and jumps to main()
├── init/                # Kernel initialization
│   └── main.c           # main() initialization sequence and idle task loop
├── kernel/              # Monolithic kernel subsystems
│   ├── Makefile         # Subsystem build definitions
│   ├── asm.s            # Low-level CPU exception trap stubs
│   ├── console.c        # VGA console output driver
│   ├── exit.c           # sys_exit, sys_waitpid, process termination
│   ├── fork.c           # sys_fork, copy_process, TSS setup
│   ├── hd.c             # AT hard disk driver (IDE controller PIO mode)
│   ├── keyboard.s       # PC keyboard interrupt handler and scancode translation
│   ├── panic.c          # Kernel panic halt routine
│   ├── printk.c         # Kernel log printing routine
│   ├── rs_io.s          # Serial RS-232 interrupt handlers
│   ├── sched.c          # Preemptive timer scheduler, sleep/wake, timer ticks
│   ├── serial.c         # Serial line initialization and control
│   ├── sys.c            # Miscellaneous POSIX system calls
│   ├── system_call.s    # int 0x80 assembly dispatch and return path
│   ├── traps.c          # CPU exception handlers (GPF, page faults, divide error)
│   ├── tty_io.c         # TTY line discipline and buffer queue handling
│   └── vsprintf.c       # String formatting engine for printk
├── mm/                  # Memory Management subsystem
│   ├── Makefile         # Memory subsystem build rules
│   ├── memory.c         # Demand paging, page table manipulation, copy-on-write
│   └── page.s           # Low-level page fault interrupt entry stub
├── fs/                  # Minix V1 Filesystem implementation
│   ├── Makefile         # Filesystem build rules
│   ├── bitmap.c         # Inode and block allocation bitmaps
│   ├── block_dev.c      # Block device read/write abstractions
│   ├── buffer.c         # Buffer cache, hash queues, LRU list, bread/bwrite
│   ├── char_dev.c       # Character device multiplexing
│   ├── exec.c           # Binary execution loader for a.out binaries
│   ├── fcntl.c          # File control system call handling
│   ├── file_dev.c       # Regular file read/write operations
│   ├── file_table.c     # Global open file table allocation
│   ├── inode.c          # Inode cache management (iget, iput)
│   ├── ioctl.c          # I/O control dispatcher
│   ├── namei.c          # Path resolution and directory parsing
│   ├── open.c           # sys_open, sys_creat, sys_close
│   ├── pipe.c           # Inter-process pipe implementation
│   ├── read_write.c     # sys_read, sys_write generic dispatchers
│   ├── stat.c           # File metadata query routines
│   ├── super.c          # Superblock management and root filesystem mounting
│   ├── truncate.c       # File truncation and block freeing
│   └── tty_ioctl.c      # Terminal terminal control ioctl handler
├── lib/                 # Minimal kernel-space standard C library stubs
│   ├── Makefile         # Library build rules
│   ├── _exit.c          # _exit system call wrapper
│   ├── close.c          # close system call wrapper
│   ├── ctype.c          # Character classification table
│   ├── dup.c            # dup system call wrapper
│   ├── errno.c          # Global errno variable
│   ├── execve.c         # execve system call wrapper
│   ├── open.c           # open system call wrapper
│   ├── setsid.c         # setsid system call wrapper
│   ├── string.c         # Inlined assembly string functions (strcpy, strcmp, etc.)
│   ├── wait.c           # waitpid system call wrapper
│   └── write.c          # write system call wrapper
├── include/             # System and kernel header files
│   ├── a.out.h          # a.out executable binary format definitions
│   ├── const.h          # System constants
│   ├── ctype.h          # Character classification prototypes
│   ├── errno.h          # POSIX error codes
│   ├── fcntl.h          # File control flags
│   ├── signal.h         # Signal numbers and sigaction structures
│   ├── stdarg.h         # Variable argument macros
│   ├── stddef.h         # Standard definitions (NULL, offsetof)
│   ├── string.h         # String manipulation prototypes
│   ├── termios.h        # Terminal I/O interface structures
│   ├── time.h           # Time conversion prototypes
│   ├── unistd.h         # Standard system call numbers and prototypes
│   ├── asm/             # Architecture-dependent assembly headers
│   │   ├── io.h         # inb/outb port I/O macros
│   │   ├── memory.h     # memcpy macros
│   │   ├── segment.h    # get_fs_byte / put_fs_byte segment register access
│   │   └── system.h     # cli, sti, iret, descriptor table configuration
│   ├── linux/           # Core Linux kernel internal headers
│   │   ├── config.h     # Hardcoded hardware parameters (HD types, clock)
│   │   ├── fs.h         # Inode, buffer head, and super block structures
│   │   ├── hdreg.h      # Hard disk controller registers
│   │   ├── head.h       # Page directory and GDT layout
│   │   ├── kernel.h     # printk and utility declarations
│   │   ├── mm.h         # Memory management page definitions
│   │   ├── sched.h      # Task struct, scheduler macros, TSS definition
│   │   ├── sys.h        # System call dispatch table array
│   │   └── tty.h        # TTY queue and mode flags
│   └── sys/             # POSIX compatibility headers
│       ├── stat.h       # File stat structure
│       ├── times.h      # Process timing structures
│       ├── types.h      # Fundamental typedefs (pid_t, uid_t, size_t)
│       ├── utsname.h    # uname system identification
│       └── wait.h       # Process wait status macros
└── tools/               # Host build utilities
    └── build.c          # Host C program to splice bootsect, setup, and system into a floppy image
```

---

## Key Source Code Files

| Source File | Functionality / Significance |
| :--- | :--- |
| `boot/boot.s` | The 512-byte floppy bootsector written in 8086 assembly. It reads `setup.s` and the kernel `system` image from disk sectors into memory using BIOS `int 0x13`, relocates itself to `0x90000`, and transfers execution. |
| `boot/head.s` | 32-bit startup code. Sets up the initial 256 interrupt descriptors in the IDT, establishes a temporary GDT, maps the first 8 MB of RAM with identity page tables, and enters protected mode by executing a far jump to `_main`. |
| `init/main.c` | Entry point `main()`. Calculates memory limits, initializes interrupts (`trap_init`), serial ports (`tty_init`), hard disk (`hd_init`), buffer cache (`buffer_init`), and the timer/scheduler (`sched_init`). Transitions from supervisor mode (ring 0) to user mode (ring 3) via `move_to_user_mode()` and forks the first user shell (`/bin/sh`). |
| `kernel/sched.c` | Implements the primordial scheduler (`schedule()`). Loops through the static `task[]` array (maximum 64 processes), evaluates remaining runtime `counter`, applies dynamic priority boosting, and switches context using the inline macro `switch_to(next)` via 386 hardware TSS task switching. |
| `kernel/fork.c` | Implements `sys_fork()` and `copy_process()`. Clones the task structure, allocates a new TSS and LDT, duplicates memory pages using copy-on-write (`copy_page_tables`), and sets process state to `TASK_RUNNING`. |
| `kernel/system_call.s` | Core assembly entry for `int 0x80`. Saves registers, validates the system call number against `sys_call_table`, jumps to the C handler, executes `do_signal()`, and restores user registers. |
| `mm/memory.c` | Physical and virtual memory manager. Manages a byte array `mem_map[]` indicating page reference counts. Handles demand zero-fill (`do_no_page()`) and copy-on-write page faults (`do_wp_page()`). Memory is strictly hardcoded to a maximum of 16 MB. |
| `fs/buffer.c` | High-performance block buffer cache. Maintains a 307-entry hash queue and a doubly linked LRU free list. Implements `getblk()`, `bread()`, `bwrite()`, and `brelse()` for disk blocks. |
| `fs/namei.c` | Directory resolution engine (`namei()`). Traverses directory inodes sequentially to resolve Unix file paths to target in-memory `struct m_inode`. |
| `fs/exec.c` | Binary program loader `do_execve()`. Parses the `a.out` header, validates magic numbers (`0407`, `0410`), releases the old address space, and sets up the new user stack with environment variables and command-line arguments. |

---

## Recommended Exploration Path

To understand the architecture of Linux 0.01, follow this linear reading sequence:

1. **Bootstrap & Transition to 32-bit Protected Mode**:
   - Start with `boot/boot.s` to see how the BIOS transfers execution to real-mode code.
   - Read `boot/head.s` to see how GDT/IDT descriptors are constructed and how paging is enabled on the 386.
2. **Subsystem Initialization**:
   - Open `init/main.c`. Trace `main()` as it initializes drivers, establishes the buffer cache, and uses the stack-popping trick `move_to_user_mode()` to drop into ring 3.
3. **Interrupts & System Calls**:
   - Study `kernel/traps.c` and `kernel/asm.s` for CPU exception handling.
   - Inspect `kernel/system_call.s` to see how userland enters the kernel via `int 0x80`.
4. **Multitasking & Memory Management**:
   - Examine `kernel/sched.c` to understand the primitive `counter` and `priority` scheduling algorithm.
   - Review `kernel/fork.c` to see how process state and task structures are duplicated.
   - Read `mm/memory.c` and `mm/page.s` to study demand paging and copy-on-write fault handling.
5. **Storage & Filesystem**:
   - Read `fs/buffer.c` to master the buffer cache hash tables.
   - Read `fs/super.c`, `fs/inode.c`, and `fs/namei.c` to trace how a path like `/bin/sh` is resolved to disk blocks.
