/ machine language assist
/ for 11/40

/ options...
/
/ dump device
.RK	= 0		/RK05 - dump to swap area
.MT	= 0		/TU10 - drive 0
.HT	= 1		/TU16 - drive 0 800bpi
/
/ character queue type
.NEWPUTC = 1
/ idle here not in l40.s
.ORIGIDLE = 0
/
/ stack limit register
SLR	= 177774
/
/ jeff rottman overlay code
/ big-unix. see param.h
.BIGUNIX = 1	/ cannot be selected with .BUFMAP
/
/ mapped buffers via ka5
.BUFMAP  = 0	/ cannot be selected with .BIGUNIX
/
//// consistency check /////
.if	.BUFMAP
.if	.BIGUNIX
BUFMAP & BIGUNIX can't be selected together
.endif
.endif
//// consistency check /////
.if	.BIGUNIX
.if	.BUFMAP
BUFMAP & BIGUNIX can't be selected together
.endif
.endif
//// consistency check /////
/ fast access to interrupt routines
.QUICK = 0

/ non-UNIX instructions
mfpi	= 6500^tst
mtpi	= 6600^tst
wait	= 1
rtt	= 6
reset	= 5
rti	= 2


.if	.BUFMAP
	.globl	_bswtch, _b
.endif

.globl	_idle,_lks
.if	.ORIGIDLE
_idle:
	mov	PS,-(sp)
/	bic	$340,PS
	clrb	PS
//*	enable clock interrupts -- just in case			  *unsw*
	bis	$100,*_lks
	wait
	mov	(sp)+,PS
	rts	pc
.endif

.globl	bustrap, trap, call
.globl	_trap
bustrap:

.if	SLR

STV	= 14			/ stack trap code

	tst	sp		/red violation
	bne	1f
	mov	$_u+[usize*64.],sp	/re-use stack
	mov	*$2,-(sp)		/PSW
	mov	*$0,-(sp)		/PC
0:	mov	$STV,-4(sp)	/stack trap
	br	9f
1:
	incb	SLR+1		/highest address protected
	cmp	SLR,sp		/better than add $400,SLR
	blo	2f
	sub	$1000,SLR	/give an extra 256 bytes
	br	0b
2:
	decb	SLR+1		/restore stack limit
	clr	-4(sp)		/real bus error
	br	8f
.endif


trap:
	mov	PS,-4(sp)
8:
	tst	nofault
	bne	1f
9:
	mov	SSR0,ssr
	mov	SSR2,ssr+4
	mov	$1,SSR0
	jsr	r0,call1; _trap
	/ no return
1:
	mov	$1,SSR0
	mov	nofault,(sp)
	rtt

.globl	_runrun, _swtch
call1:
	tst	-(sp)
/	bic	$340,PS
	clrb	PS
	br	1f

call:
	mov	PS,-(sp)
1:
	mov	r1,-(sp)
	mfpi	sp
	mov	4(sp),-(sp)
	bic	$!37,(sp)
	bit	$30000,PS
	beq	1f
	jsr	pc,*(r0)+
2:
	bis	$340,PS
	tstb	_runrun
	beq	2f
/	bic	$340,PS
	clrb	PS
	jsr	pc,_swtch
	br	2b
2:
	tst	(sp)+
	mtpi	sp
	br	2f
1:
	bis	$30000,PS
	jsr	pc,*(r0)+
	cmp	(sp)+,(sp)+
2:
	mov	(sp)+,r1
	tst	(sp)+
	mov	(sp)+,r0
	/	chrism & gregr to make t bit tracing work
	/ for system calls. ps bits 8-11 set if bpt trap occurred
	bit	$3400,2(sp)
	bne	1f
	rti
1:	bic	$3400,2(sp)
	rtt

.if	.QUICK
.globl	dquickcall
dquickcall:
	jsr	r1,0f
0:
	mov	PS,-(sp)
	bic	$!37,(sp)
	jsr	pc,*(r0)+
	tst	(sp)+
	br	1f

