# Coherent4.2.10 Source Code Archive

## Historical Overview
- **Release Date**: December 1994 (Version 4.2, patchlevel 10)
- **Author / Organization**: Mark Williams Company (MWC), founded by Robert Swartz (Chicago, Illinois)
- **Target Architecture**: Intel 80386 and 80486 (32-bit protected mode)
- **Historical Milestone**: The definitive commercial clean-room Unix clone for the IBM PC. Originally engineered in the late 1970s and 1980s by former University of Waterloo graduates (including Dave Conroy), Coherent offered near-complete AT&T Version 7 and System V compatibility for just $99.95, complete with multi-volume printed manuals. Legendarily, AT&T lawyers and engineers audited Coherent's source code line by line and concluded it was a genuine, independent, clean-room reimplementation containing zero AT&T trade secrets. In 2015, the Swartz family officially released Coherent under the open-source 3-clause BSD license.

### Key Architectural Advances
- **Full 32-Bit Monolithic Kernel (`coh.386/`)**: Upgraded from 16-bit 286 segmented real/protected mode to native 32-bit flat memory addressing with demand paging and hardware protection rings.
- **AT&T STREAMS Framework (`conf/streams/`)**: Implemented the modular System V STREAMS character I/O mechanism, enabling dynamic stacking of network protocols and line disciplines.
- **DDI / DKI Conformance (`coh.386/lib/`)**: Adhered to AT&T's Device Driver Interface / Driver-Kernel Interface specifications, standardizing driver routines (`ddi_defer.c`, `ddi_unref.c`).
- **Modular Dynamic Kernel Linking (`conf/bin/`)**: Unlike traditional monolithic Unix kernels requiring full recompilation, Coherent featured the `mkdev` / `idbuild` modular kernel configuration framework, which linked driver object modules into a customized `/coherent` kernel binary at boot time.
- **Virtual Consoles (`conf/vtkb/`, `conf/vtnkb/`)**: Support for multiple concurrent virtual terminal sessions on the PC monitor.

---

## Directory Tree & Navigation Map

```
Coherent4.2.10/
├── boot/                        # Primary bootloader subsystem
│   ├── boot.m                   # Primary bootloader machine code source
│   ├── mboot.s                  # Master Boot Record (MBR) bootloader assembly
│   ├── pboot.m                  # Partition bootloader
│   ├── patches                  # [AUX] Release notes and BIOS workaround instructions
│   └── Makefile                 # Bootloader build file
├── tboot/                       # Tertiary / Secondary bootloader
│   ├── sys/                     # Bootloader system types (typed.h, ptypes.h)
│   └── ...                      # 36 files handling kernel image loading from disk
├── coh.386/                     # Core Monolithic 32-Bit Kernel
│   ├── main.c                   # Kernel entry point and subsystem bootstrap
│   ├── clock.c                  # Real-time clock and software timer handling
│   ├── fs1.c, fs2.c, fs3.c      # Coherent Filesystem implementation (inodes, blocks, caching)
│   ├── pipe.c                   # Inter-process FIFO pipes
│   ├── poll.c                   # System V poll() asynchronous I/O multiplexing
│   ├── sig.c                    # Signal generation and delivery mechanics
│   ├── sys1.c, sys2.c, sys3.c   # System V Release 3 system call implementations
│   ├── syscoh.c                 # Coherent-specific extended system calls
│   └── lib/                     # 66 kernel library source files
│       ├── kmem_alloc.c         # Dynamic kernel heap allocator
│       ├── ddi_defer.c          # DDI/DKI deferred function execution
│       ├── proc_ssig.c          # Process signal delivery helpers
│       ├── proc_sdel.c          # Process deletion and zombie cleanup
│       └── ...                  # Standard kernel runtime routines
├── i386/                        # Machine-Dependent 386 Low-Level Hardware Layer
│   ├── intr.inc                 # Assembly interrupt entry macro definitions
│   ├── ddi.inc                  # DDI assembly interface structures
│   ├── repinsw.s, outl.s        # Inlined port I/O string primitives
│   └── lib/                     # 36 low-level assembly helper routines
├── io.386/                      # Core Device Driver Implementations
│   ├── al.c                     # Asynchronous serial port driver (COM1/COM2)
│   ├── hs.c                     # High-speed buffered serial driver
│   ├── fdisk.c                  # Hard disk partition table decoder
│   ├── bufq.c                   # Block I/O buffer queue sorting
│   └── putchar.c                # Low-level console character output
├── conf/                        # Modular Driver Configuration Subsystem (40 modules)
│   ├── at/                      # AT / IDE hard disk driver configuration
│   ├── fd/, fdc/                # Floppy disk controller configuration
│   ├── hai/, haisd/, haicd/     # Host Adapter Interface (SCSI disk/CD-ROM drivers)
│   ├── streams/                 # System V STREAMS subsystem module
│   ├── shm/, sem/, msg/         # System V IPC (Shared memory, Semaphores, Message queues)
│   ├── pty/, tty/               # Pseudo-terminals and teletype drivers
│   ├── vtkb/, vtnkb/            # Virtual terminal keyboard modules
│   ├── lp/                      # Parallel line printer driver
│   └── bin/                     # Kernel generation and link utilities
│       ├── mkdev                # Device driver configuration script
│       ├── idbuild              # Modular kernel link manager
│       └── idmkunix             # Final kernel binary linking engine
└── include/                     # System Header Files
    ├── sys/                     # System headers (97 files: stat.h, inode.h, vnode.h, etc.)
    ├── kernel/                  # Kernel internal headers (44 files: _sleep.h, pri.h, x86io.h)
    ├── coh/                     # Coherent internal headers (fakedma.h, timeout.h, md.h)
    ├── common/                  # Shared types (__caddr.h, _clock.h, gregset.h)
    ├── netinet/                 # in.h
    └── arpa/                    # inet.h
```

