# Historical Unix Source Code Archive (1970-2010)

This repository is a collection of 51 historical operating system distributions, fully unpacked and preserved for reading, searching, and studying how operating systems evolved over four decades.

The files originate from Warren Toomey's **Unix Tree** and **The Unix Heritage Society (TUHS)** archives. Every distribution in this repository has been cleaned of modern web-scraping residue (`index.html`), printer overstrike dumps, and magnetic tape headers, leaving behind pure source trees, header files, build scripts, and historical documentation.

Each operating system directory includes its own dedicated markdown guide detailing its kernel architecture, userland structure, and key source files to read first.

> **Don't want to dig through 51 OS directories to find where pipes, `vi`, or TCP congestion control were born?**  
> Jump straight to **[`LANDMARK_TOOLS.md`](LANDMARK_TOOLS.md)** - a subsystem-by-subsystem field guide linking directly to the exact `.c` and `.s` files where computing's greatest inventions first hit disk.

---

## Operating Systems Catalog

### 1. Bell Labs Research Unix
The foundational releases written by Ken Thompson, Dennis Ritchie, Doug McIlroy, and their colleagues at Bell Telephone Laboratories.

| Version | Release Date | Architecture | Guide & Notes |
| :--- | :--- | :--- | :--- |
| **PDP-7 Unix** | Jan 1970 | 18-bit DEC PDP-7 | [PDP7-Unix.md](PDP7-Unix/PDP7-Unix.md) |
| **First Edition (V1)** | Nov 1971 | DEC PDP-11/20 | [V1.md](V1/V1.md) |
| **Second Edition (V2)** | Jun 1972 | DEC PDP-11/20 | [V2.md](V2/V2.md) |
| **Third Edition (V3)** | Feb 1973 | DEC PDP-11/20 | [V3.md](V3/V3.md) |
| **Fourth Edition (V4)** | Nov 1973 | DEC PDP-11/40, 11/45 | [V4.md](V4/V4.md) |
| **Fifth Edition (V5)** | Jun 1974 | DEC PDP-11/40, 11/45 | [V5.md](V5/V5.md) |
| **Sixth Edition (V6)** | May 1975 | DEC PDP-11/40, 11/45, 11/70 | [V6.md](V6/V6.md) |
| **Seventh Edition (V7)** | Jan 1979 | DEC PDP-11/45, 11/70 | [V7.md](V7/V7.md) |
| **Addenda to 7th Edition** | Dec 1980 | DEC PDP-11 | [V7addenda.md](V7addenda/V7addenda.md) |
| **Eighth Edition (V8)** | Feb 1985 | DEC VAX-11/750, 11/780 | [V8.md](V8/V8.md) |
| **Ninth Edition (V9)** | Sep 1986 | DEC VAX, Sun-2/3 (68020) | [V9.md](V9/V9.md) |
| **Tenth Edition (V10)** | Oct 1989 | DEC VAX-11/750, VAX 8550 | [V10.md](V10/V10.md) |

---

### 2. Early Ports & USG Variants
The first ports to non-DEC architectures, internal AT&T production systems, and academic distributions.

| Version | Release Date | Architecture | Guide & Notes |
| :--- | :--- | :--- | :--- |
| **LSX** | 1976 | DEC LSI-11 (Tiny memory, no MMU) | [LSX.md](LSX/LSX.md) |
| **Mini-Unix** | Dec 1976 | DEC PDP-11/05, 11/10, 11/20 | [Mini-Unix.md](Mini-Unix/Mini-Unix.md) |
| **PWB/UNIX 1.0** | Jul 1977 | DEC PDP-11/45, 11/70 | [PWB1.md](PWB1/PWB1.md) |
| **Interdata 7/32 Port** | Jun 1977 | Interdata 7/32 (First non-DEC port) | [Interdata732.md](Interdata732/Interdata732.md) |
| **32V Unix** | May 1979 | DEC VAX-11/780 (32-bit V7 port) | [32V.md](32V/32V.md) |
| **AUSAM** | Oct 1979 | DEC PDP-11 (Fair-share scheduling) | [AUSAM.md](AUSAM/AUSAM.md) |
| **V7M 2.1** | Oct 1981 | DEC PDP-11 (DEC-supported Unix) | [V7M.md](V7M/V7M.md) |

---

