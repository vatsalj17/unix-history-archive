/ RHP04 file system bootstrap
/
/ Knows about ...
/		 Indirect blocks,
/		 Boots from any cylinder,
/		 Boots any pathname,
/		 Recovers better from errors,
/		 If 407 a.out take note of text & data size
/		 Stop loading iff file too big,
/		 Much smaller !!
/

/ some useful defines

core	=	28.			/ first core loc (in KW) not used
loadloc	=	[core*2048.]-[end-begin]
stcklo	=	loadloc - 20		/ allow 20 bytes for loader stack
ibno	=	stcklo - 2		/ indirect block number
ibuf	=	ibno - 512.		/ indirect block
inod	=	ibuf - 512.		/ inode
addr	=	inod + 8.		/ address begin here
mode	=	inod + 0.		/ flag field
////////////// 'inod' is max size of loaded program ///////////////////
buf	=	inod - 512.		/ block
names	=	buf - [20.*14.]		/ 20 file names in path name

LRG	=	10000			/ inode flag for large file algorithm
SWR	=	177570			/ switch register address
reset	=	5			/ reset instruction
	.globl	dskc			/ default cylinder for patching porpoise

start:

/ are defaults to be used ??

	mov	*$SWR,r0		/ note initail value
	mov	$-1,r4			/ assume defaults
1:	cmp	*$SWR,r0
	beq	1b			/ loop until value altered
	adc	r4			/ no defaults

/ relocate loader to high core

	mov	$loadloc,sp		/ sp,r1 now address final
	mov	sp,r1			/ resting place of code
	mov	$begin,r0		/ get loading @ 'begin'
1:	mov	(r0)+,(r1)+		/ copy
	cmp	r0,$end			/ what is
	blo	1b			/ necessary
	jmp	(sp)			/ begin execution of relocated code

/ clear core from names thru loadloc

begin:
	reset				/ initialize the system
	mov	sp,r2			/ first location to clear
2:
	clr	-(r2)			/ clear
	cmp	$names,r2		/ desired
	bne	2b			/ locations

/ initialize disk

	mov	$040,*$hpcs2		/ clear
	mov	$021,*$hpcs1		/ preset
	mov	$010000,*$hpof		/ fmt22

/ set up default file name

	inc	r4			/ are defaults to be used ??
	bne	ask			/ --> no.
	mov	$"un,(r2)+		/ default: boot /unix
	mov	$"ix,(r2)
	br	decode

/ prompt for filename to be used
/ read in path name
/ breaking on '/' into 14 ch names

ask:
	mov	$'?,r0
	jsr	pc,putc
1:
	mov	r2,r1
2:
	jsr	pc,getc
	cmp	r0,$'\n
	beq	4f
	cmp	r0,$'/
	beq	3f
	movb	r0,(r1)+
	br	2b
3:
	cmp	r2,r1
	beq	2b
	add	$14.,r2
	br	1b
4:
	mov	*$SWR,dskc		/ boot from cyl addr in switch reg.

/ start of path name decoding
/ start with first name  and root ino

decode:
	mov	$names,r2
	mov	$1,r0			/ start with "/"

/ get next inode    r0 is inode number

1:
	mov	$inod,r5		/ address of inode area
bno = . + 2
	clr	$0			/ start at block 0
	add	$31.,r0			/ step over blocks 0 & 1
	mov	r0,r3
	bic	$!17,r3			/ clear all but displacement
	ash	$5.,r3			/ r3 is displacement in block
	sub	r3,r5			/ put inode where desired
	ash	$-4.,r0			/ r0 block number of inode
	jsr	pc,rblk			/ get the block containing inode
	tst	(r2)			/ last inode ??
	beq	1f			/ --> yes.

/ read next directory looking for next name

	mov	$buf,r5			/ input block to here
2:
	jsr	pc,rmblk
	bpl	begin			/ --> end of file ??
	mov	r5,r1			/ first directory entry
3:
	mov	r2,r3			/ address of name to be looked for
	mov	r1,r4			/ r4 become current dir-entry
	add	$16.,r1			/ r1 become next dir-entry
	mov	(r4)+,r0		/ pick up next inode number
	beq	5f			/ --> dir entry not allocated
