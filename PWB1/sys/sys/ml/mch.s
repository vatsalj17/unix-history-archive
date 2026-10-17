/ machine language assist
/ for 11/45 or 11/70 CPUs

.fpp = 1

/ non-UNIX instructions
mfpi	= 6500^tst
mtpi	= 6600^tst
mfpd	= 106500^tst
mtpd	= 106600^tst
spl	= 230
ldfps	= 170100^tst
stfps	= 170200^tst
halt	= 0
wait	= 1
reset	= 5
rtt	= 6

HIPRI	= 300
HIGH	= 6

.data
.globl	start, _end, _edata, _etext, _main

/ 11/45 and 11/70 startup.
/ entry is thru 0 abs.
/ since core is shuffled,
/ this code can be executed but once

start:
/ Set up 0 & 2 to catch traps and jumps to 0
	mov	$42,0		/ illegal instruction if jump
	mov	$777,2		/ trace, high pri, catch stray if trap
	clr	PS

/ set KI0 to physical 0

	mov	$77406,r3
	mov	$KISA0,r0
	mov	$KISD0,r1
	clr	(r0)+
	mov	r3,(r1)+

/ set KI1-6 to eventual text resting place

	mov	$_end+63.,r2
	ash	$-6,r2
	bic	$!1777,r2
	mov	$_etext-8192.+63.,r5
	ash	$-6,r5
	bic	$!1777,r5
	add	r2,r5

1:
	mov	r2,(r0)+
	mov	r5,r4
	sub	r2,r4
	cmp	$200,r4
	bge	2f
	mov	$200,r4
2:
	add	r4,r2
	dec	r4
	bge	2f
	clr	r4
	br	3f
2:
	swab	r4
	bis	$6,r4
3:
	mov	r4,(r1)+
	cmp	r0,$KISA7
	blo	1b

/ set KI7 to IO seg for escape

	mov	$IO,(r0)+
	mov	r3,(r1)+

/ set KD0-7 to physical

	clr	r2
1:
	mov	r2,(r0)+
	mov	r3,(r1)+
	add	$200,r2
	cmp	r0,$KDSA7
	blos	1b

/ initialization
/ get a temp (1-word) stack
/ turn on segmentation
/ copy text to I space
/ clear bss in D space

	mov	$stk+2,sp
	mov	$25,SSR3		/ 22-bit addr KU ID space sep
	bit	$20,SSR3
	beq	1f
	mov	$70.,_cputype
	bis	$3,*$MSCR		/ Disable UNIBUS traps, non-fatal traps
1:
	inc	SSR0
	mov	$_etext,r2
	mov	$_edata,r1
	add	$_etext-8192.,r1
1:
	mov	-(r1),-(sp)
	mtpi	-(r2)
	cmp	r1,$_edata
	bhi	1b
1:
	clr	(r1)+
	cmp	r1,$_end
	blo	1b

/ use KI escape to set KD7 to IO seg
/ set KD6 to first available core

	mov	$IO,-(sp)
	mtpi	*$KDSA7
	mov	$_etext-8192.+63.,r2
	ash	$-6,r2
	bic	$!1777,r2
	add	KISA1,r2
	mov	r2,KDSA6
	movb	$usize-1,KDSD6+1

/ Turn off write permission on kernel text
/ Take stuff above data out of address space

	mov	$KISD0,r0
1:
	bic	$4,(r0)+
	cmp	r0,$KISD7
	blos	1b

	mov	$_end+63.,r0
	ash	$-6,r0
	bic	$!1777,r0
	mov	$KDSD0,r1
1:
	cmp	r0,$200
	bge	3f
	dec	r0
	bge	4f
	clr	(r1)
	br	3f
4:
	movb	r0,1(r1)
3:
	tst	(r1)+
	sub	$200,r0
	cmp	r1,$KDSD6
	blo	1b

/ set up supervisor D registers

	mov	$6,SISD0
	mov	$6,SISD1

/ set up real sp
/ clear user block

	mov	$_u+[usize*64.],sp
	mov	$_u,r0
	mov	r0,SL
