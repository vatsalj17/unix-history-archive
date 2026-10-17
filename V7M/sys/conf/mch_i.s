/ machine language assist
/ for non-separate I & D space CPU's
/ 11/23, 11/24, 11/34, 11/40, & 11/60 CPUs
/ Fred Canter 8/2/81
/
/ Added a "bic" instruction after the symbol "start:".
/ This is required in order to allow the I space only
/ versions of unix v7 to be booted via the standalone
/ boot code, which enters unix with memory management
/ enabled after loading unix into memory.
/ This change allows the I space only unix monitors
/ to be booted via the standalone boot or
/ directly via the block zero boot.
/
/ The TS11 vector is autoconfigured, see comments
/ in code below, just prior to calling main().
/
/ The concept of cputype is now defunct,
/ instead the cpu is described by the hardware
/ features present, i.e., unibus map, display register,
/ rh70 controller, and separate I & D space.
/
/ Enable memory parity error traps thru 114

/ Parameter for code size reduction,
/ .fpp in mch0.s header file.


/ non-UNIX instructions
mfpi	= 6500^tst
mtpi	= 6600^tst
ldfps	= 170100^tst
svfps	= 170200^tst
stst	= 170300^tst
halt	= 0
wait	= 1
reset	= 5
rtt	= 6

/ Mag tape dump
/ save registers in locore and
/ write all core onto mag tape.
/ entry thru 1000 abs

/ ***************************************
/ *					*
/ *	The core dump code must be	*
/ *	the first code in this file.	*
/ *					*
/ ***************************************

.globl	start, _end, _edata, _main, dump
dump:
	bit	$1,SSR0
	bne	dump

/ save regs r0,r1,r2,r3,r4,r5,r6,KIA6
/ starting at abs location 4
	inc	$-1	/ save reg's on first core
	bne	1f	/ dump attempt only.

	mov	r0,4
	mov	$6,r0
	mov	r1,(r0)+
	mov	r2,(r0)+
	mov	r3,(r0)+
	mov	r4,(r0)+
	mov	r5,(r0)+
	mov	sp,(r0)+
	mov	KISA6,(r0)+

1:

/ dump all of core (ie to first mt error)
/ onto mag tape. (9 track or 7 track 'binary')

/ The core dump tape hardware addresses are
/ defined in the mch0.s header file.

.if HTDUMP

	/register usage is as follows

	/reg 0 holds the CSR address for the tm02/3.
	/reg 1 points to UBMAP register 0 low
	/reg 2 is used to contain and calculate memory pointer
	/ for UBMAP register 0 low
	/reg 3 is used to contain and calculate memory pointer
	/ for UBMAP register 0 high
	/reg 4, r4 = 1 for map used, r4 = 0 for map not used.
	/reg 5 is used as an interation counter when mapping is enabled


	clr	r4		/clear map used indicator
/	cmp	_cputype,$44.	/is a 44 ?
/	bne	2f		/no, UBMAP not used
	tst	_ubmaps		/unibus map present ?
	beq	2f		/no
	tst	_rh70ht		/yes, rh70 controller ?
	bne	2f		/yes, don't need to use unibus map
				/no, will use map

	/this section of code initializes the Unibus map registers
	/and the memory management registers.
	/UBMAP reg 0 gets updated to point to the current
	/memory area.
	/Kernal I space 0 points to low memory
	/Kernal I space 7 points to the I/O page.

	inc	r4		/indicate that UB mapping is needed
	mov	$UBMR0,r1	/point to  map register 0
	clr	r2		/init for low map reg
	clr	r3		/init for high map reg
	mov	$77406,*$KISD0	/set KISDR0
	mov	$77406,*$KISD7	/set KISDR7
	clr	*$KISA0		/point KISAR0 to low memory
	mov	$IO,*$KISA7	/point KISAR7 to IO page
	inc	*$SSR0		/turn on memory mngt
	mov	$60,*$SSR3	/enable 22 bit mapping
	mov	r2,(r1)		/load map reg 0 low
	mov	r3,2(r1)	/load map reg 0 high
