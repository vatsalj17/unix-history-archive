/ machine language assist
/ for separate I & D space CPU's
/ 11/44, 11/45, & 11/70 CPUs
/
/ The following modifications have been made:
/
/ 1.	In order to allow a unibus disk (rm02/3, rl02, & rk06/7)
/	to be the root device on an 11/44 or 11/70, the
/	unibus map is no longer enabled in mch.s. Instead the map
/	is enabled in machdep.c, after it has been initialized.
/
/ 2.	The 11/44 CPU is supported.
/
/ 3.	Core dump code for the ts11 tape drive has been added.
/	The existing core dump code for the tm11 and tm02/3
/	tapes has been modified to use the unibus map (if required)
/	to dump all of memory.
/
/ 4.	The core dump code only saves the registers
/	on the first dump attempt. This prevents the register
/	contents from being lost if the initital core dump
/	attempt fails.
/
/ 5.	The core dump code halts after a good dump, i.e.,
/	the dump was terminated by a non existent memory
/	error (NXM). The code will hang on an unsuccessful
/	core dump, i.e., one terminated by any error
/	other than NXM.
/
/ 6.	The TS11 vector is autoconfigured, see comments
/	in code below, just prior to calling main().
/
/ 7.	The concept of cputype is now defunct,
/	instead the cpu is described by the hardware features
/	present, i.e., unibus map, display register,
/	rh70 controller, and separate I & D space.
/
/ 8.	Memory parity error traps thru 114 are now
/	enabled, they were not previously (except 11/70).
/
/ Fred Canter 8/2/81

/ Parameter for code size reduction,
/ .fpp in mch0.s header file.

/ non-UNIX instructions
mfpi	= 6500^tst
mtpi	= 6600^tst
mfpd	= 106500^tst
mtpd	= 106600^tst
spl	= 230
ldfps	= 170100^tst
stfps	= 170200^tst
stst	= 170300^tst
wait	= 1
rtt	= 6
reset	= 5
halt	= 0

.PROFIL	= 0
HIPRI	= 340
HIGH	= 7
	.if	.PROFIL
HIGH	= 6
HIPRI	= 300
	.endif

/ Mag tape core dump
/ save registers in low core and
/ write all core onto mag tape.
/ entry is thru 1000 abs

/ ***********************************************
/ *						*
/ *	The core dump code must be the first	*
/ *	code in this file and must be in	*
/ *	date space.				*
/ *						*
/ ***********************************************

.data
.globl	dump
dump:

/ save regs r0,r1,r2,r3,r4,r5,r6,KDA6
/ starting at abs location 4

	inc	$-1	/ save reg's on first dump
	bne	1f	/ attempt only.
	mov	r0,4
	mov	$6,r0
	mov	r1,(r0)+
	mov	r2,(r0)+
	mov	r3,(r0)+
	mov	r4,(r0)+
	mov	r5,(r0)+
	mov	sp,(r0)+
	mov	KDSA6,(r0)+
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
	bne	1b		/ included in this monitor
.text
.globl	start, _end, _edata, _etext, _main

/ startup code for separate I & D space CPU's
/ entry is via a trap thru 34
/ memory management must be enabled

start:
	bit	$1,SSR0
	beq	.
	mov	$trap,34
	mov	$trap,40
	mov	$trap,0
	mov	$340+15.,2
	mov	$340+15.,42
	bit	$20,SSR3
	beq	9f
