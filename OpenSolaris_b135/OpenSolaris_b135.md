# OpenSolaris_b135 Source Code Archive

## Historical Overview
- **Release Date**: March 2010 (Build 135)
- **Author / Organization**: Sun Microsystems, Inc. (under the CDDL - Common Development and Distribution License) in collaboration with the OpenSolaris community
- **Target Architectures**: Multi-platform: SPARC (UltraSPARC `sun4u`, Niagara multicore `sun4v`) and x86/x64 (`i86pc`, Xen paravirtualized `i86xpv`)
- **Historical Milestone**: Represents the apex of enterprise Unix engineering: the SunOS 5.11 / Solaris OS/Net (ON) consolidation source code. Build 135 was the final planned build for the OpenSolaris 2010.05 release prior to Oracle's acquisition of Sun Microsystems and the subsequent closure of OpenSolaris in August 2010. The source tree preserved here served as the immediate genetic ancestor and direct foundation for the open-source **illumos** project (and modern descendants like SmartOS, OmniOS, and OpenIndiana).

### Key Architectural Revolutions
- **ZFS (Zettabyte File System)**: Created by Jeff Bonwick and Matthew Ahrens. Completely reimagined enterprise storage by combining the volume manager and filesystem:
  - 128-bit address space, pooled storage (`zpool`), transactional copy-on-write design, and end-to-end data integrity checksums (defeating silent data corruption).
  - Self-healing data redundancy (RAID-Z, RAID-Z2, RAID-Z3), instant read/write snapshots and clones, and pooled dynamic allocation.
- **DTrace (Dynamic Tracing Framework)**: Engineered by Bryan Cantrill, Mike Shapiro, and Adam Leventhal (`uts/common/dtrace/`). A revolutionary zero-overhead dynamic instrumentation framework allowing live production inspection of kernel and application behavior using the safe `D` scripting language.
- **Solaris Zones (Containers)**: Lightweight operating-system-level virtualization allowing hundreds of isolated virtual environments to run on a single kernel instance with dedicated resource boundaries.
- **SMF (Service Management Facility)**: Replaced brittle, unordered `/etc/init.d` shell scripts with an XML-backed, dependency-aware service lifecycle engine (`cmd/svc/`) that automatically restarts failed daemons.
- **FMA (Fault Management Architecture)**: Predictive self-healing telemetry system that detects degrading hardware components (bad memory lines, failing CPU cores) and off-lines them before system crashes occur.
- **Crossbow**: Network virtualization framework allowing the creation of high-performance virtual network interface cards (VNICs) with strict bandwidth and CPU resource controls.

---

## Directory Tree & Navigation Map

```
OpenSolaris_b135/
├── OPENSOLARIS.LICENSE          # Common Development and Distribution License (CDDL) v1.0
├── README.license-files         # Third-party licensing documentation
├── Makefile, Makefile.master    # Top-level ON consolidation build engine
├── Makefile.master.64           # 64-bit cross-build definitions
├── Targetdirs                   # [AUX] 65 KB manifest of installation directories
├── req.flg                      # [AUX] Legacy TeamWare/SCCS dependency flag file
├── .hgignore, .hgtags           # Mercurial source control tracking residue
├── prototypes/                  # [AUX] Boilerplate source templates (CDDL copyright headers)
├── pkgdefs/                     # [AUX] 516 SVR4 package definitions (SUNWcsu, SUNWzfsr, etc.)
├── stand/                       # Standalone boot code and early filesystem probes
├── psm/                         # Platform Specific Modules (PROM interfaces, APIC drivers)
├── grub/                        # GRUB 0.97 bootloader ported to Solaris with ZFS booting
├── common/                      # Common algorithms shared across kernel and userland
│   ├── lzma/                    # In-tree LZMA compression algorithm
│   ├── crypto/                  # Unified cryptographic framework (AES, SHA, ECC, RSA)
│   ├── avl/                     # Self-balancing AVL tree implementation
│   └── xattr/                   # Extended attributes processing
├── head/                        # Standard system C headers (184 header files)
├── tools/                       # OpenSolaris build toolchain (cw compiler wrapper, cddlchk, hdrchk)
├── ucbcmd/, ucbhead/, ucblib/   # SunOS 4 (BSD 4.x) backwards-compatibility subsystem (UCB)
├── msg/                         # System localization and translation message catalogs
├── lib/                         # 214 System Libraries
│   ├── libc/                    # The Solaris C library
│   ├── libzfs/                  # Core ZFS userspace management library (used by zfs/zpool)
│   ├── libdtrace/               # DTrace compiler and consumer library
│   ├── libnvpair/               # Name-Value pair serialization library
│   ├── libscf/                  # Service Configuration Facility (SMF) library
│   └── libzonecfg/              # Zones configuration and container management library
├── cmd/                         # 448 Userland Commands and System Utilities
│   ├── dtrace/                  # The dtrace command-line utility
│   ├── zfs/, zpool/             # ZFS storage pool and dataset management commands
│   ├── zoneadm/, zonecfg/       # Solaris Zones administration commands
│   ├── svcs/, svcadm/           # SMF service inspection and control commands
│   ├── mdb/                     # Modular Debugger (MDB) command
│   ├── prstat/                  # Process statistics monitor
│   └── format/                  # Disk partitioning and defect analysis utility
└── uts/                         # UNIX Time-Sharing System (The Solaris Kernel)
    ├── common/                  # Machine-Independent Kernel Core
    │   ├── fs/zfs/              # Complete ZFS filesystem and pool engine
    │   ├── dtrace/              # Core DTrace dynamic tracing provider engine
    │   ├── os/                  # Kernel process management, scheduling, and system calls
    │   │   ├── startup.c        # Kernel bootstrap sequence
    │   │   ├── zone.c           # Solaris Zones container implementation
    │   │   └── bio.c, vfs.c     # Buffer cache and VFS operations
    │   ├── vm/                  # Solaris Virtual Memory manager (segvn, segkmem, hat)
    │   ├── inet/                # Enterprise TCP/IP stack with IPMP and Crossbow VNICs
    │   ├── kmdb/                # In-kernel modular debugger
    │   └── c2/                  # C2 / BSM security audit trail subsystem
    ├── i86pc/                   # Intel/AMD x86 & x64 PC hardware kernel architecture
    ├── i86xpv/                  # Xen paravirtualized x86 hypervisor kernel
    ├── sun4u/                   # Sun UltraSPARC Enterprise server architecture
    ├── sun4v/                   # Sun Niagara multicore architecture
    ├── sparc/                   # Common SPARC hardware routines
    └── sfmmu/                   # SPARC Fast MMU memory translation layer
```

