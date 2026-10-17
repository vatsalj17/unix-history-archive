# LSX Source Code Archive

## Historical Overview

| Attribute | Details |
| :--- | :--- |
| **System Name** | LSX (LSI-11 UNIX) |
| **Release Date** | 1976 (Developed at Bell Labs; restored in 2006 by Leonid Broukhis) |
| **Origin / Creators** | Bell Laboratories (Murray Hill, NJ) — Heinz Lycklama |
| **Source Donation** | Recovered and donated to TUHS by Leonid Broukhis via the LSX Restoration Project |
| **Target Architecture** | DEC LSI-11 (PDP-11/03) microprocessor (no MMU, 20K–28K words / 40KB–56KB total RAM) |
| **Key Historical Milestones** | First true microcomputer UNIX; ran in ~12K words with no hardware memory management unit; single-process swapping model; software emulation of missing CPU math instructions |

### Significance & Historical Context
In the mid-1970s, Digital Equipment Corporation introduced the LSI-11 (PDP-11/03), bringing PDP-11 CPU architecture to a low-cost, single-board microprocessor. However, unlike full minicomputers (PDP-11/40, 11/45, 11/70), the LSI-11 was severely crippled:
1. It possessed **no Memory Management Unit (MMU)**—meaning physical memory was directly addressed with zero hardware protection between kernel and userland.
2. It lacked the **Extended Instruction Set (EIS)**—hardware multiply (`mul`), divide (`div`), and multi-bit arithmetic shifts (`ash`) were missing.
3. System RAM was often limited to a meager 20K to 28K words (40KB to 56KB total address space).

Heinz Lycklama at Bell Labs set out to adapt Sixth Edition UNIX (V6) to run on this tiny microcomputer, producing **LSX** (alongside Mini-Unix). Because memory was so scarce, Lycklama made drastic architectural trade-offs:
- **Single Process In-Core**: Only **one** user process could reside in physical RAM at any time. When a context switch occurred, the running process was completely swapped out to floppy disk or drum, and the next process swapped in.
- **File-Backed Interprocess Communication**: UNIX pipes, which normally occupied precious kernel memory buffers, were redirected to write to temporary hidden disk files.
- **Instruction Emulation (`emul.s`)**: Whenever compiled C code executed a `mul` or `div` instruction, the CPU triggered an illegal instruction trap. LSX intercepted the trap and emulated the arithmetic instruction in software before resuming user execution.

For decades, LSX was feared permanently lost because it was treated as internal Bell Labs experimental firmware. In 2006, Leonid Broukhis recovered raw floppy disk images and reconstructed the complete source tree.

---

## Directory Tree & Navigation Map

```
LSX/
├── src/                         # Userland utilities, assembler, and C compiler
│   ├── aedboot.s, boot.s        # Floppy bootstrap loaders
│   ├── bootinstruct             # Manual instructions for booting from floppy
│   ├── as11.s ... as29.s        # Multi-pass PDP-11 assembler source
│   ├── cc.c, cc                 # C compiler driver
│   ├── check.c, clri.s, date.c  # Basic utilities (check, clri, date, cp, ls)
│   └── cc/                      # C compiler passes
├── sys/                         # Complete LSX Microcomputer Kernel Tree
│   ├── aedfd.c                  # AED floppy disk driver
│   ├── decfd.c                  # DEC RX01 / RX02 floppy disk driver
│   ├── emul.s                   # Software emulation for missing EIS instructions (mul/div)
│   ├── header.s                 # Master assembly conditional flags (LSI, EIS, CLOCK)
│   ├── header.dec.s, rf.s, lsx.s# Variant configuration headers
│   ├── low.s                    # Low-core vector table and trap vectors
│   ├── main.c                   # Kernel entry point and memory partitioning
│   ├── mch.s                    # Low-level LSI-11 machine assembly
│   ├── perfd.c, rffd.c, sykfd.c # PerSci, RF, and Sykes disk/floppy drivers
│   ├── slp.c                    # Single-process swap scheduler
│   └── sys1.c ... sys4.c        # Scaled-down UNIX system call handlers
└── tmp/                         # Temporary scratch directory
    └── add.c                    # Extraneous developer scratch script
```

---

## Key Source Code Files

### 1. Hardware Traps & Instruction Emulation
- `sys/emul.s`: The instruction emulation engine. Traps illegal instruction faults on the LSI-11 CPU, decodes the opcode byte from the faulting program counter, and emulates `mul`, `div`, `ash`, and `ashc` in software using register stacks.
- `sys/header.s`: Master assembly flag definitions configuring processor type (`LSI = 1`), instruction set features (`EIS = 0`), console UART settings (`KL = 1`), and clock options.

### 2. Microcomputer Bootstrapping & Machine Layer
- `sys/low.s`: Trap vector definitions at memory address 0. Directs CPU interrupts, power-fail traps, and hardware clock ticks to kernel handlers.
- `sys/mch.s`: Machine-language support routines implementing register saving, stack pointer adjustment, and CPU priority masking (`mtps` / `mfps`).
- `sys/main.c`: Kernel initialization. Calculates available physical memory (typically setting the user memory limit directly above kernel text/data) and launches the initial shell.

### 3. Memory-Constrained Scheduling & Disk IPC
- `sys/slp.c`: The swapping scheduler. Unlike multi-process UNIX where processes share memory, LSX coordinates saving the entire active user core image to disk before reading in the next candidate.
- `sys/decfd.c`: DEC RX01 / RX02 single-density floppy disk controller driver, handling sector interleaving and track stepping.

---

## Recommended Exploration Path

1. **Inspect Architectural Flags**:
   - Open `sys/header.s` to see how an OS was configured for a machine lacking basic arithmetic hardware.
2. **Study Software Instruction Emulation**:
   - Read `sys/emul.s` to witness the ingenious method Lycklama used to decode PDP-11 instructions on-the-fly when the hardware threw an illegal opcode trap.
3. **Trace the Single-Process Model**:
   - Compare `sys/slp.c` and `sys/main.c` with standard V6 `slp.c`. Notice how the absence of an MMU forced swapping the solitary user address space.
4. **Examine Floppy Storage Drivers**:
   - Read `sys/decfd.c` to understand how the system executed all OS I/O and process swapping over 250KB floppy disk media.
5. **Review Bootstrap Instructions**:
   - Read `src/bootinstruct` to see how operators toggled boots into LSI-11 hardware registers.
