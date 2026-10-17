|
| crtdrvr.s - self relocating C runtime startoff for Mac device driver.
|
| Copyright (C) 1984, Stanford Univ. SUMEX project
| May be used but not sold without permission.
|
| history
| 07/20/84	Croft	Created.
|
	.data
	.text
	.globl	_savea5
	.globl	drvr
	.globl	drvrOpen,drvrPrime,drvrCtl,drvrStatus,drvrClose

| driver header

drvr:
	.word	0x4f00		| locked, read, write, ctrl, status
	.word	0
	.word	0		| delay, emask
	.word	0		| menu
doffset:
	.word	reloc-drvr	| replaced by "dopen-drvr" after initialization
	.word	dprime-drvr
	.word	dctl-drvr
	.word	dstatus-drvr
	.word	dclose-drvr
	.byte	8
	.ascii	".cdriver "
	.blkb	22		| 32 bytes total for name

reloc:	jra	.L21
	.long	0,0,0,0,0,0,0,0,0,0		| longruns from rmaker
_savea5:.long	0
|
| a1 = next longrun address
| a2 = current reloc address
| d1 = relocation factor
|
.L21:
 	moveml	#0xffff,sp@-
	lea	pc@([drvr-.-2]),a1	| reloc factor
	movl	a1,d1
	lea	pc@([reloc+2-.-2]),a1
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
	beqs	.L18
	tstl	d7
	bnes	.L19
	movl	a1@+,a2
	addl	d1,a2
	bras	.L16
.L19:
	roll	#1,d7
	addl	d7,a2
	bras	.L16
|
| if shift button is pressed on entry, beep and hang around for an NMI.
|
.L18:
	btst	#0,0x17b
	beqs	.L31		| if not pressed
	movw	#1,sp@-		| sysbeep, duration 1
	.word	/A9C8
	moveq	#1,d0
.L22:
	tstl	d0
	bnes	.L22		| hang, waiting for NMI
.L31:
	movl	a5,_savea5
	movw	doff2,doffset	| above code is once-only
 	moveml	sp@+,#0xffff
|
| driver entry points
|
dopen:
	movl	#drvrOpen,d0
	bras	call
dclose:
	movl	#drvrClose,d0
	bras	call
dctl:
	movl	#drvrCtl,d0
	bras	call
dstatus:
	movl	#drvrStatus,d0
	bras	call
dprime:
	movl	#drvrPrime,d0
call:
	moveml	#0x3ffc,sp@-
	movl	a1,sp@-
	movl	a0,sp@-
	movl	d0,a0
	jsr	a0@
	addql	#8,sp
	moveml	sp@+,#0x3ffc
	cmpl	#0x40000000,d0
	bnes	done
	clrl	d0
	rts

jiodone = 0x8fc
done:
	movl	jiodone,sp@-
	rts

doff2:	.word	dopen-drvr