---

## Key Source Code Files

| Source File | Functionality / Significance |
| :--- | :--- |
| `uts/common/fs/zfs/zfs_vfsops.c` & `zfs_vnops.c` | ZFS VFS interface. Translates standard POSIX filesystem operations (`open`, `read`, `write`, `readdir`) into transactions within the ZFS storage pool. |
| `uts/common/fs/zfs/spa.c` | Storage Pool Allocator (SPA). Manages top-level virtual devices (vdevs), transaction groups (txgs), disk allocation, and RAID-Z striping/parity. |
| `uts/common/fs/zfs/vdev_raidz.c` | Self-healing RAID-Z implementation. Handles parity calculations and dynamic block reconstructions when checksum mismatches detect disk corruption. |
| `uts/common/dtrace/dtrace.c` | The DTrace kernel framework. Dynamically patches kernel instruction sequences to insert probe trampolines, compiles D intermediate representations (DIF), and records traces with zero overhead when dormant. |
| `uts/common/os/zone.c` | Solaris Zones container engine. Enforces namespace, credential, networking, and IPC isolation between non-global zones and the global host. |
| `uts/common/os/startup.c` | Kernel bootstrap entry point. Configures physical CPU structures, initializes the Virtual Memory Hat layer, sets up DTrace probes, and spawns the initial kernel processes. |
| `cmd/dtrace/dtrace.c` | DTrace userland CLI command. Interacts with `libdtrace` and `/dev/dtrace` to compile user D-scripts into byte code and format trace results. |
| `cmd/zfs/zfs_main.c` & `cmd/zpool/zpool_main.c` | Front-end commands for administering ZFS storage pools, datasets, snapshots, and replication streams. |
| `lib/libscf/` & `cmd/svc/svcadm/` | Service Management Facility (SMF). Manages dependency graphs of system daemons, querying repository states and controlling restarts. |

---

## Recommended Exploration Path

1. **ZFS Internals**:
   - Start with `uts/common/fs/zfs/zfs_vnops.c` to see how POSIX file I/O enters ZFS.
   - Dive into `uts/common/fs/zfs/spa.c` (Storage Pool Allocator) and `uts/common/fs/zfs/dsl_dataset.c` to understand how copy-on-write datasets and snapshots work.
   - Study `uts/common/fs/zfs/vdev_raidz.c` to witness how RAID-Z handles self-healing parity.
2. **DTrace Dynamic Instrumentation**:
   - Read `uts/common/dtrace/dtrace.c` to see how instruction-level dynamic probe patching is implemented safely in a running production kernel.
   - Follow into `lib/libdtrace/` and `cmd/dtrace/dtrace.c`.
3. **Solaris Zones Virtualization**:
   - Inspect `uts/common/os/zone.c` and `cmd/zoneadm/zoneadm.c` to understand operating-system-level container virtualization.
4. **Kernel Bootstrap**:
   - Trace `uts/common/os/startup.c` to follow the journey from PROM/GRUB into 64-bit multi-threaded execution.