2:
	/this section of code initializes the TM02/3

	mov	$HTCS1,r0	/get tm02/3 CSR addr
	mov	$40,10(r0)	/tm02/3 subsystem clear
	mov	$1300,32(r0)	/800 BPI + pdp11 mode
	clr	4(r0)		/clear unibus address
	mov	$1,(r0)		/nop command to tm02/3
	mov	$20,r5		/set up SOB counter for UBMAP

	/This section does the write.
	/ if mapping is needed the sob loop comes in play here
	/ when the sob falls through the UBAMP reg will be
	/ updated by 20000 to point to next loop section.

	/ if mapping not needed then just let the
	/ hardware address registers increment.

3:
	mov	$-512.,6(r0)	/set frame count
	mov	$-256.,2(r0)	/set word count
	movb	$61,(r0)	/set write comand + go
				/set ext. mem. bits to 0
1:
	tstb	(r0)		/wait for tm02/3 ready
	bge	1b
	bit	$1,(r0)		/wait for go bit clear
	bne	1b
	bit	$40000,(r0)	/any error ?
	beq	2f		/no, continue xfer
	bit	$4000,10(r0)	/yes, must be MXM error
	beq	.		/hang here if not NXM
	mov	$27,(r0)	/error is NXM, write EOF
	halt			/halt on good dump !
2:
	tst	r4		/mapping?
	beq	3b		/branch if not
	sob	r5,3b		/yes continue loop
	mov	$20,r5		/reset loop count
	add	$20000,r2	/bump low map
	adc	r3		/carry to high map
	mov	r2,(r1)		/load map reg 0 low
	mov	r3,2(r1)	/load map reg 0 high
	clr	4(r0)		/set bus addr to 0
	br	3b		/do some more
.endif

.if TUDUMP
	/register useage is as follows

	/reg 0 holds the tm11 CSR address
	/reg 1 points to UBMAP register 0 low
	/reg 2 is used to contain and calculate memory pointer
	/ for UBMAP register 0 low
	/reg 3 is used to contain and calculate memory pointer
	/ for UBMAP register 0 high
	/reg 4, r4 = 1 for map used, r4 = 0 for map not used.
	/reg 5 is used as an interation counter when mapping is enabled


	clr	r4		/clear UB map used indicator
/	cmp	_cputype,$44.	/is a 44 ?
/	beq	1f		/yes, skip next
/	cmp	_cputype,$70.	/is a 70 ?
/	bne	2f		/not a 70 either. no UBMAP
/1:
	tst	_ubmaps		/unibus map present ?
	beq	2f		/no, skip map init
	/this section of code initializes the Unibus map registers
	/and the memory management registers.
	/UBMAP reg 0 gets updated to point to the current
	/memory area.
	/Kernal I space 0 points to low memory
	/Kernal I space 7 points to the I/O page.

	inc	r4		/indicate that UB mapping is needed
	mov	$UBMR0,r1	/point to  map register 0
	clr	r2		/init for low map reg
	clr	r3		/init for high map reg
	mov	$77406,*$KISD0	/set KISDR0
	mov	$77406,*$KISD7	/set KISDR7
	clr	*$KISA0		/point KISAR0 to low memory
	mov	$IO,*$KISA7	/point KISAR7 to IO page
	inc	*$SSR0		/turn on memory mngt
	mov	$60,*$SSR3	/enable 22 bit mapping
	mov	r2,(r1)		/load map reg 1 low
	mov	r3,2(r1)	/load map reg 1 high
2:
	/this section of code initializes the TM11

	mov	$MTC,r0		/get tm11 CSR address
	mov	$60004,(r0)	/write command, no go
	clr	4(r0)		/set bus addr to 0
	mov	$20,r5		/set up SOB counter for UBMAP
	/This section does the write.
	/ if mapping is needed the sob loop comes in play here
	/ when the sob falls through the UBAMP reg will be
	/ updated by 20000 to point to next loop section.

	/ if mapping not needed then just let
	/ bus address register increment.

3:
	mov	$-512.,2(r0)	/set byte count
	inc	(r0)		/start xfer
1:
	tstb	(r0)		/wait for tm11 ready
	bge	1b
	tst	(r0)		/any error ?
	bge	2f		/no, continue xfer
	bit	$200,-2(r0)	/yes, must be NXM error
	beq	.		/hang if not NXM error
	reset			/error is NXM,
	mov	$60007,(r0)	/write EOF
	halt			/halt on good dump
