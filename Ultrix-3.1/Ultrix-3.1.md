# Ultrix-3.1 Source Code Archive

## Historical Overview

| Attribute | Details |
| :--- | :--- |
| **System Name** | ULTRIX-11 Version 3.1 (DEC ULTRIX-11 V3.1) |
| **Release Date** | January 1988 (Development span 1984–1988; corporate copyright 1984–1986) |
| **Origin / Creators** | Digital Equipment Corporation (DEC) Unix Engineering Group (UEG, Merrimack, NH) |
| **Source Donation** | Preserved from DEC distribution tapes and donated to TUHS by Steven Schultz |
| **Target Architecture** | DEC PDP-11 family (PDP-11/24, 11/44, 11/53, 11/70, 11/73, 11/83, 11/84, 11/93) |
| **Key Historical Milestones** | **The Pinnacle of 16-Bit Commercial UNIX**: Squeezed full 4.3BSD TCP/IP networking, sockets, and Ethernet into a PDP-11 using network overlays (`ovnet`); provided dual System V and BSD compatibility |

### Significance & Historical Context
By the mid-1980s, the computing world was rapidly transitioning to 32-bit platforms (VAX, Sun SPARC, Motorola 68020). However, thousands of corporations, government departments, and university laboratories retained massive investments in Digital Equipment Corporation's PDP-11 minicomputers. DEC's Unix Engineering Group (UEG) committed to delivering a premier, modern commercial operating system for these 16-bit machines, resulting in **ULTRIX-11 Version 3.1** in 1988.

Ultrix-11 V3.1 represents the technological limit of what could be engineered on a 16-bit minicomputer:
1. **Full BSD 4.3 TCP/IP on a 16-Bit CPU**: While earlier PDP-11 systems only supported single daemons or lacked network support entirely, Ultrix-11 ported the complete Berkeley socket stack, TCP/IP networking, and Ethernet drivers into the kernel.
2. **Network Overlay Segments (`ovnet/`)**: Because the PDP-11 architecture limited programs to 64KB instructions and 64KB data, fitting the TCP/IP stack alongside the file system and scheduler was seemingly impossible. DEC solved this by creating a dedicated **network overlay segment** (`ovnet/`). The network stack resided in physical RAM outside the primary kernel window and was swapped into active MMU page registers during network interrupt processing.
3. **The `mbuf11` Architecture**: Implemented a specialized buffer manager (`mbuf11.c`) that mapped packet memory buffers across PDP-11 22-bit physical address space without consuming user virtual memory.
4. **Dual-Universe Compatibility**: Provided dual environments, allowing users to compile and run AT&T UNIX System V Release 2 software (`src/libc/sysV`, `libcV`, `s5make`) alongside UC Berkeley 4.3BSD software (`src/ucb`, BSD signals, job control, `sendmail`, `rlogin`, `ftp`, `routed`).

---

## Directory Tree & Navigation Map

