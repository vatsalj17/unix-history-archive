/ C library -- smdate

/
/ char *string;
/ long date;
/
/ error = smdate(string, date);

.globl	_smdate, cerror

_smdate:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4.(r5),0f
	mov	6.(r5),r0
	mov	8.(r5),r1
	sys	0; 9f
	bec	1f
	jmp	cerror
1:
	clr	r0
	mov	(sp)+,r5
	rts	pc
.data
9:
	sys	mdate; 0: ..