2:
	tst	r4		/mapping?
	beq	3b		/branch if not
	sob	r5,3b		/yes continue loop
	mov	$20,r5		/reset loop count
	add	$20000,r2	/bump low map
	adc	r3		/carry to high map
	mov	r2,(r1)		/load map reg 0 low
	mov	r3,2(r1)	/load map reg 0 high
	clr	4(r0)		/set bus address to 0
	br	3b		/do some more
.endif

.if TSDUMP

	/register useage is as follows

	/reg 0 points to UBMAP register 1 low
	/reg 1 is used to calculate the current memory address
	/ for each 512 byte transfer.
	/reg 2 is used to contain and calculate memory pointer
	/ for UBMAP register 1 low
	/reg 3 is used to contain and calculate memory pointer
	/ for UBMAP register 1 high
	/reg 4 points to the command packet
	/reg 5 is used as an interation counter when mapping is enabled


/	cmp	_cputype,$44.	/is a 44 ?
/	beq	1f		/yes, skip next
/	cmp	_cputype,$70.	/is a 70 ?
/	bne	2f		/not a 70 either. no UBMAP
/1:
	tst	_ubmaps		/unibus map present ?
	beq	2f		/no, skip map init
	/this section of code initializes the Unibus map registers
	/and the memory management registers.
	/UBMAP reg 0 points to low memory for the TS11 command,
	/characteristics, and message buffers.
	/UBMAP reg 1 gets updated to point to the current
	/memory area.
	/Kernal I space 0 points to low memory
	/Kernal I space 7 points to the I/O page.

	inc	setmap		/indicate that UB mapping is needed
	mov	$UBMR0,r0	/point to  map register 0
	clr	r2		/init for low map reg
	clr	r3		/init for high map reg
	clr	(r0)+		/load map reg 0 low
	clr	(r0)+		/load map reg 0 high
	mov	$77406,*$KISD0	/set KISDR0
	mov	$77406,*$KISD7	/set KISDR7
	clr	*$KISA0		/point KISAR0 to low memory
	mov	$IO,*$KISA7	/point KISAR7 to IO page
	inc	*$SSR0		/turn on memory mngt
	mov	$60,*$SSR3	/enable 22 bit mapping
	mov	r2,(r0)		/load map reg 1 low
	mov	r3,2(r0)	/load map reg 1 high
2:
	/this section of code initializes the TS11

	tstb	*$TSSR		/make sure
	bpl	2b		/drive is ready
	mov	$comts,r4	/point to command packet
	add	$2,r4		/set up mod 4
	bic	$3,r4		/alignment
	mov	$140004,(r4)	/write characteristics command
	mov	$chrts,2(r4)	/characteristics buffer
	clr	4(r4)		/clear ext mem addr (packet)
	clr	tsxma		/clear extended memory save loc
	mov	$10,6(r4)	/set byte count for command
	mov	$mests,*$chrts	/show where message buffer is
	clr	*$chrts+2	/clear extended memory bits here too
	mov	$16,*$chrts+4	/set message buffer length
	mov	r4,*$TSDB	/start command
	mov	$20,r5		/set up SOB counter for UBMAP
	clr	r1		/init r1 beginning memory address
1:
	tstb	*$TSSR		/wait for ready
	bpl	1b		/not yet
	mov	*$TSSR,tstcc	/error condition (SC) ?
	bpl	2f		/no error
	bic	$!16,tstcc	/yes error, get TCC
	cmp	tstcc,$10	/recoverable error ?
	bne	8f		/no
	mov	$101005,(r4)	/yes, load write data retry command
	clr	4(r4)		/clear packet ext mem addr
	mov	r4,*$TSDB	/start retry
	br	1b
8:
	bit	$4000,*$TSSR	/is error NXM ?
	beq	.		/no, hang (not sure of good dump)
	mov	$140013,(r4)	/load a TS init command
	mov	r4,*$TSDB	/to clear NXM error
6:
	tstb	*$TSSR		/wait for ready
	bpl	6b
	mov	$1,6(r4)	/set word count = 1
	mov	$100011,(r4)	/load write EOF command
	mov	r4,*$TSDB	/do write EOF
7:
	tstb	*$TSSR		/wait for ready
	bpl	7b
	halt			/halt after good dump
9:
	br	1b
