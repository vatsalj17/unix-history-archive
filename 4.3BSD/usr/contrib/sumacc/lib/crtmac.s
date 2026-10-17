|
| crtmac.s - self relocating C runtime startoff for Macintosh.
|
| Copyright (C) 1984, Stanford Univ. SUMEX project
| May be used but not sold without permission.
|
| history
| 04/16/84	Croft	Created.
| 04/20/84	Croft	Checks mouse on entry to allow debugger NMI.
|
	.data
	.text
	.globl	_start,_end,_savea5,main
_start:	jra	.L21
	.long	0,0,0,0,0,0,0,0,0,0		| longruns from rmaker
_savea5:.long	0
|
| a1 = next longrun address
| a2 = current reloc address
| d1 = relocation factor
|
.L21:
	lea	pc@(_start-.L21),a1
	movl	a1,d1
	subql	#2,d1
	movl	a1@+,a2		| pickup 1st relocation
	addl	d1,a2
.L16:
|	for(;;) {
|		i = *a2;
|		*a2 = 0;
|		*(u_long *)a2 += (u_long)d1;
|		if (i == 0377)
|			goto start;
|		if (i == 0) {
|			a2 = *a1++;
|			a2 += d1;
|			continue;
|		}
|		a2 += (i << 1);
|	}
	movb	a2@,d7
	andl	#255,d7
	clrb	a2@
	addl	d1,a2@
	cmpl	#255,d7
	beq	.L18
	tstl	d7
	bne	.L19
	movl	a1@+,a2
	addl	d1,a2
	bra	.L16
.L19:
	roll	#1,d7
	addl	d7,a2
	bra	.L16
|
| if mouse button is pressed on entry, beep and hang around for an NMI.
|
.L18:
	clrw	sp@-
	.word	/A974		| get mouse button state
	tstb	sp@+
	beq	.L31		| if not pressed
	movw	#1,sp@-		| sysbeep, duration 1
	.word	/A9C8
	moveq	#1,d0
.L22:
	tstl	d0
	bne	.L22		| hang, waiting for NMI
|
| call main
|
.L31:
	movl	a5,_savea5
	jsr	main
	movl	_savea5,a5
	.word	/ADF4		| trap: exittoshell