```
Ultrix-3.1/
├── etc/                         # Administrative configuration and network tables
├── src/                         # Userland, network utilities, and runtime libraries
│   ├── cmd/                     # 177 command sources (sh, awk, make, s5make, tar, etc.)
│   ├── etc/                     # Administrative daemons (ftpd, routed, telnetd)
│   ├── games/                   # Terminal games source (chess, etc.)
│   ├── libape/, libcurses/      # Advanced Programmer's Editor and termcap curses
│   ├── libc/                    # Dual-universe standard C library
│   │   ├── sys/                 # BSD system call stubs
│   │   ├── sysV/                # AT&T System V system call stubs
│   │   └── net/                 # Network gethostbyname / socket runtime
│   ├── libcV/                   # Full System V compatibility runtime archive
│   ├── libF77/, libI77/, libU77/# Complete Fortran 77 compiler runtime suite
│   ├── libjobs/, libtermlib/    # Berkeley job control and terminal capabilities
│   ├── ucb/                     # Berkeley 4.3BSD command suite (Mail, ftp, rlogin, rsh, etc.)
│   └── userdev/                 # User-space device driver templates
├── sys/                         # The Complete Ultrix-11 Kernel Tree
│   ├── conf/                    # Kernel configuration rules, makefiles, and device tables
│   ├── dev/                     # 37 DEC peripheral drivers (MSCP ra disks, tk tapes, dz, dh)
│   ├── distr/                   # Distribution generation and tape mastering scripts
│   ├── mdec/                    # Machine boot blocks for all DEC PDP-11 disks
│   ├── net/                     # In-Kernel BSD TCP/IP protocol stack and Ethernet drivers
│   │   ├── if_de.c, if_qe.c     # DEUNA Unibus and DEQNA Q-bus Ethernet drivers
│   │   ├── ip_input.c, output   # IP routing, packet validation, and fragment reassembly
│   │   ├── tcp_input.c, subr.c  # TCP state machine, sliding window, and acknowledgments
│   │   └── udp_usrreq.c         # UDP protocol processing
│   ├── ovdev/                   # Device driver overlay modules
│   ├── ovnet/                   # Network protocol overlay dispatching and assembly fixes
│   ├── ovsys/                   # Core operating system overlay library
│   ├── sas/                     # Standalone diagnostic, installation, and boot system
│   └── sys/                     # Core kernel services (main, slp, mbuf11, bio, flock, ipc)
└── usr/                         # User filesystem templates
    ├── games/, local/           # User games and local administrator scripts
    ├── include/                 # System C header files (sys/, protocols/, local/)
    └── lib/                     # System libraries and uucp configuration
```

---

## Key Source Code Files

### 1. PDP-11 Network Overlays & Memory Buffers
- `sys/sys/mbuf11.c`: DEC's PDP-11 implementation of network memory buffers. Manages `mbuf` buffer chains located in extended physical memory (above the 64KB virtual boundary) using PDP-11 segmentation registers (`seg.h`).
- `sys/ovnet/asmfix1` & `sys/ovnet/asmfix2`: Post-assembly filter scripts that rewrite function calls in the network stack into inter-overlay jump vectors.

### 2. In-Kernel TCP/IP Protocol Stack
- `sys/net/tcp_input.c`: 4.3BSD-derived TCP input engine managing window sizing, sequence numbers, and state transitions inside the overlay.
- `sys/net/ip_input.c`: Inbound IP datagram parsing, routing lookup, checksum verification, and reassembly.
- `sys/net/in_pcb.c`: Internet protocol control block management for TCP and UDP socket endpoints.

### 3. DEC Hardware Ethernet Drivers
- `sys/net/if_de.c`: Driver for the DEC **DEUNA** (Unibus Ethernet Network Adapter) high-speed interface.
- `sys/net/if_qe.c`: Driver for the DEC **DEQNA** (Q-bus Ethernet Network Adapter) used on MicroPDP-11 workstations.

### 4. Dual System V / BSD Compatibility Layer
- `src/libc/sysV/`: System V system call emulation layer (providing `fcntl`, `ustat`, System V signals).
- `src/libcV/`: Specialized runtime archive for compiling AT&T System V applications on Ultrix.
- `src/ucb/`: Complete suite of Berkeley network client programs (`ftp`, `rlogin`, `rcp`, `netstat`, `Mail`).

---

## Recommended Exploration Path

1. **Study the 16-Bit Network Overlay Architecture**:
   - Inspect `sys/sys/mbuf11.c` and `sys/ovnet/makefile` to observe how DEC engineers shoehorned a full TCP/IP stack into a machine with a 64KB virtual address ceiling.
2. **Examine DEC Ethernet Implementations**:
   - Read `sys/net/if_de.c` (DEUNA) and `sys/net/if_qe.c` (DEQNA) to understand early hardware Ethernet packet ring-buffering.
3. **Trace Dual Compatibility**:
   - Compare `src/libc/sys/` with `src/libc/sysV/` to see how DEC bridged the commercial gap between AT&T System V and UC Berkeley BSD.
4. **Inspect Modern PDP-11 Storage Drivers**:
   - Read `sys/dev/ra.c` to examine DEC's MSCP (Mass Storage Control Protocol) driver communicating with intelligent disk controllers.
5. **Explore Network Applications**:
   - Browse `src/ucb/` to see classic BSD Internet client utilities adapted for PDP-11 memory constraints.
