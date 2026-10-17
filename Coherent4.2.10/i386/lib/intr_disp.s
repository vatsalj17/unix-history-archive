/ $Header: $
		.unixorder
/ Assembly-language code to perform low-level interrupt dispatching.
/ The entry points defined here are referred to by the code elsewhere that
/ defines the initial IDT contents.
/ $Log: $
/
		.include	"struct.inc"
		.include	"intr.inc"
		.include	"ddi.inc"
		.include	"select.inc"

		.globl		return_to_user		/ external
		.globl		return_from_interrupt	/ external
		.globl		trap_level		/ external
		.text

/ Generic interrupt entry-point handler. This duplicates some of the major
/ functionality of "tsave", so ensure that we stay in synch somehow.
/
/ Here, we create fairly standard saved context frame, even though it wastes
/ some time.

context_save:
		pusha
		push	%ds			/ Save current state
		push	%es
		push	%fs
		push	%gs

	/ Now we can play with the registers, fetch the trap structure
	/ address from the stack frame. We store the previous contents of
	/ the global vector mask in a stack frame slot that is unused for us.

		movw	$SEL_386_KD, %dx	/ Map %ds to kernel data
		movw	%dx, %ds

		movl	_err(%esp), %ebx	/ trap information pointer
		movl	cpu+dc_base_mask, %eax	/ get old mask
		movl	%eax, _err(%esp)	/ save old mask

		orl	_int_mask(%ebx), %eax
		movl	%eax, cpu+dc_base_mask	/ set new mask

	/ We interleave some other setup in with writing the new mask and
	/ sending EOI's to the interrupt controllers so we don't need to get
	/ into the I/O recovery delay games. The ideal choice is a segment
	/ register maniplation, since they take so long.

		outb	$PICM			/ set new master mask

		movw	%dx, %es		/ map %es to kernel data

		movb	%ah, %al
		outb	$SPICM			/ set new slave mask

	/ Now we have written the masks, we need to send EOI's to the
	/ interrupt controllers to allow the non-masked interrupts to proceed.
	/ Again, we thread some extra setup through here to avoid having to
	/ play the I/O recovery-delay game.

		movb	$EOI_COMMAND, %al
		cmpl	$0, _trapno (%esp)
		je	?master_eoi

		outb	$SPICC			/ Slave PIC EOI
?master_eoi:
		xor	%ebp, %ebp		/ Halt stack backtraces
		subl	%edx, %edx		/ set fs to NULL, since we
		movw	%dx, %fs		/ have no user context.

		outb	$PICC			/ Master PIC EOI

	/ Now we have done pretty much all the setup that is possible, we
	/ increment the global DDI/DKI flag that says we are in an interrupt
	/ context, and increment our flag that says we are in the kernel
	/ before globally enabling interrupts in the CPU.

		incb	cpu+dc_int_level	/ in an interrupt
		sti

		incl	_int_count(%ebx)	/ count interrupt

		pushl	%ebx			/ point at interrupt info
		pushl	_int_arg(%ebx)		/ arg for user function
		icall	_int_func(%ebx)		/ call the function
		addl	$8, %esp		/ discard arguments

	/ Now we can restore the PIC mask settings. We disable interrupts
	/ while we do this, because even while we don't care about this being
	/ interrupted we have to avoid infinite stack growth. From here to the
	/ final interrupt return has to be disabled, with the possible
	/ exception of the code to process deferred functions and whatnot
	/ (which cannot be re-entered due to the protocol for managing the
	/ dc_int_level/dc_user_level flag).

		movl	_err(%esp), %eax	/ load saved mask
		cli
		outb	$PICM			/ restore master mask

		movl	%eax, cpu+dc_base_mask	/ restore base mask
		cmpb	$2, cpu+dc_int_level	/ what int level are we at?

		mov	%ah, %al
		outb	$SPICM			/ restore slave mask

		jae	?skip_defer		/ if >= 2, don't do defer

	/ There are two kinds of deferred routine; DDI/DKI and Coherent. The
	/ DDI/DKI semantics don't specify when the deferred routines are run.
	/ The Coherent routines historically run just before return to user
	/ mode, but it is not clear whether this is important.

		cmpb	$0, cpu+dc_ipl		/ if ipl > 0, don't do defer.
		jne	?skip_defer

		sti				/ can run this enabled
		call	return_from_interrupt
		cli

?skip_defer:
		decb	cpu+dc_int_level

	/ Now we test to see whether we came from user mode (or the idle
	/ process, which tests the same), and if so we run the "stand" routine
	/ to check for rescheduling.

		call    trap_level
		testl   %eax, %eax		/ were we in user mode?
		jne	?skip_dispatch

		sti

		movw	$SEL_386_UD, %dx	/ now we *do* have context
		movw	%dx, %fs

		call	return_to_user
		cli

?skip_dispatch:
		pop	%gs			/ restore saved context
		pop	%fs
		pop	%es
		pop	%ds
		popa
		add	$8, %esp		/ forget err, trapno
		iret				/ Done.


INTR		.macro	int_no
/		.globl	__interrupt_\int_no
/__interrupt_\int_no:
		.globl	dev\int_no
dev\int_no:
		push	$ints + int_no * SIZE..intr_t

	.if	int_no < 8
		pushl	$0
	.else
		pushl	$1
	.endi
		jmp	context_save
	.endm

		INTR	0		/ periodic timer
		INTR	1		/ keyboard
					/ for chaining PIC B
		INTR	3		/ COM2
		INTR	4		/ COM1
		INTR	5		/ 
		INTR	6		/ floppy disk controller
		INTR	7		/ parallel port

		INTR	8		/ RTC
		INTR	9		/ original IRQ 2
		INTR	10
		INTR	11
		INTR	12
		INTR	13
		INTR	14		/ AT hard-disk
		INTR	15