2:
	/If mapping is needed this section calculates the
	/ base address to be loaded into map reg 1
	/ the algorithm is (!(r5 - 21))*1000) | 20000
	/ the complement is required because an SOB loop
	/ is being used for the counter
	/This loop causes 20000 bytes to be written
	/before the UBMAP is updated.

	tst	setmap		/UBMAP ?
	beq	3f		/no map
	mov	r2,(r0)		/load map reg 1 low
	mov	r3,2(r0)	/load map reg 1 high
	mov	r5,r1		/calculate
	sub	$21,r1		/address for this pass
	com	r1		/based on current
	mul	$1000,r1	/interation
	bis	$20000,r1	/select map register 1
	clr	4(r4)		/clear extended memory bits
3:
	/This section does the write.
	/ if mapping is needed the sob loop comes in play here
	/ when the sob falls through the UBAMP reg will be
	/ updated by 20000 to point to next loop section.

	/ if mapping not needed then just calculate the
	/ next 512 byte address pointer

	mov	r1,2(r4)	/load mem address
	mov	tsxma,4(r4)	/load ext mem address
	mov	$512.,6(r4)	/set byte count
	mov	$100005,(r4)	/set write command
	mov	r4,*$TSDB	/initiate xfer
	tst	setmap		/mapping?
	beq	4f		/branch if not
	sob	r5,9b		/yes continue loop
	mov	$20,r5		/reset loop count
	add	$20000,r2	/bump low map
	adc	r3		/carry to high map
	br	1b		/do some more
4:
	add	$512.,r1	/bump address for no mapping
	adc	tsxma		/carry to extended memory bits
	br	1b		/do again
.endif
1:
	halt			/ halt if no core dump code
	br	1b		/ included in this monitor

start:
	bic	$1,SSR0		/ Insure memory management if off !
	mov	$trap,34

/ Set location 0 and 2 to catch traps and jumps to 0
/ ,same for 40 and 42.

	mov	$42,0		/ illegal instruction if jump
	mov	$777,2		/ trace trap at high priority if trap
	mov	$42,40
	mov	$777,42

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

.if	.fpp
/ test for floating point

	mov	$1f,nofault
	setd				/ jumps to 1f if no fpu
	inc	fpp
1:
.endif
	mov	$1f,nofault
	tst	*$UBMR0		/ unibus map present ?
	inc	_ubmaps		/ yes, remember it
	mov	$20,*$SSR3	/ also set 22 bit mapping
1:
	clr	nofault

/ clear user block

	mov	$_u,r0
1:
	clr	(r0)+
	cmp	r0,$_u+[usize*64.]
	blo	1b

/ Set up the TS11 interrupt vector
/
/ (mkconf) sets the TS11 vector to 050.
/ The following code copies the vector
/ from 050 to 0224 if the TS11 is the only
/ tape on the system or to 0260 if there
/ is also a TM11 on the system.
/ If the vector at 260 is used, the code halts.
/ This allows the TS11 and TM11 to coexist
/ on the same system, see the comments in the
/ TS11 driver "/sys/dev/ts.c" for more on this
/ strangeness.

	mov	$50,r0		/ pointer to TS11 vector
	mov	$224,r1		/ pointer to first choice
	tst	2(r0)		/ is TS11 configured ?
	beq	1f		/ no, don't copy vector
	tst	*$226		/ yes, is vector at 224 used (TM11) ?
	beq	2f		/ no, copy vector to 0224
	tst	*$262		/ yes, is vector at 260 used ?
	beq	3f		/ no, copy vector to 0260
	halt			/ yes, something is very wrong - HALT !
3:
	mov	$260,r1		/ pointer to second choice
2:
	mov	(r0)+,(r1)+	/ copy vector address
	mov	(r0)+,(r1)+	/ copy new psw
	clr	-(r0)		/ clear loc 052
	clr	-(r0)		/ clear loc 050
1:
/ load all unused vector locations with
/ the same vector/jump catchers that were
/ loaded into locations 0 & 2 at start.

	mov	$40,r0
2:
	tst	(r0)+
	tst	(r0)+
3:
	tst	2(r0)
	bne	2b
	mov	$42,(r0)+
	mov	$777,(r0)+
	cmp	r0,$1000
	blt	3b

/ set up previous mode and call main
/ on return, enter user mode at 0R

	mov	$30000,PS
	jsr	pc,_main
	mov	$170000,-(sp)
	clr	-(sp)
	rtt

.globl	trap, call
.globl	_trap

/ all traps and interrupts are
/ vectored thru this routine.

