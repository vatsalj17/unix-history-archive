# NFSv2 Source Code Archive

## Historical Overview

| Attribute | Details |
| :--- | :--- |
| **System Name** | Sun Microsystems Network File System Release 2.0 (NFS 2.0 / VFS Subsystem) |
| **Release Date** | December 16, 1985 (Documented in `release.ms`) |
| **Origin / Creators** | Sun Microsystems, Inc. (Mountain View, CA) — Russel Sandberg, Bob Lyon, Bill Joy, Steve Kleiman, Dan Walsh |
| **Source Donation** | Preserved from Sun's 1985 external distribution tape and archived in TUHS |
| **Target Architecture** | DEC VAX (port for 4.2BSD) and Motorola 68010/68020 (Sun-2, Sun-3 workstations) |
| **Key Historical Milestones** | **Invention of the Virtual File System (VFS) and Vnode interface**; release of NFS Version 2 (RFC 1094); invention of the Directory Name Lookup Cache (DNLC); introduction of ONC RPC, XDR, and Yellow Pages (YP/NIS) |

### Significance & Historical Context
In 1984–1985, Sun Microsystems revolutionized distributed computing by solving one of operating systems' greatest hurdles: how to allow multiple machines across a local area network to transparently share files as if they resided on local disks.

To achieve this, Sun engineer Steve Kleiman invented the **Virtual File System (VFS)** and **Vnode** architecture. Prior to this innovation, the UNIX file system syscalls (`open`, `read`, `write`, `close`, `stat`) were hardcoded to manipulate disk blocks and in-memory inodes belonging directly to the Berkeley Fast File System (UFS). Kleiman inserted an object-oriented virtual dispatch table into the kernel (`struct vnodeops` and `struct vfsops`), decoupling file semantics from storage implementations. Local filesystems (UFS) and remote filesystems (NFS) simply implemented the same vnode operation vectors (`VOP_LOOKUP`, `VOP_READ`, `VOP_WRITE`). Today, virtually every modern operating system (Linux, macOS, FreeBSD, Windows NT, Solaris) utilizes a descendant of Sun's VFS/Vnode design.

Alongside VFS, Sun introduced:
1. **The NFSv2 Protocol (RFC 1094)**: Built over UDP and designed to be **stateless**. The server maintains no open file descriptors or client state; each request is authenticated and contains a persistent file handle (`fhandle`). If an NFS server crashes and reboots, clients simply pause until the server resumes, without lost files or application crashes.
2. **Directory Name Lookup Cache (DNLC)**: Pathname traversal across a network (e.g. `/usr/local/bin/prog`) could require dozens of network round trips. Sun invented the DNLC (`vfs_dnlc.c`) to cache directory name-to-vnode mappings in memory, dramatically accelerating network performance.
3. **ONC RPC & XDR**: Created Open Network Computing Remote Procedure Calls and eXternal Data Representation to transparently marshal structured data across machines with different CPU endianness and word sizes.
4. **Yellow Pages (YP / NIS)**: Centralized network user authentication and hosts database lookup across LAN clusters.

---

## Directory Tree & Navigation Map

```
NFSv2/
├── release.ms                   # Original troff release document dated Dec 16, 1985
├── samples/                     # Configuration templates for network boot
│   ├── rc.local                 # Sample boot script mounting NFS filesystems
│   ├── servers                  # Sample server mapping tables
│   └── services                 # Internet services configuration
└── usr/
    ├── man/                     # Manual pages (man1, man2, man3, man5, man8)
    └── src/                     # Complete NFS v2 and VFS Source Tree
        ├── bin/, ucb/           # User utilities updated for NFS paths
        ├── etc/                 # System administration daemons and mount tools
        │   ├── biod.c           # Asynchronous block I/O client daemon
        │   ├── nfsd.c           # In-kernel NFS server listener daemon
        │   ├── mount.c, umount  # Filesystem mount utilities
        │   ├── portmap.c        # ONC RPC port mapper daemon
        │   └── yp*, ypserv      # Yellow Pages (NIS) client and server daemons
        ├── include/             # System headers and RPC service headers (rpcsvc/)
        ├── lib/                 # Standard C library (libc) with RPC/XDR runtime
        ├── sun/                 # Sun Microsystems Core Kernel & Documentation
        │   ├── doc/             # Sun NFS documentation
        │   └── sys/             # The VFS and NFS Kernel Implementation
        │       ├── conf/        # Kernel build configuration for VAX (GENERIC.VAX)
        │       ├── h/           # Kernel headers (vfs.h, vnode.h, etc.)
        │       ├── net, netinet # 4.2BSD network socket and TCP/IP stack
        │       ├── nfs/         # The NFS Version 2 Protocol Implementation
        │       ├── sys/         # Core VFS subsystem (vfs.c, vfs_vnode.c, dnlc.c)
        │       ├── ufs/         # Berkeley Fast File System adapted to VFS vnodes
        │       └── vax/         # VAX hardware bootstrap and machine dependencies
        ├── usr.bin/             # User network applications
        ├── usr.etc/             # Diagnostic and administrative network tools
        │   ├── nfsstat.c        # NFS client and server statistics viewer
        │   ├── rpc.mountd.c     # NFS mount protocol server daemon
        │   ├── rpcinfo.c        # RPC query tool
        │   └── rpc.rstatd.c ... # Remote status and statistics daemons
        └── usr.lib/             # Network database libraries (libdbm)
```

