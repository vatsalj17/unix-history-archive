# V7M Source Code Archive

## Historical Overview

| Attribute | Details |
| :--- | :--- |
| **System Name** | UNIX/V7M 2.1 (UNIX Version 7 Modified for PDP-11 / V7M11) |
| **Release Date** | October 1981 (Version 2.1) |
| **Origin / Creators** | Digital Equipment Corporation (DEC) Unix Engineering Group (UEG, Merrimack, NH) — Fred Canter, Jerry Brenner, Armando Stettner, Bill Burns, Mary Anne Cacciola, Bill Munson |
| **Source Donation** | Preserved from DEC release tapes and donated to TUHS |
| **Target Architecture** | DEC PDP-11 family (PDP-11/23, 11/24, 11/34, 11/44, 11/45, and 11/70) |
| **Key Historical Milestones** | **DEC's First Supported Commercial UNIX Product**: Introduced the overlay kernel for 16-bit addressing; supported both separate and non-separate I/D spaces; foundation for ULTRIX-11 |

### Significance & Historical Context
In the late 1970s, Digital Equipment Corporation (DEC) was the primary hardware vendor for UNIX, but UNIX was an AT&T/Bell Labs software product rather than a DEC offering. Recognizing customer demand for factory-supported UNIX on the PDP-11 line, DEC formed the **Unix Engineering Group (UEG)** in Merrimack, New Hampshire. Led by Fred Canter and Jerry Brenner, UEG acquired Bell Labs' Seventh Edition UNIX (V7) and heavily modernized it for commercial deployment, releasing **V7M** in 1981.

V7M resolved the critical bottlenecks of Research V7:
1. **The 64KB Kernel Limit & Overlay Execution**: On the 16-bit PDP-11, total address space was capped at 64KB. As new device drivers and features were added, the kernel grew beyond 64KB. DEC solved this by implementing an **overlay kernel** (`ovsys`, `ovdev`, `mch_ov.s`). Using PDP-11 segmentation registers, kernel code modules were swapped into the CPU address window on demand, allowing a 100KB+ kernel to run smoothly in a 64KB instruction space.
2. **Support for Non-Separate I/D Processors**: While high-end PDP-11/45 and 11/70 machines featured separate Instruction and Data spaces (giving 64KB code + 64KB data), cheaper models (PDP-11/34, 11/23) shared a single 64KB space for both. V7M provided separate machine assembly tracks (`mch_id.s` vs `mch_i.s`) to run on either CPU class.
3. **Robust Standalone Bootstrap (`sas/`)**: V7 was notorious for tricky tape installations. DEC built a two-stage standalone bootstrap monitor (`sas/boot.c`) capable of bootstrapping from any DEC tape drive (TM11, TS11, HT) or disk controller (RK06/07, RL01/02, RM02/03/05, RP04/05/06).
4. **Hardware Error Recovery**: Added extensive hardware error recovery routines, diagnostic logging, and support for DEC's newest PDP-11/44 processors. The UEG team that produced V7M subsequently evolved into the group that created DEC's flagship ULTRIX line.

---

## Directory Tree & Navigation Map