trap:
	mov	PS,saveps
	tst	nofault
	bne	1f
	mov	SSR0,ssr
	mov	SSR2,ssr+4
	mov	$1,SSR0
	jsr	r0,call1; jmp _trap
	/ no return
1:
	mov	$1,SSR0
	mov	nofault,(sp)
	rtt
.text

.globl	_runrun
call1:
	mov	saveps,-(sp)
	bic	$HIPRI,PS
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
	jsr	pc,(r0)+
	tstb	_runrun
	beq	2f
	mov	$12.,(sp)		/ trap 12 is give up cpu
	jsr	pc,_trap
2:
	tst	(sp)+
	mtpi	sp
	br	2f
1:
	bis	$30000,PS
	jsr	pc,(r0)+
	cmp	(sp)+,(sp)+
2:
	mov	(sp)+,r1
	tst	(sp)+
	mov	(sp)+,r0
	rtt
.globl	_savfp
_savfp:
.if	.fpp
	tst	fpp
	beq	8f
	mov	2(sp),r1
	stfps	(r1)+
	setd
	movf	fr0,(r1)+
	movf	fr1,(r1)+
	movf	fr2,(r1)+
	movf	fr3,(r1)+
	movf	fr4,fr0
	movf	fr0,(r1)+
	movf	fr5,fr0
	movf	fr0,(r1)+
8:
.endif
	rts	pc

.globl	_restfp
_restfp:
.if	.fpp
	tst	fpp
	beq	8f
	mov	2(sp),r1
	mov	r1,r0
	setd
	add	$8.+2.,r1
	movf	(r1)+,fr1
	movf	(r1)+,fr2
	movf	(r1)+,fr3
	movf	(r1)+,fr0
	movf	fr0,fr4
	movf	(r1)+,fr0
	movf	fr0,fr5
	movf	2(r0),fr0
	ldfps	(r0)
8:
.endif
	rts	pc

/ save floating point error registers
/ argument is a pointer to a two-word
/ structure

.globl	_stst
_stst:
.if	.fpp
	tst	fpp
	beq	9f
	stst	*2(sp)
9:
.endif
	rts	pc

.globl	_addupc
_addupc:
	mov	r2,-(sp)
	mov	6(sp),r2		/ base of prof with base,leng,off,scale
	mov	4(sp),r0		/ pc
	sub	4(r2),r0		/ offset
	clc
	ror	r0
	mov	6(r2),r1
	clc
	ror	r1
	mul	r1,r0		/ scale
	ashc	$-14.,r0
	inc	r1
	bic	$1,r1
	cmp	r1,2(r2)		/ length
	bhis	1f
	add	(r2),r1		/ base
	mov	nofault,-(sp)
	mov	$2f,nofault
	mfpi	(r1)
	add	12.(sp),(sp)
	mtpi	(r1)
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
	rts	pc

/ Character list get/put

/.globl	_getc, _putc
/.globl	_cfreelist
/
/_getc:
/	mov	2(sp),r1
/	mov	PS,-(sp)
/	mov	r2,-(sp)
/	bis	$HIPRI,PS
/	bic	$40,PS
/	mov	2(r1),r2		/ first ptr
/	beq	9f		/ empty
/	movb	(r2)+,r0		/ character
/	bic	$!377,r0
/	mov	r2,2(r1)
/	dec	(r1)+		/ count
/	bne	1f
/	clr	(r1)+
/	clr	(r1)+		/ last block
/	br	2f
/1:
/	bit	$7,r2
/	bne	3f
/	mov	-10(r2),(r1)		/ next block
/	add	$2,(r1)
/2:
/	dec	r2
/	bic	$7,r2
/	mov	_cfreelist,(r2)
/	mov	r2,_cfreelist
/3:
/	mov	(sp)+,r2
/	mov	(sp)+,PS
/	rts	pc
/9:
/	clr	4(r1)
/	mov	$-1,r0
/	mov	(sp)+,r2
/	mov	(sp)+,PS
/	rts	pc
/
/_putc:
/	mov	2(sp),r0
/	mov	4(sp),r1
/	mov	PS,-(sp)
/	mov	r2,-(sp)
/	mov	r3,-(sp)
/	bis	$HIPRI,PS
/	bic	$40,PS
/	mov	4(r1),r2		/ last ptr
/	bne	1f
/	mov	_cfreelist,r2
/	beq	9f
/	mov	(r2),_cfreelist
/	clr	(r2)+
/	mov	r2,2(r1)		/ first ptr
/	br	2f
/1:
/	bit	$7,r2
/	bne	2f
/	mov	_cfreelist,r3
/	beq	9f
/	mov	(r3),_cfreelist
/	mov	r3,-10(r2)
/	mov	r3,r2
/	clr	(r2)+
/2:
/	movb	r0,(r2)+
/	mov	r2,4(r1)
/	inc	(r1)		/ count
/	clr	r0
/	mov	(sp)+,r3
/	mov	(sp)+,r2
/	mov	(sp)+,PS
/	rts	pc
/9:
/	mov	pc,r0
/	mov	(sp)+,r3
/	mov	(sp)+,r2
/	mov	(sp)+,PS
/	rts	pc

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
f5:	/ movei, movfi
ff1:	/ ldfps
ff2:	/ stfps
ff3:	/ stst
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
/ too bad.

	mov	r2,-(sp)
	bic	$174370,(sp)
	cmpb	1(sp),(sp)+
	beq	u7

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

