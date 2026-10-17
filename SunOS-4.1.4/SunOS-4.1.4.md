# SunOS-4.1.4 Source Code Archive

## Historical Overview

| Attribute | Details |
| :--- | :--- |
| **System Name** | SunOS 4.1.4 (Commercial designation: Solaris 1.1.2) |
| **Release Date** | November 1994 (Source tree populated January 18, 1995) |
| **Origin / Creators** | Sun Microsystems, Inc. (Mountain View, CA) — Bill Shannon, Robert Gingell, Joseph Moran, Steve Kleiman, Dan Walsh, and the SunOS kernel team |
| **Source Donation** | Preserved from internal Sun Microsystems source trees |
| **Target Architecture** | Sun SPARC family (sun4, sun4c, sun4m, sun4d: SPARCstation 1, 2, 5, 10, 20; SPARCserver 1000/2000 multiprocessors) and Motorola 68020/68030 (sun3/sun3x) |
| **Key Historical Milestones** | **The Zenith of BSD-Derived Commercial UNIX**: Final release of BSD-based SunOS before the SVR4 Solaris 2.x transition; introduced modern Virtual Memory (`mmap`, `seg_vn`, `anon`); pioneered dynamic shared libraries (`ld.so`, `libdl`); multi-universe BSD/SysV/POSIX support |

### Significance & Historical Context
Released in late 1994, **SunOS 4.1.4** represents the absolute pinnacle of classic BSD-based commercial UNIX. Following SunOS 4.1.4, Sun Microsystems discontinued the BSD lineage and completed its controversial transition to Solaris 2.x (based on AT&T UNIX System V Release 4). Revered by systems engineers for its unmatched speed, stability, and elegance, SunOS 4.1.4 was the operating system of choice across the global Internet infrastructure and scientific workstation market throughout the mid-1990s.

Key technological breakthroughs showcased in this codebase include:
1. **The Modern Virtual Memory (VM) Subsystem**: Designed by Robert Gingell, Joseph Moran, and William Shannon, SunOS 4 replaced the aging 4.3BSD paging system with an object-oriented memory architecture. It introduced address spaces (`as`), memory segment drivers (`seg_vn`, `seg_map`, `seg_dev`, `seg_u`), anonymous memory pools (`anon`), and Hardware Address Translation layers (`hat`). This design made **`mmap()` memory-mapped files** a first-class citizen and unified the filesystem cache with system virtual memory.
2. **Dynamic Shared Libraries & Dynamic Linking**: SunOS 4 was the first production UNIX to provide runtime dynamic linking (`ld.so`, `libdl`, `dlopen`, `dlsym`), drastically reducing executable sizes and system RAM consumption across desktop environments.
3. **Multi-Universe Personality System**: Allowed programmers to execute software seamlessly in BSD 4.3 (`/usr/ucb`), System V Release 3 (`/usr/5bin`), POSIX.1 (`posixlib`), or X/Open (`xpgbin`) environments simply by altering their `$PATH`.
4. **Rich Virtual File System (VFS) Suite**: Featured native support for UFS (Fast File System), NFS v2, HSFS (High Sierra / ISO 9660 CD-ROM), PCFS (MS-DOS FAT), LOFS (Loopback), TFS (Translucent File System—an early copy-on-write union filesystem), and RFS (Remote File Sharing).
5. **Multiprocessor Architecture (`sun4d`, `sun4m`)**: Contained symmetrical multiprocessing (SMP) extensions for SPARCstation 10/20 and enterprise SPARCserver 1000/2000 hardware.

---

## Directory Tree & Navigation Map

