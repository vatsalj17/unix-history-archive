# V7addenda Source Code Archive

## Historical Overview
- **Release / Snapshot Date:** December 2, 1980
- **Authors & Key Contributors:** Bell Telephone Laboratories (Computing Science Research Center), with tools developed by Ted Kowalski, Alfred Aho, Brian Kernighan, Peter Weinberger, and Dennis Ritchie.
- **Target Machine Architecture:** DEC PDP-11 (PDP-11/45, PDP-11/70).
- **Historical Milestones:**
  - **The Official V7 Patch & Update Tape:** This directory is **not** a standalone operating system, but rather an official Bell Labs addenda and bugfix package distributed on tape to Seventh Edition licensees in December 1980.
  - **The Arrival of `fsck`:** The primary highlight of this addenda is Ted Kowalski's file system consistency check and repair utility, **`fsck`** (`fsck/fsck.c`). Prior to `fsck`, Seventh Edition administrators had to use primitive, disjointed tools (`icheck`, `dcheck`, `clri`) to manually recover corrupted filesystems after crashes. `fsck` introduced automated multi-pass structural verification, becoming the universal Unix filesystem repair tool.
  - **Line Printer Subsystem Restoration:** Seventh Edition tapes had omitted line printer spooling software. The addenda restored the missing kernel driver `lp.c`, the spooling daemon `lpr/lpd.c`, and the user command `lpr/lpr.c`.
  - **Compiler & Tool Fixes:** Provided an updated PDP-11 C compiler optimizer (`c2/`) fixing "overoptimism" bugs, a refreshed `awk` processor (`awk/`), robust magnetic tape drivers (`dev/`), and fixed string prefix matching in `fgrep.c`.
  - **Source Provenance:** Sourced from `v7addenda.tar.gz` in the Unix Archive, transcribed from the original December 1980 800 bpi tar distribution tape.

---

## Directory Tree & Navigation Map

```
V7addenda/
├── README            [Authentic Bell Labs release notes and patch catalog]
├── fgrep.c           [Patched fgrep command correcting prefix search bugs]
├── lp.c              [Missing kernel line printer driver for /usr/sys/dev/lp.c]
├── lpd.8             [Manual page for the line printer daemon]
├── malloc.c          [Updated malloc(3) memory allocator fixing rare pointer bug]
├── tmac.r            [Troff macro package simulating old roff in nroff]
├── awk/              [Complete refreshed awk pattern processor source]
├── c2/               [Patched PDP-11 C language peephole optimizer]
├── dev/              [Hardened magnetic tape drivers (ht.c, tm.c)]
├── f77/              [Updated Fortran 77 compiler and runtime libraries]
├── fsck/             [The landmark fsck filesystem verification and repair tool]
└── lpr/              [Complete line printer spooler, daemon, and build makefile]
```

### Subdirectory Breakdown
- `awk/`: The complete source code of an updated `awk` engine, including grammar (`awk.g.y`), lexical scanner (`awk.lx.l`), execution runtime (`run.c`), and AST parser (`parse.c`).
- `c2/`: The PDP-11 C language peephole optimizer passes (`c20.c`, `c21.c`, `c2.h`), revised to eliminate incorrect code generation.
- `dev/`: Replacement magnetic tape device drivers for TU16/TE16 (`ht.c`) and TM11 (`tm.c`) offering improved timeout and error recovery.
- `f77/`: Upgraded Fortran 77 compiler passes and mathematical runtime libraries (`libF77`, `libI77`).
- `fsck/`: Ted Kowalski's `fsck` program (`fsck.c`) and system administration manual (`fsck.1m`).
- `lpr/`: Line printer spooler subsystem containing `lpr.c` (spool command), `lpd.c` (daemon), `spool.c`, and character translation tables.

---

## Key Source Code Files

| Component / Subsystem | File Link | Description |
| :--- | :--- | :--- |
| **Filesystem Consistency Checker** | `fsck/fsck.c` | Ted Kowalski's original `fsck`: executes multi-phase validation (Phase 1: check blocks & sizes; Phase 2: check pathnames; Phase 3: check connectivity; Phase 4: check reference counts; Phase 5: check free list). |
| **Filesystem Checker Manual** | `fsck/fsck.1m` | Section 1M manual page detailing `fsck` flags (`-y`, `-n`, `-b`, `-s`), error diagnostics, and repair philosophy. |
| **Line Printer Spooler & Daemon** | `lpr/lpr.c`, `lpr/lpd.c` | The client utility `lpr` copying jobs to the spool directory, and background daemon `lpd` dequeuing print jobs and sending them to the printer. |
| **Kernel Line Printer Driver** | `lp.c` | Low-level line printer character driver intended for `/usr/sys/dev/lp.c`. |
| **C Peephole Optimizer** | `c2/c20.c`, `c2/c21.c` | The C optimizer passes scanning assembly instruction flows to eliminate redundant register moves, dead branches, and merge jumps. |
| **Refreshed Awk Processor** | `awk/main.c`, `awk/run.c` | Core entry point and expression evaluation interpreter for the updated `awk` system. |
| **Memory Allocator Patch** | `malloc.c` | Updated user-space heap allocator correcting boundary condition bugs in memory compaction. |
| **Distribution Release Notes** | `README` | Authentic Bell Labs release notes dated 12/2/80 listing all patches, installation directives, and remaining known bugs in V7. |

---

## Recommended Exploration Path

To study this historic Bell Labs patch package:

1. **Read the Authentic Bell Labs Notes:**
   Start with `README`. It provides a fascinating glimpse into 1980 software maintenance, explaining why line printers were missing, why C compiler optimizer passes had to be "cured of overoptimism", and listing unfixed bugs in `adb`, `exec`, and `dump`.
2. **Study the Architecture of `fsck`:**
   Read `fsck/fsck.c`. Trace how it walks inodes and superblock block lists in structured phases to reconstruct corrupted file systems.
3. **Examine C Compiler Peephole Optimization:**
   Read `c2/c20.c` to observe how early compiler optimizers matched assembly text patterns to strip redundant instructions.
4. **Inspect the Spooling Architecture:**
   Look at `lpr/lpr.c` and `lpr/lpd.c` to see the classic Unix daemon/client design for hardware printer sharing.
