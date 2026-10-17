# PWB1 Source Code Archive

## Historical Overview

| Attribute | Details |
| :--- | :--- |
| **System Name** | PWB/UNIX 1.0 (Programmer's Workbench UNIX Edition 1.0) |
| **Release Date** | July 1977 |
| **Origin / Creators** | Bell Laboratories (BISP / USG, Piscataway, NJ) — Evan L. Ivie, Rudd Canaday, John R. Mashey, Ted Dolotta |
| **Source Donation** | Preserved from `spencer_pwb.tar.gz` and donated to TUHS by Henry Spencer |
| **Target Architecture** | DEC PDP-11 (specifically PDP-11/45 and PDP-11/70 with MMU) |
| **Key Historical Milestones** | Birthplace of SCCS (Source Code Control System), the PWB Mashey Shell, RJE (Remote Job Entry), `uname()`, `find`, `xargs`, `egrep`, `fgrep`, and the USG kernel directory structure |

### Significance & Historical Context
In 1973, Evan L. Ivie and Rudd Canaday founded the Programmer's Workbench (PWB) project within the Business Information Systems Programs (BISP) division of Bell Laboratories. While Research UNIX (V4, V5, V6) developed by Thompson and Ritchie was oriented toward experimental computer science research, PWB was created to serve production engineering: supporting over 1,000 developers working simultaneously on massive telecommunications and database projects. By 1977, PWB was the largest and busiest UNIX installation in the world.

Based on Sixth Edition UNIX (V6), PWB/UNIX 1.0 introduced foundational software engineering concepts:
1. **SCCS (Source Code Control System)**: Created by Marc J. Rochkind in 1972–1975, SCCS was the first major version control system in computer history, introducing interleaved deltas, source checkouts, and release tagging.
2. **The PWB Shell (Mashey Shell)**: Written by John R. Mashey, it extended the rudimentary V6 shell with structured programming primitives (`if`/`then`/`else`, `goto`, `switch`), internal variables, and environment management, laying the groundwork for shell scripting prior to Steve Bourne's V7 shell.
3. **Enterprise Mainframe Interoperability (RJE)**: PWB introduced Remote Job Entry subsystems (`hasp`, `send`, `rjestat`) allowing developers at UNIX terminals to submit batch compilation jobs directly to IBM System/360, System/370, and UNIVAC mainframes.
4. **New System Calls**: Defined the `pwbsys` multiplexer, establishing `uname()` (system identification), `ustat()` (filesystem statistics), and `utime()` (file timestamp updating).
5. **Architectural Layout**: Reorganized the flat V6 kernel into structured functional directories (`os`, `io`, `ml`, `cf`, `hd`), which was subsequently adopted by AT&T's UNIX Support Group (USG) for UNIX System III and UNIX System V.

---

## Directory Tree & Navigation Map

```
PWB1/
├── root/                        # Root filesystem templates and system binaries
│   ├── bin/                     # Essential root binaries
│   ├── etc/                     # Admin configuration and root files
│   ├── lib/                     # System C runtime archives
│   └── usr/                     # Bootstrap usr files
├── sys/                         # Complete Operating System Kernel & Source Tree
│   ├── c/                       # C compiler frontend and backend tools
│   ├── fort/                    # Fortran compiler suite and runtime
│   ├── source/                  # Userland command source code
│   │   ├── acctg/               # Process and user accounting tools
│   │   ├── lex/                 # Lexical analyzer generator
│   │   ├── plot/                # Graphics plotting filters
│   │   ├── rje/                 # Remote Job Entry mainframe submission subsystem
│   │   ├── s1/ ... s8/          # Standard UNIX commands (find, cpio, expr, etc.)
│   │   ├── sccs4/               # Marc Rochkind's SCCS source code
│   │   ├── sno/                 # Snobol language interpreter
│   │   └── util/                # System administration utilities
│   └── sys/                     # The PWB/UNIX Kernel Subsystem
│       ├── cf/                  # System configuration tables and device configs
│       ├── hd/                  # Kernel header files (param.h, user.h, var.h, etc.)
│       ├── io/                  # Character and block peripheral drivers (bio, tty, hp, etc.)
│       ├── ml/                  # Machine language PDP-11 assembly (mch.s, dump routines)
│       └── os/                  # Core operating system services and syscalls
├── type/                        # Typesetting subsystems (croff, eqn, fonts)
├── update/                      # Incremental kernel and assembly updates
└── usr/                         # User filesystem hierarchy
    ├── actg/                    # Accounting records
    ├── bin/                     # User utilities
    ├── dict/                    # Spelling dictionaries
    ├── games/                   # Terminal games
    ├── hasp/                    # HASP communications protocols
    ├── include/                 # Standard C headers
    ├── jfo/                     # John F. Ossanna's working directories
    ├── lib/                     # Libraries, lex macros, help facility
    ├── man/                     # Manual pages (man1 through man8, op)
    ├── mdec/                    # Machine bootstrap blocks
    ├── news/                    # System bulletin board files
    ├── pub/                     # Public documentation tables
    └── rje/                     # Remote Job Entry configuration and spoolers
```

---

## Key Source Code Files

### 1. PWB System Call Innovations
- `sys/sys/os/pwbsys.c`: Implements the `pwbsys` system call multiplexer. Contains the earliest implementations of:
  - `case 0: uname` — copies system identification (`pwbname`) to user space.
  - `case 1: udata` — user identification data.
  - `case 2: ustat` — reports free blocks and free inodes on mounted filesystems.
  - `case 3: utime` — sets modification and access times for files.

### 2. SCCS (Source Code Control System)
- `sys/source/sccs4/cmd/admin.c`: SCCS administration tool for creating and tuning `s.` history files.
- `sys/source/sccs4/cmd/get.c`: SCCS retrieval engine that traverses delta trees to reconstruct specific file versions.
- `sys/source/sccs4/cmd/delta.c`: Generates change deltas and records modification commentary.
- `sys/source/sccs4/hdr/defines.h`: Core data structure declarations for SCCS file headers and delta entries.

### 3. Remote Job Entry (RJE) Subsystem
- `sys/source/rje/hasp/`: The HASP workstation emulator allowing PDP-11s to communicate with IBM mainframe front-end processors.
- `sys/source/rje/send/`: Utility for dispatching batch punch card images and JCL jobs across communications links.

### 4. Kernel Architecture & Hardware Dispatching
- `sys/sys/ml/mch.s`: Low-level PDP-11 assembly: interrupt handling, memory management unit (MMU) segmentation registers, process context switches (`savu`/`retu`), and kernel-to-user memory copy routines.
- `sys/sys/os/main.c`: Kernel bootstrap initializing the buffer cache, inode pool, process table, and mounting root.
- `sys/sys/os/slp.c`: V6-derived process scheduling and swap management modified for large multi-user capacity.
- `sys/sys/io/bio.c`: Block I/O buffer management adapted to high-capacity disk drives.

---

## Recommended Exploration Path

1. **Examine the Origin of Modern System Calls**:
   - Open `sys/sys/os/pwbsys.c` to inspect how `uname` and `ustat` were initially conceived and dispatched.
2. **Study the Birth of Version Control**:
   - Navigate to `sys/source/sccs4/` and read `com/` and `cmd/` to understand how Marc Rochkind implemented interleaved delta storage.
3. **Investigate the Kernel Directory Hierarchy**:
   - Compare `sys/sys/` (`os/`, `io/`, `ml/`, `cf/`, `hd/`) with Research UNIX V6 (which placed all kernel files into a flat directory). Notice how this modularization shaped UNIX System III and System V.
4. **Trace Low-Level PDP-11 CPU Primitives**:
   - Read `sys/sys/ml/mch.s` to see how the PDP-11 segmentation registers (`KISA`, `KISD`, `UISA`, `UISD`) were loaded during execution.
5. **Explore Mainframe Batch Integration**:
   - Review `sys/source/rje/` to witness how early UNIX acted as a satellite workstation system for IBM mainframes.
