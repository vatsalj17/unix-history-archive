/ C library -- nargs

/ WARNING: this routine may not work
/ if your machine is an 11/45 or 11/70.
/ A minor hardware alteration is required
/ to enable mfpi (user->user) to work!!

mfpi	= 006500 ^ tst

.globl	_nargs

_nargs:
	mov	r5,-(sp)
	mov	2(r5),r1		/ pc of caller of caller
	mov	sp,r5
	clr	r0
	mfpi	-4(r1)
	cmp	(sp)+,$4737	/ jsr pc,*$0
	bne	8f
	mov	$2,r0
8:
	mfpi	(r1)
	cmp	*sp,$5726	/ tst (sp)+
	bne	1f
	add	$2,r0
	br	2f
1:
	cmp	*sp,$22626	/ tst (sp)+
	bne	1f
	add	$4,r0
	br	2f
1:
	cmp	*sp,$62706	/ add $0,sp
	bne	1f
	mfpi	2(r1)
	add	(sp)+,r0
	br	2f
1:
	cmp	*sp,$167	/ jmp 0
	bne	1f
	tst	(sp)+
	mfpi	2(r1)
	add	(sp)+,r1
	add	$4,r1
	br	8b
1:
	cmpb	1(sp),$1	/ upper byte of `br .'
	bne	2f
	mov	r0,*sp
	mfpi	*r1
	mov	(sp)+,r0
	swab	r0
	ash	$-7,r0
	add	r0,r1
	add	$2,r1
	mov	(sp)+,r0
	br	8b
2:
	tst	(sp)+
	asr	r0
	mov	(sp)+,r5
	rts	pc