.globl	quickcall
quickcall:
	mov	r1,-(sp)
	jsr	pc,*(r0)+
1:
	mov	(sp)+,r1
	mov	(sp)+,r0
	rti
.endif

	.globl	_savfp, _display
_savfp:
_display:
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
	mfpi	(r1)
	inc	(sp)
	mtpi	(r1)
	br	3f
2:
	clr	6(r2)
3:
	mov	(sp)+,nofault
1:
	mov	(sp)+,r2
	rts	pc

/ Character list get/put

.globl	_getc, _putc
.globl	_cfreelist

.if	.NEWPUTC

.globl	_cblockm, _cblockl


/	c = getc( cp )		struct clist *cp;

_getc:
	mov	r2,-(sp)
	mov	4(sp),r2
	mov	PS,-(sp)
	movb	$300,PS		/ spl 6
	mov	2(r2),r1	/ first ptr
	beq	5f		/ buffer empty
0:
	movb	(r1)+,r0
	bic	$!377,r0
	dec	(r2)+		/ update count
	beq	4f		/ return last block to free
	bit	_cblockm,r1
	beq	2f		/ return empty block to free
	mov	r1,(r2)
1:
	mov	(sp)+,PS
	mov	(sp)+,r2
	rts	pc

2:
	sub	_cblockl,r1
	mov	(r1),(r2)
	add	$2,(r2)		/ update first pointer
3:
	mov	_cfreelist,(r1)
	mov	r1,_cfreelist
	br	1b

4:
	clr	(r2)+
	clr	(r2)
	dec	r1
	bic	_cblockm,r1
	br	3b

5:
	mov	$-1,r0
	br	1b


/	putc( c , cp )		struct clist *cp;

_putc:
	mov	4(sp),r0
	mov	PS,-(sp)
	movb	$300,PS		/ spl 6
	mov	4(r0),r1	/ last ptr
	beq	4f		/ buffer empty
	bit	_cblockm,r1
	beq	2f		/ block full
0:
	movb	4(sp),(r1)+
	mov	r1,4(r0)
	inc	(r0)		/ update count
	clr	r0
1:
	mov	(sp)+,PS
	rts	pc

2:
	sub	_cblockl,r1
	mov	_cfreelist,-(sp)
	beq	3f		/ freelist empty ( r0 != 0 )
	mov	*(sp),_cfreelist
	mov	(sp),(r1)
	mov	(sp)+,r1
	clr	(r1)+
	br	0b

3:
	tst	(sp)+
	br	1b

4:
	mov	_cfreelist,r1
	beq	1b		/ freelist empty ( r0 != 0 )
	mov	(r1),_cfreelist
	clr	(r1)+
	mov	r1,2(r0)	/ set first ptr
	br	0b

.endif

.if	.NEWPUTC - 1

_getc:
	mov	2(sp),r1
	mov	PS,-(sp)
	mov	r2,-(sp)
/	bis	$340,PS
/	bic	$100,PS		/ spl 5
	movb	$240,PS		/ spl 5
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
/	bis	$340,PS
/	bic	$100,PS		/ spl 5
	movb	$240,PS		/ spl 5
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

.endif

.globl	_backup
.globl	_regloc
_backup:
	mov	2(sp),ssr+2
	mov	r2,-(sp)
	jsr	pc,backup
	mov	r2,ssr+2
	mov	(sp)+,r2
	movb	jflg,r0
	bne	2f
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

/ hard part
/ simulate the ssr2 register missing on 11/40

backup:
	clr	r2		/ backup register ssr1
	mov	$1,bflg		/ clrs jflg
	mov	ssr+4,r0
	jsr	pc,fetch
	mov	r0,r1
	ash	$-11.,r0
	bic	$!36,r0
	jmp	*0f(r0)
0:		t00; t01; t02; t03; t04; t05; t06; t07
		t10; t11; t12; t13; t14; t15; t16; t17

t00:
	clrb	bflg

