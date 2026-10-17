/ C library -- setjmp, longjmp
/ Overlay version -- beleives in 4 word jump vector

/	longjmp(a,v)
/ will generate a "return(v)" from
/ the last call to
/	setjmp(a)
/ by restoring sp, r5, pc, ovno from `a'
/ and doing a return.
/

.globl	_setjmp
.globl	_longjmp
.globl	csv, cret
.globl  __ovno

_setjmp:
	mov	2(sp),r0
	mov	r5,(r0)+
	mov	sp,(r0)+
	mov	(sp),(r0)+	/
	mov	__ovno,(r0)	/
	clr	r0
	rts	pc

_longjmp:
	jsr	r5,csv
	mov	4(r5),r1
	mov	6(r5),r0
	bne	1f
	mov	$1,r0
1:
	cmp	(r5),(r1)
	beq	1f
	mov	(r5),r5
	bne	1b
/ panic -- r2-r4 lost
	mov	(r1)+,r5
	mov	(r1)+,sp
	mov	(r1)+,(sp)
				/ make pseudo-frame, so can use cret
				/ to do return;
				/ this routine can be anywhere instead of
				/ forced into the root overlay
	mov	r5,-(sp)
	mov	sp,r5
	mov	(r1),-(sp)	/ old ovno
	mov	r4,-(sp)
	mov	r3,-(sp)
	mov	r2,-(sp)
	jmp	cret
1:
	mov	4(r1),2(r5)
	mov	6(r1),-2(r5)	/ rewrite ovno in frame
	jmp	cret

