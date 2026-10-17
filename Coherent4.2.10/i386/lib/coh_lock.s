/ $Header: $
/ This file contains implementations of multiprocessor locking primitives used
/ by the STREAMS and DDI/DDK subsystems. With GCC or other compilers that allow
/ in-line generation of assembly language code in C programs a separate
/ assembly-language file containing these implementations is not necessary.
/ Since the MWC C compiler that is the default development tool under Coherent
/ does not permit inlining, references to the locking functions will be turned
/ into external function calls that will be resolved by the routines below.
/
/ $Log: $
/
		.unixorder
		.globl	ATOMIC_FETCH_AND_STORE_CHAR
		.globl	ATOMIC_FETCH_AND_STORE_UCHAR
		.globl	ATOMIC_FETCH_AND_STORE_SHORT
		.globl	ATOMIC_FETCH_AND_STORE_USHORT
		.globl	ATOMIC_FETCH_AND_STORE_INT
		.globl	ATOMIC_FETCH_AND_STORE_UINT
		.globl	ATOMIC_FETCH_AND_STORE_LONG
		.globl	ATOMIC_FETCH_AND_STORE_ULONG
		.globl	ATOMIC_FETCH_AND_STORE_PTR

/ The routines work by using the i386 feature that all cycles involving a
/ single read or write are atomic regardless of alignment or lack thereof, and
/ by using the XCHG instruction, which is an atomic read-modify-write
/ instruction. Use of the atomic exchange primitive allows more efficient
/ implementation of some data structures than the more fundamental test-and-set
/ instruction (but is not as powerful as the atomic compare-and-swap
/ instruction found in the Motorola 680x0 processors).
/ Incidentally, atomic exchange is the only atomic operation in many new RISC
/ processors such as the Motorola 88100.

/ The C-language header file <sys/x86lock.h> defines data types and function
/ prototypes that should match the definitions expected in this file. Since the
/ regular read and write operations on the i386 are atomic, the definitions in
/ this file are for those operations that cannot be performed in C; the other
/ operations can be safely described by macros with the use of the 'volatile'
/ keyword to prevent optimisation of accesses to these items (since other CPUs
/ have the ability to modify the values contained in these locations, the
/ data-flow analysis often performed by compilers to allow cacheing of values
/ in registers would cause incorrect results).

/ Under Coherent, the iBCS2 function calling-sequence rules are in effect. This
/ means that the registers %ebx, %esi and %edi are used for register variables
/ and must be preserved by routines. However, all other registers are available
/ for modification; with the i386 CPU the general registers %eax, %ecx and %edx
/ are available for use as index registers with the addition of the SIB
/ instruction forat. Parameters are passed in the stack from right to left,
/ with the caller's return address being the "topmost" entry.

/ char     ATOMIC_FETCH_AND_STORE_CHAR (atomic_char_t _item, char _value);
ATOMIC_FETCH_AND_STORE_CHAR:
/ uchar_t  ATOMIC_FETCH_AND_STORE_UCHAR (atomic_uchar_t _item, uchar_t _value);
ATOMIC_FETCH_AND_STORE_UCHAR:

		movl	4(%esp), %edx	/ Address of atomic item
		movl	8(%esp), %eax	/ Value to store

		xchg	%al, (%edx)	/ Atomic fetch-and-store

		movzxb	%al, %eax	/ Just to be sure... there don't seem
					/ to be any consistent rules for value
					/ extension in 'cc'.
		ret

/ short    ATOMIC_FETCH_AND_STORE_SHORT (atomic_short_t _item, short _value);
ATOMIC_FETCH_AND_STORE_SHORT:
/ ushort_t ATOMIC_FETCH_AND_STORE_USHORT (atomic_ushort_t _item,
/					  ushort_t _value);
ATOMIC_FETCH_AND_STORE_USHORT:

		movl	4(%esp), %edx	/ Address of atomic item
		movl	8(%esp), %eax	/ Value to store

		xchg	%ax, (%edx)	/ Atomic fetch-and-store

		movzxw	%ax, %eax	/ Just to be sure... there don't seem
					/ to be any consistent rules for value
					/ extension in 'cc'.

		ret			/ return to caller

/ int      ATOMIC_FETCH_AND_STORE_INT (atomic_int_t _item, int _value);
ATOMIC_FETCH_AND_STORE_INT:
/ uint_t   ATOMIC_FETCH_AND_STORE_UINT (atomic_uint_t _item, uint_t _value);
ATOMIC_FETCH_AND_STORE_UINT:
/ long     ATOMIC_FETCH_AND_STORE_LONG (atomic_long_t _item, long _value);
ATOMIC_FETCH_AND_STORE_LONG:
/ ulong_t  ATOMIC_FETCH_AND_STORE_ULONG (atomic_ulong_t _item, ulong_t _value);
ATOMIC_FETCH_AND_STORE_ULONG:
/ _VOID  * ATOMIC_FETCH_AND_STORE_PTR (atomic_ptr_t _item, _VOID * value);
ATOMIC_FETCH_AND_STORE_PTR:

		movl	4(%esp), %edx	/ Address of atomic item
		movl	8(%esp), %eax	/ Value to store

		xchg	%eax, (%edx)	/ Atomic fetch-and-store

		ret			/ return to caller
