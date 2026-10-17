/ error = readlink(path, buf, bufsiz);
/	  char *path, *buf;
/	  int bufsiz;

.globl  _readlink
.globl	cerror

_readlink:
	mov	r5,-(sp)
	mov	sp,r5
	mov     4(r5),r0
	mov     6(r5),0f
	mov	010(r5),0f+2
	sys	local; 9f
	bec	1f
	jmp	cerror
1:
	mov	(sp)+,r5
	rts	pc
.data
9:
	sys	readlink; 0:..; ..
.text
