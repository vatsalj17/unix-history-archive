# Landmark Tools & Subsystems: A Source-Code Field Guide

**[← Back to OS Catalog (`README.md`)](README.md)**

While [`README.md`](README.md) organizes this archive chronologically across 51 operating system releases, this guide cuts across the repository **by invention**.

Every tool and kernel subsystem you rely on today (from piping `stdout` and `fork()`-ing processes to copy-on-write paging, `vi` motions, and TCP congestion windows) started as a surprisingly small C or assembly file written on machines with kilobytes of RAM. Below is the map to where those ideas were first committed to disk, with direct links to both the **original source files** and each system's **architecture guide**.

---

## Directory Index

| Section | Landmarks Included | Era Span |
| :--- | :--- | :--- |
| **[1. Shells & Command Interpreters](#1-shells--command-interpreters)** | Thompson `sh`, Mashey `sh`, Bourne `sh`, `csh`, `ash` | 1971 to 1996 |
| **[2. Text Editors & Document Engines](#2-text-editors--document-engines)** | `ed`, `ex` & `vi`, `sed`, `sam`, `troff` & `nroff` | 1970 to 1986 |
| **[3. Compilers, Interpreters & Parser Generators](#3-compilers-interpreters--parser-generators)** | B, Ritchie `cc`, `pcc`, `cfront` (C++), `yacc`, `lex`, `awk`, BSD Pascal | 1970 to 1989 |
| **[4. Build Systems & Revision Control](#4-build-systems--revision-control)** | `make`, SCCS, `diff` & `diff3` | 1975 to 1979 |
| **[5. Core Utilities & Standard Libraries](#5-core-utilities--standard-libraries)** | `grep` & `egrep`, `tar`, `stdio.h`, `cron`, `lint` | 1975 to 1979 |
| **[6. Filesystems & Storage Subsystems](#6-filesystems--storage-subsystems)** | Inodes (`namei`), `pipe()`, Fast File System (FFS), VFS & `vnode`, NFSv2, `/proc`, `ext`, ZFS | 1970 to 2010 |
| **[7. Networking & Protocols](#7-networking--protocols)** | BBN TCP/IP & `mbuf`, Berkeley Sockets, `sendmail`, Van Jacobson TCP, OpenSSH | 1981 to 2009 |
| **[8. Security & Cryptography](#8-security--cryptography)** | Salted `crypt()`, OpenBSD `pf`, `sudo` | 1979 to 2009 |
| **[9. Schedulers, Virtual Memory & Observability](#9-schedulers-virtual-memory--observability)** | Fair-Share (`lnode`), 3BSD Paging & `vfork()`, `mmap()`, Linux 0.01, DTrace, CFS & KVM | 1979 to 2010 |

---

## 1. Shells & Command Interpreters

### The Thompson Shell (`sh.s`)
> **Distro:** [First Edition Unix (1971)](V1/V1.md) · **Author:** Ken Thompson  
> **Source:** [`V1/sh.s`](V1/sh.s)

Written in raw PDP-11 assembly, this is the primordial Unix command loop. It gave the world `<` and `>` I/O redirection (implemented by closing FD `0` or `1` after `fork()` so the next `open()` automatically grabs the lowest unused slot) and `&` for background jobs. Control flow didn't even live inside the shell yet; `if` and `goto` were standalone external binaries (`/bin/if` and `/bin/goto`) that manipulated the seek offset (`lseek`) of the script's open file descriptor directly.

### The PWB Mashey Shell (`sh.c`)
> **Distro:** [PWB/UNIX 1.0 (1977)](PWB1/PWB1.md) · **Author:** John Mashey  
> **Source:** [`PWB1/sys/source/s2/sh.c`](PWB1/sys/source/s2/sh.c)

Forking an external `/bin/if` binary for every conditional was killing performance on Bell Labs' shared PDP-11s. Mashey rewrote the shell in C, pulled `if`/`then`/`else`/`goto` into internal shell state, introduced shell variables (`$a` through `$z`), and invented `$p` (the precursor to `$PATH`). It turned the shell from a simple program launcher into an automation engine.

### The Bourne Shell (`sh`)
> **Distro:** [Seventh Edition Unix (1979)](V7/V7.md) · **Author:** Stephen R. Bourne  
> **Source:** [`V7/usr/src/cmd/sh/main.c`](V7/usr/src/cmd/sh/main.c) · **ALGOL Macros:** [`V7/usr/src/cmd/sh/mac.h`](V7/usr/src/cmd/sh/mac.h) · **Allocator:** [`stak.c`](V7/usr/src/cmd/sh/stak.c)

The direct ancestor of every POSIX `sh`, Bash, and Zsh script running today. Bourne introduced environment variable inheritance (`environ`), arbitrary file descriptor plumbing (`2>&1`), proper control structures (`for`, `while`, `case`), command substitution (`` `cmd` ``), and here-documents (`<<`).

Bourne missed working on ALGOL-68 so much that he abused the C preprocessor in [`mac.h`](V7/usr/src/cmd/sh/mac.h) to turn C into ALGOL (`#define IF if(`, `#define THEN ){`, `#define FI ;}`). Even wilder is [`stak.c`](V7/usr/src/cmd/sh/stak.c): instead of calling `malloc()`, Bourne wrote a bump allocator that intentionally let pointers run off the end of the process data segment, caught the kernel's `SIGSEGV` fault, called `sbrk()` inside the signal handler to grow the heap, and resumed execution right where it faulted.

### The C Shell (`csh`)
> **Distro:** [2BSD (1979)](2BSD/2BSD.md) & [3BSD (1980)](3BSD/3BSD.md) · **Author:** Bill Joy  
> **Source:** [`2BSD/src/csh/sh.c`](2BSD/src/csh/sh.c) · [`3BSD/usr/src/cmd/csh/sh.c`](3BSD/usr/src/cmd/csh/sh.c)

Bourne won scripting, but Bill Joy won interactive terminal ergonomics. `csh` invented almost every interactive feature terminal power users rely on: command history substitution (`!`), aliases, tilde expansion (`~`), directory stacks (`pushd`/`popd`), and interactive job control (`Ctrl+Z`, `fg`, `bg`) wired into the new BSD job-control signal architecture (`SIGTSTP`, `SIGCONT`, `SIGCHLD`).

### The Almquist Shell (`ash`)
> **Distro:** [The Net/2 Distribution (1991)](Net2/Net2.md) & [Minix 2.0 (1996)](Minix2.0/Minix2.0.md) · **Author:** Kenneth Almquist  
> **Source:** [`Net2/usr/src/bin/sh/main.c`](Net2/usr/src/bin/sh/main.c) · [`Minix2.0/src/commands/ash/main.c`](Minix2.0/src/commands/ash/main.c)

A fast, brutally lean, clean-room reimplementation of the System V Bourne shell written to bypass AT&T licensing. No bloat, tiny memory footprint, and near-zero startup overhead. Ported to Linux and Debian as `dash`, it remains the default `/bin/sh` on Ubuntu/Debian and the root shell on FreeBSD and NetBSD.

---

## 2. Text Editors & Document Engines

### `ed`: The Original Line Editor
> **Distro:** [PDP-7 Unix (1970)](PDP7-Unix/PDP7-Unix.md) & [First Edition (1971)](V1/V1.md) · **Author:** Ken Thompson  
> **Source:** [`PDP7-Unix/cmd/ed1.s`](PDP7-Unix/cmd/ed1.s)

Built for noisy Teletype Model 33 terminals printing onto physical paper rolls at 10 characters per second, where redrawing a screen was physically impossible. `ed` packs Thompson's legendary 1968 regular expression engine into tight PDP-7 assembly, compiling regex patterns on the fly into an in-memory state machine. Its command syntax (`s/pattern/replacement/`, `g/re/p`) is the DNA of both `sed` and `grep`.

### `ex` and `vi`: The Visual Editor
> **Distro:** [1BSD (1978)](1BSD/1BSD.md) & [2BSD (1979)](2BSD/2BSD.md) · **Author:** Bill Joy  
> **Source:** [`1BSD/ex-1.1/ex.c`](1BSD/ex-1.1/ex.c) · **Visual Mode Entry:** [`2BSD/src/ex/ex_vmain.c`](2BSD/src/ex/ex_vmain.c) · **Motions Engine:** [`2BSD/src/ex/ex_vops.c`](2BSD/src/ex/ex_vops.c)

Working over sluggish 300-baud modems on a Lear Siegler ADM-3A CRT terminal (where the `Escape` key sat right next to `Q`, `~` was printed on the `Home` key, and `h`, `j`, `k`, `l` had physical directional arrows on the keycaps), Bill Joy hacked a full-screen visual mode (`:vi`) onto his `ex` line editor. To keep it responsive over dial-up, he built `termcap` and engineered partial screen redraws that minimized the exact number of bytes sent over the serial line. Every modal motion you fire in Vim or Neovim today traces straight back to `ex_vops.c`.

### `sed`: Stream Editor
> **Distro:** [Seventh Edition Unix (1979)](V7/V7.md) · **Author:** Lee E. McMahon  
> **Source:** [`V7/usr/src/cmd/sed/sed0.c`](V7/usr/src/cmd/sed/sed0.c)

Files were growing larger than PDP-11 core memory could hold inside an `ed` buffer. McMahon ripped the address-matching and regex substitution engine out of `ed` and turned it into a non-interactive pipeline state machine that transforms arbitrary-length byte streams one line at a time through a pattern space and hold buffer.

### `sam`: Structural Regular Expressions
> **Distro:** [Ninth Edition Unix (1986)](V9/V9.md) · **Author:** Rob Pike  
> **Source:** [`V9/jtools/src/sam/sam.c`](V9/jtools/src/sam/sam.c)

Rob Pike got tired of line-oriented Unix tools choking on multi-line C structs. Written for the Bell Labs Blit bitmapped terminal, `sam` split the editor into two processes: a deterministic state-machine backend that could run on a remote host and a thin graphical frontend running locally over a serial stream. It introduced *structural regular expressions*, using composable loops (`x`, `y`, `g`, `v`) to slice and filter arbitrary regions of text regardless of newlines, paving the way for Plan 9's `acme`.

### `troff` and `nroff`: Document Typesetting Engine
> **Distro:** [Fifth Edition (1974)](V5/V5.md) & [Seventh Edition (1979)](V7/V7.md) · **Authors:** Joe Ossanna, Brian Kernighan  
> **Source:** [`V7/usr/src/cmd/troff/t10.c`](V7/usr/src/cmd/troff/t10.c)

The program that literally bought Bell Labs its first PDP-11/20. Management refused to fund an OS research machine after Multics, so Thompson and Ritchie promised to build a text-typesetting system for the Bell Labs patent department. Ossanna wrote `roff` in assembly, evolved it into `troff` to drive a Graphic Systems CAT phototypesetter, and Kernighan later overhauled it in C. Every `man` page rendered in your terminal still speaks its macro language.

---

## 3. Compilers, Interpreters & Parser Generators

### Ken Thompson's B Language
> **Distro:** [PDP-7 Unix (1970)](PDP7-Unix/PDP7-Unix.md) · **Author:** Ken Thompson  
> **Source:** [`PDP7-Unix/cmd/ind.b`](PDP7-Unix/cmd/ind.b) · [`PDP7-Unix/cmd/lcase.b`](PDP7-Unix/cmd/lcase.b)

Rare surviving source code of **B**, Thompson's stripped-down, typeless derivative of BCPL squeezed into the tiny memory footprint of an 18-bit PDP-7. In B, there is only one data type: the machine word. Pointers, integers, and arrays are all treated as raw word offsets. It worked well on the word-addressed PDP-7, until Bell Labs bought a byte-addressable PDP-11, where handling byte-packed characters in a typeless word-addressed language became painful enough to push Dennis Ritchie to invent C's type system.

### Dennis Ritchie's Original C Compiler (`cc`)
> **Distro:** [Second Edition Unix (1972)](V2/V2.md) · **Author:** Dennis M. Ritchie  
> **Source:** [`V2/c/nc0/c00.c`](V2/c/nc0/c00.c)

The Genesis block of modern systems programming: `/* C compiler Copyright 1972 Bell Telephone Laboratories, Inc. */`. Written in an embryonic dialect of C before `struct` members had their own namespaces, before `#include` preprocessors existed, and when compound assignment operators were still written backwards (`=+`, `=-`, `=*`), which created notorious lexing ambiguities whenever someone wrote `x=-1` without spaces.

### The Portable C Compiler (`pcc`)
> **Distro:** [Seventh Edition Unix (1979)](V7/V7.md) · **Author:** Stephen C. Johnson  
> **Source:** [`V7/usr/src/cmd/pcc/code.c`](V7/usr/src/cmd/pcc/code.c) · [`V7/usr/src/cmd/pcc/order.c`](V7/usr/src/cmd/pcc/order.c)

Ritchie's original `cc` was tightly coupled to PDP-11 addressing modes and registers. Johnson engineered `pcc` around a machine-independent front end that emitted expression trees, paired with a table-driven Sethi-Ullman code generator backend. Swapping out roughly 25% of the files let you retarget C to a completely new CPU architecture (like the [Interdata 7/32](Interdata732/Interdata732.md) or [VAX-11/780](32V/32V.md)), turning Unix from a DEC-specific OS into portable software.

### Cfront: The Original C++ Translator
> **Distro:** [Tenth Edition Unix (1989)](V10/V10.md) · **Author:** Bjarne Stroustrup  
> **Source:** [`V10/cmd/cfront/cfront/main.c`](V10/cmd/cfront/cfront/main.c) · **Streams:** [`stream.c`](V10/cmd/cfront/libC/iostream/stream.c) · **Allocator:** [`_new.c`](V10/cmd/cfront/libC/new/_new.c)

Before C++ had native assembly backends, it had Cfront: a full compiler front end (written in C++ itself) that performed complete type checking, resolved classes and virtual inheritance, built an AST, and emitted raw K&R C source code for `cc` to compile. Reading this directory reveals the exact birth of `vtable` function pointer arrays, symbol name mangling, `iostream`, and the original `operator new` wrapper around `malloc()`.

### Yacc: Yet Another Compiler-Compiler
> **Distro:** [Sixth Edition (1975)](V6/V6.md) & [Seventh Edition (1979)](V7/V7.md) · **Author:** Stephen C. Johnson  
> **Source:** [`V6/usr/source/yacc/source/y1.c`](V6/usr/source/yacc/source/y1.c)

Johnson wanted to add an `XOR` (`^`) operator to Ritchie's B compiler and found the hand-rolled parser painful to modify. He took Donald Knuth's theoretical LR parsing math, applied Frank DeRemer's LALR(1) state-table compression tricks so the parser tables wouldn't blow out PDP-11 RAM, and created `yacc`. Suddenly, specifying a new language grammar took an afternoon instead of a month.

### Lex: Lexical Analyzer Generator
> **Distro:** [Seventh Edition Unix (1979)](V7/V7.md) · **Authors:** Mike Lesk, Eric Schmidt  
> **Source:** [`V7/usr/src/cmd/lex/lmain.c`](V7/usr/src/cmd/lex/lmain.c)

Co-written in the summer of 1976 by a Bell Labs intern named Eric Schmidt (decades before becoming CEO of Google). `lex` compiles regular expression token definitions into deterministic finite automaton (DFA) state-transition tables emitted directly as C arrays, feeding lexed tokens straight into `yacc`.

### AWK: Pattern Scanning & Processing Language
> **Distro:** [Seventh Edition Unix (1979)](V7/V7.md) · **Authors:** Alfred Aho, Peter Weinberger, Brian Kernighan  
> **Source:** [`V7/usr/src/cmd/awk/awk.g.y`](V7/usr/src/cmd/awk/awk.g.y) · [`main.c`](V7/usr/src/cmd/awk/main.c) · [`run.c`](V7/usr/src/cmd/awk/run.c)

What happens when a formal-languages theorist (Aho), a systems/database hacker (Weinberger), and the master of clean C prose (Kernighan) share adjacent offices. Built on a `yacc` grammar (`awk.g.y`), AWK gave Unix shell pipelines associative arrays (hash tables as a first-class primitive), implicit file/line iteration loops, and automatic field splitting (`$1`, `$2`), directly inspiring Perl, Lua, and Python.

### Berkeley Pascal (`pi`, `px`, `pxp`)
> **Distro:** [1BSD (1978)](1BSD/1BSD.md) · **Authors:** Bill Joy, Chuck Haley, Ken Thompson  
> **Source:** [`1BSD/pi/main.c`](1BSD/pi/main.c) · [`1BSD/px/`](1BSD/px/)

The Trojan horse that launched BSD. When Ken Thompson visited UC Berkeley on sabbatical in 1975, he helped two grad students (Joy and Haley) write a blazing-fast Pascal bytecode compiler (`pi`) and stack-machine interpreter (`px`). Other universities wanted the compiler so badly that Joy started mailing out magnetic tapes of their local add-ons, creating the very first Berkeley Software Distribution (`1BSD`).

---

## 4. Build Systems & Revision Control

### `make`: Dependency Graph Build Engine
> **Distro:** [Seventh Edition Unix (1979)](V7/V7.md) · **Author:** Stuart Feldman  
> **Source:** [`V7/usr/src/cmd/make/main.c`](V7/usr/src/cmd/make/main.c)

Born out of frustration after Steve Johnson spent an entire morning debugging a compiler bug, only to realize he had forgotten to recompile a modified `.c` file into the binary. Feldman went home and wrote `make` over a single weekend: a directed acyclic graph (DAG) evaluator that inspects `stat()` modification timestamps (`mtime`) to recompile only the exact targets downstream of a change.

*(Why does `make` strictly require leading `TAB` characters? Feldman used a quick-and-dirty lexing check in his first prototype; by the time he realized leading tabs were a terrible UI trap, he already had a "massive" user base of about a dozen Bell Labs colleagues and refused to break their Makefiles).*

### SCCS: Source Code Control System
> **Distro:** [PWB/UNIX 1.0 (1977)](PWB1/PWB1.md) · **Author:** Marc J. Rochkind  
> **Source:** [`PWB1/sys/source/sccs4/cmd/admin.c`](PWB1/sys/source/sccs4/cmd/admin.c) · [`delta.c`](PWB1/sys/source/sccs4/cmd/delta.c) · [`get.c`](PWB1/sys/source/sccs4/cmd/get.c)

The first real version control system in computing history. Instead of copying whole source files on every revision, SCCS invented *interleaved weave deltas* (`s.` files), storing lines tagged with birth and death revision numbers so any historical version could be reconstructed in a single forward pass. Ancestor to RCS, CVS, SVN, BitKeeper, and Git.

### `diff` and `diff3`: LCS & 3-Way Merging
> **Distro:** [Sixth Edition (1975)](V6/V6.md) & [Seventh Edition (1979)](V7/V7.md) · **Authors:** Douglas McIlroy, James W. Hunt  
> **Source:** [`V6/usr/source/s1/diff1.c`](V6/usr/source/s1/diff1.c) · [`V7/usr/src/cmd/diff.c`](V7/usr/src/cmd/diff.c) · **3-Way Merge:** [`V7/usr/src/cmd/diff3.c`](V7/usr/src/cmd/diff3.c)

Solving the Longest Common Subsequence (LCS) problem naively takes $O(M \times N)$ memory, which immediately blew out a 16-bit PDP-11's 64KB address space. Hunt and McIlroy hashed every line of text into a 16-bit integer, sorted equivalence classes, and searched "k-candidates" in near-linear time. Later, McIlroy wrote `diff3.c`, inventing automated 3-way reconciliation between two diverged files and their common ancestor: the exact algorithmic backbone behind `git merge` and `git rebase` today.

---

## 5. Core Utilities & Standard Libraries

### `grep` & `egrep`: Regular Expression Search
> **Distro:** [Sixth Edition (1975)](V6/V6.md) & [Seventh Edition (1979)](V7/V7.md) · **Authors:** Ken Thompson (`grep`), Alfred Aho (`egrep`)  
> **Source:** [`V6/usr/source/s1/grep.c`](V6/usr/source/s1/grep.c) · [`V7/usr/src/cmd/egrep.y`](V7/usr/src/cmd/egrep.y)

Doug McIlroy asked Thompson if he had a tool to search large dictionary files for patterns. Thompson claimed he stayed up all night writing a brand-new utility, when in reality he spent an hour ripping the `g/re/p` (Global / Regular Expression / Print) routine out of `ed` and wrapping it in `main()`. Four years later, Aho wrote `egrep` (`egrep.y`) using a deterministic finite automaton (DFA) that constructs state transitions lazily in memory, giving us full alternation (`|`) and `+`/`?` quantifiers with guaranteed $O(N)$ linear-time scanning and zero catastrophic backtracking.

### `tar`: Tape Archiver
> **Distro:** [Seventh Edition Unix (1979)](V7/V7.md) · **Author:** John Fuller  
> **Source:** [`V7/usr/src/cmd/tar/tar.c`](V7/usr/src/cmd/tar/tar.c)

Built to stream directory trees continuously onto magnetic tape drives without seeking backwards. Open `tar.c` and look at `union hblock`: that 512-byte header block (storing permissions, UID, file size, and `mtime` as ASCII octal strings so archives stayed endian-independent across machines) is still the exact binary format unpacking every `.tar.gz`, OCI Docker image layer, and source release on your machine today.

### `stdio.h`: Buffered Standard I/O
> **Distro:** [Seventh Edition Unix (1979)](V7/V7.md) · **Author:** Dennis Ritchie (succeeding Mike Lesk's Portable I/O)  
> **Source:** [`V7/usr/include/stdio.h`](V7/usr/include/stdio.h)

Early Unix C programs either invoked raw `read()`/`write()` syscalls on every byte or used clumsy 518-byte struct buffers. Ritchie standardized user-space stream buffering with `struct _iobuf` (`FILE`), giving C `fopen`, `fread`, `fprintf`, and inline macro definitions for `getc()` and `putc()` that decrement buffer counters directly in CPU registers and only trap into kernel space via `_filbuf()` / `_flsbuf()` when a 512-byte buffer empties or fills.

### `cron`: The Clock Daemon
> **Distro:** [Seventh Edition Unix (1979)](V7/V7.md) · **Author:** Brian Kernighan  
> **Source:** [`V7/usr/src/cmd/cron.c`](V7/usr/src/cmd/cron.c)

At barely 230 lines of C, the original `cron` is a masterclass in Unix minimalism. It parses `/usr/lib/crontab` into an in-memory character list, checks `localtime()`, forks `/bin/sh` for any matching time entry, and calls `sleep(60)` in an infinite loop.

### `lint`: The First Static C Analyzer
> **Distro:** [Seventh Edition Unix (1979)](V7/V7.md) · **Author:** Stephen C. Johnson  
> **Source:** [`V7/usr/src/cmd/lint/lint.c`](V7/usr/src/cmd/lint/lint.c)

In 1979, C had no function prototypes; if you passed a `char *` to a function expecting a `double` in another file, the compiler happily emitted the `jsr` instruction and let your stack corrupt at runtime. Johnson built `lint` on top of the `pcc` frontend to check types across separate `.c` translation units, catching mismatched arguments, unused variables, and non-portable 16-bit pointer assumptions. Every modern linter (`clang-tidy`, `clippy`, `eslint`) is named after this file, because it picked the "fluff" bits out of your C code.

---

## 6. Filesystems & Storage Subsystems

### The Hierarchical Inode Filesystem (`namei` & `iget`)
> **Distro:** [PDP-7 Unix (1970)](PDP7-Unix/PDP7-Unix.md) & [First Edition (1971)](V1/V1.md) · **Authors:** Ken Thompson, Dennis Ritchie  
> **Source:** [`PDP7-Unix/sys/s2.s`](PDP7-Unix/sys/s2.s) · [`V1/u1.s`](V1/u1.s)

The architectural bedrock of Unix storage. Instead of tying file metadata directly to directory filenames (like contemporary mainframe OSes), Thompson decoupled them: a directory is just a flat file mapping human-readable strings to an integer i-number, and that integer indexes into an on-disk array of **inodes** (information nodes) holding permissions, size, and block pointers. That single design choice is why hard links, atomic `rename()`, and unlinking an open file while a process still holds its file descriptor work seamlessly.

### Kernel Pipes (`pipe.c`)
> **Distro:** [Fifth Edition Unix (1974)](V5/V5.md) · **Authors:** Ken Thompson, Doug McIlroy  
> **Source:** [`V5/usr/sys/ken/pipe.c`](V5/usr/sys/ken/pipe.c)

For years, Doug McIlroy badgered Thompson to let programs connect like "garden hoses." In one legendary coding night, Thompson wired `pipe()` into the kernel, added `|` to the shell, and updated the userland utilities to read `stdin` when given no file arguments. Look at [`pipe.c`](V5/usr/sys/ken/pipe.c): it repurposes an in-core inode buffer as a 4,096-byte synchronized ring buffer, putting the writer to `sleep()` when full and waking the reader via `wakeup()`. Less than 160 lines of C to invent the Unix philosophy.

### Berkeley Fast File System (FFS / UFS)
> **Distro:** [4.2BSD (1983)](4.2BSD/4.2BSD.md) · **Authors:** Marshall Kirk McKusick, Bill Joy, Sam Leffler  
> **Source:** [`4.2BSD/usr/src/sys/sys/ufs_alloc.c`](4.2BSD/usr/src/sys/sys/ufs_alloc.c) · [`4.2BSD/usr/src/sys/sys/ufs_inode.c`](4.2BSD/usr/src/sys/sys/ufs_inode.c)

The old V7 filesystem placed *all* inodes at the start of the spinning disk and data blocks everywhere else, turning every file read into a mechanical disk-head seeking marathon that capped throughput at ~2% of raw hardware bandwidth. McKusick partitioned the platter into **cylinder groups** (each with its own local block bitmap, inodes, and data blocks), raised block sizes from 512B to 4KB/8KB with sub-block fragments, and factored platter rotational latency into `ufs_alloc.c`. Throughput jumped 10x overnight.

### Virtual File System (VFS) & The `vnode` Switch
> **Distro:** [Sun NFS v2 Release (1985)](NFSv2/NFSv2.md) · **Authors:** Steve Kleiman, Russel Sandberg, Bill Joy  
> **Source:** [`NFSv2/usr/src/sun/sys/sys/vfs.c`](NFSv2/usr/src/sun/sys/sys/vfs.c) · [`NFSv2/usr/src/sun/sys/sys/vfs_vnode.c`](NFSv2/usr/src/sun/sys/sys/vfs_vnode.c)

Before 1985, the Unix kernel's `open()` and `read()` syscalls were hardcoded directly to on-disk UFS structures (`struct inode`). To build NFS, Sun needed a single kernel to talk to local disks and remote servers simultaneously without littering every syscall with `if (is_remote)` branches. Kleiman implemented object-oriented polymorphism in pure C via `struct vfs` and `struct vnode`: tables of function pointers (`vop_open`, `vop_read`, `vop_lookup`) that decoupled the syscall interface from the physical filesystem. Linux's `struct file_operations` and `struct inode_operations` descend directly from this design.

### Network File System (NFS v2)
> **Distro:** [Sun NFS v2 Release (1985)](NFSv2/NFSv2.md) · **Authors:** Russel Sandberg, Bob Lyon, Steve Kleiman  
> **Source:** [`NFSv2/usr/src/sun/sys/nfs/nfs_vfsops.c`](NFSv2/usr/src/sun/sys/nfs/nfs_vfsops.c)

Plugged straight into the new `vnode` switch. What made NFSv2 radical was its **stateless server architecture**: the server kept zero record of which clients had which files open. Instead, clients passed opaque 32-byte *file handles* over Sun ONC RPC and XDR on every request. If an NFS server crashed and rebooted mid-read, the client simply kept retrying its RPC packet until the server came back online, and the user's `read()` syscall finished without even knowing the server had died.

### `/proc`: The Process Filesystem
> **Distro:** [Eighth Edition Unix (1985)](V8/V8.md) · **Author:** Tom Killian  
> **Source:** [`V8/usr/src/cmd/procmount.c`](V8/usr/src/cmd/procmount.c) · [`V8/usr/sys/sys/proca.c`](V8/usr/sys/sys/proca.c)

Before `/proc`, tools like `ps` and debuggers had to open `/dev/kmem` (raw physical kernel RAM), parse the kernel binary's symbol table (`nlist`) to guess the memory address of the `proc[]` array, seek to that address, and hope the kernel wasn't mutating the struct mid-read. Killian turned running processes into synthetic files under `/proc/<pid>`, letting userland inspect memory and control execution via standard `open()`, `read()`, `write()`, and `ioctl()` calls.

### The First Native Linux Filesystem (`ext`)
> **Distro:** [Linux 0.96c (1992)](Linux0.96c/Linux0.96c.md) · **Author:** Rémy Card  
> **Source:** [`Linux0.96c/fs/ext/freelists.c`](Linux0.96c/fs/ext/freelists.c) · [`Linux0.96c/fs/ext/namei.c`](Linux0.96c/fs/ext/namei.c)

Linus Torvalds bootstrapped early Linux on Andrew Tanenbaum's [Minix 1.1](Minix1.1/Minix1.1.md) filesystem, which capped partitions at a tiny 64 MB and truncated filenames at 14 characters. Rémy Card used Linux's new VFS layer to build `ext`, raising partitions to 2 GB and filenames to 255 characters and starting the lineage of `ext2`, `ext3`, and `ext4`.

### ZFS: Zettabyte File System
> **Distro:** [OpenSolaris Build 135 (2005 to 2010)](OpenSolaris_b135/OpenSolaris_b135.md) · **Authors:** Matthew Ahrens, Jeff Bonwick  
> **Source:** **Vnode Layer:** [`zfs_vnops.c`](OpenSolaris_b135/uts/common/fs/zfs/zfs_vnops.c) · **Storage Pool Allocator:** [`spa.c`](OpenSolaris_b135/uts/common/fs/zfs/spa.c) · **Data Management Unit:** [`dmu.c`](OpenSolaris_b135/uts/common/fs/zfs/dmu.c) · **ARC Cache:** [`arc.c`](OpenSolaris_b135/uts/common/fs/zfs/arc.c)

Bonwick and Ahrens threw out 30 years of partition tables, volume managers, and `fsck` assumptions. ZFS combined the filesystem and volume manager into a copy-on-write transactional object store where every block pointer holds a 256-bit checksum of its *child* block in a massive Merkle tree. Active data is never overwritten in place; writes allocate new blocks and ripple pointer updates up to the root `uberblock`, delivering atomic snapshots, immunity to the RAID-5 write hole (`RAID-Z`), self-healing silent bit-rot repair, and the scan-resistant Adaptive Replacement Cache (`arc.c`).

---

## 7. Networking & Protocols

### The BBN TCP/IP Stack & The `mbuf` Allocator
> **Distro:** [BBN-Vax-TCP / 4.1BSD (1981)](BBN-Vax-TCP/BBN-Vax-TCP.md) · **Authors:** Rob Gurwitz, Jack Haverty  
> **Source:** [`BBN-Vax-TCP/bbnnet/mbuf.c`](BBN-Vax-TCP/bbnnet/mbuf.c) · [`BBN-Vax-TCP/bbnnet/tcp_input.c`](BBN-Vax-TCP/bbnnet/tcp_input.c)

Funded by DARPA so every university running BSD on a VAX would have working TCP/IP ahead of the 1983 ARPANET flag-day cutover. To avoid expensive `memcpy()` calls every time a packet moved down the stack from TCP to IP to the network interface driver, Gurwitz invented the **`mbuf`** (memory buffer): chained fixed-size buffers with adjustable offset pointers that let each protocol layer prepend or strip headers in $O(1)$ time without copying payload bytes. Both BSD `mbuf` chains and Linux's `sk_buff` (`skb`) descend from this pattern.

### The Berkeley Sockets API
> **Distro:** [4.2BSD (1983)](4.2BSD/4.2BSD.md) · **Authors:** Bill Joy, Sam Leffler  
> **Source:** [`4.2BSD/usr/src/sys/sys/uipc_socket.c`](4.2BSD/usr/src/sys/sys/uipc_socket.c)

Network connections don't naturally fit the Unix "everything is a file" pathname model; you can't cleanly pass ephemeral ports, remote IP addresses, and listen backlog queues into an `open("/dev/tcp")` string. Joy and Leffler solved this with the socket syscall lifecycle: `socket()`, `bind()`, `listen()`, `accept()`, `connect()`. Once connected, it hands you back a standard file descriptor that works with normal `read()`, `write()`, and `close()`. Every network program on Earth still calls the exact API defined in `uipc_socket.c`.

### Sendmail: Internetwork Mail Router
> **Distro:** [4.1cBSD (1982)](4.1cBSD/4.1cBSD.md) & [4.2BSD (1983)](4.2BSD/4.2BSD.md) · **Author:** Eric Allman  
> **Source:** [`4.2BSD/usr/src/usr.lib/sendmail/src/main.c`](4.2BSD/usr/src/usr.lib/sendmail/src/main.c)

In 1983, email routing was a fragmented mess: ARPANET used `user@host`, UUCP used bang-paths (`host1!host2!user`), and Berknet used colons (`host:user`). Allman built `sendmail` around a rule-based header rewriting engine that sat at the gateway between incompatible networks and translated envelopes on the fly so messages could cross the early internet.

### Van Jacobson's TCP Congestion Control & Slow Start
> **Distro:** [4.3BSD-Tahoe (1988)](4.3BSD-Tahoe/4.3BSD-Tahoe.md) · **Authors:** Van Jacobson, Michael Karels  
> **Source:** [`4.3BSD-Tahoe/usr/src/sys/netinet/tcp_input.c`](4.3BSD-Tahoe/usr/src/sys/netinet/tcp_input.c)

In October 1986, the early Internet suffered catastrophic "congestion collapse": the 32 Kbps link between Lawrence Berkeley Lab and UC Berkeley saw throughput drop 1,000x to 40 bits per second because early TCP implementations blasted full receive windows onto the wire and blindly retransmitted when router queues overflowed. Jacobson applied conservation-of-packets physics directly inside `tcp_input.c`, introducing the Congestion Window (`snd_cwnd`), Slow Start, Congestion Avoidance, Fast Retransmit, and RTT variance estimation. These few hundred lines of C literally saved the global Internet from collapsing under its own growth.

### OpenSSH (`ssh`, `sshd`)
> **Distro:** [OpenBSD 4.6 (1999 to 2009)](OpenBSD-4.6/OpenBSD-4.6.md) · **Authors:** Theo de Raadt, Markus Friedl, Niels Provos  
> **Source:** [`OpenBSD-4.6/usr.bin/ssh/ssh.c`](OpenBSD-4.6/usr.bin/ssh/ssh.c)

When Tatu Ylönen's original SSH 1.x shifted to a proprietary license, the OpenBSD team forked the last open release (1.2.12), stripped out every patent-encumbered cipher over a single weekend, added SSH 2.0 support, and pioneered **privilege separation** (`privsep`): isolating untrusted network parsing inside an unprivileged `chroot()` child process while keeping keys inside a minimal privileged monitor over a socketpair.

---

## 8. Security & Cryptography

### Salted Password Hashes (`crypt`)
> **Distro:** [AUSAM (1979)](AUSAM/AUSAM.md) & [Seventh Edition Unix (1979)](V7/V7.md) · **Authors:** Robert Morris, Ken Thompson (with parallel work by Peter Ivanov)  
> **Source:** [`V7/usr/src/cmd/crypt.c`](V7/usr/src/cmd/crypt.c) · [`AUSAM/sys/ken/slp.c`](AUSAM/sys/ken/slp.c)

Because `/etc/passwd` had to be world-readable so utilities like `ls -l` could map numeric UIDs to usernames, Unix stored one-way password hashes. To stop attackers from pre-computing hashes of an entire dictionary once and comparing them against every user on every machine (what we now call a rainbow table attack), Morris and Thompson injected a 12-bit random **salt** that physically mutated the internal expansion tables (`E`-box) of the DES cipher across 25 rounds in [`crypt.c`](V7/usr/src/cmd/crypt.c), multiplying pre-computation work by 4,096x ($2^{12}$) and breaking off-the-shelf DES hardware chips.

### Packet Filter (`pf`)
> **Distro:** [OpenBSD 4.6 (2001 to 2009)](OpenBSD-4.6/OpenBSD-4.6.md) · **Author:** Daniel Hartmeier  
> **Source:** [`OpenBSD-4.6/sys/net/pf.c`](OpenBSD-4.6/sys/net/pf.c)

When the author of IPFilter retroactively changed its license terms to forbid modification, Theo de Raadt removed it from OpenBSD's tree and Daniel Hartmeier built `pf` from scratch. Engineered around a fast red-black state tree (`pf_state`), stateful packet normalization (`scrub`), NAT, and a clean configuration grammar (`pf.conf`), `pf` became the firewall backbone of OpenBSD, FreeBSD, and macOS.

### `sudo`: Delegated Root Execution
> **Distro:** [OpenBSD 4.6 (1999 to 2009)](OpenBSD-4.6/OpenBSD-4.6.md) · **Maintainer:** Todd C. Miller  
> **Source:** [`OpenBSD-4.6/usr.bin/sudo/sudo.c`](OpenBSD-4.6/usr.bin/sudo/sudo.c)

The authoritative upstream C implementation of `sudo`: evaluating `/etc/sudoers` policy grants, scrubbing dangerous environment variables (`LD_PRELOAD`, `PATH`), writing security audit logs, and elevating privileges via `setuid(0)` before `execve()`.

---

## 9. Schedulers, Virtual Memory & Observability

### Fair-Share Scheduling (`lnode`)
> **Distro:** [AUSAM (1979)](AUSAM/AUSAM.md) · **Authors:** Peter Ivanov, Bob Kummerfeld (UNSW)  
> **Source:** [`AUSAM/sys/ken/slp.c`](AUSAM/sys/ken/slp.c)

Standard V6/V7 Unix scheduled each *process* equally, meaning any clever student at the University of New South Wales could hog 90% of the PDP-11's CPU simply by running a `fork()` loop of 20 background jobs while everyone else ran one. Kummerfeld and Ivanov modified [`slp.c`](AUSAM/sys/ken/slp.c) to track CPU decay per *user account* (`lnode`), inventing fair-share scheduling and laying the conceptual foundation for modern Linux `cgroups`.

### Paged Virtual Memory & `vfork()`
> **Distro:** [3BSD (1980)](3BSD/3BSD.md) · **Authors:** Özalp Babaoğlu, Bill Joy  
> **Source:** [`3BSD/usr/src/sys/sys/vmpage.c`](3BSD/usr/src/sys/sys/vmpage.c) · [`3BSD/usr/src/sys/sys/vmproc.c`](3BSD/usr/src/sys/sys/vmproc.c)

Bell Labs' VAX port ([32V](32V/32V.md)) ignored the VAX-11/780's hardware MMU paging and still swapped entire processes to disk to free RAM. Babaoğlu and Joy wired true demand-paged virtual memory into 3BSD (`vmpage.c`) using a global two-handed clock replacement algorithm. And because copying multi-megabyte page tables during `fork()` just to immediately overwrite them with `exec()` was slow prior to Copy-on-Write, Joy added `vfork()`, letting the child borrow the parent's address space and page tables directly until `execve()`.

### Memory-Mapped Files (`mmap`) & Shared Libraries
> **Distro:** [SunOS 4.1.4 (1988 to 1994)](SunOS-4.1.4/SunOS-4.1.4.md) · **Authors:** Bill Joy, Robert Gingell, Joseph Moran  
> **Source:** **Syscall:** [`SunOS-4.1.4/sys/os/kern_mman.c`](SunOS-4.1.4/sys/os/kern_mman.c) · **Vnode Segment Driver:** [`SunOS-4.1.4/sys/vm/seg_vn.c`](SunOS-4.1.4/sys/vm/seg_vn.c) · **Hardware Translation:** [`SunOS-4.1.4/sys/sun4/vm_hat.c`](SunOS-4.1.4/sys/sun4/vm_hat.c)

Unifying the VFS `vnode` layer with the virtual memory pager, SunOS 4.0 replaced the old BSD buffer cache with a unified page cache and delivered the first production implementation of `mmap()`. By mapping `vnode` pages directly into process virtual address spaces (`seg_vn.c` via the `smmap()` system call in `kern_mman.c`), Sun also invented modern dynamic shared libraries (`.so` / `ld.so`), allowing hundreds of processes to share a single physical RAM copy of `libc`.

### Linus Torvalds' Original Linux 0.01 Scheduler & Pager
> **Distro:** [Linux 0.01 (1991)](Linux-0.01/Linux-0.01.md) · **Author:** Linus Torvalds  
> **Source:** **Scheduler:** [`Linux-0.01/kernel/sched.c`](Linux-0.01/kernel/sched.c) · **x86 TSS Switch:** [`Linux-0.01/kernel/system_call.s`](Linux-0.01/kernel/system_call.s) · **Paging & CoW:** [`Linux-0.01/mm/memory.c`](Linux-0.01/mm/memory.c)

If modern Linux's millions of lines of code feel overwhelming, read these three files. All of `schedule()` in `sched.c` is literally 35 lines of C: iterating backwards through a fixed 64-slot `task[]` array, picking the runnable task with the highest tick `counter`, and decaying counters (`(*p)->counter = ((*p)->counter >> 1) + (*p)->priority`) when all runnable tasks hit zero. Context switching uses the Intel 386 hardware Task State Segment (`ljmp` macro `switch_to(next)`), while `memory.c` implements full 386 page-table allocation, demand paging, and Copy-on-Write (`do_wp_page`) in barely 400 lines of code.

### DTrace: Dynamic Tracing Framework
> **Distro:** [OpenSolaris Build 135 (2004 to 2010)](OpenSolaris_b135/OpenSolaris_b135.md) · **Authors:** Bryan Cantrill, Mike Shapiro, Adam Leventhal  
> **Source:** [`OpenSolaris_b135/uts/common/dtrace/dtrace.c`](OpenSolaris_b135/uts/common/dtrace/dtrace.c)

Before DTrace, diagnosing a live production kernel meant either recompiling with debug `printf`s or risking a panic with raw debugger breakpoints. DTrace embedded a register-based virtual machine inside the kernel (the DIF bytecode interpreter inside `dtrace.c`, a direct philosophical precursor to modern Linux **eBPF**) that statically verifies tracing programs for safety (no unbounded loops, no arbitrary memory writes) and dynamically patches live kernel instructions in RAM only when a probe fires, achieving strictly *zero* overhead when disabled.

### Completely Fair Scheduler (CFS) & KVM Hypervisor
> **Distro:** [Linux 2.6.33.2 (2007 to 2010)](Linux-2.6.33.2/Linux-2.6.33.2.md) · **Authors:** Ingo Molnár (CFS), Avi Kivity (KVM)  
> **Source:** **CFS:** [`Linux-2.6.33.2/kernel/sched_fair.c`](Linux-2.6.33.2/kernel/sched_fair.c) · **KVM:** [`Linux-2.6.33.2/virt/kvm/kvm_main.c`](Linux-2.6.33.2/virt/kvm/kvm_main.c)

Two modern titans in one tree. In `sched_fair.c`, Molnár threw out complex interactivity heuristics and $O(1)$ active/expired runqueue arrays in favor of a time-ordered red-black tree keyed by **virtual runtime** (`vruntime`) tracked down to the nanosecond, always scheduling the leftmost node (`rb_first`) that has received the least weighted CPU time. Over in `kvm_main.c`, Avi Kivity exposed Intel VT-x and AMD-V hardware virtualization extensions through a clean character device (`/dev/kvm`), turning the standard Linux kernel into a full Type-1 hypervisor without reinventing the scheduler or memory manager.

---

**[← Back to Top](#landmark-tools--subsystems-a-source-code-field-guide)** · **[← Back to OS Catalog (`README.md`)](README.md)**
