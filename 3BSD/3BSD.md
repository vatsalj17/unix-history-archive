# 3BSD Source Code Archive

## Historical Overview
- **Release Date:** December 1979
- **Key Developers:** Bill Joy, Ozalp Babaoglu, Kirk McKusick, Bob Kridle.
- **Machine Architecture:** DEC VAX-11/780 (32-bit architecture).
- **Historical Milestones:**
  - 3BSD was the **first Berkeley Software Distribution to run on the 32-bit DEC VAX architecture**.
  - AT&T had created UNIX/32V for the VAX-11/780, but 32V lacked virtual memory paging—it merely swapped whole processes in and out of memory just like the 16-bit PDP-11 UNIX.
  - Graduate students **Ozalp Babaoglu and Bill Joy implemented full virtual memory paging** into 32V, adding demand paging, page replacement (the clock algorithm / page daemon), and address space virtualization using VAX page tables (`sys/sys/vm*.c`).
  - This breakthrough made 3BSD the world's premier operating system for the VAX, rapidly eclipsing DEC's own VMS in academic and research institutions.
  - Included the Berkeley Pascal compiler, Franz Lisp (essential for AI research at Berkeley), the C Shell, and `vi`.

---

## Directory Tree & Navigation Map
```
3BSD/
├── bin/             # Root binary utilities (sh, csh, ed)
├── etc/             # Essential administrative files, init, motd, passwd
├── usr/             # Primary system hierarchy
│   ├── src/         # System source tree
│   │   ├── sys/     # The historic 3BSD VAX virtual memory kernel
│   │   │   ├── sys/    # Machine-independent core & VM paging algorithms
│   │   │   ├── dev/    # VAX UNIBUS and MASSBUS peripheral device drivers
│   │   │   ├── stand/  # Standalone VAX bootloaders and formatters
│   │   │   ├── floppy/ # VAX console floppy disk diagnostic images
│   │   │   └── h/      # Kernel header files (<sys/*.h>)
│   │   ├── cmd/     # Complete 3BSD command suite (csh, ex, lisp, pascal, etc.)
│   │   ├── libc/    # Standard C library implementation for VAX-11
│   │   ├── lib/     # Additional language and math libraries
│   │   └── games/   # BSD games suite (trek, rogue, etc.)
│   ├── include/     # C standard headers
│   ├── doc/         # Technical papers on virtual memory, Franz Lisp, and C shell
│   ├── man/         # Section 1 through 8 manual pages
│   └── ucb/         # Berkeley user commands
```

---

## Key Source Code Files
| File Path | Description & Architectural Significance |
| :--- | :--- |
| `usr/src/sys/sys/locore.s` | VAX-11 assembly bootstrap, System Control Block (SCB) initialization, page table register configuration, and interrupt vectors. |
| `usr/src/sys/sys/main.c` | Kernel initialization: clears BSS, sizes physical memory, sets up swap devices, and starts process 0. |
| `usr/src/sys/sys/vm.c` | Babaoglu & Joy's core virtual memory demand-paging algorithms. |
| `usr/src/sys/sys/vmsched.c` | The historic "clock" page daemon algorithm (`pagedaemon`), scanning page use bits and reclaiming frames. |
| `usr/src/sys/sys/vmpt.c` | VAX page table management, translation buffer invalidation, and page allocation. |
| `usr/src/sys/sys/trap.c` | VAX hardware trap dispatcher, translating translation faults (`transfault`) into demand paging. |
| `usr/src/sys/sys/machdep.c` | VAX machine-dependent memory sizing, dump routines, and processor registers. |
| `usr/src/cmd/csh/` | Source code for the 3BSD C Shell. |
| `usr/src/cmd/lisp/` | Franz Lisp interpreter and compiler source. |

---

## Recommended Exploration Path
1. **Understand VAX Low-Level Initialization (`usr/src/sys/sys/locore.s`):**
   - Read `locore.s` to see how the VAX System Control Block (SCB) is established and how control transfers to C.
2. **Study the Birth of Demand Paged Virtual Memory:**
   - Read `usr/src/sys/sys/vm.c`, `vmpt.c`, and `vmsched.c`. Observe the clock page replacement algorithm that became the archetype for virtual memory in modern operating systems.
3. **Inspect Fault Handling (`usr/src/sys/sys/trap.c`):**
   - Follow how a VAX translation fault (page fault) triggers page-in from disk swap space.
4. **Examine Userland (`usr/src/cmd/`):**
   - Check out `usr/src/cmd/lisp/` to see the Franz Lisp implementation that fueled early AI projects.