---

## Key Source Code Files

| Source File | Functionality / Significance |
| :--- | :--- |
| `coh.386/main.c` | Kernel initialization sequence. Configures memory segments, initializes the buffer cache, mounts the root filesystem, builds process 0 (idle), and forks process 1 (`/etc/init`). |
| `coh.386/fs1.c`, `fs2.c`, `fs3.c` | The Coherent Filesystem. Implements an efficient Unix V7/SVR3 style filesystem with 32-bit inode addressing, free-block list management, and buffer cache integration. |
| `coh.386/lib/kmem_alloc.c` | Dynamic kernel memory allocator implementing `kmem_alloc()` and `kmem_free()`, providing heap management for device driver structures and network buffers. |
| `conf/bin/idbuild` | Modular kernel link script. Gathers driver object files from individual `conf/*/src` directories and invokes the linker to produce a custom `/coherent` kernel tailored to detected hardware. |
| `io.386/hs.c` | High-speed buffered serial communications driver. Critical in the 1990s for running dial-up BBS (Bulletin Board System) and UUCP nodes without buffer overruns. |
| `conf/streams/src/strm.c` | Core System V STREAMS engine. Implements stream heads, queue scheduling, message allocation (`allocb`, `freeb`), and downstream/upstream message passing. |

---

## Recommended Exploration Path

1. **Monolithic Kernel Bootstrap**:
   - Begin at `boot/mboot.s` to see the x86 master boot record loader.
   - Trace `coh.386/main.c` to see how Coherent establishes process 0 and mounts the root filesystem.
2. **Modular Kernel Linking (`conf/`)**:
   - Inspect `conf/bin/idbuild` and `conf/bin/mkdev` to understand how Coherent links a customized kernel from modular driver directories without needing source recompilation.
3. **Storage & Filesystem**:
   - Read `coh.386/fs1.c` and `coh.386/fs2.c` to study Coherent's clean-room implementation of Unix disk structures.
4. **Driver & STREAMS Subsystem**:
   - Study `conf/streams/` to understand how System V STREAMS was engineered cleanly in an independent OS.
