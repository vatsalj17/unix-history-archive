/  options.

.ht = 1		/ tu-16 is dump device
.mt = 0		/ tu-10 is dump device
.pdp70 = 1	/ 11/70 is target machine
.fp = 1		/ floating point processor present
.hpdisk = 0	/ enable ecc error correction code for hp disk
.newputc = 1	/ set if want new superior getc/putc
.display = 1	/ enable location 52 as magic pattern !!
.swtchdsp = 1	/ enable location 70 as switch count
			/ really needs .display as well
.idle = 0	/ 0=> place idle in l70.s 1=> place idle here
.powerfail = 0	/ 1=> support for powerfail 0=> no support
.systime = 1	/ 1=> code for _systime generated
.bufmap = 1	/ buffers addressed by KA5
.ubmapd = 1	/ used with above change if unibus devices
.newcopyseg = 1	/ don't use m[ft]pi instructions
.profile = 0	/ enable the profiling
.qswtch = 1	/ reflects state of PROCESS_QUEUES

/	indicate highest priority at which character queues
/	will be accessed

_spl_tty 	=	_spl5
tty_prio	=	5

/ machine language assist for a pdp-11/70 with fpp
/ extended and (improved) J. N. Rottman

/ This code will also work on an 11/45 if the test on the
/ CPU error register is replaced with a slower direct comparison
/ on the kernal SP. (pdp70=0)


/	External definitions for l.s

	.globl	dump, start, call, quickcall, code00, code01, code02, code03
	.globl	code04, code05, code06, pirqi

/	External data references

	.globl	_etext, _edata, _end, _regloc, _cfreelist, _cfreecount
	.globl	_runrun

/	External text references

	.globl	_main, _trap, _swtch
.if	.qswtch
	.globl	_qswtch
.endif

/	External data definitions

	.globl	_ka6, _cputype, nofault, ssr, _u, _uufec, _uufsav, _uuerror

/	External text definitions

	.globl	_savfp, _incupc, _display, _getc, _putc, _backup
	.globl	_fubyte, _fuword, _subyte, _suword, loadfp, _fkword
	.globl	_fuibyte, _suibyte,  _fuiword, _suiword, _spl_tty
	.globl	_savu, _retu, _aretu
.if	.idle
	.globl	_idle
.endif
.if	.bufmap
	.globl	_bswtch, _bufarea, _b
.endif
.if	.display
	.globl	diswflag
.endif
.if	.swtchdsp
	.globl	_swi_count,_lst_count
.endif
	.globl	_spl0, _spl1, _spl4, _spl5, _spl6, _spl7
	.globl	_copyin, _copyout, _clearseg, _copyseg
	.globl	_dpcmp, _ldiv, _longdiv, _lrem, csv, cret
	.globl	_btoc, _itol
	.globl	_systime, _time

/	Non-unix instructions

mfpi	=	6500 ^ tst
mtpi	=	6600 ^ tst
mfpd	=	106500 ^ tst
mtpd	=	106600 ^ tst
spl	=	230
ldfps	=	170100 ^ tst
stfps	=	170200 ^ tst
stst	=	170300 ^ tst
wait	=	1
rti	=	2
rtt	=	6
halt	=	0
reset	=	5


/
/	Mag-tape dump.
/	Save the registers in low core and write all
/	of core onto magtape.  Entry is from 44  abs.
/
/	Registers r0, r1, r2, r3, r4, r5, r6, KIA6
/	are saved in low core starting at location 4.
/	The blocksize for the type is 512. bytes.
/

.data

dump:	mov	r0, 4
	mov	$6,r0
	mov	r1,(r0)+
	mov	r2,(r0)+
	mov	r3,(r0)+
	mov	r4,(r0)+
	mov	r5,(r0)+
	mov	sp,(r0)+
	mov	KDSA6,(r0)+
.if	.bufmap
	mov	KDSA5,(r0)+
.endif
	mov	pc,46		/ protect in case of restart

.if	.mt
	mov	$MTC,r0
	mov	$60004,(r0)+
	clr	2(r0)

1:	mov	$-512.,(r0)
	inc	-(r0)

2:	tstb	(r0)
	bge	2b
	tst	(r0)+
	bge	1b

	reset
	mov	$60007,-(r0)
	br	.
.endif
.if .ht
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


/ Startup code.  Since core is shuffled, this code can
/ be execute but once.

start:	mov	$code15,*$0	/ set to catch spurious interrupts
				/ also prevent restart from zero
	reset
	clr	PS		/ These things are not defined after reset
.if	.pdp70
	clr	CPUERR
	mov	$-1,MEMERR
.endif

/ set KIO to physical 0

	mov	$77406,r3
	mov	$KISA0,r0
	mov	$KISD0,r1
	clr	(r0)+
	mov	r3,(r1)+

/ set KI1-6 to eventual text resting place

	mov	$_end+63.,r2
	ash	$-6,r2
	bic	$!1777,r2

1:	mov	r2,(r0)+
	mov	r3,(r1)+
	add	$200,r2
	cmp	r0,$KISA7
	blos	1b

/ set KI7 to IO segment for escape

	mov	$IO,-(r0)

/ set KD0-7 to  physical

	mov	$KDSA0,r0
	mov	$KDSD0,r1
	clr	r2

1:	mov	r2,(r0)+
	mov	r3,(r1)+
	add	$200,r2
	cmp	r0,$KDSA7
	blos	1b