### 3. Early Networking
Pioneering implementations of network protocols on Unix before BSD sockets became ubiquitous.

| Version | Release Date | Architecture | Guide & Notes |
| :--- | :--- | :--- | :--- |
| **SRI-NOSC Unix** | May 1979 | DEC PDP-11 + ARPANET IMP | [SRI-NOSC.md](SRI-NOSC/SRI-NOSC.md) |
| **BBN V6 with TCP/IP** | Jun 1979 | DEC PDP-11 + ARPANET IMP | [BBN-V6.md](BBN-V6/BBN-V6.md) |
| **BBN-Vax-TCP (4.1BSD)** | Nov 1981 | DEC VAX-11/780 (DARPA ref) | [BBN-Vax-TCP.md](BBN-Vax-TCP/BBN-Vax-TCP.md) |
| **Sun NFS v2 Release** | Dec 1985 | DEC VAX / Sun-2 / Sun-3 | [NFSv2.md](NFSv2/NFSv2.md) |

---

### 4. Berkeley Software Distribution (BSD)
Developed by the Computer Systems Research Group (CSRG) at UC Berkeley. Introduced virtual memory, the fast file system (FFS), and the BSD socket networking stack.

| Version | Release Date | Architecture | Guide & Notes |
| :--- | :--- | :--- | :--- |
| **1BSD** | Jan 1978 | DEC PDP-11 | [1BSD.md](1BSD/1BSD.md) |
| **2BSD** | Apr 1979 | DEC PDP-11 | [2BSD.md](2BSD/2BSD.md) |
| **2.9BSD** | Nov 1983 | DEC PDP-11 (Overlaid kernel) | [2.9BSD.md](2.9BSD/2.9BSD.md) |
| **2.11BSD** | Feb 1992 | DEC PDP-11 (Overlaid kernel) | [2.11BSD.md](2.11BSD/2.11BSD.md) |
| **3BSD** | Mar 1980 | DEC VAX-11/780 (Paged VM) | [3BSD.md](3BSD/3BSD.md) |
| **4BSD** | Oct 1980 | DEC VAX-11/780, 11/750 | [4BSD.md](4BSD/4BSD.md) |
| **4.1cBSD** | Dec 1982 | DEC VAX-11/780, 11/750 | [4.1cBSD.md](4.1cBSD/4.1cBSD.md) |
| **4.2BSD** | Sep 1983 | DEC VAX-11/780, 11/750 | [4.2BSD.md](4.2BSD/4.2BSD.md) |
| **4.3BSD** | Jun 1986 | DEC VAX-11/780, 11/750 | [4.3BSD.md](4.3BSD/4.3BSD.md) |
| **4.3BSD-Tahoe** | Jun 1988 | CCI Power 6/32 ("Tahoe") & VAX | [4.3BSD-Tahoe.md](4.3BSD-Tahoe/4.3BSD-Tahoe.md) |
| **4.3BSD-Reno** | Jun 1990 | VAX, Tahoe, HP9000/300 | [4.3BSD-Reno.md](4.3BSD-Reno/4.3BSD-Reno.md) |
| **4.3BSD-UWisc** | Jan 1987 | DEC VAX (Univ. of Wisconsin) | [4.3BSD-UWisc.md](4.3BSD-UWisc/4.3BSD-UWisc.md) |
| **The Net/2 Distribution** | Jun 1991 | Portable unencumbered code | [Net2.md](Net2/Net2.md) |
| **4.4BSD** | Jun 1993 | HP300, Intel 386, SPARC, DECstation | [4.4BSD.md](4.4BSD/4.4BSD.md) |

---

### 5. Commercial Unix Releases
Proprietary distributions built by computer vendors for their production hardware platforms.

| Version | Release Date | Architecture | Guide & Notes |
| :--- | :--- | :--- | :--- |
| **Ultrix-11 3.1** | Jan 1988 | DEC PDP-11 family | [Ultrix-3.1.md](Ultrix-3.1/Ultrix-3.1.md) |
| **SunOS 4.1.4** | Nov 1994 | Sun SPARC & 680x0 | [SunOS-4.1.4.md](SunOS-4.1.4/SunOS-4.1.4.md) |

---

### 6. Clean-Room Unix Clones
Independent systems implementing Unix system calls and userland without using original AT&T code.

