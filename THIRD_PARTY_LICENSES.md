# Third-Party Licenses & Upstream Rights Inventory

This document summarizes the copyright and licensing status of each historical operating system and subsystem preserved in this archive, as best the maintainer can determine it.

**Scope.** The original documentation in this repository (`README.md`, `LANDMARK_TOOLS.md`, the per-OS guides `*/<NAME>.md`) is (c) Vatsal and licensed under **CC BY 4.0** (see [LICENSE](LICENSE)). Original scripts in `scripts/` are licensed under the [**MIT License**](LICENSE-MIT). **None of the third-party historical source code is relicensed by this repository.** Each directory keeps its own upstream copyright, license, or proprietary status.

**Important limits of this document.**

- It is a best-effort summary, **not legal advice**, and it may contain mistakes.
- Copyright notices and license files inside each directory are authoritative over this summary.
- Several trees are large composites. When this document says a tree is "predominantly" under one license, individual files may carry different terms. Check the file headers.
- Last reviewed: November 2026.

**Status labels used in the tables below**

| Label | Meaning |
| :---: | :--- |
| Yes | A public grant permits redistribution (usually with attribution/notice conditions) |
| Mixed | Different parts have different terms, or the status is contested or unconfirmed |
| No | No public grant permits redistribution, or only non-commercial / restricted use |

The status labels describe permission to **redistribute**, not whether a license is OSI- or FSF-approved.

---

## 1. Caldera License (2002): Ancient UNIX

### The grant

On **January 23, 2002**, Caldera International, Inc. (which held the old SCO UNIX licensing business) published a license for certain early UNIX source code, made public through Warren Toomey and the Unix Heritage Society (TUHS). In summary:

- It is a fee-free, BSD-style license to use, modify and distribute the named source code, including creating derived binary products.
- It is limited to **16-bit UNIX Versions 1 through 7** (PDP-11) and **32-bit 32V UNIX**.
- It **expressly excludes UNIX System III, UNIX System V and their successors.**
- Redistributions must retain Caldera's copyright notice, license conditions and warranty disclaimer. TUHS participants describe the text as based on the 4-clause BSD license, so read the full text for the exact conditions.

The full text is kept in this repository at `LICENSES/Caldera-license.pdf`. The original is at <https://www.tuhs.org/Archive/Caldera-license.pdf>.

### Ownership caveat

TUHS's maintained copyright notes say Novell owns the copyright in these systems. Caldera/SCO's ownership claim was later contested in *SCO v. Novell*, in which the courts found that Novell, not SCO, held the UNIX copyrights. The community has nevertheless continued to rely on the 2002 grant in practice, and this repository does the same. Rights in the original AT&T/Bell Labs UNIX code are now believed to sit with OpenText (via Micro Focus and earlier owners). This repository has not independently verified that chain of title.

### Directories covered

| Directory | Notes |
| :--- | :--- |
| `V1/` to `V7/` | Named in the letter |
| `32V/` | Named in the letter |
| `PWB1/` (PWB/UNIX 1.0, 1977) | Not named in the letter, but TUHS's copyright notes list PWB/UNIX among the systems covered |
| `Mini-Unix/` (Dec 1976) | Not named in the letter, but listed by TUHS as covered |
| `LSX/` (1976) | Bell Labs V6-derived mini-kernel. Not named in the letter or in TUHS's list. Probably covered for the same reasons as Mini-Unix, but **unconfirmed** |

### Derivative trees: only the Bell Labs portions are covered

The Caldera grant covers only AT&T/Bell Labs code. Code written by others stays under its own terms.

| Directory | What is not covered by Caldera |
| :--- | :--- |
| `AUSAM/` (1979) | UNSW changes (Ian Johnstone, John Lions, Greg Rose, Peter Ivanov, et al.). Some files also carry independent copyrights of The Children's Museum (Boston), marked proprietary, and Thomas S. Duff (e.g. `source/S/ipatch.c`) |
| `Interdata732/` (1977) | Interdata-specific kernel, drivers and machine modifications by Richard Miller / University of Wollongong |
| `V7M/` (1981) | DEC's overlay kernel (`ovsys`, `ovdev`) and hardware support. TUHS lists V7M as copyright Compaq (now HP/HPE) |
| `V7addenda/` (1980) | Fixes and utilities from DEC, universities and user groups. Upstream components keep their contributors' copyrights |
| `PDP7-Unix/` (1970) | Not covered. The letter specifies the 16-bit PDP-11, and the PDP-7 is 18-bit. See Section 6 |

