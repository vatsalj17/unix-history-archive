# 1BSD Source Code Archive

## Historical Overview
- **Release Date:** March 1978 (early 1978)
- **Key Developers:** Bill Joy, Chuck Haley, Eric Allman, Ken Thompson (visiting faculty 1975–1976), Computer Systems Research Group (CSRG) precursors at UC Berkeley.
- **Machine Architecture:** DEC PDP-11 (specifically models 11/40, 11/45, and 11/70) running AT&T UNIX Version 6 (V6).
- **Historical Milestones:**
  - 1BSD was the very first Berkeley Software Distribution. It was not a standalone operating system, but an add-on distribution tape mailed to fellow UNIX license holders for $50 (covering the cost of the magnetic tape and shipping). Approximately thirty copies were distributed.
  - Introduced the instructional **Berkeley Pascal** system: the `pi` compiler, `px` bytecode interpreter, `pxp` profiler and execution formatter, and `pxref` cross-reference generator.
  - Debut of Bill Joy's **`ex` text editor** (version 1.1), the direct ancestor of `vi`. Joy enhanced standard AT&T `ed` with line addressing, regular expressions, crash-recovery daemons (`exrecover`, `expreserve`), and an early "open" visual mode.
  - Introduced **`ashell`** (the ancestor of the C Shell), an experimental prototype shell with early command history and alias concepts.
  - Included **`eyacc`** (Enhanced Yacc by Bill Joy), tuned to generate Pascal parsers with improved error diagnostic recovery.
  - Included Eric Allman's legendary Star Trek tactical simulation game (**`trek`**).

---

## Directory Tree & Navigation Map
```
1BSD/
├── READ_ME          # Original distribution tape announcement and unpacking instructions
├── TAPE             # Tape block layout and magnetic archive mapping description
├── UPDATE           # Notes on software fixes and version updates from V6 tools
├── ashell/          # Joy & Haley's prototype Berkeley shell (ancestor of csh)
├── assubs/          # Low-level assembly runtime routines (crt0.s, mcrt0.s, pcrt0.s)
├── bin/             # User command binaries and scripts (box, cxref, fixit, pss)
├── etc/             # System configuration tables and early terminal database (ttycap)
├── ex-1.1/          # Source code for the Ex Editor Version 1.1
├── exrecover/       # Ex editor crash preservation and recovery daemons
├── exrefm/          # Ex reference manual source in troff/nroff format
├── eyacc/           # Bill Joy's Enhanced Yacc parser generator source
├── fpterp/          # Floating-point interpreter assembly routines for PDP-11
├── help/            # On-line help documentation files for ex, edit, and Pascal
├── lib/             # Support files, strings tables (pi_strings), and Pascal guides
├── man1/ - man8/    # Manual pages for sections 1 (commands) through 8 (admin)
├── opcodes/         # Pascal bytecode definitions, opcode names, and generator scripts
├── pcs/             # Pascal interpreter source and micro-interpreters
├── pi/              # Pascal compiler frontend (Yacc grammar, AST construction)
├── portlib/         # Portable I/O runtime library for PDP-11
├── printd           # Script to print files on line printers
├── puman/           # Pascal User's Manual source and test programs
├── px/              # Pascal runtime executor (PDP-11 assembly interpreter)
├── pxp/             # Pascal execution profiler and pretty-printer
├── pxref/           # Pascal cross-reference program
├── s1/, s6/, s7/, s8/ # Recompiled & enhanced V6 commands organized by manual section
├── setup            # Shell script to configure tape extraction paths
├── tests/           # Pascal test suite (eightqueens.p, ancestor.p, etc.)
└── trek/            # Eric Allman's Star Trek tactical simulation game source
```

---

## Key Source Code Files
| File Path | Description & Architectural Significance |
| :--- | :--- |
| `ex-1.1/ex.c` | Core command loop, dispatch, and buffer management for the `ex` line editor. |
| `ex-1.1/ex_vmain.c` | Early implementation of the "open" (visual) terminal mode inside `ex`. |
| `ashell/sh.c` | Entry point for Joy's prototype Berkeley shell, managing commands and processes. |
| `ashell/sh_lex.c` | Lexical analyzer for the prototype shell, showing early history substitutions. |
| `pi/main.c` | The Berkeley Pascal compiler frontend driver and argument parsing. |
| `pi/yylex.c` | Fast lexical scanner for Pascal with intelligent error diagnostics. |
| `px/00head.s` | PDP-11 assembly header and register setup for the Pascal interpreter engine. |
| `px/00int.s` | Core dispatch loop for executing Pascal P-code bytecode instructions. |
| `eyacc/ey0.c` | Bill Joy's modified Yacc parser generator, expanded to parse Pascal. |
| `trek/main.c` | Eric Allman's Star Trek simulation game engine entry point. |
| `etc/ttycap` | Precursor to `termcap`, cataloging terminal escape codes for screen control. |

---

## Recommended Exploration Path
1. **Start with the Ex Editor (`ex-1.1/`):**
   - Read `ex-1.1/READ_ME` to understand Bill Joy's motivations.
   - Inspect `ex-1.1/ex.c` and `ex-1.1/ex_vmain.c` to see how Joy transformed AT&T's `ed` into a responsive terminal-oriented editor.
2. **Explore the Roots of the C Shell (`ashell/`):**
   - Read `ashell/sh.c` and `ashell/sh_lex.c` to see how command parsing and the history mechanism were originally conceived before becoming `csh` in 2BSD.
3. **Investigate the Pascal System (`pi/` & `px/`):**
   - Trace compilation in `pi/main.c` and `pi/yylex.c`.
   - Trace execution in `px/00int.s` to see how a bytecode virtual machine was implemented in PDP-11 assembly.
4. **Inspect Terminal Capabilities (`etc/ttycap`):**
   - Look at `etc/ttycap` to observe the earliest attempts to manage multiple CRT terminal types before `termcap` was formalized.