| Version | Release Date | Architecture | Guide & Notes |
| :--- | :--- | :--- | :--- |
| **Minix 1.1** | Jan 1987 | IBM PC/XT (Intel 8088 real-mode) | [Minix1.1.md](Minix1.1/Minix1.1.md) |
| **Minix 1.5** | Nov 1989 | Intel 286/386, 68000, SPARC | [Minix1.5.md](Minix1.5/Minix1.5.md) |
| **Minix 2.0** | Oct 1996 | Intel 386+ flat 32-bit | [Minix2.0.md](Minix2.0/Minix2.0.md) |
| **Xinu 7** | Jan 1987 | DEC PDP-11 / LSI-11 | [Xinu7.md](Xinu7/Xinu7.md) |
| **Coherent 4.2.10** | Dec 1994 | Intel 386/486 (Clean-room commercial) | [Coherent4.2.10.md](Coherent4.2.10/Coherent4.2.10.md) |
| **Linux 0.01** | Sep 1991 | Intel 80386 | [Linux-0.01.md](Linux-0.01/Linux-0.01.md) |
| **Linux 0.96c** | Jul 1992 | Intel 80386 | [Linux0.96c.md](Linux0.96c/Linux0.96c.md) |
| **Linux 2.6.33.2** | Apr 2010 | Multi-architecture (23 platforms) | [Linux-2.6.33.2.md](Linux-2.6.33.2/Linux-2.6.33.2.md) |

---

### 7. Modern BSD & OpenSolaris Derivatives
Production open-source descendants that rewrote or replaced all proprietary code.

| Version | Release Date | Architecture | Guide & Notes |
| :--- | :--- | :--- | :--- |
| **FreeBSD 5.3** | Nov 2004 | x86, Alpha, IA-64, SPARC64, AMD64 | [FreeBSD-5.3.md](FreeBSD-5.3/FreeBSD-5.3.md) |
| **NetBSD 5.0.2** | Feb 2010 | 50+ architectures (RUMP anykernel) | [NetBSD-5.0.2.md](NetBSD-5.0.2/NetBSD-5.0.2.md) |
| **OpenBSD 4.6** | Oct 2009 | Multi-architecture (Security focus) | [OpenBSD-4.6.md](OpenBSD-4.6/OpenBSD-4.6.md) |
| **OpenSolaris b135** | Mar 2010 | x86-64, SPARC (ZFS, DTrace, Zones) | [OpenSolaris_b135.md](OpenSolaris_b135/OpenSolaris_b135.md) |

---

## How to Explore

1. **Pick an Era or Topic:**
   - If you want to see how the very first Unix was written in PDP-7 assembly, open [PDP7-Unix.md](PDP7-Unix/PDP7-Unix.md).
   - If you want to see the original C rewrite (where pipes and `fork` took shape), read [V6.md](V6/V6.md) or [V7.md](V7/V7.md).
   - If you want to study the birth of the Berkeley Sockets API and TCP/IP, read [4.2BSD.md](4.2BSD/4.2BSD.md) and [BBN-Vax-TCP.md](BBN-Vax-TCP/BBN-Vax-TCP.md).
   - If you want to read a minimal educational microkernel, read [Minix1.1.md](Minix1.1/Minix1.1.md) or Linus's earliest kernel in [Linux-0.01.md](Linux-0.01/Linux-0.01.md).
2. **Follow the Recommendations:**
   - Every OS directory has a `<NAME>.md` file with a "Recommended Exploration Path" giving specific line-by-line reading suggestions for that system's code.

---

## Credits & Preservation
- **The Unix Heritage Society (TUHS)** and **Warren Toomey** for preserving and curating the magnetic tapes, paper listings, and archives.
- **Caldera International** for releasing Research Unix Editions 1-7 and 32V under an open-source license in 2002.
- **Nokia Bell Labs** for granting non-commercial research access to 8th, 9th, and 10th Edition sources in 2017.
- **CSRG (UC Berkeley)**, the Minix developers, the Mark Williams Company estate, the Free Software Foundation, the Linux kernel contributors, the FreeBSD, NetBSD, OpenBSD projects, and the OpenSolaris community.

The file [`unixtree.md`](unixtree.md) in this root directory is the original upstream catalog and index written by Warren Toomey for the Unix Tree project (~2010). It is preserved untouched for historical provenance and cross-referencing with the TUHS online archives.
