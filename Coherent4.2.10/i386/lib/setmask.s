/ $Header: $
		.unixorder

/ Miscellaneous functions for manipulating the interrupt mask in the Intel
/ 8259A interrupt controller (actually a high-integration functional
/ equivalent since this is a 386 or better CPU).
/
/ All functions are prototyped in <sys/inline.h>
/ $Log:
/
		.include	"struct.inc"
		.include	"pl.inc"
		.include	"ddi.inc"

		.text

		.globl	__SET_BASE_MASK
		.globl	DDI_BASE_SLAVE_MASK
		.globl	DDI_BASE_MASTER_MASK

/ void __SET_BASE_MASK (intmask_t newmask)
__SET_BASE_MASK:
		mov	4(%esp), %eax			/ new mask
		mov	%eax, cpu+dc_base_mask		/ store it
		outb	$PICM				/ mask low
		mov	%ah, %al
		outb	$SPICM				/ mask high
		ret

/ These function should only be called from base level, so that we don't
/ have to iterate over all kinds of junk to work out what the real mask
/ level should be. This condition is trivially satisfied at the moment because
/ these are only called by Coherent's functions, which aren't protected by
/ any kind of spl... (), just (possibly) an sphi ()/spl ().

/ void DDI_BASE_MASTER_MASK (uchar_t mask)
DDI_BASE_MASTER_MASK:
		movb	4(%esp), %al
		movb	%al, cpu+dc_base_mask
		outb	$PICM
		ret

/ void DDI_BASE_SLAVE_MASK (uchar_t mask)
DDI_BASE_SLAVE_MASK:
		movb	4(%esp), %al
		movb	%al, cpu+dc_base_mask+1
		outb	$SPICM
		ret

