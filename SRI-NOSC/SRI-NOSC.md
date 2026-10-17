# SRI-NOSC Source Code Archive

## Historical Overview

| Attribute | Details |
| :--- | :--- |
| **System Name** | SRI-NOSC UNIX (with Illinois ARPANET NCP Stack) |
| **Release Date** | May 1979 (Initial development 1974–1975 at UIUC; enhanced 1977–1979 at SRI and NOSC) |
| **Origin / Creators** | Center for Advanced Computation at University of Illinois (Gary R. Grossman, Steve F. Holmgren), Naval Ocean Systems Center (NOSC — Greg Noel), and Stanford Research Institute (SRI) |
| **Source Donation** | Preserved from NOSC distribution tapes and donated to TUHS |
| **Target Architecture** | DEC PDP-11 (PDP-11/40, 11/45, and 11/70 with 1822 IMP interface hardware) |
| **Key Historical Milestones** | **The First Networked UNIX System in History**: Connected UNIX to the ARPANET; pioneered ARPANET NCP protocol, early FTP, Telnet, MMDF mail, and Rand MH |

### Significance & Historical Context
In 1974, researchers at the Center for Advanced Computation at the University of Illinois Urbana-Champaign (UIUC), led by Steve Holmgren and Gary Grossman, undertook a project funded by DARPA to connect UNIX to the ARPANET. At the time, UNIX had zero networking capabilities. The result of their work was the **Illinois NCP (Network Control Protocol)** stack.

The software spread rapidly across the nascent ARPANET to major defense and research contractors, most notably the **Naval Ocean Systems Center (NOSC)** in San Diego and **Stanford Research Institute (SRI)** in Menlo Park. The archive preserved here represents the mature SRI-NOSC distribution from May 1979.

The system's architecture introduced fundamental networking patterns that preceded the BSD socket abstraction by nearly a decade:
1. **Three-Tier Architecture**:
   - **Kernel Tier (`ncpk/`)**: Minimal in-kernel drivers communicating across DEC DR11/LH11 interfaces to the BBN 1822 Interface Message Processor (IMP).
   - **Daemon Tier (`ncpd/`)**: A dedicated background daemon process (`smalldaemon.c`) that managed network state machines, connection handshakes (RFNM acknowledgments), and port assignments in user space to keep kernel memory small.
   - **Application Tier (`ncpp/`)**: The first implementations of network client/server applications for UNIX, including user/server Telnet and user/server FTP.
2. **Special Device Abstraction**: Remote network hosts were mapped into the file system namespace under `/dev/net/` (e.g. `/dev/multics`), allowing programmers to interact with remote mainframes via standard `open()`, `read()`, and `write()` calls.
3. **Pioneering E-Mail Subsystems**: Contains the foundational code for **MMDF** (Multi-channel Memorandum Distribution Facility) and the **Rand MH** (Message Handling) system.

---

## Directory Tree & Navigation Map

```
SRI-NOSC/
├── compile-old, compile.orig    # Redundant developer script backups
├── install-old, install.orig    # Redundant developer script backups
├── loadit-old, loadit.orig      # Redundant developer script backups
├── compile.new, install, loadit # Primary kernel configuration and linking scripts
├── reconfigure, mung            # Kernel generation scripts and header path fixers
├── conf/                        # System configuration tables and low-level PDP-11 startup
├── dmr/                         # Standard and network peripheral device drivers
│   └── doit                     # Intermediate scratch build script
├── doc/                         # Technical documentation and Illinois NCP papers
│   └── IllinoisNCP.nr           # Original technical paper detailing the NCP design
├── green/, green47/             # NOSC local hardware patches and configuration overrides
├── h/                           # System header files
│   └── net/                     # ARPANET NCP network protocol definitions
├── ken/                         # Core V6 kernel routines (slp.c, sys1.c, trap.c, etc.)
├── mdec/                        # Bootstrap loader code for PDP-11 disks
├── mh/                          # Historic Rand MH (Message Handler) e-mail suite
├── mmdf/                        # MMDF multi-channel internet mail distribution facility
├── ncpd/                        # NCP Protocol Daemon (user-space protocol manager)
├── ncpk/                        # NCP Kernel Module (IMP drivers and packet queues)
│   ├── drivers/                 # Hardware 1822 IMP interface drivers
│   └── dti/                     # Data transmission interface
├── ncpp/                        # Network Application Programs (Telnet, FTP, host tools)
│   ├── ftp-s/, ftp-u/           # FTP server and client programs
│   ├── tel-s/, tel-u/           # Telnet server and client programs
│   └── mail/, netpr/, reset     # Network mail sender, network line printer, net reset
├── new/                         # Staging directory for newly updated kernel drivers
└── s1/, s2/, s3/                # Standard userland command source code (ls, login, etc.)
```

---

## Key Source Code Files

### 1. Architectural Papers & Specifications
- `doc/IllinoisNCP.nr`: The definitive technical paper (*The Illinois NCP Documentation*). Details the division of labor between the kernel, the daemon, and application programs.

### 2. The NCP Kernel Module (`ncpk/`)
- `ncpk/ncpio.c`: Core kernel entry point for network I/O requests (`netopen`, `netclose`, `netread`, `netwrite`).
- `ncpk/kerbuf.c`: In-kernel network packet buffer pool manager allocating memory for packets arriving from the ARPANET IMP.
- `ncpk/contab.c`: Connection state table mapping active socket links to user process IDs.

### 3. The Protocol Daemon (`ncpd/`)
- `ncpd/smalldaemon.c`: The central NCP daemon engine. Dispatches Host-to-Host protocol messages, handles Request-for-Next-Message (RFNM) acknowledgments, and performs host status probing.
- `ncpd/files.c`: Manages virtual socket endpoints and connects them to `/dev/net/` special files.

### 4. ARPANET Applications & Mail Systems
- `ncpp/tel-u/`: The first user Telnet client written for UNIX, translating local terminal escapes into network virtual terminal (NVT) sequences.
- `ncpp/ftp-u/`: The original user FTP client implementing the ARPANET File Transfer Protocol.
- `mh/`: The iconic Rand MH Message Handling system, which treated mail messages as individual files within directories.
- `mmdf/`: The Multi-channel Memorandum Distribution Facility developed by Dave Crocker for resilient email routing across heterogenous networks.

---

## Recommended Exploration Path

1. **Read the Illinois NCP Specification**:
   - Start with `doc/IllinoisNCP.nr` to understand how early network pioneers conceptualized networking before BSD sockets existed.
2. **Examine the Kernel Network Layer**:
   - Read `ncpk/ncpio.c` and `ncpk/kerbuf.c` to see how raw packets from the 1822 IMP were buffered in the PDP-11 kernel.
3. **Trace the Userland Protocol Daemon**:
   - Inspect `ncpd/smalldaemon.c` to see how connection state machines were maintained in user space.
4. **Study the Earliest Telnet and FTP Clients**:
   - Browse `ncpp/tel-u/` and `ncpp/ftp-u/` to observe how user applications interacted with the network device nodes.
5. **Explore Early Internet Mail Architecture**:
   - Investigate `mmdf/` and `mh/` to see how modern e-mail concepts (message queues, folder concepts, delivery agents) were initially conceived.
