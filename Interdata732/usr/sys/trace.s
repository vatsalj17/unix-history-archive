	title	trace -- unix log/trace routine
	entry	dtrace
	extrn	trmask,trbuff
	extrn	printf
*
* trace(mask, label, value)
* char *label;
*
*	- log message label (first 4 chars) & value in 
*	  circular trace buffer "trbuff".
*	- if any bits in the mask are on in the trace-control word
*	  "trmask", display message on the system console
*
r0	equ	0
r1	equ	1
r2	equ	2
r3	equ	3
r4	equ	4
r5	equ	5
r6	equ	6
sp	equ	7
r8	equ	8
rf	equ	15
*
* structure of arguments
*
args	struc
mask	ds	adc
label	ds	adc
value	ds	adc
	ends
	pure
dtrace	equ	*
*
* get first 4 chars of message label
*
	l	r1,label(sp)	message pointer
	lb	r2,0(r1)	first byte
	exbr	r2,r2
	lb	r0,1(r1)	second byte
	stbr	r0,r2
	exhr	r2,r2		move to upper half of word
	lb	r0,2(r1)	third byte
	stbr	r0,r2
	exbr	r2,r2
	lb	r0,3(r1)	fourth byte
	stbr	r0,r2
*
* put message & value in trace buffer
*
	la	r4,trbuff	point to trace buffer
tr.abl	equ	*
	abl	r2,0(r4)	add to bottom of list
	bfc	4,tr.ablok	no overflow - continue
	lis	r0,0		else reset buffer to 'empty'
	sth	r0,2(r4)
	st	r0,4(r4)
	b	tr.abl		try again
tr.ablok	equ	*
	l	3,value(sp)	value
	abl	r3,0(r4)	add to bottom of list
*
* test whether this trace is masked on
*
	l	r0,mask(sp)	get mask
	n	r0,trmask	check with trace control word
	bzr	rf		no bits on - return
*
* display message on system console
*
	shi	sp,32+12	save regs first
	stm	r8,12(sp)
	la	r0,tr.fmt	message format
	st	r0,0(sp)
	st	r1,4(sp)	message
	st	r3,8(sp)	value
	bal	rf,printf	display it on console
	lm	r8,12(sp)	restore regs
	ahi	sp,32+12
	br	rf		return
*
* format for display
*
tr.fmt	equ	*
	db	c'%s %x',x'0a',0
	db	*
	end
