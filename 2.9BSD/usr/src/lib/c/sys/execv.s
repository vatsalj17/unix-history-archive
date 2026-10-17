/ error	= execv(name, argv);
/	  char *name, *argv[];

.globl	_execv,
.globl	cerror, _environ

_execv:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),0f
	mov	6(r5),0f+2
	mov	_environ,0f+4
	sys	0; 9f
	jmp	cerror
.data
9:
	sys	exece; 0:..; ..; ..
