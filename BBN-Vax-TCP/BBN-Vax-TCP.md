# BBN-Vax-TCP Source Code Archive

## Historical Overview

| Attribute | Details |
| :--- | :--- |
| **System Name** | BBN TCP/IP VAX-UNIX (BBN TCP/IP 3.0 Beta-Test Distribution for 4.1BSD) |
| **Release Date** | November 24, 1981 (with October 1982 update snapshot `bbnnet-oct82`) |
| **Origin / Creators** | Bolt Beranek & Newman Inc. (BBN, Cambridge, MA) — Rob Gurwitz, Jack Haverty; sponsored by DARPA |
| **Source Donation** | Recovered from BBN Distribution Tape #4 and donated to TUHS |
| **Target Architecture** | DEC VAX-11/780 (32-bit CISC with Unibus and Massbus adapters) running 4.1BSD |
| **Key Historical Milestones** | **The Official DARPA Reference TCP/IP Implementation**: Built to prepare ARPANET for the Jan 1, 1983 Flag Day; introduced in-kernel `mbuf` memory management and finite-state TCP before 4.2BSD sockets |

### Significance & Historical Context
In 1980, the United States Department of Defense (DoD) mandated TCP/IP as its official inter-networking standard and established **January 1, 1983 as "Flag Day"**, after which all computers connected to the ARPANET were prohibited from using the old NCP protocol. DARPA contracted Bolt Beranek and Newman (BBN)—the originators of the ARPANET packet-switching nodes—to build the official, production-grade reference implementation of TCP/IP for 32-bit VAX-11/780 systems running UNIX.

BBN documented their architecture in **IEN 168** (*The BBN TCP/IP Protocol Implementation for VAX/UNIX*, January 1981) and distributed beta tapes to research contractors in November 1981. This distribution marks the historic moment where TCP/IP moved into the UNIX kernel:
1. **The In-Kernel `mbuf` Memory Subsystem**: BBN created the first kernel memory-buffer architecture (`mbuf.c`, `mbuf.h`) designed specifically for network packets—featuring chained buffer descriptors that allowed protocol headers to be prepended and stripped without copying packet payloads.
2. **Table-Driven TCP Finite State Machine**: Unlike ad-hoc state transitions, BBN implemented a formal state table (`fsm.h`, `fsmdef.h`, `tcp_states.c`) enforcing strict RFC 793 compliance for connections (`LISTEN`, `SYN_SENT`, `ESTABLISHED`, `FIN_WAIT`, `TIME_WAIT`).
3. **The DARPA vs. Berkeley Duel**: DARPA initially envisioned BBN's TCP/IP stack as the national standard for UNIX. However, UC Berkeley's Computer Systems Research Group (CSRG), led by Bill Joy and Sam Leffler, wanted a more flexible socket abstraction for local IPC as well as networking. Berkeley incorporated ideas from BBN's stack (especially `mbufs`) but rewrote the network subsystem for **4.2BSD** (1983). The BBN VAX implementation remains the historic bridge that made the Internet Flag Day possible.

---

## Directory Tree & Navigation Map

