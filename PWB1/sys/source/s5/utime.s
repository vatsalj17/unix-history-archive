/ C library -- utime - set times in file

/ utime(name, time)
/ char *name, *time;

.globl _utime
.globl cerror

pwbsys = 57.
utime = 3

_utime:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),r1
	mov	6(r5),r0
	sys	pwbsys; utime
	bec	8f
	jmp	cerror
8:
	clr	r0
	mov	(sp)+,r5
	rts	pc
