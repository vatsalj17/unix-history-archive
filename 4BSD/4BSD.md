# 4BSD Source Code Archive

## Historical Overview
- **Release Date:** November 1980
- **Key Developers:** Bill Joy, Kirk McKusick, Eric Allman, Ken Arnold, Keith Sklower, CSRG.
- **Machine Architecture:** DEC VAX-11/780 and VAX-11/750.
- **Historical Milestones:**
  - 4BSD represented a major architectural enhancement over 3BSD on the VAX platform, cementing Berkeley UNIX as the premiere academic and research operating system.
  - **Job Control:** Bill Joy introduced full job control into the C Shell (`csh`), allowing users to suspend foreground processes (`Ctrl-Z`), run them in the background (`bg`), and recall them (`fg`, `jobs`).
  - **Eric Allman's `delivermail`:** The predecessor of Sendmail debuted here, providing multi-network mail routing between ARPANET, UUCP, and Berknet.
  - **Reliable Signals:** Addressed the historic UNIX signal race condition by implementing signal masking and blocking (`sigsetmask`, `sigpause`).
  - **Curses Library:** Ken Arnold developed the `curses` screen management library, enabling portable terminal applications with optimized redrawing.
  - **Hardware Autoconfiguration:** Implemented dynamic VAX UNIBUS and MASSBUS device discovery (`autoconf.c`).

---

## Directory Tree & Navigation Map
```
4BSD/
├── bin/             # Root binaries (sh, csh, ed)
├── dev/             # Device special node creation script (MAKEDEV)
├── etc/             # Administrative configuration files and daemons
├── usr/             # Main user directory
│   ├── src/         # System source tree
│   │   ├── sys/     # The 4BSD VAX kernel source
│   │   │   ├── sys/    # Machine-independent kernel routines
│   │   │   ├── conf/   # VAX kernel configuration files (GENERIC, VAX780)
│   │   │   ├── dev/    # VAX device drivers (dz, mba, uba, hp, ts, etc.)
│   │   │   ├── stand/  # VAX standalone boot and diagnostic utilities
│   │   │   └── h/      # Kernel system headers
│   │   ├── cmd/     # Complete userland command source (csh, delivermail, curses)
│   │   ├── libc/    # Standard C library implementation for VAX
│   │   ├── lib/     # Runtime language libraries (curses, termlib, libln, libF77)
│   │   └── games/   # BSD games suite
│   ├── include/     # Standard C include headers
│   ├── doc/         # CSRG documentation and technical reports
│   └── man/         # Section 1 through 8 manual pages
```

---

## Key Source Code Files
| File Path | Description & Architectural Significance |
| :--- | :--- |
| `usr/src/sys/sys/locore.s` | VAX low-level bootstrap, interrupt vectoring, and processor status register setup. |
| `usr/src/sys/sys/main.c` | Kernel entry point: memory detection, buffer pool allocation, autoconf trigger. |
| `usr/src/sys/sys/kern_sig.c` | Implementation of BSD's reliable signal mechanism and process stopping. |
| `usr/src/sys/sys/autoconf.c` | Probes VAX UNIBUS/MASSBUS adapters dynamically during bootstrap. |
| `usr/src/sys/sys/vm_sched.c` | Virtual memory page replacement clock algorithm and process swapping logic. |
| `usr/src/cmd/csh/sh.c` | C shell main loop with job control signal handling. |
| `usr/src/cmd/csh/sh.jobs.c` | Job control process management (`kill`, `stop`, `wait`, `fg`, `bg`). |
| `usr/src/cmd/delivermail/main.c` | Eric Allman's mail routing program handling heterogeneous network addresses. |
| `usr/src/lib/libcurses/` | Ken Arnold's screen optimization library source code. |

---

## Recommended Exploration Path
1. **Trace Job Control in the Kernel & Shell:**
   - Read `usr/src/sys/sys/kern_sig.c` to see how `SIGTSTP`, `SIGSTOP`, and `SIGCONT` were handled in the kernel.
   - Inspect `usr/src/cmd/csh/sh.jobs.c` to see how the C Shell managed job states and terminal process groups.
2. **Examine VAX Hardware Autoconfiguration:**
   - Read `usr/src/sys/sys/autoconf.c` to understand how the kernel probed VAX bus devices at boot time.
3. **Inspect the Birth of Sendmail:**
   - Read `usr/src/cmd/delivermail/main.c` to see how Eric Allman designed multi-protocol message forwarding.
4. **Study Curses (`usr/src/lib/libcurses/`):**
   - Explore how terminal screen differences were abstracted into an in-memory window matrix.
