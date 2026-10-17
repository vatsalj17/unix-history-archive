/ RHP04 file system bootstrap
/
/ Knows about ...
/		 Indirect blocks,
/		 Boots from any cylinder,
/		 Boots any pathname,
/		 Recovers better from errors,
/		 Files up to 92Kw can be loaded !!
/

/ some useful defines

core	=	92. * 32.		/ location of loader in physical memory
					/ ie twixt 92Kw & 96Kw
					/ ie the top 4k of a 32k segment so that
					/ virtual address == real address (16bits)
loadloc =	begin - end		/ put loader at top of virtual address space
stcklo	=	loadloc - 100		/ allow 100 bytes for loader stack
ibno	=	stcklo - 2		/ indirect block number
ibuf	=	ibno - 512.		/ indirect block
inod	=	ibuf - 512.		/ inode ( need this much for algorithm used)
addr	=	inod + 8.		/ address begin here
mode	=	inod + 0.		/ flag field
buf	=	inod - 512.		/ block
bno	=	buf - 2.		/ block number
names	=	bno - [20.*14.]		/ 20 file names in path name

LRG	=	10000			/ inode flag for large file algorithm
SWR	=	37570			/ switch register address
reset	=	5			/ reset instruction
	.globl	dskc			/ default cylinder for patching porpoise

start:
	reset				/ initialize the system

/ enable memory management

	clr	*$172340		/ KIA0 to address physical 0
	mov	$177600,*$172342	/ KIA1 to address i/o page
	mov	$core,*$172356		/ KIA7 to address loader
	mov	$77406,r4		/ r/w 8Kb
	mov	r4,*$172300		/ KID0 r/w 8Kb
	mov	r4,*$172302		/ KID1 r/w 8Kb
	mov	r4,*$172316		/ KID7 r/w 8Kb
	inc	*$177572		/ enable memory management (MMR0 <- 1)
					/ 18 bit mode - no I&D space

/ are defaults to be used ??

	mov	*$SWR,r0		/ note initail value
	mov	$-1,r4			/ assume defaults
1:	cmp	*$SWR,r0
	beq	1b			/ loop until value altered
	adc	r4			/ add one if now higher ==> no defaults

/ relocate loader to high core

	mov	$loadloc,sp		/ sp,r1 now address final
	mov	sp,r1			/ resting place of code
	mov	$begin,r0		/ get loading @ 'begin'
1:	mov	(r0)+,(r1)+		/ copy
	tst	r1			/ what is
	bne	1b			/ necessary
	jmp	(sp)			/ begin execution of relocated code

/ clear core from names thru loadloc

begin:
	mov	pc,sp			/ reset stack pointer
	tst	-(sp)			/ get below first instruction
	mov	sp,r2			/ first location to clear
2:
	clr	-(r2)			/ clear
	cmp	$names,r2		/ desired
	bne	2b			/ locations
	inc	-(sp)			/ always start search with inode 1 == "/"

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
	jsr	pc,getc			/ delay while switches are set

decode:

/ initialize disk

	mov	$040,*$hpcs2		/ clear
	mov	$021,*$hpcs1		/ preset
	mov	$010000,*$hpof		/ fmt22
	mov	$1071,r4		/ read into 64Kw -> 96Kw

/ start of path name decoding
/ start with first name  and root ino

	mov	$names,r2

/ get next inode    (sp) is inode number

1:
	mov	$inod,r5		/ address of inode area
	clr	*$bno			/ start at block 0
	mov	(sp),r0			/ pick up desired inode number
	add	$31.,r0			/ step over blocks 0 & 1
	mov	r0,r3
	bic	$!17,r3			/ clear all but displacement
	ash	$5.,r3			/ r3 is displacement in block
	sub	r3,r5			/ put inode where desired
	ash	$-4.,r0			/ r0 block number of inode
	jsr	pc,rblk			/ get the block containing inode
	tst	(r2)			/ last inode ??
	beq	load			/ --> yes.

/ read next directory looking for next name

	mov	$buf,r5			/ input block to here
2:
	jsr	pc,rmblk
	bpl	begin			/ --> end of file ??
	mov	r5,r1			/ first directory entry
3:
	mov	r2,r3			/ address of name to be looked for
	mov	r1,r0			/ r0 become current dir-entry ptr
	add	$16.,r1			/ r1 become next dir-entry
	mov	(r0)+,(sp)		/ pick up next inode number
	beq	5f			/ --> dir entry not allocated
4:
	cmpb	(r3)+,(r0)+		/ same file name ?
	bne	5f			/ --> no
	cmp	r0,r1
	blo	4b
	add	$14.,r2			/ step to next in pathname
	br	1b			/ go next file associated this inode
5:
	cmp	r1,$buf+512.
	blo	3b
	br	2b

/ last entry was found
/ read into 0.

load:
	clr	r5			/ set up buffer address
	mov	$71,r4			/ read into 0Kw -> 32Kw
	jsr	pc,rmblk		/ read first block
	bpl	begin			/ --> zero length file ??
	cmp	(r5),$407
	bne	4f			/ --> not suitable a.out.
2:
	mov	20(r5),(r5)+		/ shuffle first block
	cmp	r5,$760
	blo	2b
3:
	jsr	pc,rmblk		/ read successive blocks
	bpl	callout			/ --> go enter prog just loaded
4:
	add	$1000,r5		/ step buffer pointer
	bne	3b			/ --> no wrap to next 32k
	add	$400,r4			/ fixup A17,A16
	br	3b

/ routine to read in block
/ number specified by bno
/ after applying file system
/ mapping algorithm in inode.
/ bno is incremented.
/ success is n-bit set
/ failure is n-bit clear

rmblk:
	mov	*$bno,r0
	inc	*$bno
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
	beq	9f			/ --> yes
	mov	r5,-(sp)
	mov	r4,-(sp)
	mov	$1071,r4		/ read into 64Kw -> 96Kw
	mov	$ibuf,r5		/ get indirect block this buffer
	mov	r0,*$ibno
	jsr	pc,rblk
	mov	(sp)+,r4
	mov	(sp)+,r5
9:
	mov	(sp)+,r0
	asl	r0
	mov	ibuf(r0),r0
	beq	2b


rblk:

hpcs1 = 36700
hpda  = 36706
hpcs2 = 36710
hpof  = 36732
hpca  = 36734

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
	mov	r4,-(r1)		/ read
1:
	tstb	(r1)			/ io complete ??
	bpl	1b			/ --> no
	rts	pc

/ read and echo character from tty.
/ perform normal cr/lf mapping.
tks = 37560
tkb = 37562
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
tps = 37564
tpb = 37566
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

/ transfer control to prog just loaded
/ callout == 177776 in virtual address space so that
/ when mem managment disabled by the reset below the pc
/ will equal zero and hence commence executing from
/ physical location zero - the prog just loaded !!

callout:
	reset				/ disable memory management
					/ and commence exec at physical
					/ location 0

end:					/ end of useable code