/ Initialization :
/	get a temporary 1-word stack and turn on
/	segmentation.  Then copy text to I space
/	and clear the bss in data space.

	mov	$stk+2,sp
	mov	$67,SSR3		/ 22-bit map, K+U sep.
	bit	$20,SSR3
	beq	1f
	mov	$70.,_cputype

1:	inc	SSR0
	mov	$_etext,r0
	mov	$_edata,r1
	add	$_etext-8192.,r1

1:	mov	-(r1),-(sp)
	mtpi	-(r0)
	cmp	r1,$_edata
	bhi	1b

1:	clr	(r1)+
	cmp	r1,$_end
	blo	1b

/ Use KI escape to set KD7 to IO segment. Set KD6 to the
/ first available core to serve as the swap process's
/ u-table

	mov	$IO,-(sp)
	mtpi	*$KDSA7
	mov	$_etext-8192.+63.,r2
	ash	$-6,r2
	bic	$!1777,r2
	add	KISA1,r2
	mov	r2,KDSA6
	mov	$7406,KDSD6		/ USIZE long only (RW)

/ Set up the real stack at the top of the u-table, and
/ clear the rest of it

	mov	$_u+[usize*64.],sp
	mov	$_u,r0

1:	clr	(r0)+
	cmp	r0,sp
	blo	1b

.if	.bufmap
	/ set up KDSA5 for buffer pointer
	mov	KDSA7,KDSA5		/ hopefully illegal
	mov	$3406,KDSD5		/ 512 bytes, RW

.endif

/ Start system profiling

/	jsr	pc,_isprof

/ Set up previous mode as user mode and call main.
/ On return, enter user mode at 0R

	mov	$30000,PS
	jsr	pc,_main
	mov	$170000,-(sp)
	clr	-(sp)
	rtt



.text

