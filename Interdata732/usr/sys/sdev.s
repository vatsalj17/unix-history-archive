	title	7/32 unix special device routines
	entry	putchar
	entry	display
	entry	clkstart
	extrn	csw
	extrn consaddr,conscmd2
*
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
rdev	equ	1
rchar	equ	2
*
* putchar(c)
*
*  - write character <c> on the system console using sense-status i/o
*
putchar	equ	*
	lb	rdev,consaddr	device address of console
	ohi	rdev,1		use write side 
	oc	rdev,conscmd2	set up baud rate, etc.
	oc	rdev,cmddiswr	disarm write interrupts
bsyloop ssr	rdev,r0		wait till not busy
	btc	8,bsyloop
	l	rchar,0(sp)	get character to be written
	bz	done		zero - don't write anything
write	equ	*
	wdr	rdev,rchar	write the character
bsyloop1 ssr	rdev,r0		wait for bsy -> 0
	btc	8,bsyloop1
*
* if character was a newline, write a carriage return after it
*
	chi	rchar,x'0a'	nl?
	bne	done		no - return
	lhi	rchar,x'0d'	add a cr
	b	write
*
* finished -- try to restore previous status
*
done	equ	*
	oc	rdev,cmdenwr	enable write interrupts
	br	rf
*
* device commands
*
cmddiswr db	x'a3'	dis + wrt + dtr
cmdenwr db	x'63'	enb + wrt + dtr
	align	adc
*
*	display -- display panel driver
*
* called by clock() at each lfc interrupt to read console switch
*  register, and display value addressed by csw on display panel
*
display	equ	*
*
* read console switch register into csw
*
	lis	r1,1		display address always 01
	oc	r1,dis.norm	set 'normal' mode
	rhr	r1,r2		read halfword
	exbr	r2,r2		reverse bytes
	st	r2,csw		save in csw
*
* if switches are zero, display pc
*
	lr	r2,r2		zero?
	bnz	disp.ad		no - use as address
	l	r2,0(sp)	yes - display parm (pc)
	b	disp.wr
*
* use switch contents as an address in kernel space
*
disp.ad equ	*
	thi	r2,1		odd address ?
	bz	disp.ev		no - skip
	ai	r2,y'10000'	yes - use segment 1
disp.ev equ	*
	ni	r2,-4		make sure of word boundary
	l	r2,0(r2)	load value
*
* display *(csw) on display panel
*
disp.wr equ	*
	oc	r1,dis.incr	set 'incremental' mode
	exbr	r2,r2		reverse first two bytes
	whr	r1,r2		write lower two bytes
	exhr	r2,r2		get upper two bytes
	exbr	r2,r2		reverse them
	whr	r1,r2		write upper two bytes
	br	rf		return
*
* commands for display panel
*
dis.norm	db	x'80'	set normal mode
dis.incr	db	x'40'	set incremental mode
*
*	clkstart -- line-frequency clock initialization
*
* called only once, by main(), to enable interrupts from the
*  line frequency clock, which generates interrupts at twice the
*  line frequency.
*
clkstart	equ	*
	lhi	r1,x'6d'	clock address
	oc	r1,clk.enab	enable clock interrupts
	br	rf	return
clk.enab	db	x'40',0
	end
