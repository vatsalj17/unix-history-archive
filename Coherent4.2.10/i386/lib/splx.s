/ $Header: $
		.unixorder

/ Miscellaneous functions that manipulate the concrete interrupt-priority
/ level on the basis of requests to change abstract priority-levels.
/	pl_t splbase (void)
/	pl_t spltimeout (void);
/	pl_t spldisk (void);
/	pl_t splstr (void);
/	pl_t splhi (void);
/	pl_t splx (pl_t _pl);
/	pl_t splraise (pl_t _pl);
/ All are prototyped in <sys/inline.h>, and all are DDI/DKI except for the
/ splraise () function, which is only used internally by some STREAMS-related
/ multiprocessor-locking code.
/
/ $Log: $
/
		.include	"struct.inc"
		.include	"pl.inc"
		.include	"ddi.inc"

		.set	PICM, 0x21
		.set	SPICM, 0xA1

		.globl	_masktab			/ array of longs

		.text

		.globl	splbase
		.globl	spltimeout
		.globl	spldisk
		.globl	splstr
		.globl	splhi
		.globl	splx
		.globl	splraise

splbase:	mov	$plbase, %ecx
		jmp	setnew

spltimeout:	mov	$pltimeout, %ecx
		jmp	setnew

spldisk:	mov	$pldisk, %ecx
		jmp	setnew

splstr:		mov	$plstr, %ecx
		jmp	setnew

splhi:		mov	$plhi, %ecx
		jmp	setnew

splx:
		movzxb	4(%esp), %ecx			/ new pl
setnew:
		movl	_masktab(%ecx,4), %eax		/ new mask

	/ send the mask out to the PICs after including the base mask info.
		orl	cpu+dc_base_mask, %eax
		outb	$PICM
		movb	%ah, %al
		outb	$SPICM

	/ swap the new IPL for the old one, and return the old one.
		movzxb	cpu+dc_ipl, %eax
		movb	%cl, cpu+dc_ipl

		ret

splraise:
		movzxb	4(%esp), %ecx			/ new pl

		cmpb	cpu+dc_ipl, %cl
		jg	setnew

		movzxb	cpu+dc_ipl, %eax		/ just return old
		ret

