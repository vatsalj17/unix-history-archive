/ error	= execve(name, argv, envp);
/	  char *name, *argv[], *envp[];

.globl	_execve
.globl	cerror

_execve:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),0f
	mov	6(r5),0f+2
	mov	8(r5),0f+4
	sys	0; 9f
	jmp	cerror
.data
9:
	sys	exece; 0:..; ..; ..
