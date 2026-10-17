# 2BSD Source Code Archive

## Historical Overview
- **Release Date:** May 1979
- **Key Developers:** Bill Joy, Chuck Haley, Kurt Shoens (Mail), Ken Arnold, Eric Schmidt (Berknet), Keith Sklower.
- **Machine Architecture:** DEC PDP-11 (running AT&T UNIX Version 6 or Version 7).
- **Historical Milestones:**
  - The second Berkeley Software Distribution (2BSD) expanded the Berkeley environment for PDP-11 systems, offering an unprecedented leap in interactive usability.
  - **The C Shell (`csh`):** Debut of Bill Joy's C-style shell, introducing interactive history substitution (`!`), aliases, job control prelude, tilde (`~`) expansion, and filename completion.
  - **Visual Mode Editor (`vi`):** Official debut of `vi` (version 2 of `ex`), introducing full-screen cursor movement, modal text manipulation, and macro definitions.
  - **`termcap` (Terminal Capabilities Database):** Joy created `termlib` and the `/etc/termcap` database, eliminating hardware lock-in by abstracting escape sequences across disparate visual display terminals (Lear Siegler ADM-3A, DEC VT52, Hazeltine).
  - **Berkeley Mail (`Mail`):** Kurt Shoens created the interactive `Mail` system, supporting message folders, aliases, subject headers, and reply chains.
  - **Berknet:** Eric Schmidt implemented an early network communication protocol (`src/net/`) allowing PDP-11 systems to exchange files and remote mail over serial lines.

---

## Directory Tree & Navigation Map
```
2BSD/
├── READ_ME          # Release notes and directory inventory
├── install.ms       # Detailed roff installation instructions for PDP-11 V6/V7
├── create           # Shell script to build distribution archives
├── setup            # Environment configuration script
├── tar.1, tar.ms    # Documentation for Berkeley's tar utility
├── tarit            # Script to write distribution files to magnetic tape
├── bin/             # Compiled binaries for V6 systems (ucb/, etc/, lib/)
├── doc/             # Troff source documentation (csh, vi, Mail, me, pascal, net)
├── man/             # Manual pages for Berkeley utilities and libraries
├── misc/            # Helper scripts and kernel patches (access.sys, fetchi.sys, ar.c)
├── src/             # Complete userland source tree
│   ├── csh/         # Bill Joy's C Shell source code
│   ├── ex/          # The Ex / Vi editor source code
│   ├── termlib/     # The original termcap library implementation
│   ├── Mail/        # Kurt Shoens' Mail user agent
│   ├── net/         # Eric Schmidt's Berknet PDP-11 networking subsystem
│   ├── me/          # The Berkeley -me roff macro package
│   ├── pascal/      # Berkeley Pascal system (pi0, pi1, px, pxp, pxref)
│   ├── eyacc/       # Enhanced Yacc parser generator
│   └── *.c, *.sh    # Standalone utilities (apropos, ctags, finger, head, etc.)
└── upgrade/         # Tools, headers, and libraries to upgrade V6 to support 2BSD
```

---

## Key Source Code Files
| File Path | Description & Architectural Significance |
| :--- | :--- |
| `src/csh/sh.c` | Main command execution loop and startup logic for `csh`. |
| `src/csh/sh.lex.c` | Lexical analyzer handling command lines and history expansion (`!`). |
| `src/csh/sh.exp.c` | Evaluator for C-style arithmetic and conditional expressions in scripts. |
| `src/ex/ex.c` | Main driver and command dispatcher for `ex` and `vi`. |
| `src/ex/ex_v.c` | Screen manager and visual display loop for full-screen `vi` editing. |
| `src/ex/ex_vops.c` | Visual operators (delete, change, yank, put, insert) implementation. |
| `src/termlib/termcap.c` | The foundational `tgetent()`, `tgetnum()`, `tgetflag()`, and `tgetstr()` APIs. |
| `src/termlib/tgoto.c` | Cursor addressing computation engine using termcap strings. |
| `src/Mail/main.c` | Entry point for Kurt Shoens' interactive mail program. |
| `src/net/net.c` | Eric Schmidt's Berknet daemon and packet routing routines. |

---

## Recommended Exploration Path
1. **Explore `termlib` (`src/termlib/`):**
   - Read `src/termlib/termcap.c` to see how Bill Joy solved terminal fragmentation with string capabilities.
2. **Study `vi`'s Visual Core (`src/ex/`):**
   - Read `src/ex/ex_v.c` to see how cursor movement and screen refreshes were implemented on slow 300–1200 baud terminal connections.
3. **Analyze the C Shell (`src/csh/`):**
   - Trace input tokenization in `src/csh/sh.lex.c` and expression evaluation in `src/csh/sh.exp.c`.
4. **Inspect Early Networking (`src/net/`):**
   - Read `src/net/net.c` to observe how Eric Schmidt built Berknet before IP networking was introduced to UNIX.