/ Entry to the system from all traps, interrupts, and aborts is
/ handled in the following code.
/
/ Each device's vector contains a new kernal PS at the device's
/ bus grant priority,  and a pointer to a code segment of the
/ form:
/	jsr r0,call; _xxxxxxx
/	jsr r0,quickcall; _xxxxxxx
/ where `xxxxxxx' is the name of the device's interrupt routine.
/
/ Traps and aborts are more complicated. Entry is always in
/ kernal mode at priority 7, and enters the following routines:
/
/ 1)	code00		Floating point asynchronous interrupt. If
/			the user is active, the interrupt is reported
/			back immediately.  Otherwise, since the interrupt
/			is asynchronous, it could occur from kernal mode
/			or supervisor mode. In this case the interrupt is
/			just posted for the user, and if the interrupted
/			priority is high, the actual processing is PIRQ'd
/			at priority 1. Since the FEA and FEC are volitile,
/			they are saved when the interrupt occurs. They
/			cannot be restored, but may be retrieved via a system
/			call.
/
/ 2)	code01		IOT.  If the request came from the supervisor state,
/			it is a request for an interrupt exit from a super-
/			visor interrupt handler.  The handler could not do 
/			the rti directly since it could have interrupted a kernal
/			task.
/				If the previous mode was user mode, the trap
/			is posted in the usual way. For efficiency, no check
/			is made to distinguish supervisor from kernal iot's,
/			which should never occur.
/
/ 3)	code02		BPT/trace trap, illegal instruction, system entry:
/			If these are from the user they are handled in the
/			standard manner.
/				Any of these calls from the supervisor abort
/			it with an indicative error code.
/				Any of these calls from the kernal are fatal.
/
/  4)	code03		Parity errors.
/			errors.
/
/ 5)	code04		EMT. If this code is from the supervisor, it is a
/			request for a kernal function. If the function code
/			is legal, the function is performed and control returns;
/			otherwise, the supervisor is aborted.
/				From user mode this is just a trappable signal.
/
/ 6)	code05		Bus error. The cpu error register is interrogated to
/			see if this a stack overflow (code07) or a bus error.
/			The following applies just to real bus errors:
/				nofault_______ is checked; if it is non-zero it is
/			the address of an error catching routine to which control
/			should be returned on the rti. Otherwise, bus errors
/			are handled like class (3).
/
/ 7)	code06		Segmentation violation. If nofault is set, the
/			segmentation error registers are thawed, and control
/			returns immediately.  If the error comes from super-
/			visor state, the supervisor is aborted. If it comes
/			from kernal state, the system dies.  If it comes from
/			user state, the error registers are saved for a possible
/			stack page fault backup.
/
/ 8)	code07		Bad PIRQ, stack overfow. (see (6)). These are fatal
/			software botches.
/
/
/	The codes reported to trap are:
/	NON-FATAL
/		0	User floating point exception
/		1	Kernal floating point exception
/		2	Illegal instruction
/		3	BPT/trace trap
/		4	SYS entry
/		5	IOT
/		6	EMT
/		7	Bus error
/		10	Segmentation violation
/		12	Parity
/		15	Trap thru location zero
/		16	Power fail recovery completed
/			( trap is called from powerf.s )
/	FATAL-
/		22	Kernal illegal instruction
/		23	Kernal BPT
/		24	Kernal SYS
/		25	Kernal IOT
/		26	Kernal EMT
/		27	Kernal Bus error
/		30	Kernal Segmentation
/		32	Kernal parity
/		33	Bad PIRQ
/		34	Stack limit overflow
/

code00:	stst	*_uufec		/ save FEC and FEA in active u
	bit	$340,2(sp)	/ from high priority ?
	beq	codeff		/ no - handle immediately
	bis	$PIRQ1,PIRQ	/ set for later processing
	rti			/ and split

pirq1:	bic	$PIRQ1,PIRQ	/ dispatch from PIRQ request 1


codeff:	clr	-(sp)		/ trap code
	bit	$20000,PS	/ from user mode ?
	bne	1f		/ yes
	inc	(sp)		/ set to post and leave
1:	jbr	callt1		/ don't add in displacment

pirq2:
pirq3:
pirq4:
pirq5:
pirq6:
pirq7:
	clr	PIRQ		/ Illegal PIRQ's - clear register
	mov	$33,-(sp)	/ set code

	br	callt1		/ report fatal error

code05:

.if	.pdp70			/ Use CPU error register
	mov	CPUERR,_cpuereg	/ get CPU error register
	clr	CPUERR		/ now reset it !
	bit	$14,_cpuereg	/ yellow/red stack violation ??
	bne	3f		/ yes, so branch.
	bit	$160,_cpuereg	/ odd addr, bus timeout, no mem?
	bne	1f		/ yes, so branch
	bit	$200,_cpuereg	/ no, but illegal halt ??
	beq	1f		/ no, so branch
	mov	$2,-(sp)	/ yes, set illegal instruction
	br	callt1

.bss
slrbef:		. = . + 2
r0save:		. = . + 2
.globl	spsave
spsave:		. = . + 2
.globl	_cpuereg
_cpuereg:	. = . + 2
.text

3:
	tst	slrbef		/ prior stack limit trap ??
	bne	9f		/ --> yes
	bit	$4,_cpuereg
	beq	8f		/ --> YELLOW Violation
9:
	mov	r0,r0save
	mov	sp,spsave
	mov	sp,r0		/ save current (offending??) sp
	mov	$142000,sp	/ reset stack.
	mov	2(r0),-(sp)	/ save the troublesome ps
	mov	 (r0),-(sp)	/ save the troublesome pc
	mov	r0save,r0
	br	7f
8:	sub	$400,SLR	/ let the yellow happen
	inc	slrbef		/ indicate we already got one !!
7:
/*agsm
	mov	$34,-(sp)	/ report code
	br	callt1
.endif

1:	tst	nofault		/ catchable?
	beq	1f		/ no
2:	mov	nofault,(sp)	/ adjust return
	rti			/ exit

1:	mov	$7,-(sp)	/ proper code
	br	callt

code06:	tst	nofault		/ catchable ?
	beq	1f		/ no
	mov	$1,SSR0		/ thaw registers
	br	2b		/ and take nofault exit

1:	bit	$20000,PS	/ user mode ?
	beq	1f		/ no - don't overwrite ssr
	mov	SSR0,ssr	/ save for possible backup
	mov	SSR1,ssr+2
	mov	SSR2,ssr+4

1:	mov	$1,SSR0		/ thaw error registers
	mov	$10,-(sp)	/ segmentation code
	br	callt

code03:				/ Parity. Try to be clever about it.
.if	.pdp70
	bis	$3,MEMCNTRL	/ disable traps for ....
				/ don't want multiple errors
				/ unless they are aborts
	mov	MEMERR,_memereg	/ Save Mem error register
	mov	MEMHIA,_memhigha
	mov	MEMLOA,_memlowa
	mov	_memereg,MEMERR	/ turn off error bits
	mov	$12,-(sp)	/ trap type is 12
	bit	$101000,_memereg/ fatal error ??
	beq	callt1		/ --> no, just flag as a parity error
	tst	nofault		/ is the error capable of being caught ??
	beq	callt		/ --> no, clobber user/system then
	mov	nofault,2(sp)	/ fix return from trap
	br	callt1		/ and treat as simple parity error
.endif
.if	.pdp70 - 1
	mov	$12,-(sp)	/ Trap type 12
	br	callt
.endif

.if	.pdp70
.bss
	.globl	_memhigha, _memlowa, _memereg
_memereg:	. = . + 2
_memhigha:	. = . + 2
_memlowa:	. = . + 2
.text
.endif

code15:				/ Hardware-type botches
	mov	PS,-(sp)	/ previous mode is unimportant
	br	callt2		/ so ignore it here

code01:
code02:
code04:	mov	PS,-(sp)	/ save emt trap code
/	br	callt


callt:
	bit	$20000,PS	/ user fault ??
	bne	callt1		/ --> yes
	add	$20,(sp)	/ indicate kernal fault
callt1:
	spl	0		/ out of critical section
callt2:				/ entry if not to drop pri
	mov	(sp),-(sp)	/ save device code
	mov	r0,2(sp)
	mov	r1,-(sp)
	mfpd	sp		/ get a sp
	mov	4(sp),-(sp)	/ device again
	mov	$1f,r0		/ point to trap
	br	2f

.data; 1: _trap; .text




call:	mov	PS,-(sp)	/ Entry from device interrupt vector
	mov	r1,-(sp)
	mfpd	sp
	mov	4(sp),-(sp)

.if	.powerfail
	.globl	_powflag

2:	tst	_powflag	/ powerfail in progress ??
	beq	2f		/ --> NO
	inc	_powflag
	halt
.endif
2:	bic	$!37,(sp)	/ extra bits in code bit 4 indicates user/kernal
	mov	nofault,6(sp)
	clr	nofault		/ catch errors
	bit	$20000,PS	/ from user mode ?
	beq	1f		/ no - call routine and split

	mov	$20,*_uufsav	/ set maint. bit
	jsr	pc,*(r0)+	/ call routine

2:	spl	6		/ interlock
	tstb	_runrun
	beq	2f		/ see if we should schedule
	spl	0		/ end of interlock
	jsr	pc,_savfp
.if	.qswtch
	jsr	pc,_qswtch	/ return process to run queue
.endif
.if	.qswtch-1
	jsr	pc,_swtch
.endif
	br	2b

2:	jsr	pc,loadfp

	tst	(sp)+		/ "dev"
	mtpd	sp
	br	2f

1:	jsr	pc,*(r0)+
	cmp	(sp)+,(sp)+	/ dump dev and sp

2:	mov	(sp)+,r1
	mov	(sp)+,nofault
	mov	(sp)+,r0
	/ fix015	gregr & chrism
	/	to make t bit tracing work properly
	bit	$3400,2(sp)
	bne	1f
	rti
1:	bic	$3400,2(sp)
	/ end of fix015
	rtt

/
/ quickcall
/
/	Allows faster entry to device interrupt routines.
/	No call to 'swtch' on return. No save of 'dev'.
/	Use with discretion for block devices.
/
quickcall:
	mov	r1,-(sp)
	jsr	pc,*(r0)+
	mov	(sp)+,r1
	mov	(sp)+,r0
	rti

/
/
/	PIRQ interrupt
/

pirqi:	mov	PIRQ,-(sp)	/ PIRQ interrupt dispatch
	movb	(sp),PS		/ set to active PIRQ priority
	bic	$!16,(sp)	/ prepare word dispatch
	add	$1f-2,(sp)
	mov	*(sp)+,pc	/ disptach on PIRQ priority

.data; 1: pirq1; pirq2; pirq3; pirq4; pirq5; pirq6; pirq7; .text

/
/	Save the user's floating point registers.
/	Use the maint. bit to indicate if done.
/

_savfp:

.if	.fp
	mov	_uufsav,r1
	bit	$20,(r1)
	beq	1f
	stfps	(r1)+
	setd			/ added by KFH 24/12/79
	movf	fr0,(r1)+
	movf	fr4,fr0
	movf	fr0,(r1)+
	movf	fr5,fr0
	movf	fr0,(r1)+
	movf	fr1,(r1)+
	movf	fr2,(r1)+
	movf	fr3,(r1)+
.endif

1:	rts	pc

/
/	Reload the user's floating point registers.
/

loadfp:

.if	.fp
	mov	_uufsav,r1
	bit	$20,(r1)	/ floating registers saved?
	bne	1f		/ no
	mov	(r1),r0		/ saved fps
	bic	$4000,(r1)	/ interrupt on floating undefined
	ldfps	(r1)+
	setd			/ added by KFH 24/12/79
	movf	(r1)+,fr0
	movf	(r1)+,fr1
	movf	fr1,fr4
	movf	(r1)+,fr1
	movf	fr1,fr5
	movf	(r1)+,fr1
	movf	(r1)+,fr2
	movf	(r1)+,fr3
	ldfps	r0		/ user's cc's
.endif

1:	rts	pc

/
/	Handle counts for user profiles
/

_incupc:
	mov	r2,-(sp)
	mov	6(sp),r2	/ base of prof with base, leng, off, scale
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
	mov	$2f,nofault
	mfpd	(r1)
	inc	(sp)
	mtpd	(r1)
	br	3f

2:	clr	6(r2)

3:	clr	nofault

1:	mov	(sp)+,r2
	rts	pc

/
/	Display the contents of the word whose address is
/	in the switch registers. If bit 0 is set, the word
/	refers to user space; otherwise, it refers to
/	kernal data space
/

_display:
.if	.display
	tst	diswflag		//// previously waiting ??
	bne	0f			//// -> yes.
	mov	$5,displw		//// slow up the display.
	br	2f
0:	clr	diswflag
	dec	displw
	jge	2f			//// -> display not to change 
	mov	$2,displw
	mov	dispat+0,r0
	mov	dispat+2,r1
	ashc	dishift,r0		//// form next dispat in sequence
	mov	r0,dispat+0
	mov	r1,dispat+2
	ashc	$8.,r0			//// extract the dispat for display
	bit	$077776,r0		//// need to start dishifting the other way ??
	bne	1f			//// -> no.
	neg	dishift
1:	mov	r0,*$52			//// store for later display
2:

.data
dispat:	17;0			//// starting pattern (only use middle 22 bits)
dishift:	1		//// either 1 or -1 (the dishift direction)
displw:		1		//// use to delay the display
diswflag:	0		//// set by _idle to indicate wait done
.text
.endif
.if	.swtchdsp
	mov	_lst_count,r0
	mov	r0,r1
	ash	$-3,r1
	adc	r1		/ round, not truncate
	sub	r1,r0
	add	_swi_count,r0
	mov	r0,_lst_count
	clr	_swi_count
	mov	$177777,r1
	ash	r0,r1
	com	r1
	mov	r1,*$70		/// save here for later display
.endif
	mov	PS,-(sp)
	mov	$30340,PS	/ previous mode is user
	mov	nofault,-(sp)	/save
	mov	$3f,nofault
1:	mov	CSW,r1
	bit	$1,r1
	beq	1f
	dec	r1
	mfpi	(r1)
	mov	(sp)+,r0
	br	4f

1:	mov	(r1),r0
4:	mov	r0,CSW
3:	mov	(sp)+,nofault	/restore
	mov	(sp)+,PS
	rts	pc

/
/	Character list get/put
/

.if	.newputc

.globl	_cblockm, _cblockl


/	c = getc( cp )		struct clist *cp;

_getc:
	mov	r2,-(sp)
	mov	4(sp),r2
	mov	PS,-(sp)
	spl	6
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
	spl	6
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

.if	.newputc - 1
_getc:	mov	2(sp),r1
	mov	PS,-(sp)
	mov	r2,-(sp)
	spl	tty_prio
	mov	2(r1),r2	/ first ptr
	beq	9f		/ empty
	movb	(r2)+,r0	/ character
	bic	$!377,r0
	mov	r2,2(r1)
	dec	(r1)+
	bne	1f
	clr	(r1)+
	clr	(r1)+		/ last ptr
	br	2f

1:	bit	$7,r2
	bne	3f
	mov	-10(r2),(r1)	/ next block
	add	$2,(r1)

2:	dec	r2
	bic	$7,r2
	mov	_cfreelist,(r2)
	mov	r2,_cfreelist
	inc	_cfreecount

3:	mov	(sp)+,r2
	mov	(sp)+,PS
	rts	pc

9:	clr	4(r1)
	mov	$-1,r0
	br	3b

_putc:	mov	2(sp),r0
	mov	4(sp),r1
	mov	PS,-(sp)
	mov	r2,-(sp)
	mov	r3,-(sp)
	spl	tty_prio
	mov	4(r1),r2	/ last ptr
	bne	1f
	mov	_cfreelist,r2
	beq	9f
	mov	(r2),_cfreelist
	dec	_cfreecount
	clr	(r2)+
	mov	r2,2(r1)
	br	2f

1:	bit	$7,r2
	bne	2f
	mov	_cfreelist,r3
	beq	9f
	mov	(r3),_cfreelist
	dec	_cfreecount
	mov	r3,-10(r2)
	mov	r3,r2
	clr	(r2)+

2:	movb	r0,(r2)+
	mov	r2,4(r1)
	inc	(r1)
	clr	r0
3:	mov	(sp)+,r3
	mov	(sp)+,r2
	mov	(sp)+,PS
	rts	pc

9:	mov	pc,r0
	br	3b
.endif

/
/	Backup a user's registers after a stack page
/	fault
/

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
	rts	pc

1:	mov	r1,-(sp)
	asr	(sp)
	asr	(sp)
	asr	(sp)
	bic	$!7,r1
	movb	_regloc(r1),r1
	asl	r1
	add	r0,r1
	sub	(sp)+,(r1)
	rts	pc

/
/	Interspace communitcation routines
/

_fuibyte:
	mov	2(sp),r1
	bic	$1,r1
	jsr	pc,giword
	br	2f

_fubyte:
	mov	2(sp),r1
	bic	$1,r1
	jsr	pc,gword

2:	cmp	r1,2(sp)
	beq	1f
	swab	r0

1:	bic	$!377,r0
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

1:	movb	6(sp),(sp)

2:	mov	(sp)+,r0
	jsr	pc,piword
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

1:	movb	6(sp),(sp)


2:	mov	(sp)+,r0
	jsr	pc,pword
	rts	pc

_fuiword:
	mov	2(sp),r1
	jsr	pc,giword
	rts	pc

_fuword:
	mov	2(sp),r1
	jsr	pc,gword
	rts	pc

giword:
	mov	$err,nofault
	mfpi	(r1)
	br	1f

gword:
	mov	$err,nofault
	mfpd	(r1)
1:	mov	(sp)+,r0
	br	2f


_suiword:
	mov	2(sp),r1
	mov	4(sp),r0
	jsr	pc,piword
	rts	pc
_suword:
	mov	2(sp),r1
	mov	4(sp),r0
	jsr	pc,pword
	rts	pc
piword:
	mov	$err,nofault
	mov	r0,-(sp)
	mtpi	(r1)
	br	1f

pword:
	mov	$err,nofault
	mov	r0,-(sp)
	mtpd	(r1)

1:	clr	r0
2:	clr	nofault
	rts	pc


EFAULT = 14.				/ fix037

err:	mov	$-1,r0
	movb	$EFAULT, *_uuerror	/ fix037
	tst	(sp)+
	br	2b

/ fix036
/ check the existence of a word in kernel D space
/	fkword(addr)
/ returns 1 for success, 0 for failure
_fkword:
	mov	nofault,-(sp)
	mov	$1f,nofault
	tst	*4(sp)
	mov	(sp)+,nofault
	mov	$1,r0
	rts	pc
1:
	mov	(sp)+,nofault
	clr	r0
	rts	pc
	

/
/	Block/word transfers to/from kernal/user/supervisor space
/
/
/	call from C is
/		copy[in/out](from, to, byte_count, segflag)
/	on entry previous mode must be user
/	from and to must be even addresses,
/	byte_count must also be even
/	segflag is:
/		0:	to/from user data
/		1:	to/form kernal data
/		2:	to/from user text
/		3:	to/from supervisor region



_copyin:
	jsr	pc,copsu
		br 4f		/ transfer from user data
		br 2f		/ transfer from kernal data
		br 1f		/ transfer from supervisor

1:	mfpi	(r0)+
	mov	(sp)+,(r1)+
	sob	r2,1b
	br	5f

2:	mov	(r0)+,(r1)+	/ transfer from krnal data
	sob	r2,2b		/ word count
	br	5f

4:	mfpd	(r0)+		/ transfer from user data
	mov	(sp)+,(r1)+
	sob	r2,4b
	br	5f

_copyout:
	jsr	pc,copsu
		br 4f		/ transfer to user data
		br 2b		/ transfer to system data
		br 3f		/ transfer to user text

3:	mov	(r0)+,-(sp)	/ transfer to supervisor i/d
	mtpi	(r1)+
	sob 	r2,3b
	br	5f

4:	mov	(r0)+,-(sp)
	mtpd	(r1)+		/ transfer to user data
	sob	r2,4b

5:	clr	r0		/ successful transfer
6:	mov	(sp)+,PS	/ common exit
	clr	nofault
	mov	(sp)+,r2	/ count register
	rts	pc		/ exit

copsu:	mov	(sp)+,r0	/ general set-up for copyxx
	mov	r2,-(sp)
	mov	PS,-(sp)
	mov	14(sp),-(sp)	/ get segflag
	cmp	(sp),$3		/ if its to supervisor mod
	bne	1f		/ then set previous mode
	bic	$20000,PS	/ to supervisor mode

1:	asl	(sp)		/ word displacment
	add	r0,(sp)		/return address
	mov	10(sp),r0	/ from
	mov	12(sp),r1	/ to
	mov	14(sp),r2	/ byte count
	asr	r2		/ to word count
	mov	$1f,nofault	/ error catcher
	rts	pc		/ to offset branch

1:	mov	$-1,r0		/ error catcher
	br	6b		/ exit with indicator




.if	.idle
/
/	Just idle while nothing is active to speed up transfers
/

_idle:
.if	.display
	inc	diswflag	/ indicate wait in progress
.endif
	wait
	rts	pc
.endif

/
/	Routines to do context switching
/

_savu:	mov	(sp)+,r1
	mov	(sp),r0
	mov	sp,(r0)+
	mov	r5,(r0)+
.if	.bufmap
	mov	KDSA5,(r0)+		/buffer address
.endif
	jmp	(r1)

_aretu:	mov	(sp)+,r1
	mov	(sp),r0
	spl	7		/ critical part where stack is switched
	br	1f

_retu:	mov	(sp)+,r1
	spl	7		/ critical region
	mov	(sp),KDSA6
	mov	$_u,r0
1:
	mov	(r0)+,sp
	mov	(r0)+,r5
.if	.bufmap
	mov	(r0)+,KDSA5
.endif
	spl	0
	jmp	(r1)

_bswtch:
.if	.bufmap
	/ converts a buffer header pointer
	/ to a memory block reference and sets KA5

baddr = 16		/ offset of b_addr in buf header struct

	mov	2(sp),r0
	mov	baddr(r0),r1
	mov	baddr+2(r0),r0
	ashc	$-6,r0
.if	.ubmapd
	add	_bufarea,r1
.endif
	mov	r1,KDSA5
.endif
	rts	pc


/
/	Routine to set priorities
/

_spl0:	spl	0
	rts	pc

_spl1:	spl	1
	rts	pc

_spl4:	spl	4
	rts	pc

_spl5:	spl	5
	rts	pc

_spl6:	spl	6
	rts	pc

_spl7:	spl	7
	rts	pc


/
/	Copy and clear user sements (64 bytes)
/	Called with previous mode = user mode
/

_copyseg:
.if	1-.newcopyseg
	mov	UISA0,-(sp)		/ appropriate bottom two user regs.
	mov	UISA1,-(sp)
	mov	UISD0,-(sp)
	mov	UISD1,-(sp)
	mov	12(sp),UISA0		/from segment
	mov	14(sp),UISA1		/ to segment
	mov	$6,UISD0
	mov	$6,UISD1
	mov	r2,-(sp)
	clr	r0
	mov	$8192.,r1
	mov	$32.,r2

1:	mfpi	(r0)+			/ get a word
	mtpi	(r1)+			/ put a word
	sob	r2,1b			/ loop 'till done

	mov	(sp)+,r2
	mov	(sp)+,UISD1
	mov	(sp)+,UISD0
	mov	(sp)+,UISA1
	mov	(sp)+,UISA0
	rts	pc
.endif
.if	.newcopyseg
parvec = 114
	mov	PS, -(sp)		/ save priority etc
	mov	r2, -(sp)
	clr	r0
	mov	$5*20000, r1
	mov	$8., r2			/ count for sobbing
	mov	*$parvec,-(sp)		/ in case of parody
	mov	$3f,*$parvec
	spl	7
	mov	10(sp), KDSA0		/ from addr
	mov	12(sp), KDSA5		/ to addr
1:
	mov	(r0)+, (r1)+
	mov	(r0)+, (r1)+
	mov	(r0)+, (r1)+
	mov	(r0)+, (r1)+
	sob	r2, 1b

	clr	KDSA0
.if	1-.bufmap
	mov	$5*200, KDSA5
.endif
	mov	(sp)+, *$parvec
	mov	(sp)+, r2
	mov	(sp)+, PS
	rts	pc
3:	/ new parity trap routine
	mov	2(sp),-(sp)		/ a convenient PS
	clr	KDSA0
.if	1-.bufmap
	mov	$5*200,KDSA5
.endif
	jsr	pc,*6(sp)		/ saved spot
	/ rti comes to here - note that the ps is not on the stack anymore
	mov	14(sp),KDSA0
	mov	16(sp),KDSA5
	rti
.endif

_clearseg:
.if 1-.newcopyseg
	mov	UISA0,-(sp)		/ get a user register
	mov	UISD0,-(sp)
	mov	6(sp),UISA0
	mov	$6,UISD0
	mov	$32.,r1
	clr	r0

1:	clr	-(sp)			/ clear 32. words
	mtpi	(r0)+
	sob	r1,1b
	
	mov	(sp)+,UISD0
	mov	(sp)+,UISA0
	rts	pc
.endif
.if	.newcopyseg
parvec = 114
	mov	$5*20000, r0
	mov	$32.,r1			/ count for sobbing
.if	1-.bufmap
	mov	PS, -(sp)		/ save priority etc
	mov	*$parvec,-(sp)		/ in case of parody
	mov	$3f,*$parvec
	spl	7
	mov	6(sp), KDSA5		/ to addr
.endif
.if	.bufmap
	mov	2(sp), KDSA5		/ to addr
.endif
1:
	mov	$0,(r0)+		/ A clr causes a DATIP cycle
					/ which can give unwanted parities
	sob	r1, 1b

.if	1-.bufmap
	mov	$5*200, KDSA5
	mov	(sp)+, *$parvec
	mov	(sp)+, PS
.endif
	rts	pc
.if	1-.bufmap
3:	/ new parity trap routine
	mov	2(sp),-(sp)		/ a convenient PS
	mov	$5*200,KDSA5
	jsr	pc,*6(sp)		/ saved spot
	/ rti comes to here - note that the ps is not on the stack anymore
	mov	12(sp),KDSA5
	rti
.endif
.endif



.if	.hpdisk
.globl	_hpecc

/
/	Error correction for hp disk
/

_hpecc:
	mov	r3,-(sp)	/ save some registers
	mov	r2,-(sp)
	mov	UISA0,-(sp)	/ and some segmentation registers
	mov	UISD0,-(sp)
	mov	PS,-(sp)	/ for previous mode set
	mov	HPPOS,r3	/ bit position of error burst
	dec	r3		/ adjust for zero offset
	clr	r2		/ into word and bit pair
	div	$16.,r2		/ r2 word, r3 bit offset
	mov	r2,-(sp)	/ save for later end check
	mov	r2,r1		/ convert to bytes
	asl	r1
	sub	$512.,r1	/ back up one sector
	sxt	r0		/ will be negative always
	add	HPBAE,r0	/ high order physical address
	add	HPBA,r1		/ low order physical address
	adc	r0		/ long arithmetic
	div	$64.,r0		/ into KT-11 pages
	bis	$30000,PS	/ previous is now user mode
	mov	$406,UISD0	/ short read/write page (128 bytes)
	mov	r0,UISA0	/ into this page
	mov	r1,r2		/ word offset in block
	mov	HPPAT,r1	/ get correction pattern
	clr	r0		/ double precision
	ashc	r3,r0		/ fix by bit offset
	mfpi	(r2)		/ get first word
	xor	r1,(sp)		/ fix low order
	mtpi	(r2)+		/ replace word
	cmp	(sp)+,$255	/ don't exceed block
	beq	1f		/ only one word correction
	mfpi	(r2)		/ fetch second word
	xor	r0,(sp)		/ high order correction
	mtpi	(r2)		/ and replace word
1:
	mov	(sp)+,PS	/ now do the mandatory restore
	mov	(sp)+,UISD0	/ sequence
	mov	(sp)+,UISA0
	mov	(sp)+,r2
	mov	(sp)+,r3
	mov	HPWC,r0		/ see if in mid-transfer
	beq	1f		/ no - return 0, we're done
	inc	HPCS1		/ complete transfer and return(!=0)
1:	
	rts	pc

HPCS1	=	176700		/ Disk registers used above
HPWC	=	176702
HPBA	=	176704
HPPOS	=	176744
HPPAT	=	176746
HPBAE	=	176750
.endif

/	General utility routines
/

_btoc:	mov	2(sp),r1
	clr	r0
	add	$63.,r1
	adc	r0
	ashc	$-6,r0
	mov	r1,r0
	rts	pc

_dpcmp:	mov	2(sp),r0
	mov	4(sp),r1
	sub	6(sp),r0
	sub	8(sp),r1
	sbc	r0
	bge	1f
	cmp	r0,$-1
	bne	2f
	cmp	r1,$-512.
	bhi	3f

2:	mov	$-512.,r0
	rts	pc

1:	bne	2f
	cmp	r1,$512.
	blo	3f

2:	mov	$512.,r1

3:	mov	r1,r0
	rts	pc


_ldiv:	clr	r0
	mov	2(sp),r1
	div	4(sp),r0
	rts	pc


_longdiv:
	mov	2(sp),r0
	mov	4(sp),r1
	div	6(sp),r0
	rts	pc

_lrem:	clr	r0
	mov	2(sp),r1
	div	4(sp),r0
	mov	r1,r0
	rts	pc

/	routine to return a address of string representing
/	system idea of the time
/	call	systime( &string )
/	string to contain after call 'hh:mm'


_systime:
.if	.systime
	jsr	r5,csv		/ save registers
	mov	4(r5),r4	/ address of string area
	add	$6,r4		/ address end of string
	clrb	-(r4)		/ null at end
	mov	_time,r0	/ get systems idea of time
	mov	_time+2,r1

	div	$28800.,r0	/ r0= no of 8 hr lots since start 1970
				/ r1= no of seconds into 8 hour lot
	mov	r0,r3
	clr	r2
	div	$3,r2		/ r2= no of days since start of 1970
				/ r3= no of 8 hours in day
	clr	r0
	div	$60.,r0		/ r0= no of minutes
	mov	r0,r1
	clr	r0
	div	$60.,r0		/ r0= no of hour into 8 hr grp
				/ r1= no of minutes
	jsr	pc,numbin	/ put in minutes
	movb	$':,-(r4)	/ separator

	mov	r3,r1
	mul	$8.,r1
	add	r0,r1		/ r1= no of hours
	jsr	pc,numbin	/ put in hours
	mov	r4,r0		/ pass back address of string
	jmp	cret		/ return
numbin:
	mov	r0,-(sp)
	clr	r0
	div	$10.,r0
	bis	$'0,r0
	bis	$'0,r1
	movb	r1,-(r4)
	movb	r0,-(r4)
	mov	(sp)+,r0
.endif
	rts	pc

_itol:	mov	4(sp),r1
	mov	2(sp),r0
	rts	pc

csv:	mov	r5,r0
	mov	sp,r5
	mov	r4,-(sp)
	mov	r3,-(sp)
	mov	r2,-(sp)
	jsr	pc,(r0)


cret:	mov	r5,r2
	mov	-(r2),r4
	mov	-(r2),r3
	mov	-(r2),r2
	mov	r5,sp
	mov	(sp)+,r5
	rts	pc

_u	= 140000
usize	= 16.
.if	.bufmap
_b	= 120000
bsize	= 8.
.endif

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
KDSA0	= 172360
KDSA5	= 172372
KDSA6	= 172374
KDSA7	= 172376
KDSD0	= 172320
KDSD5	= 172332
KDSD6	= 172334
SISD0	= 172200
SISA0	= 172240
SISD7	= 172216
SISD6	= 172214
SISA6	= 172254
MTC	= 172522

UISA0	= 177640		/ user instruction address 0
UISA1	= 177642		/ user instruction address 1
UISD0	= 177600		/ user instruction descriptor 0
UISD1	= 177602		/ user instruction descriptor 1

PIRQ	= 177772		/ program interrupt request
PIRQ1	= 001000		/ request at priority 1
PIRQ2	= 002000		/ request at priority 2
PIRQ3	= 004000		/ request at priority 3
PIRQ4	= 010000		/ request at priority 4
PIRQ5	= 020000		/ request at priority 5
PIRQ6	= 040000		/ request at priority 6
PIRQ7	= 100000		/ request at priority 7

.if	.pdp70
MEMLOA	= 177740		/ low address of parity
MEMHIA	= 177742		/ hi  address of parity
MEMERR	= 177744		/ memory system error register
MEMCNTRL= 177746		/ memory control register
CPUERR	= 177766		/ cpu error register
.endif
SLR	= 177774		/ stack limit register

IO	= 177600


.data

_ka6:		KDSA6		/ pointer to u-page segment address register
_cputype:	 45.		/ pdp-11/45 or pdp-11/70
stk:		0		/ one word stack for segmentation set-up

.bss
nofault:	.=.+2	/ bus error/segmentation violation bypass
ssr:		.=.+6	/ saved backup registers
_cfreecount:	.=.+2	/ free character count

.text
	.globl	_halt
_halt:
	mov	2(sp),*$CSW
	halt
	rts	pc

.data

.if	.powerfail-1		/ powerfail not supported
	.globl	powfail		/ must be in data segment

powfail:				/ entry at priority level 7
	0				/ halt - nought else to do
	br	powfail			/ loop just in case 'continued'
.endif


.if	.profile
/ System Profiler - Greg Rose july 78.

.globl	_profstart,_pshift,_pmask,_pusertix,_pfreq,_pnbytes,_pspls
.bss
_profstart:	. = . + 2	/ start addr of profile area (64 byte blocks)
_pusertix:	. = . + 4	/ long number of ticks in user space
_pnbytes:	. = . + 2	/ number of bytes in the profile area
_pspls:		. = . + [4*8.]	/ number of ticks at each priority level
_pshift = 0			/ number of bits shift for addr
_pmask = [177777 \< [16.-_pshift]] | 3	/ bits to turn off after shifting
_pfreq = -6353.			/ number of half micros between ticks
				/ in fact PRIME, no relation to HZ

/ Note - initialisation is all done in once.c, since it allocates core for the
/ profile buffer, and it's easier there.

.text
.globl	profile
profile:
	mov	$_pfreq,*$160002	/ schedule next interrupt
	mov	2(sp),r0	/ get previous ps - < 0 => was user mode.
	bmi	1f		/ branch if was user
	bic	$!340,r0	/ clear all but priority
	ash	$-3,r0		/ r0 = pri * 4
	add	$1,_pspls+2(r0)
	adc	_pspls(r0)
	mov	(sp),r0		/ old pc
.if _pshift
	ash	$-_pshift,r0
.endif
	bic	$_pmask,r0
	mov	$014340,PS
	mfpd	(r0)		/ fetch first word - note reverse of a normal long
	inc	(sp)
	bne	2f
	mtpd	(r0)+
	mfpd	(r0)
	inc	(sp)
2:
	mtpd	(r0)
	rti
1:
	add	$1,_pusertix
	adc	_pusertix+2
	rti
.endif
