/ C library -- locking
lock = 56.

/	readlock( fd )
/	writelock( fd )
/	lock( fd , n )
/	unlock()

.globl	_readlock,_writelock,_unlock,_lock,cerror
_readlock:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),r0
	sys	lock;	0
	bec	1f
	jmp	cerror
1:
	clr	r0
	mov	(sp)+,r5
	rts	pc

_writelock:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),r0
	sys	lock;	1
	bec	1b
	jmp	cerror

_unlock:
	mov	r5,-(sp)
	mov	sp,r5
	sys	lock;	2
	bec	1b
	jmp	cerror

_lock:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),1f
	mov	6(r5),r0
	sys	indir;	0f
	bec	1b
	jmp	cerror
.data
0:
	sys	lock
1:	0
.text
