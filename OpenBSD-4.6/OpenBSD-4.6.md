# OpenBSD-4.6 Source Code Archive

## Historical Overview
- **Release Date**: October 19, 2009 (Release 4.6; official song/artwork: *"Planet of the Users"*)
- **Author / Organization**: Theo de Raadt and the OpenBSD Project (Calgary, Alberta, Canada)
- **Target Architectures**: 13 supported architectures in `sys/arch/`: `i386`, `amd64`, `sparc`, `sparc64`, `macppc`, `arm`, `hppa`, `landisk`, `luna88k`, `sgi`, `socppc`, `vax`, and `zaurus`.
- **Historical Milestone**: The premier benchmark for proactive software security, extreme code auditing, and uncompromising software freedom in the Unix lineage. Branching from NetBSD in 1995 under the fierce leadership of Theo de Raadt, OpenBSD established the world-renowned track record: *"Only two remote holes in the default install, in a heck of a long time!"*

### Key Architectural & Security Revolutions
- **Proactive Security Engineering**: Rather than reacting to vulnerabilities with after-the-fact patches, OpenBSD audits entire codebases for systemic flaw patterns:
  - **Safe String Primitives**: Todd C. Miller and Theo de Raadt designed and standardized `strlcpy()` and `strlcat()`, eradicating classic C-string buffer overflows.
  - **W^X (Write XOR Execute)**: Memory pages cannot be simultaneously writable and executable, defeating arbitrary shellcode injection.
  - **Randomized Heap Allocator (`lib/libc/stdlib/malloc.c`)**: Guard pages, randomized page placement, and canaries cause buffer overruns to immediately crash rather than execute hostile payloads.
  - **Kernel Cryptographic Randomness**: Direct integration of `arc4random()` across the kernel and libc for predictable-free entropy.
- **Privilege Separation (privsep)**: Pioneered the architecture of dividing monolithic network daemons into unprivileged worker chroots and minimal privileged master controllers.
- **PF (Packet Filter)**: Developed by Daniel Hartmeier after IPFilter was ejected due to licensing conflicts. PF (`sys/net/pf.c`, `sbin/pfctl/`) became the gold standard for stateful firewalling, NAT, packet normalization, and traffic shaping.
- **The Upstream Birthplace of OpenSSH (`usr.bin/ssh/`, `usr.sbin/sshd/`)**: OpenBSD developers created and maintain OpenSSH, which powers secure shell access for the entire global Internet infrastructure.
- **Clean Networking Daemons**: Engineered clean-room, robust routing protocols: OpenBGPD (`usr.sbin/bgpd/`), OpenOSPFD, OpenNTPD, OpenIKED, relayd, and spamd.
- **CARP (Common Address Redundancy Protocol)**: Free, unencumbered failover clustering protocol created as an open patent-free alternative to Cisco VRRP.

---

## Directory Tree & Navigation Map

