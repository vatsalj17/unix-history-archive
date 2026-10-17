/ Startup code for two-stage bootstrap

/ Modified for use with I space only CPUs
/ (11/23, 11/24, 11/34, 11/40, & 1160)
/ as well as I & D space CPUs
/ (11/44, 11/45, & 11/70).
/
/ Modified to allow unibus disks to be
/ the "root" device (rl01/2, rk06/7, & rm02/3).
/ This involves not enabling the unibus map
/ in the standalone bootstrap.
/ Instead the map is enabled, after it has been
/ initialized, in (machdep.c) the unix monitor.
/
/ Modified for use with overlay kernel.
/
/ Fred Canter 7/27/81


/ non-UNIX instructions
mfpi	= 6500^tst
stst	= 170300^tst
mtpi	= 6600^tst
mfpd	= 106500^tst
mtpd	= 106600^tst
spl	= 230
ldfps	= 170100^tst
stfps	= 170200^tst
wait	= 1
rtt	= 6
halt	= 0
reset	= 5
trap	= 104400

.globl	_end
.globl	_main
.globl	_cpuid
	jmp	start

/ trap vectors

	sbtrap;340	/Bus error
	sbtrap;341	/Illegal instruction
	sbtrap;342	/BPT
	sbtrap;343	/IOT
	sbtrap;344	/Power fail
	sbtrap;345	/EMT
	sbtrap;346	/Trap instruction

trapok:	0	/ 0 = halt on bus error trap
		/ 1 = rtt on bus error trap
_cpuid:	0	/ 0 = I & D space cpu
		/ 1 = I space only cpu
saveps:	0	/ Save the PS on a trap

start:
	reset
	mov	$340,PS
	mov	$157776,sp

/ Check for separate I & D space CPU,
/ by attempting to set bits 0-2 of 
/ memory management status register 3.
/ If SSR3 is not present or if bits 0-2
/ can't be set, then the CPU does not have 
/ separate I & D space.

	inc	trapok		/ Allow bus error trap
	mov	$7,*$SSR3	/ Will trap if no SSR3
	tst	_cpuid		/ did a trap occur ?
	bne	1f		/ yes, no SSR3, skip following
	tst	*$SSR3		/ no, SSR3 exists, did sep. I&D bits set ?
	bne	1f		/ yes, separate I & D space CPU
	inc	_cpuid		/ no, non separate I & D space CPU
1:
	clr	trapok		/ further traps will be fatal

/ If seperate I & D space CPU,
/ set kernel I+D to physical 0 and IO page.
/ If I space only CPU,
/ set kernel I to physical 0 and IO page.

	clr	r1
	mov	$77406,r2
	mov	$KISA0,r3
	mov	$KISD0,r4
	jsr	pc,setseg
	mov	$IO,-(r3)
	tst	_cpuid	/ If I space only CPU
	bne	1f	/ Don't set up all MM reg's
	clr	r1
	mov	$KDSA0,r3
	mov	$KDSD0,r4
	jsr	pc,setseg
	mov	$IO,-(r3)

/ Set user I+D to physical 64K (words) and IO page.

1:
	mov	$4000,r1
	mov	$UISA0,r3
	mov	$UISD0,r4
	jsr	pc,setseg
	mov	$IO,-(r3)
	tst	_cpuid	/ If I space only CPU
	bne	1f	/ Don't set up all MM reg's
	mov	$4000,r1
	mov	$UDSA0,r3
	mov	$UDSD0,r4
	jsr	pc,setseg
	mov	$IO,-(r3)

/ Enable 22 bit mapping and seperate I & D
/ space for kernel and user modes.
/ This is not done if CPU is I space only.

	mov	$25,SSR3	/ 22-bit mapping
	bit	$20,SSR3
	beq	1f
	mov	$3,MSCR
1:
	mov	$30340,PS
	inc	SSR0


/ copy program to user I space
	mov	$_end,r0
	asr	r0
	clr	r1
1:
	mov	(r1),-(sp)
	mtpi	(r1)+
	sob	r0,1b

/ continue execution in user space copy

	mov	$140340,-(sp)
	mov	$user,-(sp)
	rtt
user:
	mov	$_end+512.,sp
	mov	sp,r5

	jsr	pc,_main

	trap

	br	user

setseg:
	mov	$8,r0
1:
	mov	r1,(r3)+
	add	$200,r1
	mov	r2,(r4)+
	sob	r0,1b
	rts	pc

.globl	_setseg
_setseg:
	mov	2(sp),r1
	mov	r2,-(sp)
	mov	r3,-(sp)
	mov	r4,-(sp)
	mov	$77406,r2
	mov	$KISA0,r3
	mov	$KISD0,r4
	jsr	pc,setseg
	tst	_cpuid	/If CPU is I space only,
	beq	1f	/ use alternate mapping
	mov	$IO,-(r3)
1:
	mov	(sp)+,r4
	mov	(sp)+,r3
	mov	(sp)+,r2
	rts	pc

/ Disable separate I & D space, so that
/ non-separate I & D space unix monitors
/ and overlay test monitors can be booted
/ on separate I & D space CPU's.
/
/ _ssid and _snsid must not be called
/ on CPU's without SSR3, such as 11/34,
/ 11/40, & 11/60.

.globl _snsid
_snsid:
	bic	$7,*$SSR3
	mov	$IO,*$KISA7	/ map to I/O space
	rts	pc

/ Enable separate I & D space, in case it
/ got turned off somehow.

.globl _ssid
_ssid:
	bis	$5,*$SSR3
	rts	pc

/ clrseg(addr,count)
.globl	_clrseg
_clrseg:
	mov	4(sp),r0
	beq	2f
	asr	r0
	bic	$!77777,r0
	mov	2(sp),r1
1:
	clr	-(sp)
	mtpi	(r1)+
	sob	r0,1b
2:
	rts	pc


/ mtpi(word,addr)
.globl	_mtpi
_mtpi:
	mov	4(sp),r0
	mov	2(sp),-(sp)
	mtpi	(r0)+
	rts	pc

.globl	__rtt
__rtt:
	halt

/ Standalone bootstrap trap handler.
/ Any trap, other than bus error, will cause a halt.
/ Bus error will halt if (trapok) is zero,
/ otherwise it sets the (_cpuid) CPU I or I & D space
/ indicator to I space only CPU and returns.

sbtrap:
	mov	*$PS,saveps
	cmpb	saveps,$340
	bne	1f
	tst	trapok
	bne	2f
1:
	halt
	br	1b
2:
	inc	_cpuid
	clr	trapok
	rtt

PS	= 177776
SSR0	= 177572
SSR1	= 177574
SSR2	= 177576
SSR3	= 172516
KISA0	= 172340
KISA1	= 172342
KISA6	= 172354
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
SISD0	= 172200
SISD1	= 172202
UISA0	= 177640
UISD0	= 177600
UDSA0	= 177660
UDSD0	= 177620
MSCR	= 017777746	/ 11/70 memory control register
IO	= 177600
SWR	= 177570

.data

