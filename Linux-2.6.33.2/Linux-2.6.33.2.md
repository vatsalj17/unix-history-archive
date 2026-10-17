# Linux-2.6.33.2 Source Code Archive

## Historical Overview
- **Release Date**: April 1, 2010
- **Author / Organization**: Linus Torvalds and thousands of developers worldwide (The Linux Foundation)
- **Target Architectures**: 23 supported hardware architectures in `arch/`: `x86` (unified 32/64-bit), `arm`, `mips`, `powerpc`, `sparc`, `ia64`, `s390`, `blackfin`, `cris`, `frv`, `h8300`, `m32r`, `m68k`, `m68knommu`, `microblaze`, `mn10300`, `parisc`, `score`, `sh`, `um` (User-Mode Linux), `avr32`, `xtensa`.
- **Historical Milestone**: Represents the apex of the mature Linux 2.6 kernel series. While Linux 0.01 contained 88 files and ~10,000 lines of code targeting a single PC with 16 MB of RAM, Linux 2.6.33.2 contains **31,528 files** and over **13.5 million lines of code**, running on everything from tiny embedded cell phones (Android) to massive multi-terabyte supercomputers.

### Key Architectural Revolutions
- **CFS (Completely Fair Scheduler)**: Engineered by Ingo Molnar, replacing the previous O(1) scheduler with a red-black tree tracking virtual runtime (`vruntime`), delivering nanosecond scheduling fairness.
- **Cgroups & Namespaces**: Comprehensive resource partitioning (CPU, memory, blkio, network) and process namespace isolation (`PID`, `NET`, `IPC`, `MNT`, `UTS`), forming the technological foundation for modern container runtimes (Docker, Kubernetes, LXC).
- **SLUB Allocator**: Written by Christoph Lameter, replacing the aging SLAB allocator with a lockless, per-CPU slab allocator optimized for multi-socket multicore NUMA machines.
- **KVM (Kernel-based Virtual Machine)**: In-tree hypervisor in `virt/kvm/` that turns the Linux kernel into a Type-1 bare-metal hypervisor leveraging Intel VT-x and AMD-V hardware virtualization extensions.
- **Next-Generation Storage & Filesystems**: Mature Ext4 (delayed allocation, extents, multiblock allocator), early mainline support for Btrfs, scalable XFS, NILFS2 log-structured filesystem, and Device Mapper.
- **Dynamic Tick & Power Management**: Support for tickless idle (`CONFIG_NO_HZ`) and CPU frequency governors (`cpufreq`), drastically reducing energy consumption in data centers and mobile devices.
- **In-Kernel Tracing (`tools/perf/`, `ftrace`)**: Production performance instrumentation through the `perf` subsystem, tracepoints, and ring-buffer events.

---

## Directory Tree & Navigation Map

