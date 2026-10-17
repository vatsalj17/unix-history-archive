/ write -- write to another user

.AUSAM = 1
.if	.AUSAM
	.u_type = 0
	.u_ttyid = .u_type+1
	.u_u_id = .u_ttyid+1
	.u_logintime = .u_u_id+2
	.u_u_name = .u_logintime+4
	.u_end = .u_u_name+8.
.endif
.if	.AUSAM-1
	.u_u_name = 0
	.u_ttyid = .u_u_name+8.
	.u_logintime = .u_ttyid+2.
	.u_end = .u_logintime+6.
.endif
.globl	ttyn

	cmp	(sp)+,$2
	beq	1f
	bgt	2f
	mov	$2,r0
	sys	write; argm; eargm-argm
	mov	$1,r0
	sys	exit
2:
	movb	*4(sp),ltty
1:
	tst	(sp)+
	mov	(sp)+,r5
	sys	open; utmp; 0
	bec	1f
	mov	$2,r0
	sys	write; film; efilm-film
	mov	$1,r0
	sys	exit
1:
	mov	r0,ufil
1:
	mov	ufil,r0
	sys	read; ubuf; 16.
	tst	r0
	bne	2f
	jmp	8f
2:
	tstb	ltty
	beq	2f
	cmpb	ltty,ubuf+.u_ttyid
	bne	1b
2:
	mov	$ubuf+.u_u_name,r3
	mov	r5,r4
	mov	$9.,r2
2:
	dec	r2
	beq	2f
.if	.AUSAM-1
	cmpb	(r4)+,(r3)+
	beq	2b
	tstb	-1(r4)
	bne	1b
	cmpb	$' ,-1(r3)
	bne	1b
.endif
.if	.AUSAM
	cmpb	(r4),(r3)+
	bne	1b
	tstb	(r4)+
	bne	2b
.endif
2:
	movb	ubuf+.u_ttyid,ttyno
	sys	open; ttyx; 1
	bes	3f
	sys	fstat; statbuf
	bes	3f
	bit	$2,statbuf+4		/modes (write by others)
	bne	2f
3:
	mov	r0,-(sp)	/ save some registers.
	mov	r1,-(sp)
	sys	getuid		/ let root do it
	tst	r0
	bne	9f		/ if this isn't root
	mov	(sp)+,r1
	mov	(sp)+,r0
	br	2f
9:
	mov	$2,r0
	sys	write; dnymes; ednymes-dnymes
	mov	$1,r0
	sys	exit
2:
	mov	r0,ttyf
/	clr	r0		/ if want std input		ILJ
/	jsr	pc,ttyn		/ if want std input		ILJ
	mov	$2,r0		/ more likely to be terminal	ILJ
	jsr	pc,ttyn+2	/ ttyn entry clears r0 anyway	ILJ
	mov	r0,r3
	movb	r0,ltty
	mov	statbuf,r4
	mov	ufil,r0
	sys	seek; 0; 0
1:
	mov	ufil,r0
	sys	read; ubuf; 16.
	tst	r0
	beq	unknown
	cmpb	r3,ubuf+.u_ttyid
	bne	1b
	mov	$ubuf+.u_u_name,r0
	mov	$8.,r1
1:
.if	.AUSAM-1
	cmpb	$' ,(r0)+
.endif
.if	.AUSAM
	tstb	(r0)+
.endif
	beq	1f
	dec	r1
	bne	1b
1:
	neg	r1
	add	$8,r1
	mov	r1,0f
6:
	mov	ttyf,r0
	sys	write; mesg; emesg-mesg
	mov	ttyf,r0
	sys	0; 5f
.data
5:
	sys	write; ubuf+.u_u_name; 0:2
.text
	mov	ttyf,r0
	sys	write; bmesg; ebmesg-bmesg
	mov	ttyf,r0
	sys	write; ltty; 1
	mov	ttyf,r0
	sys	write; qnl; 4
	sys	signal; 2; 9f
7:
	clr	r0
	sys	read; ch; 1
	tst	r0
	beq	9f
	tst	nlflg
	beq	1f
	cmp	ch,$'!
	bne	1f
	sys	fork
		br mshproc
	bec	0f
	mov	$2,r0
	sys	write; tryagn; ltryagn
0:
	sys	wait
	mov	$2,r0
	sys	write; excl; 2
	br	7b
1:
	clr	nlflg
	cmp	ch,$'\n
	bne	1f
	inc	nlflg
1:
	mov	ttyf,r0
	sys	write; ch; 1
	br	7b
8:
	movb	(r5)+,ch
	beq	8f
	mov	$2,r0
	sys	write; ch; 1
	br	8b
8:
	tstb	ltty
	beq	8f
	mov	$2,r0
	sys	write; ltty-1; 2
8:
	mov	$2,r0
	sys	write; errmsg; eerrmsg-errmsg
	mov	$1,r0
	sys	exit
9:
	mov	ttyf,r0
	sys	write; endmsg; eendmsg-endmsg
	clr	r0
	sys	exit

unknown:
	mov	$"??,ubuf+.u_u_name
	br	6b

mshproc:
	sys	exec; msh; mshp
	mov	$1,r0
	sys	exit

.data
nlflg:
	1
.text

mshp:
	msh
	minust
	0
msh:
	</bin/sh\0>
minust:
	<-t\0>
argm:
	<Arg count\n>
eargm:
film:
	<Cannot open utmp\n>
efilm:
.data
	< >		/ is ltty -1
ltty:
	.=.+1
.text
excl:
	<!\n>
qnl:
	<...\n>
.data
ttyx:
	</dev/ttyx\0>
ttyno	= .-2
.text
utmp:
	</etc/utmp\0>
endmsg:
	<EOT\n>
eendmsg:
errmsg:
	< not logged in.\n>
eerrmsg:
mesg:
	<\nMessage from >
emesg:
bmesg:
	< at AGSM on tty>
ebmesg:
dnymes:
	<Permission denied.\n>
ednymes:
tryagn:	<Try again\n>	;	ltryagn = . - tryagn
	.even
	.bss

ttyf:	.=.+2
ubuf:	.=.+16.
statbuf:.=.+40.
ch:	.=.+2
ufil:	.=.+2
signal = 48.
