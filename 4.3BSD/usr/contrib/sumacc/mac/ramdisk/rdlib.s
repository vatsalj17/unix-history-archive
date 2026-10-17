	.insrt	"h.h"

	.text
	.globl	NewPtrSys
NewPtrSys:
	movl	sp@(4),d0
	.word	__newptr+_sys_
	movl	a0,d0
	rts

	.globl	bcopy
bcopy:
	movl	sp@(4),a0
	movl	sp@(8),a1
	movl	sp@(12),d0
	subql	#1,d0
.L1:
	movb	a0@+,a1@+
	dbra	d0,.L1
	rts

	.globl	bzero
bzero:
	movl	sp@(4),a0
	movl	sp@(8),d0
	subql	#1,d0
.L2:
	clrb	a0@+
	dbra	d0,.L2
	rts