t10:
	mov	r1,r0
	swab	r0
	bic	$!16,r0
	jmp	*0f(r0)
0:		u0; u1; u2; u3; u4; u5; u6; u7

u6:	/ single op, m[tf]pi, sxt, illegal
	bit	$400,r1
	beq	u5		/ all but m[tf], sxt
	bit	$200,r1
	beq	1f		/ mfpi
	bit	$100,r1
	bne	u5		/ sxt

/ simulate mtpi with double (sp)+,dd
	bic	$4000,r1	/ turn instr into (sp)+
	br	t01

/ simulate mfpi with double ss,-(sp)
1:
	ash	$6,r1
	bis	$46,r1		/ -(sp)
	br	t01

u4:	/ jsr
	mov	r1,r0
	jsr	pc,setreg	/ assume no fault
	bis	$173000,r2	/ -2 from sp
	rts	pc

t07:	/ EIS
	clrb	bflg

u0:	/ jmp, swab
u5:	/ single op
	mov	r1,r0
	br	setreg

t01:	/ mov
t02:	/ cmp
t03:	/ bit
t04:	/ bic
t05:	/ bis
t06:	/ add
t16:	/ sub
	clrb	bflg

t11:	/ movb
t12:	/ cmpb
t13:	/ bitb
t14:	/ bicb
t15:	/ bisb
	mov	r1,r0
	ash	$-6,r0
	jsr	pc,setreg
	swab	r2
	mov	r1,r0
	jsr	pc,setreg

/ if delta(dest) is zero,
/ no need to fetch source

	bit	$370,r2
	beq	1f

/ if mode(source) is R,
/ no fault is possible

	bit	$7000,r1
	beq	1f

/ if reg(source) is reg(dest),
/ then if delta(source) = 0
/ assume fault is in destination
/ otherwise,
/ too bad.

	mov	r2,-(sp)
	bic	$174370,(sp)
	cmpb	1(sp),(sp)+
	bne	3f
	bit	$174000,r2
	bne	t17
	rts	pc
3:

/ start source cycle
/ pick up value of reg

	mov	r1,r0
	ash	$-6,r0
	bic	$!7,r0
	movb	_regloc(r0),r0
	asl	r0
	add	ssr+2,r0
	mov	(r0),r0

/ if reg has been incremented,
/ must decrement it before fetch

	bit	$174000,r2
	ble	2f
	dec	r0
	bit	$10000,r2
	beq	2f
	dec	r0
2:

/ if mode is 6,7 fetch and add X(R) to R

	bit	$4000,r1
	beq	2f
	bit	$2000,r1
	beq	2f
	mov	r0,-(sp)
	mov	ssr+4,r0
	add	$2,r0
	jsr	pc,fetch
	add	(sp)+,r0
2:

/ fetch operand
/ if mode is 3,5,7 fetch *

	jsr	pc,fetch
	bit	$1000,r1
	beq	1f
	bit	$6000,r1
	bne	fetch
1:
	rts	pc

t17:	/ illegal
u1:	/ br
u2:	/ br
u3:	/ br
u7:	/ illegal
	incb	jflg
	rts	pc

setreg:
	mov	r0,-(sp)
	bic	$!7,r0
	bis	r0,r2
	mov	(sp)+,r0
	ash	$-3,r0
	bic	$!7,r0
	movb	0f(r0),r0
	tstb	bflg
	beq	1f
	bit	$2,r2
	beq	2f
	bit	$4,r2
	beq	2f
1:
	cmp	r0,$20
	beq	2f
	cmp	r0,$-20
	beq	2f
	asl	r0
2:
	bisb	r0,r2
	rts	pc

0:	.byte	0,0,10,20,-10,-20,0,0

fetch:
	bic	$1,r0
	mov	nofault,-(sp)
	mov	$1f,nofault
	mfpi	(r0)
	mov	(sp)+,r0
	mov	(sp)+,nofault
	rts	pc

