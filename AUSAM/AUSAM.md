# AUSAM Source Code Archive

## Historical Overview

| Attribute | Details |
| :--- | :--- |
| **System Name** | AUSAM (Australian Unix Share Accounting Method) |
| **Release Date** | October 1979 |
| **Origin / Creators** | University of New South Wales (UNSW) & University of Sydney, Australia — Peter Ivanov, Ian Johnstone, Chris Maltby, Bob Kummerfeld, Greg Rose, supervised in part by John Lions |
| **Source Donation** | Extracted from the UNSW 04 magnetic tape and donated to TUHS by the University of New South Wales |
| **Target Architecture** | DEC PDP-11 (PDP-11/34, PDP-11/40, PDP-11/45, and PDP-11/70) |
| **Key Historical Milestones** | First comprehensive Fair-Share / Share Accounting scheduler; pioneer in UNIX multi-user security, password aging, and disk limits; numerous bug fixes contributed back to Bell Labs for V7 and System V |

### Significance & Historical Context
In the mid-to-late 1970s, Australian universities were early adopters of UNIX. At the University of New South Wales (UNSW)—where Professor John Lions authored his celebrated *Lions' Commentary on UNIX 6th Edition*—and the University of Sydney, hundreds of undergraduate students relied on single PDP-11 minicomputers for their coursework. Standard Bell Labs Sixth Edition UNIX (V6) was completely unsuited for this hostile, resource-constrained academic environment: it lacked resource quotas, fine-grained accounting, and process isolation.

To solve this, Australian computer scientists developed **AUSAM (Australian Unix Share Accounting Method)**. AUSAM completely overhauled the V6 kernel with:
1. **Share Accounting & Fair-Share Scheduling**: Rather than relying purely on simple priority boosts, the scheduler allocated CPU time proportional to a user or group's assigned "shares," preventing runaway processes from monopolizing the machine.
2. **Security & Cryptographic Passwords**: Introduced salted passwords, shadow-like validation files (`/usr/adm/records`), supervisor permission delegation, and login restrictions.
3. **Resource Controls**: Enforced disk block quotas, connect-time limits, and CPU time limits per user.
4. **Upstream Impact on Bell Labs**: The Australian developers identified and fixed hundreds of race conditions, deadlocks, and buffer bugs in V6 (denoted in the code by comments like `/* fix nnn */`). Many of these fixes were communicated to Ken Thompson and Dennis Ritchie and were incorporated directly into **Seventh Edition UNIX (V7)** and later early releases of **UNIX System V**.

---

## Directory Tree & Navigation Map

```
AUSAM/
├── READ_ME                      # Original distribution notice from UNSW
├── makdir                       # Shell script to build installation directory structure
├── distribut.doc/               # UNSW distribution papers and system documentation
│   ├── blurb.n                  # Overview of UNSW enhancements and system features
│   ├── security.n               # Documentation on AUSAM security model
│   └── equip.n, tape.n          # Hardware requirements and tape layouts
├── doc/                         # Comprehensive user and administrator manuals
│   ├── ausam/                   # Detailed AUSAM specification (sec0.n - sec7.n)
│   ├── edit.tut/                # Text editor tutorial
│   ├── intro.agsm/              # Introduction for Australian Graduate School of Management
│   ├── man/                     # Manual pages (man1 through man8)
│   └── plot/                    # Vector plotting documentation
├── include/                     # System C header files
├── source/                      # Complete userland application source tree
│   ├── ale/                     # Screen / display line editor
│   ├── as/                      # PDP-11 assembler
│   ├── c_compiler/              # Ritchie PDP-11 C compiler suite
│   ├── ded/                     # Display editor (precursor to modern full-screen editors)
│   ├── liba/, libS/, libt/      # AUSAM shared runtime and utility libraries
│   ├── libc/                    # Standard C library implementation
│   ├── make/                    # Early make build tool
│   ├── mdec/                    # Bootstrap loader code for DEC PDP-11
│   ├── plot/                    # Graphics plotting utilities
│   └── S/                       # 191 primary user and system maintenance commands
├── sys/                         # Primary AUSAM Operating System Kernel
│   ├── conf/                    # Machine configuration tables and device definitions
│   ├── dmr/                     # Device drivers (disks, tapes, terminals, consoles)
│   ├── ken/                     # Core OS routines (scheduler, memory, syscalls, filesystems)
│   ├── other/                   # System tuning scripts and diagnostic tools
│   └── sysd/                    # System descriptor tables and dump scripts
└── sys106/                      # Later version 106 AUSAM kernel tree
    ├── conf/                    # Configuration for build 106
    ├── dmr/                     # Drivers for build 106
    └── ken/                     # Core kernel sources for build 106
```

---

## Key Source Code Files

### 1. Share Scheduling & Accounting Primitives
- `sys/lnode.h`: Definition of the `lnode` structure. Represents a user's persistent resource account (shares allocated, CPU time consumed, disk space used, connect time) linked across all concurrent processes belonging to that user ID.
- `sys/ken/slp.c`: AUSAM scheduler. Computes dynamic process priority based on the user's allocated shares (`lnode`) versus actual CPU time accumulated, implementing fair-share scheduling.
- `sys/pzomb.h`: Data structure managing zombie processes and capturing precise process exit metrics for billing and accounting records.

### 2. Upstream Bug Fixes & Kernel Stability
- `sys/ken/alloc.c`: Disk block allocator. Heavily annotated with `fix` comments detailing race condition resolutions and disk block exhaustion handling.
- `sys/ken/iget.c`: Inode retrieval and buffer synchronization containing fixes that later made their way into Bell Labs V7.
- `sys/ken/sys1.c`: Implementation of `fork()`, `exit()`, `wait()`, and `exec()` wired into user account resource checks.

### 3. Userland Security & Accounting Applications
- `source/S/login.c`: AUSAM multi-tier authentication. Verifies password hashes, checks account expiration, enforces dial-up line permissions, and initializes the user's `lnode` accounting slot.
- `source/S/passwd.c`: AUSAM password changer enforcing password aging and password history.
- `source/ded/`: The "Display Editor" (DED), an early terminal screen editor developed in Australia.

---

## Recommended Exploration Path

1. **Understand Fair-Share Principles**:
   - Read `doc/ausam/sec0.n` through `doc/ausam/sec2.n` to understand the administrative accounting design.
   - Inspect `sys/lnode.h` to see how the user account structure is organized in memory.
2. **Analyze the Fair-Share Scheduler**:
   - Open `sys/ken/slp.c` and study how `sched()` and priority recalculation balance process CPU usage against share allocations.
3. **Trace the Historic "Fix" Comments**:
   - Grep for `fix` across `sys/ken/` (`alloc.c`, `fio.c`, `pipe.c`, `slp.c`). Observe how Australian developers patched critical concurrency and memory management bugs in Research UNIX V6.
4. **Compare Dual Kernel Snapshots**:
   - Compare `sys/ken/` with `sys106/ken/` to observe how the kernel was refined across revisions.
5. **Explore Userland Tools**:
   - Inspect `source/S/login.c` and `source/ded/` to see university terminal software from the late 1970s.
