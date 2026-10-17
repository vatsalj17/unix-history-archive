	entry	dump,uregs,eregs
	extrn	memtop
r0	equ	0
r1	equ	1
r2	equ	2
r3	equ	3
rf	equ	15
*
* Dump all of core to mag tape
*
	pure
dump	equ	*
	lpsw	dump.psw	reg set 15, disabled
dump1	equ	*
	stm	r0,uregs	save reg set 15
	lis	r0,0		reg set 0, disabled
	epsr	r1,r0
	stm	r0,eregs	save reg set 0
*
* Set up tape
*
dump.rs	equ	*
	lhi	r1,x'f0'	selch address
	lhi	r2,x'c5'	magtape address
	oc	r1,selch.st	stop selch
	oc	r2,mt.cl	clear tape controller
	oc	r2,mt.rw	rewind tape
	bal	rf,dump.wt
	oc	r2,mt.wf	write a file mark
	bal	rf,dump.wt
	oc	r2,mt.bs	backspace over the file mark
	bal	rf,dump.wt
*
* Dump core in 8k blocks
*
	lis	r3,0		start at origin
dump.lp	equ	*
	st	r3,dump.ad	start address
	wd	r1,dump.ad+1	write address to selch
	wh	r1,dump.ad+2
	ahi	r3,8191		end address
	st	r3,dump.ad
	wd	r1,dump.ad+1	write address to selch
	wh	r1,dump.ad+2
	oc	r2,mt.wr	start tape writing
	oc	r1,selch.go	start selch
dump.ss	ssr	r1,r0		wait for selch
	btc	x'8',dump.ss
	oc	r1,selch.st	stop selch
	bal	rf,dump.wt
	ais	r3,1		next core block
	c	r3,memtop	end of core?
	bl	dump.lp
*
* Write file mark, rewind tape & halt CPU
*
	oc	r2,mt.wf	write file mark
	bal	rf,dump.wt
	oc	r2,mt.rw	rewind tape
	bal	rf,dump.wt
	li	r0,y'8000'	go into disabled wait state
	epsr	r1,r0
	b	dump.rs		if restarted, try again
*
* Subroutine to wait for 'No Motion' status on tape
*
dump.wt	equ	*
	ssr	r2,r0		sense tape status
	thi	r0,x'10'	no motion?
	bz	dump.wt		no - loop
	br	rf		yes - return
*
* Mag tape commands
*
mt.cl	db	x'20'	clear controller
mt.rw	db	x'38'	rewind tape
mt.wf	db	x'30'	write file mark
mt.bs	db	x'11'	backspace
mt.wr	db	x'22'	write
*
* Selch commands
*
selch.st	db	x'08'	stop selch
selch.go	db	x'10'	start selch
*
* Register save area
*
	impur
	align	8
eregs	das	16	register set 0
uregs	das	16	register set 15
*
dump.psw	dc	y'f0',dump1
dump.ad	das	1
	end
