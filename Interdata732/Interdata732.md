# Interdata732 Source Code Archive

## Historical Overview

| Attribute | Details |
| :--- | :--- |
| **System Name** | Wollongong Interdata UNIX (Level 6 UNIX Port to Interdata 7/32) |
| **Release Date** | 1977 (Booted April 1977; documented 1978 ACM SOSP / USENIX 1998) |
| **Origin / Creators** | University of Wollongong, Australia — Richard Miller, Ross Nealon; supervised by Prof. Juris Reinfelds |
| **Source Donation** | Donated to TUHS (The Unix Heritage Society) by Richard Miller |
| **Target Architecture** | Interdata 7/32 (32-bit minicomputer, big-endian, linear address space, 16 32-bit general-purpose registers) |
| **Key Historical Milestones** | **The First UNIX Port in History**: First time UNIX was ported to a non-DEC architecture; proved UNIX was a portable operating system rather than a PDP-11 proprietary artifact |

### Significance & Historical Context
Until 1977, UNIX existed exclusively on Digital Equipment Corporation (DEC) hardware—originally the PDP-7, then the PDP-11. Because the operating system and Ritchie's C compiler were filled with implicit assumptions about 16-bit word size, PDP-11 byte order (middle-endian), and PDP-11 Memory Management Unit (MMU) registers, many in the computer science community believed UNIX was fundamentally tethered to PDP hardware.

In early 1976, the University of Wollongong acquired an Interdata 7/32, a 32-bit minicomputer with a completely different architecture: 32-bit words, big-endian byte ordering, 16 general-purpose 32-bit registers, and a primitive memory segmentation scheme. Richard Miller undertook the monumental task of porting Sixth Edition UNIX (V6) to this foreign machine:
1. **Compiler Retargeting**: Miller modified Dennis Ritchie's PDP-11 C compiler to generate code for the Interdata 7/32 and authored an entire Interdata assembler (`as`) and linker.
2. **Beating Bell Labs**: Dennis Ritchie and Steve Johnson at Bell Labs were concurrently attempting to port UNIX to an Interdata 8/32. However, Richard Miller booted UNIX on the Interdata 7/32 at Wollongong on **April 28, 1977**—several months before Bell Labs succeeded.
3. **Proof of Portability**: Miller's port demonstrated that an operating system written in C could be migrated to a wildly different architecture without a complete redesign. This success directly prompted Steve Johnson to build the Portable C Compiler (PCC) and influenced Bell Labs to overhaul the kernel for Version 7 UNIX, ensuring portability across 16-bit, 32-bit, little-endian, and big-endian systems.

---

## Directory Tree & Navigation Map

```
Interdata732/
├── etc/                         # Root administrative configuration files
│   ├── group                    # Group definitions
│   ├── passwd                   # User password database
│   ├── rc                       # System boot startup script
│   └── ttys                     # Terminal line enable table
└── usr/
    ├── bin/                     # User utilities
    ├── doc/                     # Wollongong documentation and C compiler manuals
    │   ├── cdoc/                # C compiler design notes
    │   ├── cman/                # C compiler reference manual
    │   ├── ctut/                # C tutorial
    │   └── man/                 # Interdata UNIX man pages
    ├── include/                 # Standard C library header files
    ├── lib/                     # Library databases (quiz, etc.)
    ├── mail/                    # Mail delivery spool
    ├── pub/                     # Public documentation tables
    ├── rat/                     # Ratfor (Rational Fortran) preprocessor
    ├── rm/                      # Remote machine communication tools
    ├── source/                  # Complete userland application and compiler source
    │   ├── READ_ME              # Wollongong userland source distribution notes
    │   ├── as/                  # Richard Miller's Interdata 7/32 assembler
    │   ├── boot/                # Standalone disk bootstrap routines
    │   ├── c/                   # Retargeted 32-bit C compiler suite
    │   ├── chicago/             # Early networking and communications tools
    │   ├── cmds/                # User commands adapted for 32-bit execution
    │   ├── cref/                # Cross-reference utility
    │   ├── fort/                # Fortran compiler
    │   ├── liba/, libc/, stdio/ # Runtime libraries and standard I/O library
    │   ├── lpd/, lpd_melb/      # Line printer daemons
    │   ├── m6/, ratfor/, sno/   # Macro processors and scripting interpreters
    │   ├── troff/, yacc/        # Document preparation and parser generator
    │   └── wgong/               # Wollongong-specific administrative utilities
    └── sys/                     # The Level 6 Interdata 7/32 Kernel Tree
        ├── READ_ME              # Wollongong kernel copyright notice (1977, 1978)
        ├── run, print           # Kernel compilation shell script and printer listing tool
        ├── alloc.c, iget.c ...  # Core V6 kernel routines adapted for 32-bit architecture
        ├── cio.c, cli.c, dsp.c  # Interdata peripheral drivers
        ├── dsk.c                # Interdata disk pack controller driver
        ├── dsk.c_old            # Historical backup copy of disk driver
        ├── dump.s, trace.s      # Crash dump routines and kernel stack tracing
        ├── fptrap.s             # Floating-point exception trap handler
        ├── long.s               # Multi-word integer emulation routines
        ├── low.s                # Low-core vector table and hardware interrupt handlers
        ├── msm.c, mt.c          # Mass storage module and magnetic tape drivers
        ├── sdev.s, start.s      # Device register definitions and CPU boot sequence
        ├── selch.c, selch.h     # Interdata Selector Channel DMA controller driver
        └── vdu.c                # Visual Display Unit (terminal) driver
```

