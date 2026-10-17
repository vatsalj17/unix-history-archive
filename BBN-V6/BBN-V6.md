# BBN-V6 Source Code Archive

## Historical Overview

| Attribute | Details |
| :--- | :--- |
| **System Name** | BBN V6 UNIX with TCP/IP |
| **Release Date** | June 1979 (Development 1978–1979; build timestamps dated April–June 1979) |
| **Origin / Creators** | Bolt Beranek & Newman Inc. (BBN, Cambridge, MA) — Michael A. Wingfield, Rob Gurwitz, Jack Haverty, Charlie D. Haber (`cdh`), Chiu |
| **Source Donation** | Preserved from BBN magnetic tapes and donated to TUHS |
| **Target Architecture** | DEC PDP-11 (PDP-11/40, PDP-11/45, and PDP-11/70 with ARPANET 1822 IMP interface) |
| **Key Historical Milestones** | **First Working TCP/IP Stack for UNIX**: Authored at BBN under DARPA contract; implemented TCP as a userland daemon using Rand ports IPC and `await()`/`capac()` syscalls |

### Significance & Historical Context
In the late 1970s, DARPA directed the transition of the ARPANET from the legacy Network Control Protocol (NCP) to the newly designed Transmission Control Protocol / Internet Protocol (TCP/IP) suite developed by Vint Cerf and Bob Kahn. As the primary contractor that built the original ARPANET IMPs, Bolt Beranek and Newman (BBN) was commissioned to create the reference TCP/IP implementations for minicomputers.

This codebase represents BBN's historic **1978–1979 TCP/IP implementation for Sixth Edition UNIX (V6)** on the PDP-11. Because the PDP-11 kernel had limited memory and in-kernel socket architectures did not yet exist, BBN designed a hybrid system:
1. **Kernel IPC Enhancements**: BBN modified the V6 kernel with **Rand ports** interprocess communication and added new system calls: `await()` and `capac()` (located in `ken/awaitr.c`) to allow processes to sleep waiting for asynchronous events on communication channels without busy-waiting.
2. **The TCP Daemon (`tcp/`)**: Authored by Michael A. Wingfield, the entire TCP protocol engine was implemented as a single persistent daemon process (`tcp.c`). The daemon managed a table of Transmission Control Blocks (TCBs), calculated checksums, executed timeout retransmissions, and handled sliding-window flow control.
3. **Application Interaction (`telnet/`)**: Userland applications (such as BBN Telnet) communicated with the TCP daemon across named message ports (`CmdTcpName`, `CmdUsrName`, `SendName`, `RecvName`).
4. **Coexistence with NCP (`ncpkernel/`)**: Provided dual-stack capability, allowing the PDP-11 to communicate simultaneously over legacy ARPANET NCP and experimental TCP/IP.

---

## Directory Tree & Navigation Map

```
BBN-V6/
├── BUILD.INFO, history          # Build instructions and developer change log
├── libraries                    # Library generation shell script
├── conf/                        # System configuration and low-level PDP-11 startup
│   ├── BUILD.INFO~, c.c~ ...    # Editor backup files and scratch diffs (diff1)
│   ├── bbn-config, config       # BBN kernel generator tools
│   └── l.s, m70.s               # Low-core assembly and trap vector dispatchers
├── dmr/                         # Device drivers (IMP 1822 interface, disks, terminals)
├── doc/                         # Technical documentation
│   └── ipc/                     # Documentation on Rand ports IPC and await/capac syscalls
├── h/                           # System and network header files
├── ken/                         # Core OS kernel routines
│   ├── awaitr.c                 # BBN await() and capac() asynchronous syscalls
│   └── main.c, slp.c ...        # Core V6 kernel routines with BBN modifications
├── lib/                         # Standard C library routines
├── ncpkernel/                   # Legacy ARPANET NCP protocol kernel support
├── newtty/                      # Updated terminal line discipline handlers
├── progs/                       # Administrative utilities and network test programs
├── tcp/                         # The Historic BBN TCP Protocol Engine Daemon
│   ├── tcp.c, tcpstru.h         # Michael Wingfield's TCP daemon and TCB definitions
│   ├── psipin.c, psipout.c      # Packet switch interface packet input/output routines
│   └── auth.c, log.c            # Authentication and event logging
└── telnet/                      # BBN TCP Telnet client implementation
    ├── main.c, tcplib.c         # Telnet client application and TCP port library
    └── cmds.c, netcmd.c         # Interactive command processors
```

---

## Key Source Code Files

### 1. The Historic TCP Daemon
- `tcp/tcp.c`: Authored by Michael A. Wingfield at BBN. The core TCP protocol daemon. Manages the Transmission Control Block table (`struct TcpTcb TCBList[NUMBTCBS]`), coordinates sliding window sequence spaces, handles SYN/ACK handshakes, and processes incoming TCP packets.
- `tcp/tcpstru.h`: Fundamental TCP data structure definitions: TCB layout, port mappings, packet buffer headers, and state machines.
- `tcp/psipin.c` & `tcp/psipout.c`: Packet Switch Interface routines reading from and writing to the ARPANET 1822 IMP network interface.

### 2. Kernel Asynchronous Event System Calls
- `ken/awaitr.c`: Implements BBN's custom `await()` and `capac()` system calls. Allowed processes to sleep on multiple port descriptors and wake up when capacity was available to read or write—the direct conceptual ancestor of `select()` and `poll()`.
- `history`: The original handwritten developer change log (June 1978 to mid-1979) documenting kernel commits by Charlie Haber (`cdh`), Chiu, and Rob Gurwitz.

### 3. TCP Applications & Telnet
- `telnet/main.c`: Main loop for the BBN TCP Telnet client.
- `telnet/tcplib.c`: User-space client library abstracting port message exchanges with the background TCP daemon into pseudo-sockets.

### 4. Hardware IMP Interface & ARPANET Drivers
- `dmr/dh.c` & `dmr/dm.c`: Asynchronous communications multiplexer drivers updated by BBN for high-speed terminal switching.
- `ncpkernel/imp11a.c`: Hardware driver for the DEC LH11 / IMP11-A ARPANET 1822 interface.

---

## Recommended Exploration Path

1. **Read the Developer History**:
   - Begin with `history` to see the step-by-step engineering log from June 1978 to 1979 as BBN engineers integrated TCP and overlay files.
2. **Examine the Earliest TCP Implementation**:
   - Read `tcp/tcpstru.h` and `tcp/tcp.c` to understand how the TCP protocol was structured as a user-space daemon prior to in-kernel sockets.
3. **Trace Asynchronous System Calls**:
   - Study `ken/awaitr.c` to see how BBN solved the event multiplexing problem years before BSD created `select()`.
4. **Follow Application IPC**:
   - Trace `telnet/tcplib.c` to observe how user applications packaged packets and dispatched them to the TCP daemon over message ports.
5. **Inspect the Dual-Stack IMP Interface**:
   - Review `ncpkernel/imp11a.c` to see how the physical ARPANET IMP 1822 hardware was driven.
