|
| rdinit.s - init resource to bring in ramdisk driver at boot time
|

	.nlist
	.insrt	"h.h"
	.list

|
| code below lifted from crtmac.s
|

	.text
	.globl	_start,_savea5
_start:	jra	.L21
	.long	0,0,0,0,0,0,0,0,0,0		| longruns from rmaker
_savea5:.long	0
|
| a1 = next longrun address
| a2 = current reloc address
| d1 = relocation factor
|
.L21:
	moveml	#0xffff,sp@-
	lea	pc@([_start-.-2]),a1	| reloc factor
	movl	a1,d1
	lea	pc@([_start+2-.-2]),a1
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
.L18:

|
| rdinit
|

	.text
rdinit:
|	movl	#0x11000,bufptr
	moveq	#[ioqelsize/2]-1,d0
0$:	clrw	sp@-	| clear a parameter blk off the stack
	dbra	d0,0$

	clrw	sp@-
	.word	__button
	tstw	sp@+
	bne	initexit	|if user wants to skip the ramdisk

	movl	sp,a0
	lea	rdname,a1
	movl	a1,a0@(iofilename)
	.word	__open
	bne	initexit

	clrl	sp@-
	movl	rdtype,sp@-
	movw	#31,sp@-
	.word	__getresou
	.word	__detachre
|
| add to drive queue
|
	moveq	#dqellnth,d0	| get size of drive queue element
	.word	__newptr+_sys_	|	and allocate one
	movw	#0,a0@(dqfsid)	| zero means local file system
	moveq	#3,d0	| call it drive number 3
	swap	d0
	movw	#-32,d0	|	my refnum
	.word	__adddrive

	lea	mparams,a0	| point to params
	.word	__mountvol	| mount as a volume

initexit:
	addw	#ioqelsize,sp
	lea	_start,a0
	.word	__recoverhandle
	.word	__disposhandle
	moveml	sp@+,#0xffff
	rts

	.even
rdtype:	.ascii	"DRVR"
	.even
rdname:	.byte	8
	.ascii	".ramdisk "
	.even

mparams:
	.blkb	12	| header
	.word	0,0	| completion routine	(nil)
	.word	0	| ioresult
	.word	0,0	| misc don't-cares
	.word	3	| drive num