1:
 	mov	(sp)+,nofault
	clrb	r2			/ clear out dest on fault
	mov	$-1,r0
	rts	pc

.bss
bflg:	.=.+1
jflg:	.=.+1
.text

.globl	_fubyte, _subyte
.globl	_fuibyte, _suibyte
.globl	_fuword, _suword
.globl	_fuiword, _suiword
_fuibyte:
_fubyte:
	mov	2(sp),r1
	bic	$1,r1
	jsr	pc,gword
	cmp	r1,2(sp)
	beq	1f
	swab	r0
1:
	bic	$!377,r0
	rts	pc

_suibyte:
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
_fuword:
	mov	2(sp),r1
fuword:
	jsr	pc,gword
	rts	pc

gword:
	mov	PS,-(sp)
	bis	$340,PS
	mov	nofault,-(sp)
	mov	$err,nofault
	mfpi	(r1)
	mov	(sp)+,r0
	br	1f

_suiword:
_suword:
	mov	2(sp),r1
	mov	4(sp),r0
suword:
	jsr	pc,pword
	rts	pc

pword:
	mov	PS,-(sp)
	bis	$340,PS
	mov	nofault,-(sp)
	mov	$err,nofault
	mov	r0,-(sp)
	mtpi	(r1)
	clr	r0			/  indicate success
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
_copyin:
	jsr	pc,copsu
1:
	mfpi	(r0)+
	mov	(sp)+,(r1)+
	sob	r2,1b
	br	2f

_copyout:
	jsr	pc,copsu
1:
	mov	(r0)+,-(sp)
	mtpi	(r1)+
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

.globl	_savu, _retu, _aretu
_savu:
	bis	$340,PS
	mov	(sp)+,r1
	mov	(sp),r0
	mov	sp,(r0)+
	mov	r5,(r0)+
.if	.BIGUNIX
	mov	KISA5,(r0)+		/check user.h
.endif
.if	.BUFMAP
	mov	KISA5,(r0)+		/save current buffer pointer
.endif
/	bic	$340,PS
	clrb	PS
	jmp	(r1)

_aretu:
	bis	$340,PS
	mov	(sp)+,r1
	mov	(sp),r0
	br	1f

_retu:
	bis	$340,PS
	mov	(sp)+,r1
	mov	(sp),KISA6
	mov	$_u,r0
1:
	mov	(r0)+,sp
	mov	(r0)+,r5
.if	.BIGUNIX
	mov	(r0)+,KISA5		/check user.h
.endif
.if	.BUFMAP
	mov	(r0)+,KISA5		/restore buffer pointer
.endif
/	bic	$340,PS
	clrb	PS
	jmp	(r1)

.globl	_spl0, _spl1, _spl4, _spl5, _spl6, _spl7
_spl0:
/	bic	$340,PS
	clrb	PS
	rts	pc

_spl1:
/	bis	$40,PS
/	bic	$300,PS
	movb	$40,PS
	rts	pc

_spl4:
	movb	$200,PS
	rts	pc
_spl5:
/	bis	$340,PS
/	bic	$100,PS
	movb	$240,PS
	rts	pc

_spl6:
/	bis	$340,PS
/	bic	$40,PS
	movb	$300,PS
	rts	pc

_spl7:
	bis	$340,PS
	rts	pc

.globl	_copyseg
_copyseg:
	mov	PS,-(sp)
	mov	UISA0,-(sp)
	mov	UISA1,-(sp)
	mov	$30340,PS
	mov	10(sp),UISA0
	mov	12(sp),UISA1
	mov	UISD0,-(sp)
	mov	UISD1,-(sp)
	mov	$6,UISD0
	mov	$6,UISD1
	mov	r2,-(sp)
	clr	r0
	mov	$8192.,r1
	mov	$32.,r2
1:
	mfpi	(r0)+
	mtpi	(r1)+
	sob	r2,1b
	mov	(sp)+,r2
	mov	(sp)+,UISD1
	mov	(sp)+,UISD0
	mov	(sp)+,UISA1
	mov	(sp)+,UISA0
	mov	(sp)+,PS
	rts	pc

