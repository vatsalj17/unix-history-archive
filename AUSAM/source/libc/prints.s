/ C library -- prints

/ prints(fd, string);
/
/ print "string" on file "fd" without buffering

.globl	_prints

_prints:
	mov	r5,-(sp)
	mov	sp,r5
	mov	6(r5),r0
	mov	r0,0f
1:
	tstb	(r0)+
	bne	1b
	sub	6(r5),r0
	dec	r0
	mov	r0,0f+2
	mov	4(r5),r0
	sys	indir; 9f
	mov	(sp)+,r5
	rts	pc

.data
9:
	sys	write; 0: ..; ..