```
SunOS-4.1.4/
├── INTENTS, RULES, RELEASE      # Internal developer policies and guidelines
├── 5bin/, 5include/, 5lib/      # AT&T UNIX System V Release 3 compatibility universe
├── adm/, files/, pub/           # Administrative configuration and public tables
├── bin/                         # Root binaries (sh, csh, awk, diff, make, sed, etc.)
├── demo/                        # Multimedia and graphics demos (SUNVIEW, XGL, SOUND)
├── diagnostics/                 # Sundiag hardware diagnostic suite
├── etc/                         # Core system daemons (fsck, in.routed, lockd, mount, etc.)
├── games/                       # Terminal and window games (hack, sail, battlestar, etc.)
├── include/                     # Master C header files (arpa/, protocols/)
├── lang/                        # SunPro compiler suite (C compiler, lint, cflow, iropt)
├── lib/                         # Standard C library (libc) and runtime start (csu)
├── man/                         # Comprehensive man pages (man1 - man8, policy, arch)
├── old/                         # Obsolete utility sources (compact, eyacc, filemerge)
├── posixlib/, xpgbin/, xpglib/  # POSIX.1 and X/Open conformance libraries and utilities
├── release/                     # Sun internal release engineering scripts
├── sccs/                        # Complete SCCS revision control system source
├── sundist/                     # Sun distribution CD-ROM and tape mastering tools
├── sys/                         # The Complete SunOS 4.1.4 Operating System Kernel
│   ├── boot/                    # Bootstrapping code for Sun workstations
│   ├── hsfs/, lofs/, pcfs/      # ISO 9660, Loopback, and MS-DOS FAT filesystems
│   ├── krpc/, nfs/, rfs/        # In-kernel ONC RPC, NFS v2, and AT&T RFS
│   ├── lwp/                     # Lightweight Processes (early kernel thread library)
│   ├── net, netinet, nettli     # TCP/IP sockets, STREAMS, and Transport Level Interface
│   ├── os/                      # Core kernel services (exec, exit, fork, vfs, dnlc)
│   ├── sbusdev/, scsi/          # SBus framebuffers (cg3, cg6) and SCSI subsystem (sd, st)
│   ├── sparc/                   # SPARC instruction set architecture core routines
│   ├── specfs/, sys/            # Device vnode filesystem and kernel headers
│   ├── sun/, sun4, sun4c, sun4m # Machine-dependent code for SPARC workstation lines
│   ├── tfs/                     # Translucent File System (early union mount filesystem)
│   └── vm/                      # The SunOS 4 Virtual Memory Subsystem (seg, as, anon)
├── ucb/, ucbinclude/, ucblib/   # UC Berkeley 4.3BSD compatibility universe
└── usr.bin/, usr.etc/, usr.lib/ # User utilities, administrative tools, and shared libraries
```

---

## Key Source Code Files

### 1. The Revolutionary SunOS Virtual Memory Subsystem (`sys/vm/`)
- `sys/vm/vm_as.c` & `sys/vm/as.h`: Address Space management (`struct as`). Abstracts a process's entire virtual memory map as a sorted list of independent memory segments.
- `sys/vm/seg_vn.c`: The vnode segment driver. Implements `mmap()` file mappings, demand paging of executable binaries directly from disk or NFS, and copy-on-write page faults.
- `sys/vm/seg_map.c`: Fast kernel-buffer address mapping cache used by the VFS layer to avoid block cache copies.
- `sys/vm/vm_anon.c`: Manages anonymous memory (private stack and heap pages backed by swap space rather than named files).
- `sys/vm/vm_page.c`: Physical memory page allocation, free list management, and the page daemon (`pageout`).

### 2. Core Kernel and SPARC Architecture
- `sys/sparc/locore.s`: SPARC low-level machine assembly. Handles SPARC register window overflow and underflow traps (`window_overflow`, `window_underflow`), processor interrupt levels (`PIL`), and system calls.
- `sys/os/vfs.c` & `sys/os/vfs_dnlc.c`: Production-hardened VFS mount table and Directory Name Lookup Cache.
- `sys/sun4m/autoconf.c`: Hardware device probe and autoconfiguration for SPARCstation 10/20 architectures.

### 3. Specialty Filesystems
- `sys/tfs/`: The Translucent File System (TFS). One of the earliest copy-on-write overlay file systems, allowing developers to see a base read-only source tree while recording local changes into a private overlay layer.
- `sys/pcfs/`: Sun's MS-DOS FAT filesystem driver for reading PC floppy disks and partitions.
- `sys/hsfs/`: High Sierra / ISO 9660 filesystem driver supporting CD-ROM drives on workstations.

### 4. Dynamic Linking and Runtime Libraries
- `usr.lib/libdl/`: Implementation of dynamic library runtime linking (`dlopen`, `dlsym`, `dlclose`, `dlerror`).

---

## Recommended Exploration Path

1. **Study Modern Virtual Memory Design**:
   - Begin with `sys/vm/as.h` and `sys/vm/vm_as.c` to understand how processes address virtual memory through abstract segment drivers.
   - Inspect `sys/vm/seg_vn.c` to see the definitive implementation of `mmap()` and file-backed virtual memory.
2. **Examine SPARC Low-Level Traps**:
   - Read `sys/sparc/locore.s` to witness how the SPARC RISC architecture handled sliding register window overflow/underflow exceptions.
3. **Explore Specialty Virtual Filesystems**:
   - Investigate `sys/tfs/` to see the earliest prototype of union-mounting and copy-on-write filesystem semantics.
4. **Inspect Dynamic Linking Architecture**:
   - Browse `usr.lib/libdl/` to see how Sun solved dynamic library loading at runtime.
5. **Analyze the Multi-Universe Environment**:
   - Compare `5bin/` and `ucb/` to understand how Sun provided dual AT&T System V and Berkeley BSD compatibility on the same machine.