---

## Key Source Code Files

### 1. The Virtual File System (VFS) and Vnode Subsystem
- `usr/src/sun/sys/sys/vfs.c`: Core VFS routines: mount table management, root filesystem mounting, and filesystem type dispatching (`vfs_mount`, `vfs_unmount`, `vfs_root`, `vfs_statfs`).
- `usr/src/sun/sys/sys/vfs_vnode.c`: Vnode life-cycle management: allocation (`vn_alloc`), reference counting (`VN_HOLD`, `VN_RELE`), and inactive reclamation.
- `usr/src/sun/sys/sys/vfs_dnlc.c`: Steve Kleiman's Directory Name Lookup Cache (DNLC). Uses a hash table to cache `(directory vnode, component name) -> target vnode` translations.
- `usr/src/sun/sys/sys/vfs_lookup.c` & `usr/src/sun/sys/sys/vfs_pathname.c`: Pathname traversal engine decomposing paths into components and delegating resolution to vnodes.

### 2. The NFS Version 2 Protocol Implementation (`nfs/`)
- `usr/src/sun/sys/nfs/nfs_vfsops.c`: NFS implementation of the VFS operations vector: mounting remote NFS exports and managing client mount structures.
- `usr/src/sun/sys/nfs/nfs_vnodeops.c`: NFS client vnode operations: `nfs_lookup`, `nfs_read`, `nfs_write`, `nfs_create`, `nfs_remove`, `nfs_getattr`, and `nfs_setattr`.
- `usr/src/sun/sys/nfs/nfs_server.c`: In-kernel NFS server dispatch table processing incoming client RPC requests over UDP port 2049.
- `usr/src/sun/sys/nfs/nfs_xdr.c`: External Data Representation serializers and deserializers for NFS arguments and results (file handles, attributes, directory entries).

### 3. User-Space Daemons & Yellow Pages (NIS)
- `usr/src/etc/nfsd.c`: Spawns the in-kernel worker threads that service incoming NFS server RPC requests.
- `usr/src/etc/biod.c`: Client block I/O asynchronous worker daemon enabling read-ahead and write-behind for NFS mounts.
- `usr/src/usr.etc/rpc.mountd.c`: Mount daemon handling client mount authorization against `/etc/exports` and returning root file handles.
- `usr/src/etc/portmap.c`: ONC RPC port mapper dynamically mapping RPC program numbers (e.g. 100003 for NFS, 100005 for MOUNT) to active UDP/TCP ports.

---

## Recommended Exploration Path

1. **Read the Official Release Announcement**:
   - Begin by reading `release.ms` to see the original December 1985 release notes from Sun Microsystems.
2. **Study the Architecture of VFS and Vnodes**:
   - Inspect `usr/src/sun/sys/h/vfs.h` and `usr/src/sun/sys/sys/vfs.c` to observe the function pointer table (`vfsops`) that revolutionized operating system storage.
3. **Trace Directory Name Caching**:
   - Read `usr/src/sun/sys/sys/vfs_dnlc.c` to understand how Sun bypassed expensive network lookups using in-memory path hashing.
4. **Follow an NFS RPC Request**:
   - Follow a file read from client invocation in `usr/src/sun/sys/nfs/nfs_vnodeops.c` (`nfs_read`), across serialization in `usr/src/sun/sys/nfs/nfs_xdr.c`, to server execution in `usr/src/sun/sys/nfs/nfs_server.c`.
5. **Inspect the Mounting Infrastructure**:
   - Review `usr/src/usr.etc/rpc.mountd.c` and `usr/src/etc/biod.c` to observe how the client and server coordinated authentication and background block buffering.