```
BBN-Vax-TCP/
├── history                      # Developer modification log dating back to 32V
├── BBNSYS/                      # BBN kernel build directory and device configurations
│   ├── make.log, make.out       # Leftover build logs from release machine
│   ├── makefile.bak             # Backup copy of build makefile
│   └── conf files, assym.s      # Device headers (dh.h, dz.h, hp.h, pty.h)
├── bbnnet/                      # The November 1981 BBN TCP/IP In-Kernel Stack
│   ├── con.h, fsm.h, fsmdef.h   # Finite state machine definitions and connection blocks
│   ├── host.c, icmp.c, icmp.h   # Host tables and ICMP control message handling
│   ├── ip.h, ip_input.c, output # IP datagram reassembly, routing, and checksums
│   ├── mbuf.c, mbuf.h           # BBN network memory buffer (mbuf) allocation engine
│   ├── netmain.c, netuser.c     # Kernel network daemon entry and user syscall interface
│   ├── raw_input.c              # Raw IP packet dispatcher
│   └── tcp_input.c, states.c ...# TCP protocol engine, state machine, and timers
├── bbnnet-oct82/                # Updated snapshot of BBN stack dated October 1982
├── conf/                        # System configuration tables and startup scripts
├── dev/                         # VAX-11/780 peripheral device drivers
├── doc/                         # BBN network manuals and configuration guides
├── etc/                         # Administrative scripts and `/etc/net` network tables
├── floppy/                      # VAX-11/780 console floppy disk driver images
├── h/                           # 4.1BSD and BBN kernel header files
├── include/                     # User C library header files
├── lib/                         # System runtime libraries
├── src/                         # Userland application source tree
│   ├── ftp/                     # Early TCP-based ARPANET File Transfer Protocol
│   ├── host/                    # Hostname table parsing and resolution
│   ├── libbbn/, libnet/, libstr/# BBN network communication libraries
│   ├── mtp/                     # Mail Transfer Protocol (precursor to SMTP)
│   ├── telnet/                  # ARPANET TCP Telnet client
│   └── util/                    # Network testing and debugging tools
├── stand/                       # Standalone VAX boot programs and diagnostics
└── sys/                         # 4.1BSD core kernel source tree (main, slp, bio, etc.)
```

---

## Key Source Code Files

### 1. The Kernel Memory Buffer Subsystem (`mbufs`)
- `bbnnet/mbuf.h`: Declaration of `struct mbuf`. Defines the packet header link pointers, data byte offset, and length fields that enabled zero-copy network protocol processing.
- `bbnnet/mbuf.c`: Kernel allocator routines: `m_get` (allocates an mbuf), `m_free` (releases an mbuf to the freelist), and `m_copy` (duplicates packet chains).

### 2. IP Protocol Engine & ICMP
- `bbnnet/ip_input.c`: Validates IP headers, computes checksums, manages the IP fragment reassembly queue, and delivers datagrams to TCP, UDP, or raw sockets.
- `bbnnet/ip_output.c`: Fragments outbound IP datagrams exceeding network interface MTU and encapsulates them for local networks (1822 IMP or Ethernet).
- `bbnnet/icmp.c`: Implements Internet Control Message Protocol (Destination Unreachable, Time Exceeded, Echo/Ping).

### 3. TCP Protocol & State Machine
- `bbnnet/tcp_input.c`: Core TCP input processing. Extracts TCP headers, validates sequence numbers, performs sliding window calculations, and handles out-of-order packets.
- `bbnnet/tcp_states.c` & `bbnnet/fsmdef.h`: The table-driven Finite State Machine governing TCP connection lifecycles according to RFC 793.
- `bbnnet/tcp_procs.c`: Reassembly logic, acknowledgment generation, and exponential backoff retransmission timers.

### 4. Userland Internet Applications
- `src/ftp/`: The BBN FTP client and server running over native TCP.
- `src/telnet/`: Telnet client utilizing BBN's network library (`libnet`).
- `src/mtp/`: The Mail Transfer Protocol daemon, the direct predecessor to RFC 821 SMTP.

---

## Recommended Exploration Path

1. **Read the Original Tape Information**:
   - Check `LABEL` and `history` to understand the distribution packaging.
2. **Study the Birth of `mbufs`**:
   - Read `bbnnet/mbuf.h` and `bbnnet/mbuf.c` to observe the architecture that still powers network buffers across all modern BSDs and macOS.
3. **Trace the IP Layer**:
   - Follow packet flow through `bbnnet/ip_input.c` and `bbnnet/ip_output.c`.
4. **Examine the TCP Finite State Machine**:
   - Inspect `bbnnet/fsmdef.h` and `bbnnet/tcp_states.c` to see how BBN codified RFC 793 states into a clean state transition matrix.
5. **Compare Dual Snapshots**:
   - Compare `bbnnet/` (Nov 1981) against `bbnnet-oct82/` (Oct 1982) to see how bugs were resolved in the final weeks leading to the 1983 ARPANET Flag Day.