1:
	clr	(r0)+
	cmp	r0,sp
	blo	1b

/ set up previous mode and call main
/ on return, enter user mode at 0R

	mov	$30000,PS
	jsr	pc,_main
	mov	$170000,-(sp)
	clr	-(sp)
	rtt

.globl	trap4, trap14, trap24, trap250
.globl	_trap, trap, call, _stray
trap4:
	mov	PS,saveps
	tst	sp		/test for Red Zone Violation
	beq	9f
	tst	nofault
	bne	3f
	br	2f

trap14:
	mov	PS,saveps
	cmpb	$377,2(sp)	/ test if stray interrupt
	bne	2f
	mov	(sp)+,saveps	/ save pc of stray and fix stack
	tst	(sp)+
	jsr	r0,call1; _stray

trap24:
	mov	PS,saveps
	tst	power	/ check power fail up or down
	bne	9f
	inc	power
	br	2f

trap250:
	mov	PS,saveps
	tst	nofault
	bne	1f
	mov	SSR0,ssr
	mov	SSR1,ssr+2
	mov	SSR2,ssr+4
	mov	$1,SSR0
2:
	jsr	r0,call1; _trap
1:
	mov	$1,SSR0
3:
	mov	nofault,(sp)
	rtt

trap:
	spl	0
	jsr	r0,call; _trap

/ final death
9:
	reset
	mov	r0,stk
	mov	$0f,r0
1:
	tstb	*$177564
	bge	1b
	movb	(r0)+,*$177566
	tstb	(r0)
	bne	1b
	mov	stk,r0
1:
	halt
	br	1b
0:
	<\0death\0>
.text

.globl	_runrun, _qswtch
call1:
	mov	saveps,-(sp)
	spl	0
	br	1f

call:
	mov	PS,-(sp)
1:
	mov	r1,-(sp)
	mfpd	sp
	mov	4(sp),-(sp)
	bic	$!37,(sp)
	bit	$30000,PS
	beq	1f
.if .fpp
	mov	$20,_u+4		/ FP maint mode
.endif
	jsr	pc,*(r0)+
2:
	spl	HIGH
	tstb	_runrun
	beq	2f
	spl	0
	jsr	pc,_savfp
	jsr	pc,_qswtch
	br	2b
2:
.if .fpp
	mov	$_u+4,r1
	bit	$20,(r1)
	bne	2f
	mov	(r1)+,r0
	ldfps	r0
	movf	(r1)+,fr0
	movf	(r1)+,fr1
	movf	fr1,fr4
	movf	(r1)+,fr1
	movf	fr1,fr5
	movf	(r1)+,fr1
	movf	(r1)+,fr2
	movf	(r1)+,fr3
	ldfps	r0
2:
.endif
	tst	(sp)+
	mtpd	sp
	br	2f
1:
	bis	$30000,PS
	jsr	pc,*(r0)+
	cmp	(sp)+,(sp)+
2:
	mov	(sp)+,r1
	tst	(sp)+
	mov	(sp)+,r0
	rtt

.globl	_savfp
_savfp:
.if .fpp
	mov	$_u+4,r1
	bit	$20,(r1)
	beq	1f
	stfps	(r1)+
	movf	fr0,(r1)+
	movf	fr4,fr0
	movf	fr0,(r1)+
	movf	fr5,fr0
	movf	fr0,(r1)+
	movf	fr1,(r1)+
	movf	fr2,(r1)+
	movf	fr3,(r1)+
1:
.endif
	rts	pc

.globl	_incupc
_incupc:
	mov	r2,-(sp)
	mov	6(sp),r2	/ base of prof with base,leng,off,scale
	mov	4(sp),r0	/ pc
	sub	4(r2),r0	/ offset
	clc
	ror	r0
	mul	6(r2),r0	/ scale
	ashc	$-14.,r0
	inc	r1
	bic	$1,r1
	cmp	r1,2(r2)	/ length
	bhis	1f
	add	(r2),r1		/ base
	mov	nofault,-(sp)
	mov	$2f,nofault
	mfpd	(r1)
	inc	(sp)
	mtpd	(r1)
	br	3f
