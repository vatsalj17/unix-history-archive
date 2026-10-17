	title	unix double-precision integer arithmetic subroutines
	entry	dpadd,dpcmp
	entry	ldiv,lrem
	entry	lshift
r0	equ	0
r1	equ	1
r2	equ	2
r3	equ	3
r4	equ	4
r5	equ	5
r6	equ	6
sp	equ	7
re	equ	14
rf	equ	15
*
* dpadd(long, addend)
*   int long[2], addend;
*
*	- add integer addend to double-integer long
*
dpadd	equ	*
	l	r1,0(sp)	ptr to first operand
	l	r0,4(sp)	second operand
	am	r0,4(r1)	add to low-order part
	bfcr	x'c',rf		no overflow or carry - return
	lis	r0,1		add carry to high-order part
	am	r0,0(r1)
	br	rf
*
* dpcmp(hi1, lo1, hi2, lo2)
*
*	- compare double-integers hi1lo1 and hi2lo2
*	- return hi2lo2-hi1lo1 if the magnitude of the result is < 512
*	- else return -512 or 512 according as first operand is lt or gt
*
dpcmp	equ	*
	l	r0,4(sp)	low 1
	l	r1,0(sp)	high 1
	s	r0,12(sp)	subtract low 2
	bnc	ncarry
	sis	r1,1		borrow
ncarry	equ	*
	s	r1,8(sp)	subtract high 2
	bnm	great
* less - check very large diff
	chi	r1,-1		any high order part?
	bne	lessx
	chi	r0,-512		less than limit?
	bnlr	rf
lessx	equ	*
	lhi	r0,-512
	br	rf
* greater or equal - check very large diff
great	equ	*
	bnz	greatx		high order part?
	chi	r0,512		or gt than limit?
	bnpr	rf
greatx	equ	*
	lhi	r0,512
	br	rf
*
* ldiv (dividend, divisor)
*
*	- divide unsigned integer dividend by signed divisor
*
ldiv	equ	*
	lis	r0,0
	l	r1,0(sp)
	d	r0,4(sp)
	lr	r0,r1	quotient
	br	rf
*
* lrem (dividend, divisor)
*
lrem	equ	*
	lis	r0,0
	l	r1,0(sp)
	d	r0,4(sp)
	br	rf
*
* lshift (long, n)
*   integer long[2];
*
*	- shift double-integer to the left n bits
*	- n may be negative (right-shift)
*	- return low-order word of result
*
lshift	equ	*
	l	r1,0(sp)	ptr to arg 1
	l	r0,0(r1)	high order part
	l	r1,4(r1)	low order part
	l	r2,4(sp)	no. of shifts
	bm	rshift		negative - shift right
	sll	r1,0(r2)	shift left
	lr	r0,r1		return low order part
	br	rf
*
rshift	equ	*
	srls	r1,1		shift low order part
	srls	r0,1		shift high order part
	bnc	nocarry		if carry ...
	oi	r1,y'80000000'		move carry bit to loworder
nocarry	equ	*
	ais	r2,1		count shifts
	bm	rshift		repeat
	lr	r0,1		return low order part
	br	rf
*
	end
