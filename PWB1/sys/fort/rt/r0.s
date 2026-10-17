/ fortran runtime startup
/ calls main and provides error
/ messages

.globl	main
.globl	pdec
.globl	temp
.globl	rerr

.globl	trace
.globl	fptrap


.comm	erret,2
.comm	argp,2

ldfps	= 170100^tst

	sys	signal; 4; fptrap
	ldfps	$5400
	sys	signal; 8.; fperr
	mov	sp,argp
	mov	$main,r4
	jmp	*(r4)+

fperr:
	jsr	r5,rerr; 998.

rerr:
	mov	(r5)+,r1
	tst	erret
	beq	1f
	jsr	pc,erret
1:
	mov	$2,r0
	sys	write; m1; m2-m1
	jsr	r5,pdec
	mov	$2,r0
	sys	write; m2; 1
	jsr	pc,trace
	sys	exit

pdec:
	clr	r0
	div	$10.,r0
	mov	r1,-(sp)
	mov	r0,r1
	beq	1f
	jsr	r5,pdec
1:
	mov	(sp)+,r0
	add	$'0,r0
	movb	r0,temp
	mov	$2,r0
	sys	write; temp; 1
	rts	r5

m1:
	<Runtime error >
m2:
	<\n>
.even
.bss
temp:	.=.+16.
