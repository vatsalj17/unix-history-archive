# PDP7-Unix Source Code Archive

## Historical Overview
- **Release / Development Era:** 1969 – 1970
- **Authors & Key Contributors:** Ken Thompson, Dennis Ritchie, Rudd Canaday, Doug McIlroy, and Joe Ossanna at Bell Telephone Laboratories (Murray Hill, New Jersey, Computer Science Research Center).
- **Target Machine Architecture:** Digital Equipment Corporation (DEC) PDP-7 (18-bit word minicomputer, 4K to 8K 18-bit words of core memory, RB09 fixed-head disk, paper tape reader/punch, and a DEC Graphic-2 vector display terminal).
- **Historical Milestones:**
  - **The Genesis of UNIX:** Conceived after Bell Labs withdrew from the cumbersome MULTICS project in early 1969. Ken Thompson found an idle DEC PDP-7 and, famously aided by Ritchie and Canaday, implemented a lightweight multi-tasking operating system with a hierarchical file system in late summer 1969.
  - **The First Inode and Hierarchical Filesystem:** Introduced inodes (information nodes), directory files containing filename-to-inode mappings, and pathname resolution (`namei`).
  - **Origins of the Shell and Pipeline Concept:** The initial design featured independent processes connected by basic redirection and communication primitives, leading McIlroy and Thompson to formalize pipelines in later editions.
  - **Language Evolution:** Written 100% in DEC PDP-7 assembly language (`.s`), with early applications written in Ken Thompson's **B language** (`.b`), the direct precursor to C.
  - **Source Provenance:** Surviving paper scans discovered in Bell Labs notebooks by Norman Wilson and Robert Morris, subsequently transcribed into digital assembly code by Warren Toomey and the TUHS (The Unix Heritage Society) team.

---

## Directory Tree & Navigation Map

```
PDP7-Unix/
├── cmd/              [Userland commands, utilities, early B programs, assembler]
└── sys/              [Kernel source code in 18-bit PDP-7 assembly]
```

### Subdirectory Breakdown
- `sys/`: The entire monolithic PDP-7 Unix kernel divided across 9 functional segments (`s1.s` through `s9.s`), instruction definition macros (`sop.s`), bootstrap loaders, and symbol maps.
- `cmd/`: Historical command-line utilities, file maintenance tools, the PDP-7 native assembler (`as.s`), Ken Thompson's B programs (`ind.b`, `lcase.b`), and Graphic-2 terminal graphics routines.

---

## Key Source Code Files

| Component / Subsystem | File Link | Description |
| :--- | :--- | :--- |
| **Trap Vector & Interrupt Entry** | `sys/s1.s` | Hardware interrupt handler (`orig: hlt; jmp pibreak`), syscall dispatching on `CAL` (PDP-7 call subroutine instruction), accumulator saving (`u.ac`), and user-to-kernel context transition. |
| **Filesystem & Inode Management** | `sys/s2.s` | Implementation of pathname traversal (`namei`), inode lookup (`iget`), file status syscall (`.status`), and inode copy routines. |
| **Process Table & Dispatcher** | `sys/s3.s` | Process table lookup routine (`searchu`), process queue traversal, and process state manipulation. |
| **Block Allocation & Disk I/O** | `sys/s4.s` | Disk block allocator (`alloc`), free block caching (`s.fblks`, `s.nfblks`), disk buffer reading and zeroing (`copyz`). |
| **Process Swapping & Permissions** | `sys/s5.s` | Swapping process core memory to disk (`dskswap`), reading/writing user state (`userdata`), and user ID / access permission verification (`access`). |
| **File Truncation & Inode Deallocation** | `sys/s6.s` | File truncation (`itrunc`) releasing indirect and direct disk blocks back to the free list. |
| **Device Interrupts & Peripheral I/O** | `sys/s7.s` | Priority interrupt chain (`pibreak`), DEC Graphic-2 display terminal driver, paper tape reader/punch. |
| **Kernel Globals & Memory Manifests** | `sys/s8.s` | Kernel constants (`mnproc = 10` for max 10 processes, `dspbsz = 270`), run status flags, swap parameters, and pointer tables (`ulist`, `ofiles`). |
| **Cold Boot & Disk Initialization** | `sys/s9.s` | Cold boot loader (`coldentry`): formats the disk, zeros the i-list from block 0 to 709, and creates the free list from block 710 to 6399. |
| **Opcode Macro Definitions** | `sys/sop.s` | PDP-7 machine instruction manifests and mnemonic definitions (`lac`, `dac`, `tad`, `jms`, `dzm`, `ral`, `hlt`). |
| **System Install & Boot Harnesses** | `sys/maksys.s`, `sys/trysys.s` | Low-level utilities to write a compiled `a.out` kernel onto raw disk track 18x and test-boot physical memory. |
| **Assembly Symbol Map** | `sys/sysmap` | Transcribed symbol address table mapping system call entry points (`.fork`, `.exit`, `.read`, `.write`) to their 18-bit octal core addresses. |
| **Process Initialization** | `cmd/init.s` | The earliest Unix `init` daemon that attaches teletypes, spawns user login sessions, and manages terminal lines. |
| **PDP-7 Native Assembler** | `cmd/as.s` | The assembly language assembler written for PDP-7 Unix to assemble kernel and userland sources. |
| **Early B Language Programs** | `cmd/ind.b`, `cmd/lcase.b` | Extremely rare surviving examples of Ken Thompson's original **B language** source code (`ind.b` program indentation utility, `lcase.b` case mapping utility). |
| **Interactive Text Editor** | `cmd/ed1.s`, `cmd/ed2.s` | Ken Thompson's original assembly implementation of the standard Unix editor `ed`, incorporating regular expression algorithms based on his 1968 CACM paper. |

---

## Recommended Exploration Path

For a systems programmer wishing to understand how the very first Unix operated:

1. **Start with Machine Instructions:**
   Read `sys/sop.s` first. Because PDP-7 assembly differs substantially from modern x86 or ARM (using instructions like `lac` load accumulator, `dac` deposit accumulator, and `tad` two's complement add), understanding these macros will demystify the assembly syntax.
2. **Examine the Kernel Entry & Traps:**
   Study `sys/s1.s` to see how the PDP-7 hardware `CAL` instruction is intercepted at location `020` to invoke system calls, and how user registers are saved.
3. **Trace Pathname Resolution & Inodes:**
   Read `sys/s2.s` to understand `namei`—how Unix parsed slash-separated directory paths by reading directory entries sequentially to extract inode numbers.
4. **Follow Memory Allocation & Swapping:**
   Look at `sys/s4.s` for the free block list management, and `sys/s5.s` to see how the system swapped user memory directly to the RB09 drum/disk when switching between its maximum of 2 concurrent users.
5. **Inspect the Birth of B and Early Userland:**
   Open `cmd/ind.b` to see syntax from Thompson's B language, and look at `cmd/ed1.s` to see how the line editor was written.
