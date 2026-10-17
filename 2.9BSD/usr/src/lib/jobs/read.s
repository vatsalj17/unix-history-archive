/*	@(#)read.s	2.2	SCCS id keyword	*/
/ C library -- read

/ nread = read(file, buffer, count);
/ nread ==0 means eof; nread == -1 means error

.globl	_read
.globl	cerror

_read:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),r0
	mov	8(r5),-(sp)
	mov	6(r5),-(sp)
	sys	read+200
	bec	1f
	cmp	(sp)+, (sp)+
	jmp	cerror
1:
	cmp	(sp)+, (sp)+
	mov	(sp)+,r5
	rts	pc
