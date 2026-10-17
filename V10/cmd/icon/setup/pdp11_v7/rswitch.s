.globl	_coswitc
.text
_coswitc:
	jsr	r5,csv
	jbr	L1
L2:
	mov	4(r5),r0
	mov	6(r5),r1
	mov	sp,(r0)
	mov	r5,2(r0)
	mov	r2,4(r0)
	mov	r3,6(r0)
	mov	r4,8.(r0)
	tst	10(r5)
	jne	L4
	mov	(r1),sp
	clr	r5
	clr	(sp)
	clr	-(sp)
	jsr	pc,*$_interp
	tst	(sp)+
	mov	$L5,(sp)
	jsr	pc,*$_syserr
	jbr	L6
L4:
	mov	(r1),sp
	mov	2(r1),r5
	mov	4(r1),r2
	mov	6(r1),r3
	mov	8.(r1),r4
L6:L3:	jmp	cret
L1:	jbr	L2
.globl
.data
L5:.byte	151,156,164,145,162,160,50,51,40,162,145,164,165,162
.byte	156,145,144,40,151,156,40,143,157,163,167,151,164,143,150,0