2:
	clr	6(r2)
3:
	mov	(sp)+,nofault
1:
	mov	(sp)+,r2
	rts	pc

.globl	_display
_display:
	dec	dispdly
	bge	2f
	clr	dispdly
	mov	PS,-(sp)
	mov	$HIPRI,PS
	mov	CSW,r1
	bit	$1,r1
	beq	1f
	bis	$30000,PS
	dec	r1
1:
	jsr	pc,fuword
	mov	r0,CSW
	mov	(sp)+,PS
	cmp	r0,$-1
	bne	2f
	mov	$60.,dispdly		/ 1 sec delay after CSW fault
2:
	rts	pc

/ Character list get/put

.globl	_getc, _putc
.globl	_cfreelist

_getc:
	mov	2(sp),r1
	mov	PS,-(sp)
	mov	r2,-(sp)
	spl	6
	mov	2(r1),r2	/ first ptr
	beq	9f		/ empty
	movb	(r2)+,r0	/ character
	bic	$!377,r0
	mov	r2,2(r1)
	dec	(r1)+		/ count
	bne	1f
	clr	(r1)+
	clr	(r1)+		/ last block
	br	2f
1:
	bit	$7,r2
	bne	3f
	mov	-10(r2),(r1)	/ next block
	add	$2,(r1)
2:
	dec	r2
	bic	$7,r2
	mov	_cfreelist,(r2)
	mov	r2,_cfreelist
3:
	mov	(sp)+,r2
	mov	(sp)+,PS
	rts	pc
9:
	clr	4(r1)
	mov	$-1,r0
	mov	(sp)+,r2
	mov	(sp)+,PS
	rts	pc

_putc:
	mov	2(sp),r0
	mov	4(sp),r1
	mov	PS,-(sp)
	mov	r2,-(sp)
	mov	r3,-(sp)
	spl	6
	mov	4(r1),r2	/ last ptr
	bne	1f
	mov	_cfreelist,r2
	beq	9f
	mov	(r2),_cfreelist
	clr	(r2)+
	mov	r2,2(r1)	/ first ptr
	br	2f
1:
	bit	$7,r2
	bne	2f
	mov	_cfreelist,r3
	beq	9f
	mov	(r3),_cfreelist
	mov	r3,-10(r2)
	mov	r3,r2
	clr	(r2)+
2:
	movb	r0,(r2)+
	mov	r2,4(r1)
	inc	(r1)		/ count
	clr	r0
	mov	(sp)+,r3
	mov	(sp)+,r2
	mov	(sp)+,PS
	rts	pc
9:
	mov	pc,r0
	mov	(sp)+,r3
	mov	(sp)+,r2
	mov	(sp)+,PS
	rts	pc

.globl	_backup
.globl	_regloc
_backup:
	mov	2(sp),r0
	movb	ssr+2,r1
	jsr	pc,1f
	movb	ssr+3,r1
	jsr	pc,1f
	movb	_regloc+7,r1
	asl	r1
	add	r0,r1
	mov	ssr+4,(r1)
	clr	r0
2:
	rts	pc
1:
	mov	r1,-(sp)
	asr	(sp)
	asr	(sp)
	asr	(sp)
	bic	$!7,r1
	movb	_regloc(r1),r1
	asl	r1
	add	r0,r1
	sub	(sp)+,(r1)
	rts	pc


.globl	_fubyte, _subyte
.globl	_fuword, _suword
.globl	_fuibyte, _suibyte
.globl	_fuiword, _suiword
_fuibyte:
	mov	2(sp),r1
	bic	$1,r1
	jsr	pc,giword
	br	2f

_fubyte:
	mov	2(sp),r1
	bic	$1,r1
	jsr	pc,gword

2:
	cmp	r1,2(sp)
	beq	1f
	swab	r0
1:
	bic	$!377,r0
	rts	pc

