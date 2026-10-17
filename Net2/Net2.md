# Net2 Source Code Archive

## Historical Overview
- **Release Date:** June 1991
- **Key Developers:** Keith Bostic, Kirk McKusick, Michael J. Karels, William Jolitz (386 port), and hundreds of volunteer contributors.
- **Machine Architecture:** Multi-platform: Intel 80386 (`i386`), HP 9000/300 (`hp300`), DEC VAX (`vax`), and CCI Tahoe (`tahoe`).
- **Historical Milestones:**
  - Berkeley Networking Release 2 (Net/2) was the **watershed moment in the history of open-source operating systems**.
  - **The Keith Bostic Clean-Room Crusade:** In the late 1980s, BSD was still encumbered by AT&T proprietary source code licensing. Keith Bostic organized a massive community campaign to systematically rewrite every single AT&T command, library, and kernel module from scratch based on manual pages and POSIX specifications.
  - **William Jolitz's 386 Port (`386BSD`):** William Jolitz contributed the initial Intel 80386 PC architecture port (`usr/src/sys/i386/`), demonstrating that a complete, 32-bit UNIX operating system could run on inexpensive, off-the-shelf PC hardware.
  - **Mach Virtual Memory (`vm/`):** Replaced the 1979 Babaoglu/Joy VAX paging system with the clean, architecture-independent Mach virtual memory subsystem (`vm_map.c`, `vm_fault.c`, `vm_pageout.c`).
  - **The Genesis of Modern BSDs:** Net/2 contained nearly an entire bootable, open-source operating system (only 6 kernel files still had AT&T code). It formed the **direct common ancestor of 386BSD, BSD/386 (BSDi), FreeBSD, and NetBSD**.
  - **USL v. BSDi Lawsuit:** Triggered the historic 1992 lawsuit where AT&T's Unix System Laboratories sued BSDi and UC Berkeley, a legal battle whose settlement in 1994 permanently validated open-source UNIX code.

---

## Directory Tree & Navigation Map
```
Net2/
└── usr/             # Top-level distribution hierarchy
    └── src/         # The unencumbered Net/2 source tree
        ├── sys/     # The unencumbered Net/2 operating system kernel
        │   ├── i386/       # William Jolitz's Intel 80386 PC architecture port
        │   ├── hp300/      # HP 9000/300 (Motorola 68000 family) machine support
        │   ├── vax/        # Machine-dependent DEC VAX support
        │   ├── tahoe/      # Machine-dependent CCI Tahoe support
        │   ├── vm/         # Mach-derived clean virtual memory subsystem
        │   ├── kern/       # Machine-independent kernel routines and VFS layer
        │   ├── ufs/        # Berkeley Fast File System (FFS)
        │   ├── nfs/        # Rick Macklem's unencumbered NFS client and server
        │   ├── netinet/    # High-performance Berkeley TCP/IP network stack
        │   ├── net/        # Core network routing, interfaces, packet filtering
        │   ├── netiso/     # ISO OSI protocol suite
        │   ├── netns/      # Xerox Network Systems protocol suite
        │   └── conf/       # Kernel configuration generator
        ├── bin/     # Clean-room rewritten system utilities (cat, cp, ls, sh)
        ├── sbin/    # Clean-room rewritten administration binaries (init, fsck)
        ├── usr.bin/ # Clean-room rewritten user utilities (make, awk, vi, etc.)
        ├── usr.sbin/# Clean-room rewritten system daemons (inetd, ftpd, routed)
        ├── libexec/ # Clean-room daemon helpers and wrappers
        ├── lib/     # Completely unencumbered C runtime libraries
        └── share/   # Clean-room manual pages, roff macros, and skel files
```

---

## Key Source Code Files
| File Path | Description & Architectural Significance |
| :--- | :--- |
| `usr/src/sys/i386/i386/locore.s` | **William Jolitz's 386 bootstrap:** Switches CPU to 32-bit protected mode, enables paging, and sets up GDT/IDT tables. |
| `usr/src/sys/i386/i386/machdep.c` | Machine-dependent initialization for the IBM PC AT architecture (ISA bus, memory sizing). |
| `usr/src/sys/vm/vm_fault.c` | Mach-derived virtual memory page fault handler, managing address maps and shadow objects. |
| `usr/src/sys/vm/vm_pageout.c` | The Mach pageout daemon, scanning memory objects and writing dirty pages to swap. |
| `usr/src/sys/kern/vfs_bio.c` | Clean-room implementation of the unified kernel buffer cache. |
| `usr/src/sys/ufs/ufs_alloc.c` | Unencumbered Berkeley Fast File System cylinder group block allocator. |
| `usr/src/sys/netinet/tcp_input.c` | The reference Berkeley TCP input processing engine with Van Jacobson congestion control. |
| `usr/src/bin/sh/` | Kenneth Almquist's **ash** (Almquist shell), a clean-room rewrite of the Bourne shell free of AT&T code. |

---

## Recommended Exploration Path
1. **Trace the PC Architecture Bootstrap (`usr/src/sys/i386/`):**
   - Begin at `usr/src/sys/i386/i386/locore.s` to see how William Jolitz booted UNIX on an IBM PC clone and transitioned from real mode to 32-bit protected mode.
2. **Examine the Mach VM Subsystem (`usr/src/sys/vm/`):**
   - Read `usr/src/sys/vm/vm_fault.c` and `vm_map.c` to understand how BSD adopted Mach's memory object model.
3. **Study Clean-Room Userland Rewrites (`usr/src/bin/sh/`):**
   - Inspect Kenneth Almquist's `ash` in `usr/src/bin/sh/` to see how the Bourne shell was completely reimplemented without AT&T code.
4. **Understand the Lineage of FreeBSD and NetBSD:**
   - Observe how Net/2's clean tree structure became the starting repository for both FreeBSD 1.0 (Nate Williams, Jordan Hubbard) and NetBSD 0.8 (Chris Demetriou, Theo de Raadt, et al.) in 1993.