```
Linux-2.6.33.2/
├── COPYING                  # GNU General Public License v2
├── CREDITS                  # Historical register of significant kernel contributors
├── MAINTAINERS              # Contact list of kernel subsystem maintainers
├── Makefile                 # Top-level build orchestration Makefile
├── Kbuild                   # Top-level kernel build definitions
├── README                   # Kernel building instructions and prerequisites
├── REPORTING-BUGS           # Guide for reporting kernel panics and regressions
├── arch/                    # Machine-dependent code for 23 hardware architectures
│   ├── x86/                 # Unified 32-bit (i386) and 64-bit (x86_64) architecture
│   ├── arm/                 # ARM architecture (v4 through v7, OMAP, PXA)
│   ├── powerpc/             # IBM PowerPC (32-bit and 64-bit POWER)
│   ├── mips/                # MIPS architectures (routers, SGI workstations)
│   ├── sparc/               # Sun SPARC and UltraSPARC architectures
│   ├── s390/                # IBM System/390 and zSeries mainframes
│   ├── ia64/                # Intel Itanium 64-bit architecture
│   └── ...                  # 16 other architectures (blackfin, m68k, sh, etc.)
├── block/                   # Block layer, request queuing, I/O schedulers (CFQ, Deadline, Noop)
├── crypto/                  # Cryptographic cipher API (AES, SHA, DES, hardware crypto)
├── Documentation/           # Thousands of subsystem design documents, HOWTOs, and ABI specs
├── drivers/                 # Hardware device drivers (86 categories: net, pci, usb, scsi, etc.)
│   ├── ata/                 # Serial ATA (SATA) and PATA drivers
│   ├── char/                # TTY, serial, hardware RNG, IPMI drivers
│   ├── gpu/                 # Direct Rendering Manager (DRM) graphics drivers (Intel, Radeon, Nouveau)
│   ├── net/                 # Ethernet, wireless (mac80211), and WAN network interface cards
│   ├── pci/                 # PCI bus enumeration and MSI interrupt handling
│   ├── scsi/                # SCSI host adapters, disk/tape targets, fiber channel
│   └── usb/                 # USB core, host controllers (EHCI, OHCI, UHCI, xHCI), and peripherals
├── firmware/                # Binary microcode and firmware images loaded into hardware
├── fs/                      # Virtual Filesystem Switch (VFS) and 68+ filesystems
│   ├── ext2/, ext3/, ext4/  # Standard Linux Extended filesystems
│   ├── btrfs/               # Next-generation copy-on-write filesystem
│   ├── xfs/                 # High-performance 64-bit journaling filesystem
│   ├── nfs/, nfsd/          # Network File System client and server implementations
│   ├── proc/, sysfs/        # In-memory virtual diagnostic filesystems
│   └── dcache.c, inode.c    # VFS directory cache (dentry) and inode management
├── include/                 # System and kernel header directories
│   ├── linux/               # Core kernel header definitions
│   ├── asm-generic/         # Platform-independent fallback definitions
│   ├── net/                 # Networking structures (sk_buff, sockets, protocols)
│   └── trace/               # Tracepoint definitions for ftrace and perf
├── init/                    # System bootstrap and userspace transition
│   ├── main.c               # start_kernel() - the universal C entry point
│   ├── calibrate.c          # BogoMIPS timer delay calibration
│   └── initramfs.c          # In-kernel CPIO archive extractor for early userspace
├── ipc/                     # Inter-Process Communication (semaphores, message queues, shared memory)
├── kernel/                  # Fundamental OS engine
│   ├── sched.c              # Core scheduler engine and load balancing
│   ├── sched_fair.c         # Completely Fair Scheduler (CFS) implementation
│   ├── cgroup.c             # Control Groups resource accounting infrastructure
│   ├── fork.c               # copy_process, clone system call implementation
│   ├── rcutree.c            # Tree-based Read-Copy Update (RCU) implementation
│   ├── time/                # High-resolution timers (hrtimer) and tickless clock
│   ├── power/               # Suspend to RAM, hibernation (swsusp), and power states
│   └── trace/               # Function tracer (ftrace) and ring buffer engine
├── lib/                     # In-kernel utility library (zlib, crc32, string, sorting)
├── mm/                      # Advanced Virtual Memory Management
│   ├── page_alloc.c         # Buddy allocator managing physical page frames
│   ├── slub.c               # Modern SLUB slab allocator
│   ├── memory.c             # Fault handling, demand paging, COW
│   ├── rmap.c               # Reverse mapping (tracking pages back to page tables)
│   ├── vmscan.c             # Page reclaim and Out-Of-Memory (OOM) killer
│   └── memcontrol.c         # Memory cgroup resource limits
├── net/                     # Multi-protocol networking stack
│   ├── core/                # Core packet queuing, sk_buff manipulation, dev.c
│   ├── ipv4/, ipv6/         # IPv4 and IPv6 protocol implementations
│   ├── netfilter/           # In-kernel firewall, packet inspection, and NAT (iptables)
│   ├── wireless/            # 802.11 wireless networking stack (cfg80211, mac80211)
│   └── sched/               # Traffic control (tc) and packet queuing disciplines
├── samples/                 # Sample kernel modules demonstrating kprobes, tracepoints
├── scripts/                 # Kernel build toolchain, Kconfig menuconfig, checkpatch.pl
├── security/                # Linux Security Modules (LSM)
│   ├── selinux/             # NSA Security-Enhanced Linux (MAC policies)
│   ├── smack/               # Simplified Mandatory Access Control Kernel
│   ├── tomoyo/              # Pathname-based security module
│   └── keys/                # In-kernel cryptographic key management
├── sound/                   # ALSA (Advanced Linux Sound Architecture) core and drivers
├── tools/                   # Userland profiling utilities
│   └── perf/                # The Linux `perf` performance analysis tool
├── usr/                     # Early userspace initramfs cpio image builder
└── virt/                    # Virtualization infrastructure
    └── kvm/                 # Kernel-based Virtual Machine hypervisor core
```

