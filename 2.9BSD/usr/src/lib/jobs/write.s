/*	@(#)write.s	2.2	SCCS id keyword	*/
/ C library -- write

/ nwritten = write(file, buffer, count);
/
/ nwritten == -1 means error

.globl	_write
.globl	cerror

_write:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),r0
	mov	8(r5),-(sp)
	mov	6(r5),-(sp)
	sys	write+200
	bec	1f
	cmp	(sp)+, (sp)+
	jmp	cerror
1:
	cmp	(sp)+, (sp)+
	mov	(sp)+,r5
	rts	pc
