# NetBSD-5.0.2 Source Code Archive

## Historical Overview
- **Release Date**: February 6, 2010 (NetBSD 5.0 initial release April 2009; 5.0.2 maintenance update)
- **Author / Organization**: The NetBSD Foundation, Inc.
- **Target Architectures**: Over 50 supported hardware architectures in `sys/arch/` (including `i386`, `amd64`, `sparc`, `sparc64`, `alpha`, `vax`, `hp300`, `macppc`, `amiga`, `atari`, `sgimips`, `dreamcast`, `zaurus`, and `xen`)
- **Historical Milestone**: The culmination of the massive NetBSD 5 kernel overhaul led by developer Andrew Doran. Embodying NetBSD's legendary motto, *"Of course it runs NetBSD"*, version 5.0.2 combined extreme multi-platform portability with an ultra-modern symmetric multiprocessing (SMP) kernel, lockless concurrency algorithms, 1:1 POSIX threading, and the groundbreaking RUMP anykernel architecture.

### Key Architectural Revolutions
- **SMP Multiprocessor Kernel Rewrite**: Andrew Doran re-engineered the kernel locking architecture from the ground up, implementing adaptive mutexes (`sys/kern/kern_mutex.c`), reader/writer locks (`kern_rwlock.c`), spin locks, lockless queues, and per-CPU memory pools.
- **1:1 Kernel Threading Model**: NetBSD 5 abandoned the complicated M:N Scheduler Activations threading model in favor of a clean, high-performance 1:1 Light Weight Process (LWP) design (`sys/kern/kern_lwp.c`), drastically improving POSIX thread performance.
- **RUMP (Run-time User-space Maintained Programs)**: Invented by Antti Kantee (`sys/rump/`). RUMP introduced the "anykernel" concept, allowing unmodified NetBSD kernel drivers, network stacks, and filesystems to be compiled and executed directly as userland libraries and microkernel servers.
- **UVM Virtual Memory Architecture**: Refined Dr. Charles Cranor's UVM virtual memory manager (`sys/uvm/`) with loaning page cache mechanisms, fine-grained locking, and anonymous memory map optimizations.
- **Universal Cross-Compilation (`build.sh`)**: The industry benchmark for OS build systems. `build.sh` enables anyone to cross-compile the entire NetBSD operating system, userland, and kernel for any of the 50+ target platforms from any Unix-like host (Linux, macOS, Solaris) without root privileges.
- **PUFFS (Pass-to-Userspace Framework for File Systems)**: Allowed userland filesystems to be written easily and integrated with the kernel VFS.

---

## Directory Tree & Navigation Map