---

## Key Source Code Files

| Source File | Functionality / Significance |
| :--- | :--- |
| `init/main.c` | Universal kernel entry point `start_kernel()`. Initializes CPU architectures, page allocator, RCU, scheduling, irq, vfs, rootfs, and launches PID 1 (`/sbin/init`). |
| `kernel/sched_fair.c` | Ingo Molnar's Completely Fair Scheduler (CFS). Uses a self-balancing red-black tree indexed by `vruntime` to ensure optimal process CPU distribution. |
| `kernel/cgroup.c` | Core Control Groups infrastructure. Manages hierarchical groupings of processes to monitor and throttle CPU, memory, and I/O resources (the foundation of modern containers). |
| `kernel/rcutree.c` | Tree Read-Copy Update (RCU). Provides lockless, wait-free concurrent read access to shared data structures across thousands of CPU cores. |
| `mm/page_alloc.c` | Physical memory buddy allocator (`alloc_pages()`). Manages page frame blocks grouped by powers of two across NUMA memory zones (`ZONE_DMA`, `ZONE_NORMAL`, `ZONE_HIGHMEM`). |
| `mm/slub.c` | Christoph Lameter's SLUB allocator. Replaced SLAB with an elegant, queue-free memory cache utilizing page struct flags and fast per-CPU local buffers. |
| `fs/dcache.c` | VFS directory cache. Implements the lock-free dentry hash table and path lookup acceleration (`d_lookup`). |
| `fs/ext4/super.c` | Ext4 filesystem registration. Implements 48-bit block addressing, delayed allocation, and extent trees supporting 16-terabyte files. |
| `net/core/dev.c` | Network packet processing backbone. Dispatches incoming frames via NAPI polling and routes packets up to the network layer via `netif_receive_skb()`. |
| `virt/kvm/kvm_main.c` | Kernel-based Virtual Machine (KVM) core. Exposes `/dev/kvm` ioctls allowing userspace (QEMU) to manage hardware-accelerated guest virtual machines. |
| `tools/perf/perf.c` | Userland entry point for `perf`. Directly interfaces with `sys_perf_event_open` to profile hardware performance counters, cache misses, and kernel execution traces. |

---

## Recommended Exploration Path

1. **Bootstrap & Early Initialization**:
   - Begin at `arch/x86/kernel/head_64.S` to see 64-bit long mode initialization.
   - Follow into `init/main.c:start_kernel()` to observe how thousands of modern kernel subsystems register themselves.
2. **Scheduling & Concurrency**:
   - Study `kernel/sched_fair.c` to see how the CFS red-black tree tracks execution credits.
   - Inspect `kernel/rcutree.c` to understand how read-copy update enables lock-free read concurrency.
3. **Memory Management**:
   - Explore `mm/page_alloc.c` for physical page allocation and `mm/slub.c` for kernel object caches.
4. **Storage & Virtualization**:
   - Follow a disk read from `fs/namei.c` -> `fs/ext4/inode.c` -> `block/blk-core.c`.
   - Inspect `virt/kvm/kvm_main.c` to understand how Linux intercepts hardware hypervisor traps.