4:
	cmpb	(r3)+,(r4)+		/ same file name ?
	bne	5f			/ --> no
	cmp	r4,r1
	blo	4b
	add	$14.,r2			/ step to next in pathname
	br	1b			/ next file associated this inode
5:
	cmp	r1,$buf+512.
	blo	3b
	br	2b

/ last entry was found
/ read into 0.

1:
	clr	r5			/ set up buffer address
	mov	$buf,r4			/ don't read beyond this point.
	clr	r3			/ entry point address
	jsr	pc,rmblk		/ read first block
	bpl	begin			/ --> zero length file ??
	cmp	(r5),$407
	bne	4f			/ --> not suitable a.out.
	mov	2(r5),r4		/ get text size
	add	4(r5),r4		/ add data size
	bvs	begin			/ --> try again if overflow
	mov	12(r5),r3		/ pick up entry point address
	bic	$1,r3			/ make even
2:
	mov	20(r5),(r5)+		/ shuffle first block
	cmp	r5,$760
	blo	2b
3:
	jsr	pc,rmblk		/ read successive blocks
	bmi	4f			/ --> move onto next block
	mov	r3,pc			/ enter the program just loaded.
4:
	add	$1000,r5		/ step buffer pointer
	cmp	r5,r4			/ too big ??
	blo	3b			/ --> no.
	cmp	$buf,r4			/ protection | end of text+data
	beq	begin			/ --> protection
	mov	r3,pc			/ enter the program

/ routine to read in block
/ number specified by bno
/ after applying file system
/ mapping algorithm in inode.
/ bno is incremented.
/ success is n-bit set
/ failure is n-bit clear

rmblk:
	mov	bno,r0
	inc	bno
	bit	$LRG,*$mode		/ large file ?
	bne	1f			/ --> yes
	asl	r0
	mov	addr(r0),r0
	bne	rblk
2:
	cln				/ nothing read
	rts	pc
/ large algorithm
/ huge algorithm is not implemented
1:
	clr	-(sp)
	movb	r0,(sp)
	clrb	r0
	swab	r0
	asl	r0
	mov	addr(r0),r0
	beq	2b
	cmp	r0,*$ibno		/ same indirect block ??
	beq	9f
	mov	r5,-(sp)
	mov	$ibuf,r5		/ get indirect block this buffer
	mov	r0,*$ibno
	jsr	pc,rblk
	mov	(sp)+,r5
9:
	mov	(sp)+,r0
	asl	r0
	mov	ibuf(r0),r0
	beq	2b


rblk:

hpcs1 = 176700
hpda  = 176706
hpcs2 = 176710
hpof  = 176732
hpca  = 176734

	mov	r0,r1
	clr	r0
	div	$22.,r0			/ r1 = sector
	mov	r1,-(sp)
	mov	r0,r1
	clr	r0
	div	$19.,r0			/ r0 = cyl   r1 = trk
	bisb	r1,1(sp)
dskc = . + 2
	add	$220.,r0		/ cylinder offset
	mov	r0,*$hpca
	mov	$hpda,r1
	mov	(sp)+,(r1)		/ set trk/sector
	mov	r5,-(r1)		/ buffer address
1:
	mov	$-256.,-(r1)
	mov	$71,-(r1)		/ read
1:
	tstb	(r1)			/ io complete ??
	bpl	1b			/ --> no
	rts	pc

/ read and echo character from tty.
/ perform normal cr/lf mapping.
tks = 177560
tkb = 177562
getc:
	tstb	*$tks
	bge	getc
	mov	*$tkb,r0
	bic	$!177,r0
	cmp	r0,$'\r
	bne	putc
	mov	$'\n,r0

/ put a character on the tty.
/ also performs delay.
tps = 177564
tpb = 177566
putc:
	cmp	r0,$'\n
	bne	1f
	mov	$'\r,r0
	jsr	pc,putc
	mov	$'\n,r0
1:
	tstb	*$tps
	bpl	1b
	mov	r0,*$tpb
	rts	pc

end:					/ end of useable code
