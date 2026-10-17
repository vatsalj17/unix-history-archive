/ abort/restart/repeat line printer daemons

kill=	37.	/ (not in assembler)

	cmp	(sp),$1		/ default printer?
	ble	1f
	cmpb	*4(sp),$'-
	bne	2f
	inc	4(sp)
2:
	cmpb	*4(sp),$'1
	blt	8f
	cmpb	*4(sp),$'3
	bgt	8f
	movb	*4(sp),locpos
1:
	mov	2(sp),r2	/ save how called
	sys	open; lock; 0	/ pid of lpd, uid kept in this file
	bec	0f		/ if opened ok
	mov	$2,r0
	sys	write; m1; m1l	/ "Lpd not active"
	sys	exit
0:
	sys	read; pid; 4	/ collect pid
	bes	1f		/ if error
	cmp	r0,$4		/ read 4 bytes ?
	beq	0f		/ yep. ok.
1:	mov	$2,r0
	sys	write; m2; m2l	/ "Read failure"
	sys	exit
0:
	cmp	$-1,uid		/ is printer active
	bne	0f
	mov	$2,r0
	sys	write; m6; m6l	/ "Printer inactive"
	sys	exit
0:
	sys	getuid		/ check if uids match
	tst	r1
	beq	0f
	cmp	r1,uid		/ real uid must match with that in file
	beq	0f
	mov	$2,r0
	sys	write;m7;m7l	/ "Permission denied"
	sys	exit
0:
	mov	pid,r0
	cmpb	(r2),$'p	/ called as "prstop" ?
	beq	0f		/ yep. skip this bit.
	inc	signo		/ make into "restart" sig.
	cmpb	2(r2),$'s	/restart not repeat
	beq	0f
	inc	signo
0:	sys	indir; kil	/ send signal
	bec	0f		/ if sent ok
	mov	$2,r0
	sys	write; m3; m3l	/ "Unable to signal lpd"
	sys	exit
0:
	mov	$2,r0
	sys	write; m4; m4l	/ "Done"
	clr	r0
	sys	exit
8:
	mov	$2,r0
	sys	write; m5; m5l	/ "Bad arg"
	sys	exit

.data

kil:	sys	kill; signo: 5	/ increment for restart

lock:	</tmp/lpd/lock>
locpos:	<\0\0>

m1:	<Lpd not active\n>
m1l=	.-m1
m2:	<Read failure\n>
m2l=	.-m2
m3:	<Unable to signal lpd\n>
m3l=	.-m3
m4:	<Done\n>
m4l=	.-m4
m5:	<Bad arg\n>
m5l=	.-m5
m6:	<Printer inactive\n>
m6l=	.-m6
m7:	<Permission denied\n>
m7l=	.-m7		/ KFH fix

.bss

pid:	.=.+2
uid:	.=.+2