_suibyte:
	mov	2(sp),r1
	bic	$1,r1
	jsr	pc,giword
	mov	r0,-(sp)
	cmp	r1,4(sp)
	beq	1f
	movb	6(sp),1(sp)
	br	2f
1:
	movb	6(sp),(sp)
2:
	mov	(sp)+,r0
	jsr	pc,piword
	clr	r0
	rts	pc

_subyte:
	mov	2(sp),r1
	bic	$1,r1
	jsr	pc,gword
	mov	r0,-(sp)
	cmp	r1,4(sp)
	beq	1f
	movb	6(sp),1(sp)
	br	2f
1:
	movb	6(sp),(sp)
2:
	mov	(sp)+,r0
	jsr	pc,pword
	clr	r0
	rts	pc

_fuiword:
	mov	2(sp),r1
fuiword:
	jsr	pc,giword
	rts	pc

_fuword:
	mov	2(sp),r1
fuword:
	jsr	pc,gword
	rts	pc

giword:
	mov	PS,-(sp)
	spl	HIGH
	mov	nofault,-(sp)
	mov	$err,nofault
	mfpi	(r1)
	mov	(sp)+,r0
	br	1f

gword:
	mov	PS,-(sp)
	spl	HIGH
	mov	nofault,-(sp)
	mov	$err,nofault
	mfpd	(r1)
	mov	(sp)+,r0
	br	1f

_suiword:
	mov	2(sp),r1
	mov	4(sp),r0
suiword:
	jsr	pc,piword
	rts	pc

_suword:
	mov	2(sp),r1
	mov	4(sp),r0
suword:
	jsr	pc,pword
	rts	pc

piword:
	mov	PS,-(sp)
	spl	HIGH
	mov	nofault,-(sp)
	mov	$err,nofault
	mov	r0,-(sp)
	mtpi	(r1)
	br	1f

pword:
	mov	PS,-(sp)
	spl	HIGH
	mov	nofault,-(sp)
	mov	$err,nofault
	mov	r0,-(sp)
	mtpd	(r1)
1:
	mov	(sp)+,nofault
	mov	(sp)+,PS
	rts	pc

err:
	mov	(sp)+,nofault
	mov	(sp)+,PS
	tst	(sp)+
	mov	$-1,r0
	rts	pc

.globl	_copyin, _copyout
.globl	_copyiin, _copyiout
_copyiin:
	jsr	pc,copsu
1:
	mfpi	(r0)+
	mov	(sp)+,(r1)+
	sob	r2,1b
	br	2f

_copyin:
	jsr	pc,copsu
1:
	mfpd	(r0)+
	mov	(sp)+,(r1)+
	sob	r2,1b
	br	2f

_copyiout:
	jsr	pc,copsu
1:
	mov	(r0)+,-(sp)
	mtpi	(r1)+
	sob	r2,1b
	br	2f

_copyout:
	jsr	pc,copsu
1:
	mov	(r0)+,-(sp)
	mtpd	(r1)+
	sob	r2,1b
2:
	mov	(sp)+,nofault
	mov	(sp)+,r2
	clr	r0
	rts	pc

copsu:
	mov	(sp)+,r0
	mov	r2,-(sp)
	mov	nofault,-(sp)
	mov	r0,-(sp)
	mov	10(sp),r0
	mov	12(sp),r1
	mov	14(sp),r2
	asr	r2
	mov	$1f,nofault
	rts	pc

1:
	mov	(sp)+,nofault
	mov	(sp)+,r2
	mov	$-1,r0
	rts	pc

.globl	_idle, _waitloc
_idle:
	mov	PS,-(sp)
	spl	0
	wait
_waitloc:
	mov	(sp)+,PS
	rts	pc

.globl	_savu, _retu, _aretu
_savu:
	spl	HIGH
	mov	(sp)+,r1
	mov	(sp),r0
	mov	sp,(r0)+
	mov	r5,(r0)+
	spl	0
	jmp	(r1)