---

## 2. Linux Kernel Snapshots

The three Linux versions in this repository are different legal snapshots and should not be conflated.

### `Linux-0.01/` (September 1991): Torvalds's original custom terms

Linux 0.01 was released under Linus Torvalds's original terms (`RELNOTES-0.01`). In summary:

- Full source must be available, free of charge.
- Copyright notices must stay intact.
- **It may not be distributed for a fee, not even "handling" costs.**

Torvalds moved to the GNU GPL starting with release 0.12 (January 1992). The 0.01 snapshot was not itself relicensed, so treat it as **non-commercial, no-fee redistribution only**.

### `Linux0.96c/` (July 1992): GPL

Releases from 0.12 onward are under the GNU GPL. This snapshot has no top-level `COPYING` file, so a copy of the GPL is provided in `LICENSES/GPL-2.0.txt` for reference.

### `Linux-2.6.33.2/` (April 2010): predominantly GPLv2 only

The kernel is **GPLv2 only** (see `Linux-2.6.33.2/COPYING`, which also contains Linus's note on the syscall interface). The tree is not entirely one license:

- `firmware/WHENCE` lists the origin and redistribution terms of individual firmware blobs, which are not all GPL.
- Some files are dual-licensed (e.g. GPL/BSD) or carry other permissive notices.

---

## 3. Nokia Bell Labs Research Unix, Editions 8, 9, 10: Covenant Not to Assert

**Applies to:** `V8/` (Feb 1985), `V9/` (Sep 1986), `V10/` (Oct 1989)

### Nature of the grant

In March 2017, Alcatel-Lucent USA Inc., on behalf of itself and Nokia Bell Laboratories, issued a statement to TUHS. It is **not a license grant**. It is a covenant not to assert copyright. In summary:

- ALU-USA agrees, "to the extent of its ability to do so," not to assert its copyright against **non-commercial** copying, distribution, performance, display, or creation of derivative works of Research Unix Editions 8, 9 and 10.
- The statement also says it does not transfer ownership of, or relinquish, any intellectual property rights (including patent rights), and that neither ALU-USA nor Nokia Bell Laboratories provides support or warranties, including any warranty that the editions do not infringe third-party rights.
- A copy of the statement is kept in this repository at `LICENSES/Nokia-Research-Unix-8-9-10-statement.pdf`. The original is on the TUHS archive under `Distributions/Research/Dan_Cross_v8/` (`statement_regarding_Unix_3-7-17.pdf`).

### Practical consequences

1. **Not a formal license.** It conveys no license and assigns no rights. It is a promise not to sue over non-commercial use.
2. **Third-party code is excluded.** Because of the "to the extent of its ability" wording, the covenant does not reach code that Bell Labs did not own. These trees contain such code. For example, `V10/cmd/gcc`, `V10/cmd/bison` and `V10/cmd/flex` are GNU projects (GPL), `V9/X11/src` is the MIT/X Consortium X Window System, `V10/lbin/kermit` is Columbia University's Kermit, and there are BSD-derived components. Those keep their own licenses.
3. **No commercial use.** Commercial copying, distribution, inclusion in commercial products, or commercial derivative works fall outside the covenant, and you would need your own permission from the rights holder.
4. **Not open source** under the OSD or the Free Software Definition, because of the commercial restriction.

---

## 4. NFSv2: Sun NFS Release 2.0 (December 1985)

**Applies to:** `NFSv2/`

`NFSv2/release.ms` ("Notes for NFS Release 2.0", December 16, 1985) states that the release is the first release of Sun's NFS to non-Sun developers, that the software must be licensed from Sun, and that it is a subset of Sun version 2.0 source, containing the files Sun added or changed relative to Berkeley 4.2BSD for VAX.

### Composition (approximate, from a file-by-file review)

- **Sun proprietary code (~118+ files):** NFS kernel subsystems and user commands carrying `Copyright (c) 1984, 1985 by Sun Microsystems, Inc.`
- **Sun RPC (`usr/src/lib/libc/rpc/`):** carries Sun's RPC legend, which allows use, copying and modification without charge but states that users are not authorized to license or distribute it to anyone else except as part of a product or program they developed. **That is not a general redistribution permission.** It should not be read as a permissive license for this archive.
- **Modified 4.2BSD files (~218+ files):** carry Sun SCCS tags recording their Berkeley origin.
- **Retained UC Berkeley copyrights (~11 files):** e.g. `usr/src/etc/restore/*`.

### Status

NFSv2 is a **composite, non-free developer release** for 4.2BSD licensees. No public license permitting redistribution is known.

---

## 5. Commercial Proprietary Distributions

### `SunOS-4.1.4/` (November 1994): Oracle / Sun Microsystems

- **Rights holder:** Sun Microsystems, Inc. (now Oracle America, Inc.), with substantial code from the Regents of the University of California (4.3BSD) and AT&T (System V).
- **Status:** Proprietary. The maintainer is not aware of any public license or redistribution waiver for SunOS 4.x.
- **Note:** Hosting by TUHS does not give third parties the right to redistribute it. Redistributing it here may draw a copyright notice from the rights holder.

### `Ultrix-3.1/` (Ultrix-11 3.1, January 1988): Digital Equipment Corporation

- **Rights holder:** DEC, whose rights passed to Compaq and then to HP/HPE. TUHS lists Ultrix-11 as copyright Compaq.
- **Status:** Proprietary. `Ultrix-3.1/src/cmd/COPYRIGHT` states that the software is furnished under a license and may be used and copied only under that license, and that it derives from software licensed from UC Berkeley and Bell Laboratories. No open-source release is known.

### Repository disposition

These trees are included for historical scholarship and emulator testing. **This repository does not claim they are licensed for redistribution.** They may be removed, or replaced with a download script, at any time. If you represent a rights holder, see the removal-request section at the end of this document.

---

## 6. Educational, Clean-Room, and Restored Distributions

### `PDP7-Unix/` (1970): reconstruction

- **Historical code:** Ken Thompson's 1969/1970 PDP-7 assembly was recovered and reconstructed from scanned printouts by Warren Toomey, Norman Wilson and the TUHS community. Rights in the underlying AT&T code are believed to sit with OpenText. The 2002 Caldera letter covers 16-bit PDP-11 systems, not the 18-bit PDP-7, so **its status is unconfirmed**.
- **Scaffolding:** simulators and reconstruction tools produced by the restoration project are licensed under **GPLv3**.

### `Minix1.1/`, `Minix1.5/`, `Minix2.0/`: BSD-licensed

Minix was originally published with Prentice-Hall textbooks under restrictive educational terms (see `Minix2.0/src/LICENSE` for the historical 1996 Prentice-Hall educational license agreement). In April 2000, Andrew Tanenbaum and Prentice-Hall relicensed Minix under a **BSD-style license**, which was applied to Minix 2 and retroactively to earlier releases.

Individual components contributed by others keep their own notices, e.g. Kenneth Almquist's `ash`. `Minix1.5/` also bundles third-party programs under their own terms, including `commands/elvis`, `commands/kermit`, `commands/zmodem`, `elle/` and the `amoeba/` directory.

### `Coherent4.2.10/` (December 1994): BSD-licensed

Developed by Mark Williams Company as an independent implementation of Version 7 Unix. In January 2015, Robert Swartz, founder and copyright holder of Mark Williams Company, released the source under the **3-clause BSD license**.

### `Xinu7/` (January 1987): educational terms

Copyright Douglas E. Comer / Prentice-Hall. Distributed with the 1987 textbook for instruction and research on LSI-11 hardware. **Not under an OSI-approved license.** See `Xinu7/README`.

---

## 7. Networking Contributions: BBN and SRI-NOSC

### `BBN-V6/` (June 1979) and `BBN-Vax-TCP/` (November 1981)

- **Provenance:** developed by Bolt Beranek and Newman under DARPA contracts (BBN Report No. 3724, *UNIX TCP User's Guide*).
- **Base and additions:** the base is Bell Labs V6 or 4.1BSD. The TCP/IP stack, connection tables, buffer management and utilities (`netstat`, `telnet`) were written by BBN.
- **Status:** the Caldera grant covers only Bell Labs code, not BBN's networking code. Work done by a contractor under a government contract is normally copyrighted by the contractor, not public domain. The copyright holder is presumably BBN or a successor, and **no public license is known**.

### `SRI-NOSC/` (May 1979)

- **Provenance:** SRI International and the Naval Ocean Systems Center, under DARPA funding, to connect Unix to the ARPANET.
- **Contents:** MMDF mail delivery, `ncpp` network control programs, and ARPANET IMP interface drivers.
- **Status:** SRI's work follows the contractor rule above. Work by US Navy employees may be a US government work not subject to copyright in the US, but this has not been sorted file by file. **Treat the status as unconfirmed.**

---

## 8. Berkeley Software Distribution (BSD) and Modern Derivatives

### Early BSD releases

**Applies to:** `1BSD/`, `2BSD/`, `2.9BSD/`, `2.11BSD/`, `3BSD/`, `4BSD/`, `4.1cBSD/`, `4.2BSD/`, `4.3BSD/`, `4.3BSD-UWisc/`

- Berkeley's own code is copyright the Regents of the University of California, under UCB terms that vary by release and file.
- Portions substantially derived from Bell Labs/AT&T UNIX remain Novell/AT&T copyright, per TUHS. Where that code derives from V7 or 32V, the Caldera grant applies. Code derived from System III/V is not covered.
- Third-party contributions carry their own copyrights. 4.3BSD, for example, added files copyrighted by DEC, Tektronix and others.
- `4.3BSD-UWisc/` appears to include Sun NFS/RPC/YP-derived components (`nfs`, `rpc`, `etc/yp`). Treat those as Sun copyright with Sun's notices.
- The 2BSD line (`2BSD/`, `2.9BSD/`, `2.11BSD/`) is heavily based on V7 and 32V code, and parts of its status are still debated among TUHS participants.

### 4.3BSD-Tahoe, 4.3BSD-Reno

Mixed tree. Many files carry an early, precursor form of the BSD license. A substantial part still includes AT&T-derived code, so these are not clean "4-clause BSD" releases.

### `Net2/` (June 1991)

Berkeley released Net/2 as free of AT&T code. USL disputed this in *USL v. BSDi* (1992 to 1994). The settlement led to 4.4BSD-Lite (1994), with a small number of files removed and others modified. Whether Net/2 itself is clean is **contested**: some TUHS participants argue the Caldera grant covers the AT&T-derived portions, while earlier commentary treated it as contaminated.

### `4.4BSD/` (June 1993)

This predates the 1994 settlement and 4.4BSD-Lite. It is **not** Lite, so the disputed AT&T-derived files may still be present. Do not treat it as clean 4-clause BSD without checking.

### Modern BSD descendants

| Directory | Predominant license | Notable exceptions in this tree |
| :--- | :--- | :--- |
| `FreeBSD-5.3/` | BSD-style (see `COPYRIGHT`) | `sys/gnu` and `sys/contrib`; third-party libraries such as BIND, bzip2, zlib, expat and sendmail |
| `NetBSD-5.0.2/` | BSD-style | `external/gpl2`, `external/intel-fw-eula` (Intel firmware under an EULA), `external/mit`, `crypto/dist` (OpenSSL), and `dist/` third-party code (BIND, DHCP, NTP, tcpdump, nvi, etc.) |
| `OpenBSD-4.6/` | BSD / ISC | `gnu/` (GNU toolchain components), `lib/libssl` (OpenSSL), `kerberosV` |

### `OpenSolaris_b135/` (March 2010): predominantly CDDL

CDDL v1.0 (`OPENSOLARIS.LICENSE`) is a file-level copyleft license. The tree also includes third-party components under other licenses, such as GRUB 0.97 (GPL), parted and ntfsprogs (GPL), Perl, libsasl, bzip2 and OpenSSL. See `README.license-files` and `pkgdefs/license_files` in the tree.

---

## 9. Other Third-Party Material in This Repository

- **`unixtree.md`** is the catalog and index written by Warren Toomey for the Unix Tree project (~2010). It is his work, preserved untouched, and is **not** covered by this repository's CC BY license.
- **Per-OS guides** (`*/<NAME>.md`) are original commentary, but any short excerpts or file names quoted from the source remain under the original terms.

---

## 10. Classification Matrix

| Directory | Primary upstream terms | Redistribute? | Key rights holders | Notes |
| :--- | :--- | :---: | :--- | :--- |
| **`PDP7-Unix/`** | Historical code: AT&T/OpenText; scaffolding: GPLv3 | Mixed | Ken Thompson / OpenText / TUHS | 18-bit PDP-7; not in the 2002 letter |
| **`V1/` to `V7/`** | Caldera License (2002) | Yes | Bell Labs / Caldera / Novell lineage | Keep Caldera notice |
| **`32V/`** | Caldera License (2002) | Yes | Bell Labs / Caldera / Novell lineage | Named in the letter |
| **`PWB1/`, `Mini-Unix/`** | Caldera scope per TUHS | Yes | Bell Labs | Not named in the letter |
| **`LSX/`** | Probably Caldera scope | Mixed | Bell Labs | Unconfirmed |
| **`V7addenda/`** | Mixed | Mixed | Bell Labs / DEC / universities | Per-file |
| **`V8/`, `V9/`, `V10/`** | ALU/Nokia covenant (2017) | No | Nokia Bell Labs / third parties | **Non-commercial only**; includes GPL/MIT components |
| **`Interdata732/`** | Base: Caldera; port: Wollongong | Mixed | Bell Labs / Richard Miller | Port code owned by Miller / Wollongong |
| **`AUSAM/`** | Base: Caldera; additions: UNSW etc. | Mixed | Bell Labs / UNSW / Children's Museum / Duff | Some third-party code marked proprietary |
| **`V7M/`** | Base: Caldera; additions: DEC | Mixed | Bell Labs / DEC (HP/HPE) | Overlay kernel is DEC code |
| **`BBN-V6/`, `BBN-Vax-TCP/`** | Base: Caldera/BSD; networking: BBN | Mixed | Bell Labs / BBN / DARPA | Contractor work; no public license known |
| **`SRI-NOSC/`** | Base: Caldera; networking: SRI/NOSC | Mixed | Bell Labs / SRI / US Navy | Status unconfirmed |
| **`1BSD/` to `4.3BSD/`, `4.3BSD-UWisc/`** | UCB terms + AT&T-derived parts | Mixed | Regents of UC / Novell lineage / others | Per-file; not uniformly 4-clause |
| **`4.3BSD-Tahoe/`, `4.3BSD-Reno/`** | UCB (precursor + 4-clause) + AT&T-derived | Mixed | Regents of UC / Novell lineage | Mixed |
| **`Net2/`** | UCB terms | Mixed | Regents of UC | Contested (*USL v. BSDi*) |
| **`4.4BSD/`** | UCB terms (pre-Lite) | Mixed | Regents of UC | Predates 4.4BSD-Lite |
| **`FreeBSD-5.3/`** | BSD-style (mixed) | Yes | The FreeBSD Project / others | Third-party components |
| **`NetBSD-5.0.2/`** | BSD-style (mixed) | Mixed | The NetBSD Foundation / others | Contains an Intel firmware EULA and GPL code |
| **`OpenBSD-4.6/`** | BSD / ISC (mixed) | Yes | The OpenBSD Project / others | Contains GPL and OpenSSL-licensed code |
| **`Linux-0.01/`** | Torvalds custom (1991) | No | Linus Torvalds | **No fee, non-commercial**; never relicensed |
| **`Linux0.96c/`** | GPL | Yes | Linus Torvalds & contributors | No `COPYING` in tree; see `LICENSES/GPL-2.0.txt` |
| **`Linux-2.6.33.2/`** | GPLv2 only (predominantly) | Yes | Linus Torvalds & contributors | Firmware per `WHENCE` |
| **`Minix1.1/`, `Minix1.5/`, `Minix2.0/`** | BSD-style (April 2000) | Yes | Prentice Hall / Andrew Tanenbaum | `Minix1.5/` bundles third-party programs |
| **`Coherent4.2.10/`** | 3-clause BSD (January 2015) | Yes | Robert Swartz / Mark Williams Co. | Released by the copyright holder |
| **`Xinu7/`** | Educational (Prentice-Hall) | No | Douglas Comer / Prentice-Hall | Not OSI-approved |
| **`NFSv2/`** | Composite (Sun proprietary + BSD + Sun RPC) | No | Sun Microsystems / UC Berkeley | "Must be licensed from Sun" |
| **`SunOS-4.1.4/`** | Proprietary | No | Sun / Oracle | No public license known |
| **`Ultrix-3.1/`** | Proprietary | No | DEC / Compaq / HP / HPE | No public license known |
| **`OpenSolaris_b135/`** | CDDL 1.0 (predominantly) | Yes | Sun / Oracle / others | Third-party components in tree |

---

## 11. Corrections and Removal Requests

If you hold rights in any material here and want it removed, credited differently, or relabeled, please **open an issue on this repository's GitHub page**. Requests will be handled promptly. If you find an error in this inventory, an issue or pull request is welcome too.
