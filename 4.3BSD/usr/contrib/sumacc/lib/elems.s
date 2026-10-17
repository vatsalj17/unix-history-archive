|-----------------------------------------------------------
| 25may84: C version, uses macros to save my fingers. (croft)
|-----------------------------------------------------------
	.insrt	"../h/sanemacs.h"

__fp68k = /a9eb		| avoids including toolmacs.h
__elems68k = /a9ec

	.text

	.macro c1,l,c
	.globl l
l:	movw	#c,d0
	bras	call1
	.endm
	.macro c2,l,c
	.globl l
l:	movw	#c,d0
	bras	call2
	.endm
	.macro c3,l,c
	.globl l
l:	movw	#c,d0
	bras	call3
	.endm

call1:	movl	sp@+,a0
	movw	d0,sp@-
	.word	__elems68k
	subql	#4,sp
	jmp	a0@

call2:	movl	sp@+,a0
	movl	sp@,d1
	movl	sp@(4),sp@
	movl	d1,sp@(4)
	movw	d0,sp@-
	.word	__elems68k
	subql	#8,sp
	jmp	a0@

	c1,Log2X,flog2x
	c1,LnX,flnx
	c1,Ln1X,fln1x
	c1,Exp2X,fexp2x
	c1,ExpX,fexpx
	c1,Exp1X,fexp1x
	c2,XpwrI,fxpwri
	c2,XpwrY,fxpwry
	c1,SinX,fsinx
	c1,CosX,fcosx
	c1,TanX,ftanx
	c1,AtanX,fatanx
	c1,RandomX,frandomx
	c3,Compound,fcompoundx
	c3,Annuity,fannuityx

call3:	movl	sp@+,a0
	movl	sp@,d1
	movl	sp@(8),sp@
	movl	d1,sp@(8)
	movw	d0,sp@-
	.word	__elems68k
	subw	#12,sp
	jmp	a0@