```
OpenBSD-4.6/
├── Makefile                     # System build makefile
├── Makefile.cross               # [AUX] Cross-compilation target makefile
├── bin/                         # Fundamental userland utilities (sh, ksh, ls, cp, mv, cat, df)
├── sbin/                        # System administration binaries
│   ├── pfctl/                   # Packet Filter (PF) management utility
│   ├── fdisk/                   # MBR disk partitioning tool
│   ├── disklabel/               # BSD disklabel editor
│   ├── mount/                   # Mount filesystem front-end
│   └── sysctl/                  # Kernel dynamic tuning interface
├── usr.bin/                     # Standard userland utilities
│   ├── ssh/                     # OpenSSH client (ssh, scp, sftp, ssh-keygen, ssh-agent)
│   ├── mg/                      # Lightweight Emacs-compatible editor
│   ├── mandoc/                  # High-performance roff/man parser and formatter
│   └── ...                      # 226 standard Unix utilities
├── usr.sbin/                    # Secure system daemons and servers
│   ├── sshd/                    # OpenSSH daemon server
│   ├── bgpd/                    # OpenBGPD Border Gateway Protocol routing daemon
│   ├── ospfd/                   # OpenOSPFD routing daemon
│   ├── ntpd/                    # OpenNTPD Network Time Protocol daemon
│   ├── spamd/                   # Spamd tarpit antispam daemon
│   ├── relayd/                  # Load balancing and SSL termination proxy
│   ├── httpd/                   # Secure Apache 1.3 based web daemon with privsep
│   │   └── htdocs/              # Sample Apache web server HTML files & icons
│   └── ...                      # 127 system management daemons
├── lib/                         # Standard C libraries (libc with hardened malloc and strlcpy)
├── libexec/                     # System helpers (ld.so dynamic linker, spamlogd, login_reject)
├── etc/                         # Secure default configuration files and rc startup scripts
├── games/                       # Classic BSD gaming terminal tools
├── gnu/                         # [AUX] Third-party GNU tools (egcs/gcc, gdb, perl, groff, texinfo)
├── include/                     # System header files
├── kerberosV/                   # Heimdal Kerberos V authentication implementation
├── regress/                     # [AUX] System regression test suite
├── share/                       # Manual pages, termcap, timezone tables
└── sys/                         # The OpenBSD Kernel Tree
    ├── kern/                    # Core machine-independent kernel
    │   ├── init_main.c          # main() - system initialization sequence
    │   ├── subr_pool.c          # Kernel memory pool allocator with canaries
    │   ├── kern_fork.c          # Process creation, PID randomization, and resource limits
    │   ├── kern_sig.c           # Signal delivery mechanics
    │   └── kern_synch.c         # Sleep queues and context switching
    ├── uvm/                     # UVM virtual memory manager with W^X enforcement
    │   ├── uvm_fault.c          # Page fault handler enforcing non-executable protections
    │   └── uvm_page.c           # Physical memory page frame allocator
    ├── net/                     # Network protocol stack & Packet Filter (PF)
    │   ├── pf.c                 # Packet Filter state engine and packet filtering rules
    │   ├── pf_norm.c            # IP packet normalization, defragmentation, scrub
    │   ├── pf_table.c           # Radix tree IP lookup tables for high-speed matching
    │   ├── pf_ioctl.c           # /dev/pf ioctl interface for pfctl
    │   ├── if_carp.c            # Common Address Redundancy Protocol (CARP)
    │   └── if.c, route.c        # Generic network interface and routing table
    ├── netinet/                 # IPv4, TCP, UDP, ICMP protocols
    ├── netinet6/                # IPv6 protocol suite
    ├── crypto/                  # In-kernel cryptographic framework and hardware accelerators
    ├── dev/                     # Machine-independent device drivers
    └── arch/                    # Architecture-dependent code (i386, amd64, sparc64, macppc, etc.)
```

---

## Key Source Code Files

| Source File | Functionality / Significance |
| :--- | :--- |
| `sys/net/pf.c` | The Packet Filter (PF) core engine. Evaluates incoming/outgoing network packets against state tables, handles NAT translations, evaluates keep-state connections, and enforces firewall policy. |
| `sys/net/pf_norm.c` | Packet scrubber and normalizer (`pf_normalize_ip`). Reassembles IP fragments and strips illegal TCP flag combinations (SYN+FIN, illegal options) to prevent firewall evasion attacks. |
| `usr.bin/ssh/ssh.c` & `usr.sbin/sshd/sshd.c` | OpenSSH client and server. The authoritative upstream source code for the world's most widely deployed secure remote access system, showcasing privilege separation and clean cryptography. |
| `lib/libc/string/strlcpy.c` & `strlcat.c` | Todd C. Miller's famous bounded string functions. Designed to replace unsafe `strcpy()` and clumsy `strncpy()`, guaranteeing null-termination and preventing off-by-one errors. |
| `lib/libc/stdlib/malloc.c` | OpenBSD's security-hardened userspace memory allocator. Uses randomized page mappings, guard pages (mprotecting unmapped boundary pages to catch overruns), and memory canaries. |
| `sys/kern/init_main.c` | Kernel bootstrap `main()`. Initializes entropy pools, sets up memory pools with guard boundaries, mounts the root filesystem, and launches `/sbin/init`. |
| `sys/uvm/uvm_fault.c` | UVM page fault resolver enforcing W^X policies. Rejects requests to grant execute permissions to memory ranges marked writable. |
| `sys/net/if_carp.c` | Common Address Redundancy Protocol (CARP). Enables multiple redundant firewalls and routers to share a virtual IP address with automated sub-second failover. |

---

## Recommended Exploration Path

1. **Defensive Software Engineering in libc**:
   - Start with `lib/libc/string/strlcpy.c` and `lib/libc/string/strlcat.c`.
   - Study `lib/libc/stdlib/malloc.c` to see how heap allocator design can proactively stop security exploits.
2. **Packet Filter (PF)**:
   - Read `sys/net/pf.c` and `sys/net/pf_norm.c` to understand modern stateful firewalling.
   - Trace how `sbin/pfctl/pfctl.c` communicates with the kernel via `/dev/pf`.
3. **OpenSSH & Privilege Separation**:
   - Inspect `usr.sbin/sshd/sshd.c` and `usr.sbin/sshd/privsep.c` to see how daemons partition authority between root and sandboxed unprivileged users.
4. **Kernel Hardening**:
   - Inspect `sys/uvm/uvm_fault.c` to observe W^X memory protection enforcement during page fault handling.
