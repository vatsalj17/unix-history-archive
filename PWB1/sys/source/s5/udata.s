/ C library -- udata - log functions

.globl	_logpost, _logname, _logdir, _logtty
.globl	cerror

pwbsys = 57.
udata = 1

_logpost:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),r0
	mov	pc,r1
	sys	pwbsys; udata
	bec	1f
	jmp	cerror
1:
	clr	r0
	mov	(sp)+,r5
	rts	pc

_logname:
	mov	r5,-(sp)
	mov	sp,r5
	jsr	pc,logread
	mov	$logname,r0
	br	8f

_logdir:
	mov	r5,-(sp)
	mov	sp,r5
	jsr	pc,logread
	mov	$logdir,r0
	br	8f

_logtty:
	mov	r5,-(sp)
	mov	sp,r5
	jsr	pc,logread
	mov	$logtty,r0
8:
	mov	(sp)+,r5
	rts	pc

logread:
	mov	$logdata,r0
	clr	r1
	sys	pwbsys; udata
	bec 	1f
	jmp	cerror
1:
	rts	pc
.bss
logdata: logname: . = .+8.
logdir:	. = .+22.
logtty:	. = .+2.
