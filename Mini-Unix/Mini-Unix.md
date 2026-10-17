# Mini-Unix Source Code Archive

## Historical Overview

| Attribute | Details |
| :--- | :--- |
| **System Name** | Mini-Unix (also designated MX or Mini-UNIX) |
| **Release Date** | December 10, 1976 (Bell Labs Technical Memorandum 76-1352-7) |
| **Origin / Creators** | Bell Laboratories (Murray Hill, NJ) — Heinz Lycklama |
| **Source Donation** | Preserved in the TUHS archive; documented via original Bell Labs memoranda |
| **Target Architecture** | DEC PDP-11 minicomputers lacking MMU hardware (PDP-11/05, 11/10, 11/15, 11/20, and LSI-11) with 28K words (56KB) total RAM |
| **Key Historical Milestones** | Brought multi-user UNIX time-sharing to low-cost hardware lacking memory management; squeezed full kernel into 12K words (24KB); supported up to 4 concurrent users via swapping |

### Significance & Historical Context
In 1976, standard Research Sixth Edition UNIX (V6) required a PDP-11/40, PDP-11/45, or PDP-11/70 equipped with a KT-11 Memory Management Unit (MMU) to provide hardware memory protection and address relocation. These systems were expensive and out of reach for smaller research laboratories, secondary educational institutions, and industrial automation teams who owned cheaper low-end PDP-11s (such as the PDP-11/05 or 11/10) limited to 28K words (56KB) of total core memory and lacking an MMU entirely.

Heinz Lycklama of Bell Laboratories created **Mini-Unix** to solve this problem. By radically redesigning the kernel:
1. **12K Word Kernel**: Lycklama compressed the entire UNIX kernel into just 12K words (24KB) of memory, leaving 16K words (32KB) for running user programs.
2. **Multi-User Time Sharing Without MMU**: Despite the total absence of hardware memory protection, Mini-Unix supported up to 4 simultaneous users. It achieved this through an aggressive round-robin swapping scheduler that swapped user processes between memory and disk storage (RK05, RP03, or HP disk drives).
3. **Instruction Emulation**: To accommodate cheaper CPUs lacking the PDP-11 Extended Instruction Set (EIS), Mini-Unix incorporated an in-kernel emulator (`emul.s`) that caught illegal instruction traps and emulated hardware multiplication (`mul`), division (`div`), and multi-bit shifts (`ash`).
4. **Pruned System Calls**: To conserve precious memory words, non-essential system calls (`ptrace`, `prof`, `pipe`, `getgid`, `setgid`) were omitted or altered.

---

## Directory Tree & Navigation Map

```
Mini-Unix/
├── mx, hpmx, rkmx, rpmx         # Pre-compiled PDP-11 binary kernel images
├── dev/, mnt/, tmp/             # Empty root directory templates
├── bin/                         # Root binaries (sh, ls, ed, cp, rm, etc.)
├── etc/                         # System administration files (passwd, rc, ttys)
├── lib/                         # C compiler passes and runtime libraries
└── usr/
    ├── adm/, lpd/, tmp/         # Empty user administrative directories
    ├── bin/                     # User utilities compiled for 28K address space
    ├── doc/                     # Documentation and papers
    │   └── new/                 # Lycklama's original Bell Labs memos (memo1, memo2)
    ├── games/                   # Terminal games
    ├── lib/                     # Standard libraries and databases
    ├── mdec/                    # Machine bootstrap blocks for DEC disk drives
    ├── pub/                     # Character and reference documentation tables
    ├── source/                  # Complete userland application source tree
    └── sys/                     # The Complete Mini-Unix Operating System Kernel
        ├── dev/                 # PDP-11 device drivers (rk, rp, hp, ht, tm, kl, lp, etc.)
        ├── mxsys/               # Core Mini-Unix kernel sources
        │   ├── *.c, *.s         # Operating system C and assembly sources
        │   └── *.o              # Pre-compiled PDP-11 object code leftovers
        └── source/              # Kernel build utilities (init, ld, check, sh, kdmp)
```

---

## Key Source Code Files

### 1. Architectural Memoranda & Specifications
- `usr/doc/new/memo1`: Heinz Lycklama's December 10, 1976 Bell Labs Technical Memorandum (*TM 76-1352-7: The MINI-UNIX System*). Explains the memory budget, design constraints, and hardware support.
- `usr/doc/new/memo2`: Detailed configuration and installation guide for building Mini-Unix for specific PDP-11 controllers.

### 2. Core Kernel and Swapping Scheduler
- `usr/sys/mxsys/main.c`: Kernel initialization sequence. Partitions core memory, initializes device tables, mounts the root filesystem, and launches process 1 (`init`).
- `usr/sys/mxsys/slp.c`: Round-robin swapping scheduler. Manages multi-user scheduling across up to 4 users without MMU hardware support by moving processes to and from disk swap space.
- `usr/sys/mxsys/mch.s`: Machine-language PDP-11 assembly handling context switching, interrupt priority masking, and low-level stack management.
- `usr/sys/mxsys/low.s`: Hardware vector table mapping interrupts and traps to kernel service routines.

### 3. CPU Math Emulation & Hardware Drivers
- `usr/sys/mxsys/emul.s`: Trap-based instruction emulator providing software implementations of `mul`, `div`, and `ash` for low-end PDP-11 processors lacking EIS.
- `usr/sys/dev/rk.c`: DEC RK05 moving-head disk cartridge driver (the most common storage media for Mini-Unix).
- `usr/sys/dev/hp.c`: High-capacity RP04/RP05/RP06 disk pack driver.

---

## Recommended Exploration Path

1. **Read the Original Bell Labs Memo**:
   - Begin by reading `usr/doc/new/memo1` to understand how Heinz Lycklama engineered an operating system to fit within a strict 12K word memory budget.
2. **Examine the Multi-User Swapper**:
   - Inspect `usr/sys/mxsys/slp.c` to see how round-robin time-slicing provided multi-user time-sharing despite lacking hardware virtual memory or memory protection.
3. **Trace Instruction Emulation**:
   - Study `usr/sys/mxsys/emul.s` to see how missing math instructions were trapped and calculated in software.
4. **Compare with LSX**:
   - Compare `usr/sys/mxsys/` with Bell Labs `LSX/sys/`. Notice that while LSX was aimed at single-user floppy disk setups, Mini-Unix was built for multi-user minicomputers with hard disks.
5. **Inspect Peripheral Drivers**:
   - Review `usr/sys/dev/rk.c` and `usr/sys/dev/hp.c` to observe how disk I/O was buffered and prioritized.