---

## Key Source Code Files

### 1. Interdata 7/32 Hardware Vector & Startup
- `usr/sys/low.s`: Interdata low-core interrupt and trap vector initialization written in Interdata assembly. Handles Program Status Doubleword (PSD) swapping for device interrupts, system calls, and machine checks.
- `usr/sys/start.s`: Hardware bootstrap code executed when the machine is booted from ROM or disk. Sets up the kernel stack pointer (`ksp`), enables memory segmentation, and invokes `main()`.

### 2. DMA Channel & Peripheral Drivers
- `usr/sys/selch.c` and `usr/sys/selch.h`: Driver for the Interdata **Selector Channel** (SELCH), the hardware DMA engine that handled high-speed block transfers between memory and disk/tape drives.
- `usr/sys/dsk.c`: Interdata moving-head disk pack driver managing track seeking, sector positioning, and block I/O requests.
- `usr/sys/vdu.c`: Visual Display Unit (VDU) console driver managing serial asynchronous communication.

### 3. Porting Core Kernel Services to 32-Bit
- `usr/sys/main.c`: Kernel entry point initializing buffer pools, mounting the root filesystem, and launching the swapper. Notice how 16-bit assumptions regarding pointer arithmetic and physical addresses were cleaned up.
- `usr/sys/trap.c`: System call dispatcher translating Interdata SVC (Supervisor Call) instructions into UNIX system call invocations.
- `usr/sys/slp.c`: Process scheduler managing process states (`SSLEEP`, `SRUN`, `SZOMB`) and process swapping to disk.

### 4. Compiler and Toolchain Construction
- `usr/source/as/`: The complete assembler developed by Richard Miller to generate object modules for the Interdata 7/32 instruction set.
- `usr/source/c/`: Dennis Ritchie's original C compiler with code generator modifications retargeted for 32-bit registers and big-endian memory layout.

---

## Recommended Exploration Path

1. **Read the Original Documentation**:
   - Begin with `usr/sys/READ_ME` and `usr/source/READ_ME` to see the 1977–1978 copyright notices from the University of Wollongong.
2. **Examine the 32-Bit Machine Layer**:
   - Inspect `usr/sys/low.s` and `usr/sys/start.s` to observe how Miller handled the Interdata register set and Program Status Doubleword (PSD).
3. **Study the Selector Channel (SELCH)**:
   - Read `usr/sys/selch.c` to see how DMA on the Interdata was coordinated—a marked departure from the DEC Unibus map.
4. **Compare Kernel Data Structures**:
   - Compare `usr/sys/param.h` and `usr/sys/user.h` against standard V6 PDP-11 headers to observe how type declarations were adapted for 32-bit pointers and integers.
5. **Inspect the Toolchain**:
   - Browse `usr/source/as/` and `usr/source/c/` to understand the bootstrapping engineering that made the first UNIX port possible.