t17:	/ floating point instructions
	clrb	bflg
	mov	r1,r0
	swab	r0
	bic	$!16,r0
	jmp	*0f(r0)
0:		f0; f1; f2; f3; f4; f5; f6; f7

f0:
	mov	r1,r0
	ash	$-5,r0
	bic	$!16,r0
	jmp	*0f(r0)
0:		ff0; ff1; ff2; ff3; ff4; ff5; ff6; ff7

f1:	/ mulf, modf
f2:	/ addf, movf
f3:	/ subf, cmpf
f4:	/ movf, divf
ff4:	/ clrf
ff5:	/ tstf
ff6:	/ absf
ff7:	/ negf
	inc	fflg
	mov	r1,r0
	br	setreg

f6:
	bit	$400,r1
	beq	f1	/ movfo
	br	f5	/ movie

f7:
	bit	$400,r1
	beq	f5	/ movif
	br	f1	/ movof

ff0:	/ cfcc, setf, setd, seti, setl
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
	tstb	fflg
	beq	3f
	asl	r0
	stfps	r1
	bit	$200,r1
	beq	3f
	asl	r0
3:
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
fflg:	.=.+1
.text

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
	bis	$HIPRI,PS
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
	bis	$HIPRI,PS
	mov	nofault,-(sp)
	mov	$err,nofault
	mov	r0,-(sp)
	mtpi	(r1)
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
	rts	pc
.globl	_copyin, _copyout
.globl	_copyiin, _copyiout
_copyiin:
_copyin:
	jsr	pc,copsu
1:
	mfpi	(r0)+
	mov	(sp)+,(r1)+
	sob	r2,1b
	br	2f

_copyiout:
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

.globl	_idle, _waitloc
_idle:
	mov	PS,-(sp)
	bic	$HIPRI,PS
	wait
waitloc:
	mov	(sp)+,PS
	rts	pc
	.data
_waitloc:
	waitloc
	.text

.globl	_save
_save:
	mov	(sp)+,r1
	mov	(sp),r0
	mov	r2,(r0)+
	mov	r3,(r0)+
	mov	r4,(r0)+
	mov	r5,(r0)+
	mov	sp,(r0)+
	mov	r1,(r0)+
	clr	r0
	jmp	(r1)

.globl	_resume
_resume:
	mov	2(sp),r0		/ new process
	mov	4(sp),r1		/ new stack
	bis	$HIPRI,PS
	mov	r0,KISA6		/ In new process
	mov	(r1)+,r2
	mov	(r1)+,r3
	mov	(r1)+,r4
	mov	(r1)+,r5
	mov	(r1)+,sp
	mov	$1,r0
	bic	$HIPRI,PS
	jmp	*(r1)+

.globl	_spl0, _spl1, _spl4, _spl5, _spl6, _spl7, _splx
_spl0:
	mov	PS,r0
	bic	$HIPRI,PS
	rts	pc

_spl1:
	mov	PS,r0
	bis	$HIPRI,PS
	bic	$300,PS
	rts	pc

_spl4:
	mov	PS,r0
	bis	$HIPRI,PS
	bic	$140,PS
	rts	pc

_spl5:
	mov	PS,r0
	bis	$HIPRI,PS
	bic	$100,PS
	rts	pc