.globl	_clearseg
_clearseg:
	mov	PS,-(sp)
	mov	UISA0,-(sp)
	mov	$30340,PS
	mov	6(sp),UISA0
	mov	UISD0,-(sp)
	mov	$6,UISD0
	clr	r0
	mov	$32.,r1
1:
	clr	-(sp)
	mtpi	(r0)+
	sob	r1,1b
	mov	(sp)+,UISD0
	mov	(sp)+,UISA0
	mov	(sp)+,PS
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

.globl	dump
dump:
	bit	$1,SSR0
	bne	dump

/ save regs r0,r1,r2,r3,r4,r5,r6,KISA6,KISA5
/ starting at abs location 4

	mov	r0,4
	mov	$6,r0
	mov	r1,(r0)+
	mov	r2,(r0)+
	mov	r3,(r0)+
	mov	r4,(r0)+
	mov	r5,(r0)+
	mov	sp,(r0)+
	mov	KISA6,(r0)+
.if	.BIGUNIX
	mov	KISA5,(r0)+
.endif
.if	.BUFMAP
	mov	KISA5,(r0)+
.endif

.if	.MT
/ dump all of core (ie to first mt error)
/ onto mag tape. (9 track or 7 track 'binary')

	mov	$MTC,r0
	mov	$60004,(r0)+
	clr	2(r0)
1:
	mov	$-512.,(r0)
	inc	-(r0)
2:
	tstb	(r0)
	bge	2b
	tst	(r0)+
	bge	1b
	reset

/ end of file and loop

	mov	$60007,-(r0)
	br 	.
.endif

.if	.RK
	mov	$RKCS,r0		/ load rkcs
	mov	$2,(r0)+		/ load function write
	clr	2(r0)			/ clear rkba
	mov	$rkda,4(r0)		/ load rkda - drive 0 blk 2020
1:
	mov	$-256.,(r0)		/ word count - 256 - could be bigger
	inc	-(r0)			/ go
2:
	tstb	(r0)			/ wait for done
	bge	2b
	tst	(r0)+			/ got error ?
	bge	1b			/ no - continue

/end of physical memory -- loop

	br	.
.endif

.if	.HT
	reset
	mov	$1300,*$HTTC
	mov	$HTCS1,r0
	mov	$60,(r0)
1:
	mov	$177000,*$HTFC
	mov	$177400,*$HTWC
	inc	(r0)
2:
	tstb	(r0)
	bge	2b
	tst	(r0)
	bge	1b
	mov	$27,(r0)
	br	.

HTCS1	=	172440
HTWC	=	172442
HTBA	=	172444
HTFC	=	172446
HTCS2	=	172450
HTTC	=	172472

.endif

.if	.BIGUNIX

.globl	start, _end, _edata, _main, _eover, _eu, _eto, _etoa, _eres
start:
	mov	$trap,*$0		/ pick up trap thru 0
	bit	$1,SSR0
	bne	start			/ loop if restart
	reset

/ initialize systems segments

	mov	$KISA0,r0
	mov	$KISD0,r1
	mov	$200,r4
	clr	r2
	mov	$8.,r3
1:
	mov	r2,(r0)+
	mov	$77406,(r1)+		/ 4k rw
	add	r4,r2
	sob	r3,1b


	mov	$IO,-(r0)
	mov	$_eto,-(r0)
	mov	$_eover,r2
	mov	$_etoa,r3
	inc	SSR0
2:
	cmp	$_eres,r2
	bne	1f
	sub	$[3*usize]+3,(r0)
1:
	bit	$77,r3
	bne	1f
	add	$100,r3
	dec	(r0)
1:
	mov	-(r2),-(r3)
	cmp	r2,$_edata
	bhi	2b
	mov	$_eu,(r0)
	mov	$_u+[usize*64.],sp