_aretu:
	spl	7
	mov	(sp)+,r1
	mov	(sp),r0
	br	1f

_retu:
	spl	7
	mov	(sp)+,r1
	mov	(sp),KDSA6
	mov	$_u,r0
1:
	mov	(r0)+,sp
	mov	(r0)+,r5
	spl	0
	jmp	(r1)

.globl	_spl0, _spl1, _spl4, _spl5, _spl6, _spl7
_spl0:
	spl	0
	rts	pc

_spl1:
	spl	1
	rts	pc

_spl4:
	spl	4
	rts	pc

_spl5:
	spl	5
	rts	pc

_spl6:
	spl	6
	rts	pc

_spl7:
	spl	HIGH
	rts	pc

.globl	_copyseg
_copyseg:
	mov	PS,-(sp)
	mov	4(sp),SISA0
	mov	6(sp),SISA1
	mov	$10000+HIPRI,PS
	mov	r2,-(sp)
	clr	r0
	mov	$8192.,r1
	mov	$32.,r2
1:
	mfpd	(r0)+
	mtpd	(r1)+
	sob	r2,1b
	mov	(sp)+,r2
	mov	(sp)+,PS
	rts	pc

.globl	_clearseg
_clearseg:
	mov	PS,-(sp)
	mov	4(sp),SISA0
	mov	$10000+HIPRI,PS
	clr	r0
	mov	$32.,r1
1:
	clr	-(sp)
	mtpd	(r0)+
	sob	r1,1b
	mov	(sp)+,PS
	rts	pc

.globl	_dpadd
_dpadd:
	mov	2(sp),r0
	add	4(sp),2(r0)
	adc	(r0)
	rts	pc

.globl	_dpcmp
_dpcmp:
	mov	2(sp),r0
	mov	4(sp),r1
	sub	6(sp),r0
	sub	8(sp),r1
	sbc	r0
	bge	1f
	cmp	r0,$-1
	bne	2f
	cmp	r1,$-512.
	bhi	3f
2:
	mov	$-512.,r0
	rts	pc
1:
	bne	2f
	cmp	r1,$512.
	blo	3f
2:
	mov	$512.,r1
3:
	mov	r1,r0
	rts	pc

.globl	_ldiv
_ldiv:
	clr	r0
	mov	2(sp),r1
	div	4(sp),r0
	rts	pc

.globl	_lrem
_lrem:
	clr	r0
	mov	2(sp),r1
	div	4(sp),r0
	mov	r1,r0
	rts	pc

.globl	_lshift
_lshift:
	mov	2(sp),r1
	mov	(r1)+,r0
	mov	(r1),r1
	ashc	4(sp),r0
	mov	r1,r0
	rts	pc

.globl	csv
csv:
	mov	r5,r0
	mov	sp,r5
	mov	r4,-(sp)
	mov	r3,-(sp)
	mov	r2,-(sp)
	jsr	pc,(r0)

.globl cret
cret:
	mov	r5,r1
	mov	-(r1),r4
	mov	-(r1),r3
	mov	-(r1),r2
	mov	r5,sp
	mov	(sp)+,r5
	rts	pc

.globl	_u
_u	= 140000
usize	= 12.

CSW	= 177570
PS	= 177776
SSR0	= 177572
SSR1	= 177574
SSR2	= 177576
SSR3	= 172516
KISA0	= 172340
KISA1	= 172342
KISA7	= 172356
KISD0	= 172300
KISD7	= 172316
KDSA0	= 172360
KDSA6	= 172374
KDSA7	= 172376
KDSD0	= 172320
KDSD6	= 172334
SISA0	= 172240
SISA1	= 172242
SISD0	= 172200
SISD1	= 172202
MSCR	= 177746
IO	= 177600
SL	= 177774

.data
.globl	_ka6
.globl	_cputype

_ka6:	KDSA6
_cputype:45.
stk:	0

.bss
.globl	ssr
nofault:.=.+2
ssr:	.=.+6
dispdly:.=.+2
saveps:	.=.+2
power:	.=.+2
