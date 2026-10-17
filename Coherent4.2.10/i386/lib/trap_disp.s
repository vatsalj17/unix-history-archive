/ $Header: $
		.unixorder
/ Assembly-language code to perform low-level trap dispatching. The trap-
/ handler entry point here is referred to by any code that can catch a trap
/ into ring 1 (the ring-0 trap-handler is special, and separate).
/ $Log: $
/
		.include	"struct.inc"
		.include	"intr.inc"
		.include	"ddi.inc"
		.include	"select.inc"

		.globl		__idle
		.globl		__idle_end
		.globl		trap0
		.globl		trap1_usr
		.globl		trap2
		.globl		trap3
		.globl		trap4
		.globl		trap5
		.globl		trap6
		.globl		trap7
		.globl		trap8
		.globl		trap9
		.globl		trap10
		.globl		trap11
/		.globl		trap12
		.globl		trap14
		.globl		trap16
		.globl		syscall_286
		.globl		syscall_386
		.globl		signal_386

	/ externals

		.globl		disflag		/ data
		.globl		trap_level
		.globl		return_to_user
		.globl		__debug_usr__
		.globl		trap
		.globl		fptrap
		.globl		emtrap
		.globl		gpfault
		.globl		pagefault
		.globl		msigend

		.text

/ We place the idle process code in this module because it interacts with the
/ trap and interrupt-dispatch code.
/ Try flagging at user-level first so that when interrupts are
/ enabled it is correct.

__idle:	
		sti				/ can always interrupt
/		decb	cpu+dc_user_level	/ flag at user-level

?loop:		movl	$1, disflag		/ suggest dispatch

	/ Under normal circumstances, we would perform a halt here; however,
	/ this would mean we had to be at ring 0, which is really a problem
	/ given our current IDT/TSS and privelege ring arrangement. We really
	/ want to be able to virtualize interrupts so that the ring-0
	/ dispatcher reflects the interrupt to the device-driver ring, and we
	/ want the halt to happen in the dispatch layer (probably ring 0)
	/ rather than having an idle process.

		jmp	?loop
__idle_end:

/ Generic trap/exception entry-point handler. This duplicates some of the
/ major functionality of the interrupt-dispatching code, so look at that too
/ whenever this gets changed.

__trap_save:
		/ If we entered here via anything but an interrupt
		/ gate, interrupts are still enabled.  Caveat interruptor.
		pusha
		push	%ds			/ Save current state
		push	%es
		push	%fs
		push	%gs

		xor	%ebp, %ebp		/ Halt stack backtraces

		movw	$SEL_386_KD, %dx	/ Map ds, es
		movw	%dx, %ds
		movw	%dx, %es

		movw	$SEL_386_UD, %dx
		movw	%dx, %fs

	/ Enable interrupts and call-back the code that called us.

		sti
		icall	_trapno(%esp)		/ call our caller

	/ Now we test to see whether we came from user mode (or the idle
	/ process, which tests the same), and if so we run the routine
	/ for "return-to-user-mode" to check for rescheduling.

		call	trap_level
		testl	%eax, %eax
		jne	?skip_dispatch

		call	return_to_user

?skip_dispatch:
		cli

		pop	%gs			/ restore saved context
		pop	%fs
		pop	%es
		pop	%ds
		popa
		add	$8, %esp		/ forget err, trapno
		iret				/ Done.

///////
/ Trap and interrupt linkage.
/ Each of the machine traps has a special little
/ linkage, that sets up the type code and sends
/ control off to the common trap processor. Device
/ interrupts are done here.
///////

trap0:
		push	$0			/ Divide error.
		call	__trap_save
		jmp	trap

/ The debug vector is tricky.
/
/ If single stepping user code, the vector must point into Ring 1 code
/ so that a ptraced child can be synchronized with its parent.
/	use trap1_usr for this
/
/ If single stepping the kernel, the vector must point into Ring 0 code
/ so context switches switch out the debug stack frame.
/	use trap1_ker for this

trap1_usr:
		push	$1			/ Single step.
		call	__trap_save
		jmp	__debug_usr__

trap2:
		push	$2			/ Non-maskable interrupt.
		call	__trap_save
		jmp	trap

trap3:
		push	$3			/ INT 3 (breakpoint).
		call	__trap_save
		jmp	trap

trap4:
		push	$4			/ Overflow.
		call	__trap_save
		jmp	trap

trap5:
		push	$5			/ Bound check.
		call	__trap_save
		jmp	trap

trap6:
		push	$6			/ Invalid opcode.
		call	__trap_save
		jmp	trap

trap7:
		push	$7			/ Processor Extension
		call	__trap_save		/ not available.
		jmp	emtrap

trap8:
		movl	$8, (%esp)		/ Double Exception detected
		call	__trap_save
		jmp	trap

trap9:
		push	$9			/ Processor extension
		call	__trap_save		/ segment overrun
		jmp	trap

trap10:
		movl	$10, (%esp)		/ Invalid TSS
		call	__trap_save
		jmp	trap

trap11:
		movl	$11, (%esp)		/ Segment not present
		call	__trap_save
		jmp	trap

/ Now in Ring 0
/trap12:
/		movl	$12, (%esp)		/ Stack segment overrun or
/		call	__trap_save		/ not present
/		jmp	trap

trap14:
		call	__trap_save
		jmp	pagefault

trap16:
		push	$16			/ Floating point error
		call	__trap_save
		jmp	fptrap

syscall_286:
		push	$0x22			/ Old format system calls.
		call	__trap_save		/ (random cookie)
		jmp	trap

/ 32-bit system call. Note that the this is entered from a call gate, and that
/ the call gate requests a fake parameter. This fake parameter takes a 32-bit
/ slot above the return CS:EIP and below the return SS:ESP which is where the
/ flags are placed by an interrupt gate. We then use an interlevel IRET to
/ change the user flags.
		.set	FAKE_EFLAGS, 12
syscall_386:
		/ WARNING!!! interupts are not shut off by a call
		/ gate!!  They are only shut off by an interrupt gate.
		/ Interrupts are on at this point!

		push	%eax			/ save %eax
		pushf				/ modify current flags
		pop	%eax
		mov	%eax, FAKE_EFLAGS(%esp)
		pop	%eax			/ restore %eax

		push	$0x20
		call	__trap_save
		jmp	trap

signal_386:
		push	%eax
		pushf
		pop	%eax
		mov	%eax, FAKE_EFLAGS(%esp)
		pop	%eax

		push	$0x20			/ New format signal return.
		call	__trap_save
		jmp	msigend