/ clear bss

	mov	$_edata,r0
1:
	clr	(r0)+
	cmp	r0,$_end
	blo	1b

/ clear user block

	mov	$_u,r0
1:
	clr	(r0)+
	cmp	r0,$_u+[usize*64.]
	blo	1b

/ set up previous mode and call main
/ on return, enter user mode at 0R

	mov	$30000,PS
	jsr	pc,_main
	mov	$170000,-(sp)
	clr	-(sp)
	rtt
.endif

.if	.BIGUNIX - 1

.globl	start, _end, _edata, _main
start:
	mov	$trap,*$0		/ pick up trap thru 0
	bit	$1,SSR0
	bne	start			/ loop if restart
	reset

/ initialize systems segments

	mov	$KISA0,r0
	mov	$KISD0,r1
	mov	$200,r4
	clr	r2
	mov	$6,r3
1:
	mov	r2,(r0)+
	mov	$77406,(r1)+		/ 4k rw
	add	r4,r2
	sob	r3,1b

/ initialize user segment

	mov	$_end+63.,r2
	ash	$-6,r2
	bic	$!1777,r2
	mov	r2,(r0)+		/ ksr6 = sysu
	mov	$usize-1\<8|6,(r1)+

/ initialize io segment
/ set up counts on supervisor segments

	mov	$IO,(r0)+
	mov	$77406,(r1)+		/ rw 4k

/ get a sp and start segmentation

	mov	$_u+[usize*64.],sp
	inc	SSR0

/ clear bss

	mov	$_edata,r0
1:
	clr	(r0)+
	cmp	r0,$_end
	blo	1b

/ clear user block

	mov	$_u,r0
1:
	clr	(r0)+
	cmp	r0,$_u+[usize*64.]
	blo	1b
.if 	.BUFMAP
/
/	set up buffer pointer
/
	mov	$-1,KISA5		/ illegal address initially
	mov	$3406,KISD5		/ 512 bytes, RW
.endif

/ set up previous mode and call main
/ on return, enter user mode at 0R

	mov	$30000,PS
	jsr	pc,_main
	mov	$170000,-(sp)
	clr	-(sp)
	rtt

_bswtch:
.if	.BUFMAP
	/ converts a buffer header pointer
	/ to a memory block reference and sets KA5

baddr = 16		/ offset of b_addr in buf header struct

	mov	2(sp),r0
	mov	baddr(r0),r1
	mov	baddr+2(r0),r0
	ashc	$-6,r0
	mov	r1,KISA5
.endif
	rts	pc
.endif


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
	mov	r5,r2
	mov	-(r2),r4
	mov	-(r2),r3
	mov	-(r2),r2
	mov	r5,sp
	mov	(sp)+,r5
	rts	pc

.globl	_u
_u	= 140000
.if	.BUFMAP
_b	= 120000
bsize   = 8.
.endif
usize	= 16.

PS	= 177776
SSR0	= 177572
SSR2	= 177576
KISA0	= 172340
KISA5	= 172352
KISA6	= 172354
KISD0	= 172300
KISD5	= 172312
.if	.MT
MTC	= 172522
.endif

.if	.RK
RKCS	= 177404			/******** unsw
/calculate disk address of dump block
dblock	= 2020.		/room for 416 blocks (104kw) at end of swap area
			/blocks 2020 to 2435 inclusive
			/optimized rk format assumed
drive	= 0		/drive 0. should be swapdev
rksec	= dblock%12.
rksur	= [dblock%24.]\/12.
rktrack	= dblock\/24.
rkda	= [drive*10000]+[rktrack*40]+[rksur*20]+rksec
.endif

UISA0	= 177640
UISA1	= 177642
UISD0	= 177600
UISD1	= 177602
IO	= 7600

.data
.globl	_ka6, _cputype
_ka6:	KISA6
_cputype:40.

.bss
.globl	nofault, ssr, badtrap
nofault:.=.+2
ssr:	.=.+6
badtrap:.=.+2
