	add	$4,sp
	sys	open;reader;0
	bes	9f
	mov	r0,r5
0:
	mov	(sp)+,r4
	cmp	$177777,r4
	beq	8f
	mov	r4,1f
	sys	creat;1:..;644
	bec	1f
	mov	$2,r0
	sys	write;m1;m1l
	br	0b
1:
	mov	r0,r4
2:	mov	r5,r0
	sys	read;buf;81.
	bes	3f
	tst	r0
	beq	3f
	mov	$buf+80.,r0
1:
	cmpb	-(r0),$40		/blank
	beq	1b
	inc	r0
	movb	$12,(r0)+
	sub	$buf,r0
	mov	r0,1f
	mov	r4,r0
	sys	write;buf;1:..
	br	2b
3:	mov	r4,r0
	sys	close
	br	0b
9:
	mov	$2,r0
	sys	write;m2;m2l
8:	sys	exit
.data
reader:	</dev/cr0\0>
m1:	<Cant create file\n>
m1l = .-m1
m2:	<Cant open reader\n>
m2l = .-m2
.even
.bss
buf:	.=.+81.