```
V7M/
├── adm/, dict/, games/, man/    # Administrative records, dictionaries, games, man pages
├── bin/, pub/, spool/           # Root binaries, public tables, printer/mail spools
├── doc/                         # Complete UNIX documentation set (troff, ed, shell)
├── include/                     # Standard C system header files
│   └── sys/                     # Kernel header symlinks/copies
├── lib/                         # Compiler support libraries, learn scripts, uucp
├── src/                         # Complete userland application and library sources
│   ├── cmd/                     # 135 core userland commands (sh, awk, adb, cc, etc.)
│   ├── games/                   # Terminal games source (chess, etc.)
│   ├── libc/                    # Standard C library (sys, stdio, gen, crt)
│   ├── libF77/, libI77/, libm/  # Fortran 77 and floating-point math libraries
│   ├── libdbm/, libmp/, libplot/# DBM, multiple precision, and graphics libraries
│   └── libfpsim/                # Floating-point software simulator
└── sys/                         # The Complete DEC V7M Kernel Hierarchy
    ├── README                   # DEC UEG source overview and configuration guide
    ├── addenda/                 # Western Electric UNIX/V7 official addenda dated 12/2/80
    ├── conf/                    # System configuration, makefiles, and machine startup
    │   ├── mch0.s, mch_i.s      # PDP-11 machine assembly for non-separate I/D CPUs
    │   ├── mch_id.s             # Machine assembly for separate I/D space CPUs
    │   └── mch_ov.s             # Machine assembly for overlay kernel execution
    ├── dev/                     # 43 DEC hardware peripheral device drivers
    ├── h/                       # Kernel data structure header files
    ├── mdec/                    # Primary disk boot sector code (RP, RL, HP, HK)
    ├── ovdev/                   # Driver object modules and staging for overlay kernel
    ├── ovsys/                   # Kernel subsystem object modules for overlay kernel
    │   └── *.junk, *.sv         # Build scratch scripts from overlay generator
    ├── sas/                     # Standalone System: 2-stage bootstrap monitor and installer
    └── sys/                     # Core operating system services (main, slp, trap, etc.)
```

---

## Key Source Code Files

### 1. Overlay Kernel & Assembly Core
- `sys/conf/mch_ov.s`: Machine-language overlay dispatcher. Intercepts function calls across overlay segments, alters PDP-11 Page Address Registers (PARs), and branches to the newly mapped kernel code.
- `sys/conf/mch_id.s`: Machine assembly for PDP-11/44, 11/45, and 11/70 processors with separate Instruction and Data (I/D) space.
- `sys/conf/mch_i.s`: Machine assembly for PDP-11/23, 11/24, and 11/34 processors with shared I/D space.

### 2. Standalone Boot Monitor (`sas/`)
- `sys/sas/boot.c`: DEC standalone two-stage bootstrap program. Prompts the operator for the boot device, parses file systems in standalone mode, loads `/unix`, and jumps to location 0.
- `sys/sas/maketape.c`: Tool used by DEC release engineers to manufacture bootable V7M distribution magnetic tapes with standalone bootblocks and filesystem dumps.

### 3. Core Kernel & DEC Hardware Drivers
- `sys/sys/main.c`: Kernel initialization sequence initializing buffer headers, testing memory boundaries, and starting process 0.
- `sys/sys/machdep.c`: Machine-dependent PDP-11 hardware setup, memory parity error recovery, and processor identification.
- `sys/dev/rl.c`: DEC RL01 / RL02 5MB/10MB removable cartridge disk driver.
- `sys/dev/hk.c`: DEC RK06 / RK07 moving-head cartridge disk driver.
- `sys/dev/ts.c`: DEC TS11 magnetic tape streamer driver.

---

## Recommended Exploration Path

1. **Review DEC's Engineering Overview**:
   - Read `sys/README` to see how DEC engineers categorized the source tree into overlay kernels, standalone tools, and device drivers.
2. **Study PDP-11 Kernel Overlays**:
   - Inspect `sys/conf/mch_ov.s` and `sys/ovsys/mklib_ov` to understand how DEC overcame the 16-bit 64KB architectural limit.
3. **Trace the Standalone Bootstrap**:
   - Examine `sys/sas/boot.c` to understand how bare-metal PDP-11 systems booted from tape or disk without an underlying BIOS.
4. **Examine Device Drivers**:
   - Browse `sys/dev/` to inspect production-quality DEC hardware drivers (`rl.c`, `hk.c`, `ts.c`).
5. **Inspect Western Electric Addenda**:
   - Review `sys/addenda/` to see the official late-1980 bug patches distributed by Bell Labs / Western Electric for V7.