_spl6:
	mov	PS,r0
	bis	$HIPRI,PS
	bic	$40,PS
	rts	pc

_spl7:
	mov	PS,r0
	bis	$HIPRI,PS
	rts	pc

_splx:
	mov	2(sp),PS
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

/ Long quotient

	.globl	ldiv
ldiv:
	jsr	r5,csv
	mov	10.(r5),r3
	sxt	r4
	bpl	1f
	neg	r3
1:
	cmp	r4,8.(r5)
	bne	hardldiv
	mov	6.(r5),r2
	mov	4.(r5),r1
	bge	1f
	neg	r1
	neg	r2
	sbc	r1
	com	r4
1:
	mov	r4,-(sp)
	clr	r0
	div	r3,r0
	mov	r0,r4		/high quotient
	mov	r1,r0
	mov	r2,r1
	mov	r0,-(sp)	/ *
	div	r3,r0
	bvc	1f
	mov	r2,r1		/ *
	mov	(sp),r0		/ *
	sub	r3,r0		/ this is the clever part
	div	r3,r0
	tst	r1
	sxt	r1
	add	r1,r0		/ cannot overflow!
1:
	tst	(sp)+		/ *
	mov	r0,r1
	mov	r4,r0
	tst	(sp)+
	bpl	9f
	neg	r0
	neg	r1
	sbc	r0
9:
	jmp	cret

hardldiv:
	4

/ Long remainder

	.globl	lrem
lrem:
	jsr	r5,csv
	mov	10.(r5),r3
	sxt	r4
	bpl	1f
	neg	r3
1:
	cmp	r4,8.(r5)
	bne	hardlrem
	mov	6.(r5),r2
	mov	4.(r5),r1
	mov	r1,r4
	bge	1f
	neg	r1
	neg	r2
	sbc	r1
1:
	clr	r0
	div	r3,r0
	mov	r1,r0
	mov	r2,r1
	mov	r0,-(sp)	/ *
	div	r3,r0
	bvc	1f
	mov	r2,r1		/ *
	mov	(sp),r0		/ *
	sub	r3,r0
	div	r3,r0
	tst	r1
	beq	9f
	add	r3,r1
1:
	tst	(sp)+		/ *
	tst	r4
	bpl	9f
	neg	r1
9:
	sxt	r0
	jmp	cret

/ The divisor is known to be >= 2^15.	Only 16 cycles are
/ needed to get a remainder.
hardlrem:
	4

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
usize	= 16.

HIPRI	= 340

PS	= 177776
SSR0	= 177572
SSR2	= 177576
SSR3	= 172516
KISA0	= 172340
KISA6	= 172354
KISA7	= 172356
KISD0	= 172300
KISD7	= 172316
UISA0	= 177640
UISA1	= 177642
UISD0	= 177600
UISD1	= 177602
UBMR0	= 170200
IO	= 177600

.data
.globl	_ka6
.globl _sepid, _ubmaps, _cdreg, _nmser
.globl _rh70ml, _rh70hp, _rh70ht, _rh70hs, _rh70hm

_ka6:	KISA6

/ The following seven parameters describe the
/ hardware features available on this CPU.

_sepid:	0	/ seperate I & D space
		/ always zero in mch_i.s

_ubmaps: 0	/ unibus map
		/ set at startup time

_rh70hs: 0	/ hs - massbus controller type
_rh70ml: 0	/ ml - "	"	"
_rh70hp: 0	/ hp - "	"	"
_rh70hm: 0	/ hm - "	"	"
_rh70ht: 0	/ ht - "	"	"
		/ RH11 only in mch_i.s,
		/ but checked in main.c (binit()) anyway

_nmser: 0	/ number of memory system error registers
		/ always zero in mch_i.s

_cdreg: 0	/ console display register
		/ never in mch_i.s

stk:	0

.bss
nofault:.=.+2
ssr:	.=.+6
saveps: .=.+2
power:	.=.+2
.if	TSDUMP
tsxma:	.=.+2	/ts11 extended memory address bits
setmap:	.=.+2	/UB map usage indicator
tstcc:	.=.+2	/ts11 temp location for TCC
comts:	.=.+12	/ts11 command packet
chrts:	.=.+10	/ts11 characteristics
mests:	.=.+16	/ts11 messge buffer
.endif
.if	.fpp
fpp:	.=.+2
.endif