```
NetBSD-5.0.2/
├── BUILDING                     # Comprehensive build instructions for build.sh
├── Makefile, Makefile.inc       # Top-level makefile wrappers
├── UPDATING                     # Historical transition notes for developers
├── build.sh                     # [BUILD] Legendary cross-compilation bootstrap script
├── bin/                         # Fundamental userland utilities (sh, csh, ls, cp, mv)
├── sbin/                        # System administration binaries (init, fsck, mount, ifconfig)
├── usr.bin/                     # Standard user programs (253 tools: top, make, vi, grep, awk)
├── usr.sbin/                    # System administrative services (176 daemons: cron, inetd, sshd)
├── lib/                         # Standard C libraries (libc, libpthread, libcrypto, libm)
├── libexec/                     # Dynamic linker (ld.elf_so), mail helpers, system daemons
├── common/                      # Code shared between kernel and userland (libprop property lists)
├── compat/                      # Binary compatibility layers (Linux, Solaris, OSF/1, SVR4)
├── crypto/                      # Cryptographic utilities and IPsec engine
├── dist/                        # Upstream third-party source packages (bind, ntp, dhcp, pf, wpa)
├── distrib/                     # [AUX] Install sets, boot floppies, ramdisk images, sysinst installer
├── doc/                         # System documentation, licensing, and architectural notes
├── etc/                         # Boot scripts (rc.d), password database, system defaults
├── external/                    # Third-party tools with BSD glue (gcc, gdb, bsd-family)
├── games/                       # BSD classic games (adventure, trek, rogue, hack)
├── include/                     # System C header files
├── regress/, tests/             # [AUX] Regression test suites & ATF (Automated Testing Framework)
├── rescue/                      # Statically linked crunchgen recovery tools
├── share/                       # Manual pages, termcap, locale, zoneinfo
├── tools/                       # [AUX] Host toolchain cross-compiler generation harness
├── x11/                         # NetBSD X11 / X.org build infrastructure
└── sys/                         # Complete NetBSD Kernel Tree
    ├── kern/                    # Core machine-independent kernel
    │   ├── init_main.c          # main() - kernel initialization sequence
    │   ├── kern_mutex.c         # Adaptive and spin mutex implementation
    │   ├── kern_rwlock.c        # Reader/writer locks with lock-stealing support
    │   ├── kern_lwp.c           # 1:1 Light Weight Process (LWP) thread manager
    │   ├── kern_synch.c         # Scheduling, sleep queues, and context switching
    │   └── vfs_subr.c           # Virtual Filesystem Switch (VFS) operations
    ├── uvm/                     # UVM Virtual Memory Subsystem
    │   ├── uvm_fault.c          # Page fault handler (demand zero, COW, page swap-in)
    │   ├── uvm_page.c           # Physical page frame allocator and free lists
    │   ├── uvm_map.c            # Virtual memory map structures and address ranges
    │   └── uvm_amap.c           # Anonymous memory allocation maps
    ├── rump/                    # Antti Kantee's RUMP Anykernel Subsystem
    │   ├── librump/             # Core RUMP kernel runner in userspace
    │   ├── rumpfs/              # Userspace filesystem hypercall bridge
    │   └── rumpkern/            # Virtual scheduler and memory allocator for userspace
    ├── fs/                      # Filesystem drivers (FFS, LFS, tmpfs, puffs, sysvbfs, nfs)
    ├── net/, netinet/           # Networking stack (IPv4, IPv6, TCP, UDP, routing)
    └── arch/                    # Machine-dependent code for 50+ architectures
        ├── i386/, amd64/        # Intel/AMD x86 platforms
        ├── sparc/, sparc64/     # Sun SPARC platforms
        ├── vax/                 # DEC VAX minicomputers
        ├── macppc/              # Apple PowerPC Macintoshes
        ├── amiga/, atari/       # Motorola 68000 computers
        └── ...                  # Dozens of embedded and workstation architectures
```

---

## Key Source Code Files

| Source File | Functionality / Significance |
| :--- | :--- |
| `sys/kern/init_main.c` | Kernel bootstrap entry point `main()`. Configures machine-independent subsystems, allocates per-CPU idle threads, initializes UVM, mounts root, and executes `/sbin/init`. |
| `sys/kern/kern_mutex.c` | Andrew Doran's modern SMP mutex implementation. Features adaptive locking: if the lock holder is running on another CPU, the thread spins briefly; otherwise, it yields to the sleep queue. |
| `sys/kern/kern_lwp.c` | Core 1:1 threading engine. Manages Light Weight Processes (`lwp_create`, `lwp_exit`, `lwp_switch`), providing high-concurrency multitasking with minimal lock contention. |
| `sys/uvm/uvm_fault.c` | Core of the UVM memory manager. Resolves page faults with fine-grained locking, performs page loaning between processes and network buffers, and interacts with swap backends. |
| `sys/rump/librump/rumpkern/rump.c` | Core of the RUMP Anykernel. Initializes a virtual NetBSD kernel inside a userspace application process, providing POSIX threads with access to raw kernel drivers. |
| `build.sh` | Universal cross-compilation driver script. Bootstraps a host C compiler, builds cross-assemblers and cross-linkers, and generates bootable release sets for any target architecture. |
| `sys/fs/puffs/puffs_vfsops.c` | Pass-to-Userspace Framework for File Systems (PUFFS). Dispatches kernel VFS operations across a character device to userspace filesystem daemons. |

---

## Recommended Exploration Path

1. **Cross-Compilation Philosophy**:
   - Inspect `build.sh` and `BUILDING` to appreciate how NetBSD achieved complete platform neutrality.
2. **SMP Locking & 1:1 Concurrency**:
   - Study `sys/kern/kern_mutex.c` and `sys/kern/kern_rwlock.c` to see how adaptive locks operate on multi-core systems.
   - Examine `sys/kern/kern_lwp.c` to trace how kernel threads (LWPs) are scheduled.
3. **The RUMP Anykernel Innovation**:
   - Explore `sys/rump/` to witness how an entire kernel can be compiled to run within user space as a shared library (`librump`).
4. **Virtual Memory**:
   - Read `sys/uvm/uvm_fault.c` to see one of the cleanest modern virtual memory implementations in existence.