/	mov	$70.,_cputype
	mov	$4,_nmser	/ 4 memory system error reg's
	mov	$3,*$MSCR
	bit	$2,*$MSCR	/*** if bit 1 didn't set
	bne	9f		/*** then its an 11/44.
/	mov	$44.,_cputype	/*** yea !,its an 11/44.
	clr	*$MSCR		/*** clear the mem cont. reg.
	clr	_cdreg		/ no display register
	mov	$2,_nmser	/ 2 memory system error reg's
9:
	clr	PS
	br	9f
/ 	inc	$-1
/ 	bne	.
/ 	reset
/ / Set loc. 0 to trap to system, in case of
/ / hardware glitch
/ 	mov	$trap,0		/ in case of bad trap through 0
/ 	mov	$340+15.,2	/ high pri, trap type 15
/ 	clr	PS
/ 
/ / set KI0 to physical 0
/ 
/ 	mov	$77406,r3
/ 	mov	$KISA0,r0
/ 	mov	$KISD0,r1
/ 	clr	(r0)+
/ 	mov	r3,(r1)+
/ 
/ / set KI1-6 to eventual text resting place
/ 
/ 	mov	$_end+63.,r2
/ 	ash	$-6,r2
/ 	bic	$!1777,r2
/ 1:
/ 	mov	r2,(r0)+
/ 	mov	r3,(r1)+
/ 	add	$200,r2
/ 	cmp	r0,$KISA7
/ 	blos	1b
/ 
/ / set KI7 to IO seg for escape
/ 
/ 	mov	$IO,-(r0)
/ 
/ / set KD0-7 to physical
/ 
/ 	mov	$KDSA0,r0
/ 	mov	$KDSD0,r1
/ 	clr	r2
/ 1:
/ 	mov	r2,(r0)+
/ 	mov	r3,(r1)+
/ 	add	$200,r2
/ 	cmp	r0,$KDSA7
/ 	blos	1b
/ 
/ / initialization
/ / get a temp (1-word) stack
/ / turn on segmentation
/ / copy text to I space
/ / clear bss in D space
/ 
/ 	mov	$stk+2,sp
/ /*** turn map on in machdep.c not here.
/ 	mov	$25,SSR3		/ 22-bit, K+U sep
/ 	bit	$20,SSR3
/ 	beq	1f
/ 	mov	$70.,_cputype
/ 	mov	$3,*$MSCR		/ Disable UNIBUS traps, non-fatal traps
/ 	bit	$2,*$MSCR	/*** if bit 1 didn't set
/ 	bne	1f		/*** then its an 11/44.
/ 	mov	$44.,_cputype	/*** yea !,its an 11/44.
/ 	clr	*$MSCR		/*** clear the mem cont. reg.
/ 1:
/ 	inc	SSR0
/ 	mov	$_etext+100,r2
/ 	mov	$_edata+100,r1
/ 	add	$_etext-8192.,r1
/ 1:
/ 	mov	-(r1),-(sp)
/ 	mtpi	-(r2)
/ 	cmp	r1,$_edata
/ 	bhi	1b
/ 1:
/ 	clr	(r1)+
/ 	cmp	r1,$_end
/ 	blo	1b
/ 
/ / use KI escape to set KD7 to IO seg
/ / set KD6 to first available core
/ / If profiling, snag supervisor registers.
/ 
/ 	mov	$IO,-(sp)
/ 	mtpi	*$KDSA7
9:
	mov	$_etext-8192.+63.,r2
	ash	$-6,r2
	bic	$!1777,r2
	add	KISA1,r2
	.if	.PROFIL
	mov	r2,SISA2
	mov	r2,_proloc
	mov	$77406,SISD2
	add	$200,r2
	mov	r2,SISA2+2
	mov	$77406,SISD2+2
	add	$200,r2
	.endif
	mov	r2,KDSA6

/ Turn off write permission on kernel text
/ Take stuff above data out of address space

	mov	$KISD0,r0
1:
	mov	$77402,(r0)+
	cmp	r0,$KISD7
	blos	1b

	mov	$_end+63.,r0
	ash	$-6,r0
	bic	$!1777,r0
	mov	$KDSD0,r1
1:
	cmp	r0,$200
	bge	2f
	dec	r0
	bge	4f
	clr	(r1)
	br	3f
4:
	movb	r0,1(r1)
	br	3f
2:
	movb	$177,1(r1)
3:
	tst	(r1)+
	sub	$200,r0
	cmp	r1,$KDSD5
	blos	1b

/ set up supervisor D registers

9:
	mov	$6,SISD0
	mov	$6,SISD1

/ set up real sp
/ clear user block
/ test for floating point hardware

	mov	$_u+[usize*64.],sp
.if	.fpp
	mov	$1f,nofault
	setd			/ jump to 1f if this traps
	inc	fpp
1:
.endif
	mov	$1f,nofault
	tst	UBMR0		/ unibus map present ?
	inc	_ubmaps		/ yes, remember it
1:
	clr	nofault		/ no unibus map
	mov	$_u,r0
1:
	clr	(r0)+
	cmp	r0,sp
	blo	1b
	.if	.PROFIL
	mov	$40000,r0
	mov	$10000,PS	/ prev = super
1:
	clr	-(sp)
	mtpi	(r0)+
	cmp	r0,$100000
	blo	1b
	jsr	pc,_isprof
	.endif

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
/ load all usused vector locations with the
/ address of the trap handler and
/ trap type 15 to catch stray vectors.

	mov	$40,r0
2:
	tst	(r0)+
	tst	(r0)+
3:
	tst	2(r0)
	bne	2b
	mov	$trap,(r0)+
	mov	$340+15.,(r0)+
	cmp	r0,$1000
	blt	3b

/ set up previous mode and call main
/ on return, enter user mode at 0R

	mov	$30000,PS
	jsr	pc,_main
	mov	$170000,-(sp)
	clr	-(sp)
	rtt

.globl	_rkboot, _rpboot
_rkboot:
	jmp	*$173000

_rpboot:
	jmp	*$173006


.globl	trap, call
.globl	_trap

/ all traps and interrupts are
/ vectored thru this routine.

trap:
	mov	PS,saveps
	tst	_cdreg		/ do following on 11/45 or 11/70 only !
	beq	1f		/ 11/44 has no PIRQ or display register
	clrb	PIRQ+1		/ clear the programmed interrupt request
				/ register, or else the system will hang
				/ in a for ever loop of PIRQ's if the trap
				/ was a PIRQ (trap type 7).
				/ UNIX does not use the PIRQ register.
1:
	tst	nofault
	bne	1f
	mov	SSR0,ssr
	mov	SSR1,ssr+2
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
	jsr	pc,(r0)+
	tstb	_runrun
	beq	2f
	mov	$12.,(sp)		/ trap 12 is give up cpu
	jsr	pc,_trap
2:
	tst	(sp)+
	mtpd	sp
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
	beq	9f		/ No FP hardware
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
9:
.endif
	rts	pc

.globl	_restfp
_restfp:
.if	.fpp
	tst	fpp
	beq	9f
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
9:
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
	mov	6(sp),r2	/ base of prof with base,leng,off,scale
	mov	4(sp),r0	/ pc
	sub	4(r2),r0	/ offset
	clc
	ror	r0
	mov	6(r2),r1
	clc
	ror	r1
	mul	r1,r0		/ scale
	ashc	$-14.,r0
	inc	r1
	bic	$1,r1
	cmp	r1,2(r2)	/ length
	bhis	1f
	add	(r2),r1		/ base
	mov	nofault,-(sp)
	mov	$2f,nofault
	mfpd	(r1)
	add	12.(sp),(sp)
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
/ 	cmp	$44.,_cputype	/*** 11/44 has no display reg.
	tst	_cdreg		/ display register present ?
	beq	2f		/ no
	dec	dispdly		/ yes
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
	mov	$120.,dispdly		/ 2 sec delay after CSW fault
2:
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
	spl	7
	mov	r0,KDSA6		/ In new process
	mov	(r1)+,r2
	mov	(r1)+,r3
	mov	(r1)+,r4
	mov	(r1)+,r5
	mov	(r1)+,sp
	mov	$1,r0
	spl	0
	jmp	*(r1)+

.globl	_spl0, _spl1, _spl4, _spl5, _spl6, _spl7, _splx
_spl0:
	mov	PS,r0
	spl	0
	rts	pc

_spl1:
	mov	PS,r0
	spl	1
	rts	pc

_spl4:
	mov	PS,r0
	spl	4
	rts	pc

_spl5:
	mov	PS,r0
	spl	5
	rts	pc

_spl6:
	mov	PS,r0
	spl	6
	rts	pc

_spl7:
	mov	PS,r0
	spl	HIGH
	rts	pc

_splx:
	mov	2(sp),PS
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

/ The divisor is known to be >= 2^15.  Only 16 cycles are
/ needed to get a remainder.
hardlrem:
	4

/.globl	lmul
/lmul:
/	mov	r2,-(sp)
/	mov	r3,-(sp)
/	mov	8(sp),r2
/	sxt	r1
/	sub	6(sp),r1
/	mov	12.(sp),r0
/	sxt	r3
/	sub	10.(sp),r3
/	mul	r0,r1
/	mul	r2,r3
/	add	r1,r3
/	mul	r2,r0
/	sub	r3,r0
/	mov	(sp)+,r3
/	mov	(sp)+,r2
/	rts	pc

.globl	csv
csv:
	mov	r5,r0
	mov	sp,r5
	mov	r4,-(sp)
	mov	r3,-(sp)
	mov	r2,-(sp)
	jsr	pc,(r0)

.globl	cret
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

CSW	= 177570
PS	= 177776
SSR0	= 177572
SSR1	= 177574
SSR2	= 177576
SSR3	= 172516
PCSR	= 172100
KISA0	= 172340
KISA1	= 172342
KISA7	= 172356
KISD0	= 172300
KISD7	= 172316
KDSA0	= 172360
KDSA6	= 172374
KDSA7	= 172376
KDSD0	= 172320
KDSD5	= 172332
SISA0	= 172240
SISA1	= 172242
SISA2	= 172244
SISD0	= 172200
SISD1	= 172202
SISD2	= 172204
MSCR	= 177746	/ 11/70 memory control register
			/ 11/44 cache control register
UBMR0	= 170200	/ unibus map reg. base address
PIRQ	= 177772	/ programmed interrupt request register

IO	= 177600
SWR	= 177570
.data
.globl	_ka6
.globl _sepid, _ubmaps, _cdreg, _nmser
.globl _rh70hs, _rh70ml, _rh70hp, _rh70ht, _rh70hm

_ka6:	KDSA6

/ The following seven parameters describe the
/ hardware features available on this CPU.

_sepid:	1	/ seperate I & D space
		/ always one in mch_id.s

_ubmaps: 0	/ unibus map
		/ set at startup time

_rh70hs: 0	/ hs - massbus controller type
_rh70ml: 0	/ ml - "	"	"
_rh70hp: 0	/ hp - "	"	"
_rh70hm: 0	/ hm - "	"	"
_rh70ht: 0	/ ht - "	"	"
		/ checked in main.c (binit())

_nmser: 0	/ number of memory system error registers
		/ set up at startup time

_cdreg: 1	/ console display register
		/ set up at startup time
stk:	0

/ The following TS11 command and message buffers,
/ must be in initialized data space instead of
/ bss space. This allows them to be mapped by the
/ first M/M mapping register, which is the only one
/ used durring a core dump.

.if	TSDUMP
tsxma:	0	/ts11 extended memory address bits
setmap:	0	/UB map usage indicator
tstcc:	0	/ts11 temp location for TCC
comts:		/ts11 command packet
	0
	0
	0
	0
	0
chrts:		/ts11 characteristics
	0
	0
	0
	0
mests:		/ts11 message buffer
	0
	0
	0
	0
	0
	0
	0
.endif

.bss
nofault:.=.+2
.if	.fpp
fpp:	.=.+2
.endif
ssr:	.=.+6
dispdly:.=.+2
saveps:	.=.+2

.text
/ system profiler
/  Expects to have a KW11-P in addition to the line-frequency
/  clock, and it should be set to BR7.
/  Uses supervisor I space register 2&3 (40000-100000)
/  to maintain the profile.

	.if	.PROFIL
CCSB	= 172542
CCSR	= 172540

.globl	_sprof, _xprobuf, _probsiz, _mode
_probsiz = 37777

_isprof:
	mov	$1f,nofault
	mov	$_sprof,104	/ interrupt
	mov	$340,106	/ pri
	mov	$100.,CCSB	/ count set = 100
	mov	$113,CCSR	/ count down, 10kHz, repeat
1:
	clr	nofault
	rts	pc

_sprof:
	mov	r0,-(sp)
	mov	PS,r0
	ash	$-10.,r0
	bic	$!14,r0
	add	$1,_mode+2(r0)
	adc	_mode(r0)
	cmp	r0,$14		/ user
	beq	done
	mov	2(sp),r0	/ pc
	asr	r0
	asr	r0
	bic	$140001,r0
	cmp	r0,$_probsiz
	blo	1f
	inc	_outside
	br	done
1:
	mov	$10340,PS		/ prev = super
	mfpi	40000(r0)
	inc	(sp)
	mtpi	40000(r0)
	bne	done
	mov	r1,-(sp)
	mov	$_xprobuf,r1
2:
	cmp	(r1)+,r0
	bne	3f
	inc	(r1)
	br	4f
3:
	tst	(r1)+
	bne	2b
	sub	$4,r1
	mov	r0,(r1)+
	mov	$1,(r1)+
4:
	mov	(sp)+,r1
done:
	mov	(sp)+,r0
	mov	$113,CCSR
	rtt

/ count subroutine calls during profiling
/  of the system.

.globl	mcount, _profcnts, _profsize

.bss
_profcnts:
	.=.+[6*340.]

.globl countbase
.data
countbase:
	_profcnts
_profsize:
	340.
.text

mcount:
	mov	(r0),r1
	bne	1f
	mov	countbase,r1
	beq	2f
	add	$6,countbase
	cmp	countbase,$_profcnts+[6*340.]
	blo	3f
	clr	countbase
	rts	pc
3:
	mov	(sp),(r1)+
	mov	r1,(r0)
1:
	inc	2(r1)
	bne	2f
	inc	(r1)
2:
	rts	pc

.bss
_xprobuf:.=.+512.
_proloc:.=.+2
_mode:	.=.+16.
_outside: .=.+2

	.endif
