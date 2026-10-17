	title	Interdata unix low-core initialization

* conditional assembly switches

FPREGS	equ	0	if 1, floating-point regs must be saved
DPREGS	equ	0	if 1, double-precision regs must be saved
PIC	equ	0	if 1, include precision interrupt clock timing
*				(must also be defined in start.s)

	entry	kisa0,uisa0,ka6
	entry	kisa,uisa
	entry	addrsw,ksp
	entry	consaddr,conscmd2
	entry	trmask
	entry	memtop,consdev,rootdev,swapdev,swplo,nswap
	entry	savfp
    ifnz	PIC
	entry	sytim,utime,itime
	entry	stclk.a,stclk.b
    endc
	extrn	start,dump
	extrn	dispint,trap,clock
	extrn	selchint,cntlintr,dskintr
	extrn	mtintr
	extrn	vdurint,vduxint
	extrn	lpint
	extrn	msmcintr,msmintr
	extrn	swtch,runrun
	extrn	proc
	extrn	u


* psw bit definitions

ps.wait equ	x'8000'	wait state
ps.io	equ	x'4000'	immediate interrupt mask
ps.mm	equ	x'2000'	machine malfunction interrupt mask
ps.af	equ	x'1000'	arith fault interrupt mask
ps.il	equ	x'0800'	multi level interrupts (8/32)
ps.rp	equ	x'0400'	memory relocation / protection
ps.sq	equ	x'0200'	system queue service mask
ps.prot equ	x'0100'	protect mode
ps.ureg equ	x'00f0'	user register set

* psw definitions

ps.user equ	ps.io+ps.mm+ps.af+ps.rp+ps.prot+ps.ureg
ps.idle equ	ps.wait+ps.io+ps.mm+ps.af+ps.rp+ps.ureg
ps.kern equ	ps.io+ps.mm+ps.af+ps.rp+ps.ureg
ps.disb equ	ps.mm+ps.af+ps.rp+ps.ureg
ps.trap equ	ps.mm+ps.af

* register definitions

r0	equ	0
r1	equ	1
r2	equ	2
r3	equ	3
r4	equ	4
r5	equ	5
r6	equ	6
r7	equ	7
r8	equ	8
r9	equ	9
ra	equ	10
rb	equ	11
rc	equ	12
rd	equ	13
re	equ	14
rf	equ	15
sp	equ	r7

	title	low-core reserved locations

* Note:		This code should be absolute, since it is loaded into
*	physical low core.  Unfortunately the assembler doesn't handle absolute
*	code, so it must be faked as pure. 


*	org	0		start at physical low core
	pure			(not really -- see note above)
low	equ	*

* reserved memory location definitions

isp	equ	low+x'd0'	interrupt-service pointer table
mac.r0	equ	low+x'300'	memory-access controller seg regs
mac.rs	equ	low+x'340'	memory-access controller status reg

fsav	equ	2*adc		offset of f.p. reg save area (u_fsav)
dsav	equ	8*4+fsav	offset of d.p. reg save area (u_fsav[8])
fsaved	equ	8*8+dsav	offset of 'f.p. regs saved' flag (u_fsav[24])


* trap psw's and pointers


fpregs	ds	8*4		floating-point regs
psw.mmo ds	8		machine malfunction old psw
	dc	0,0			-
psw.ii	dc	ps.trap		illegal instruction new psw
	dc	trap.ii
psw.mm	dc	0		machine malfunction new psw
	dc	trap.mm
	dc	0,0			-
psw.af	dc	ps.trap		arith fault new psw
	dc	trap.af

* '50' sequence for magtape load

	org	low+x'50'
	al	x'300'
	b	x'80'

	org	low+x'60'
	b	start		lsu start address
	b	dump		dump start address

	org	low+x'78'	LSU addresses
consaddr db	y'10'		device address of 'system console'
conscmd2 db	x'38'		pals command 2 for 'system console'
	org	low+x'80'
	dc	sq		&(system queue)
	dc	z(pf.psw)	&(power-fail psw save area)
	dc	z(pf.regs)	&(power-fail reg save area)
psw.sqs dc	ps.trap		system queue service new psw
	dc	trap.sq
psw.mf	dc	ps.trap		memory fault new psw
	dc	trap.mf

* svc table

psw.svc dc	ps.trap		svc new psw status
	dc	z(trap.isv)	svc 0
	dc	z(trap.isv)	svc 1
	dc	z(trap.isv)	svc 2
	dc	z(trap.isv)	svc 3
	dc	z(trap.isv)	svc 4
	dc	z(trap.isv)	svc 5
	dc	z(trap.isv)	svc 6
	dc	z(trap.isv)	svc 7
	dc	z(trap.isv)	svc 8
	dc	z(trap.isv)	svc 9
	dc	z(trap.isv)	svc 10
	dc	z(trap.isv)	svc 11
	dc	z(trap.isv)	svc 12
	dc	z(trap.isv)	svc 13
	dc	z(trap.svc)	svc 14 - system call
    ifnz	PIC
	dc	z(trap.tim)	svc 15
    else
	dc	z(trap.isv)	svc 15
    endc

	dc	0,0,0,0,0

* interrupt service pointer table

	org	isp	initialize unused entries to ignore interrupts
	nlist
	do	256
	dc	z(int.null)
	list
*	do	256
*	dc	z(int.null)		listing suppressed

* display console interrupt

	org	2*x'01'+isp	address 01
	dc	z(int.disp)

* line printer interrupt

	org	2*x'62'+isp
	dc	z(int.lp)

* clock interrupts

	org	2*x'6d'+isp	line frequency clock
	dc	z(int.lfc)

* selector channel interrupt

	org	2*x'f0'+isp	** selch
	dc	z(int.sch)

* disk controller interrupt

	org	2*x'b6'+isp	disk controller
	dc	z(int.dcnt)

* disk interrupts

	org	2*x'c6'+isp	removable disk
	dc	z(int.dsk)
	org	2*x'c7'+isp	fixed disk
	dc	z(int.dsk)

* msm disc interrupts

	org	2*x'fb'+isp	controller
	dc	z(int.mcnt)
	org	2*x'fc'+isp	drive 0
	dc	z(int.mdsk)
	org	2*x'fd'+isp	drive 1
	dc	z(int.mdsk)

* magtape interrupts

	org	2*x'c5'+isp	tape transport 1
	dc	z(int.mt)

* pals interrupts

	org	2*x'10'+isp		local vdu's
	dc	z(int.vdur),z(int.vdux)	tty0
	dc	z(int.vdur),z(int.vdux)	tty1
	dc	z(int.vdur),z(int.vdux)	tty2
	dc	z(int.vdur),z(int.vdux)	tty3
	dc	z(int.vdur),z(int.vdux)	tty4
	dc	z(int.vdur),z(int.vdux)	tty5
	dc	z(int.vdur),z(int.vdux)	tty6
	dc	z(int.vdur),z(int.vdux)	tty7
	dc	z(int.vdur),z(int.vdux)	ttya
	dc	z(int.vdur),z(int.vdux)	ttyb
	dc	z(int.vdur),z(int.vdux)	ttyc
	dc	z(int.vdur),z(int.vdux)	ttyd
	dc	z(int.vdur),z(int.vdux)	ttye
	dc	z(int.vdur),z(int.vdux)	ttyf
	dc	z(int.vdur),z(int.vdux)	ttyg
	dc	z(int.vdur),z(int.vdux)	ttyh

	title	low-core data areas

* fixed area - should not be moved
*	used for emergency patching and 'ps' command

	org	mac.r0+x'100'	skip mac area
	dc	a(proc)		address of proc table for 'ps' command
trmask	dc	0		mask for execution trace
memtop	dc	256*1024	max possible memory (changed to actual amount of
*					memory by start.s)
consdev	dc	0		console device number (major/minor)
rootdev	dc	y'0200'		root filesystem device number
swapdev	dc	y'0200'		swap area device number
swplo	dc	8000		first block in swap area (must be >0)
nswap	dc	1600		number of blocks in swap area

* hardware-dependent area

pf.psw	ds	8		power-fail psw save area
pf.regs ds	32*4		power-fail reg save area
sq	dc	0,0		system queue (unused)

* mac segmentation registers

kisa0	equ	*		kernel mode seg regs
	dc	0,0,0,0,0,0,0,0
	dc	0,0,0,0,0,0,0,0
kisae	equ	*-8		seg e

uisa0	equ	*		user mode seg regs
	dc	0,0,0,0,0,0,0,0
	dc	0,0,0,0,0,0,0,0

kisa	dc	a(kisa0)	ptr to kernel seg regs
uisa	dc	a(uisa0)	ptr to user seg regs
ka6	dc	a(kisae)	ptr to per-process segment

* non-reentrant save area for address-space switching

	align	4
ksp	ds	4		kernel mode stack pointer save
nr.intp ds	4		interrupt handler address save
nr.psw	ds	8		psw save
nr.regs ds	16*4		register save
	title	trap transfer vector

trap.mm equ	*		machine malfunction
	btc	6,trap.mp
	lm	re,psw.mmo	get saved old psw
	bal	r6,trapx
	dc	h'4'

trap.mp equ	*		memory parity error
	lm	re,psw.mmo	get saved old psw
	b	trap.mf

trap.af equ	*		arithmetic fault
	bal	r6,trapx
	dc	h'8'

trap.mf equ	*		memory fault
	lis	r0,0
	st	r0,mac.rs		clear mac status register
	bal	r6,trapx
	dc	h'0'

trap.ii equ	*		protect mode / illegal instruction
	bal	r6,trapx
	dc	h'1'

trap.isv equ	*		illegal supervisor call
	bal	r6,trapx
	dc	h'10'

trap.svc equ	*		supervisor call
	lr	r3,rd		'stat' is effective svc arg address
	bal	r6,trapx
	dc	h'6'

trap.sq equ	*		system queue service
	bal	r6,trapx
	dc	h'4'

    ifnz	PIC
trap.tim  equ	*		svc 15 request 10us clock

*	return in users r0 the current value of 
*	sytim , utime , or itime
*	user must supply in r1 one of 0,4,8 to get
*	the corresponding value of the above

	stm	re,nr.psw	save old psw for quick return
	epsr	rc,rc		current ps
	ohi	rc,ps.ureg	switch to user regs
	epsr	r0,rc
	ni	r1,x'c'		make sure offset is legal
	l	r0,sytim(r1)	load requested time value
	lpsw	nr.psw		exit quickly without telling anyone
    endc


* common trap routine

trapx	equ	*
	lh	r2,0(r6)	get trap code
	epsr	rd,rd		current psw
	li	rb,ps.kern	new psw: kernel mode, enabled
	la	rc,trap
	b	call		go call c trap handler
	title	interrupt transfer vector

int.null equ	*		unknown device
	lpswr	r0		ignore interrupt

int.disp equ	*		display console
	la	rc,dispint
	b	int

int.lp	equ	*		line printer interrupt
	la	rc,lpint
	b	int

int.lfc equ	*		line frequency clock
	la	rc,clock
	b	int

int.sch equ	*		selector channel
	la	rc,selchint
	b	int

int.dcnt equ	*		disk controller
	la	rc,cntlintr
	b	int

int.dsk equ	*	disk file
	la	rc,dskintr
	b	int

int.mcnt equ	*		msm disk controller
	la	rc,msmcintr
	b	int

int.mdsk equ	*		msm disk drive
	la	rc,msmintr
	b	int

int.mt	equ	*	mag tape
	la	rc,mtintr
	b	int

int.vdur equ	*	local vdu input
	la	rc,vdurint
	b	int

int.vdux equ	*	local vdu output
	la	rc,vduxint
	b	int

* common interrupt routine

int	equ	*
	lr	re,r0		move psw to correct regs
	lr	rf,r1
	epsr	rd,rd		current psw
	nhi	rd,x'ffff'-ps.il	all interrupt levels off (8/32)
	epsr	r0,rd
	li	rb,ps.disb	new psw: kernel mode, disabled
	b	call		go call c interrupt-handler
	title	call -- interface to c trap / interrupt handlers

* input: re-rf - old psw & loc
*	rd	- current (interrupt) psw status
*        rc    - interrupt routine address
*        rb    - interrupt routine psw status
*        r2    - device address ( or trap code )
*        r3    - device status ( or svc arg address )

call	equ	*

    ifnz	PIC
*	read and restart the pic (precision interrupt clock)

	lhi	r0,x'6c'	addr of pic
	rhr	r0,r1		read current interval

sc1	wh	r0,stclk.a	restart clock
	ssr	r0,r4
	btc	8,sc1
	oc	r0,stclk.b
	lhi	r5,x'fff'	max interval 
	sr	r5,r1		current clock interval
    endc


* if trap from user mode, switch to kernel address space 

	thi	re,ps.prot	user mode?
	bz	kernel		no - kernel already

    ifnz	PIC
*	add new interval to user time

	am	r5,utime	increment user time
    endc

	la	r1,kisa0	kernel seg regs
	bal	r6,addrsw	switch address space
	b	nkernel

* else trap from kernel mode -- get stack pointer from register set f

kernel	equ	*

    ifnz	PIC
	thi	re,ps.wait
	bz	kernel2		not in wait
	am	r5,itime	if wait add currnt interval to idle time
	b	kernel3
kernel2	am	r5,sytim	if kernel and not wait increment sys time
kernel3	equ	*
    endc

	st	rd,nr.psw	set up resume psw
	la	r1,nkernel
	st	r1,nr.psw+adc
	lr	r1,rd		current psw
	ohi	r1,ps.ureg	switch to reg set f
	epsr	r0,r1
	st	sp,ksp		save stack pointer
	lpsw	nr.psw		back to reg set 0
nkernel	equ	*

* enable memory relocation / protection

	ohi	rd,ps.rp	enable mac
	epsr	r0,rd

* save psw & status on kernel stack

	l	sp,ksp		get kernel stack pointer
	shi	sp,14*adc	space for 14 words
	stm	re,11*adc(sp)	save old psw
	st	rb,13*adc(sp)	save new psw
	st	r2,0(sp)	save dev code
	st	r3,adc(sp)	save status

* switch to user register set, and save regs

	st	rc,nr.intp	save routine address
	st	sp,ksp		save stack pointer
	ohi	rd,ps.ureg	switch to user regs
	epsr	r0,rd
	stm	r0,nr.regs	save all regs
	l	sp,ksp		restore stack pointer
	lm	r8,nr.regs	stack regs r0-sp
	stm	r8,2*adc(sp)

* reload user high regs ( to be saved by standard c linkage )
*  and call c trap handler

	lm	r8,8*adc+nr.regs	restore regs r8-rf
	st	rf,10*adc(sp)	stack link reg
	l	r1,nr.intp	trap routine address
	l	r0,13*adc(sp)	new psw
	epsr	r2,r0
	balr	rf,r1		call trap routine

* on return from trap routine, check whether higher-priority process
* is now ready to run

	l	r1,11*adc(sp)	old psw
	thi	r1,ps.prot	user mode ?
	bz	noswtch		no - don't switch kernel process

switch	equ	*
	li	r0,ps.disb	disable interrupts
	epsr	r1,r0
	lb	r1,runrun	higher-priority process waiting?
	lr	r1,r1
	bz	nswtch		no - restore interrupted process
	li	r0,ps.kern	enable interrupts
	epsr	r1,r0
    ifnz	FPREGS!DPREGS
	bal	rf,savfp	save floating-point regs
    endc
	bal	rf,swtch	reschedule cpu to new process
	b	switch		check again
nswtch	equ	*

* restore status of interrupted process

  ifnz	FPREGS!DPREGS
	l	r0,u+fsaved		f.p. registers saved?
	bz	nofp
	lis	r0,0
	st	r0,u+fsaved
    ifnz	FPREGS
	lme	r0,u+fsav	restore floating-point regs
    endc
    ifnz	DPREGS
	lmd	r0,u+dsav	restore double floating-point regs
    endc
nofp	equ	*
  endc

noswtch	equ	*
	li	r0,ps.disb	disable interrupts
	epsr	r1,r0
	l	rf,10*adc(sp)	restore link reg
	stm	r8,8*adc+nr.regs	save r8-rf
	lm	r8,2*adc(sp)	save r0-sp
	stm	r8,nr.regs
	lm	re,11*adc(sp)	old psw
	ahi	sp,14*adc	pop stack

* if previous mode was user, switch back to user address space

	thi	re,ps.prot	user mode?
	bz	kernel1		no - stay in kernel mode
	epsr	rd,rd		current psw
	nhi	rd,x'ffff'-ps.rp	disable memory access controller
	epsr	r0,rd
	la	r1,uisa0	user seg regs
	bal	r6,addrsw
kernel1 equ	*

* return to previous status

	st	sp,ksp		save kernel stack pointer
	ni	re,y'ffffffff'-ps.wait	turn off 'wait' bit
	stm	re,nr.psw	save return psw

    ifnz	PIC
*	read the pic and restart 
*	update sys time

	lhi	r0,x'6c'	pic addr
	rhr	r0,r1		read current interval
sc2	wh	r0,stclk.a	restart
	ssr	r0,r4
	btc	8,sc2
	oc	r0,stclk.b
	lhi	r5,x'fff'	max interval
	sr	r5,r1
	am	r5,sytim	add real interval to sys time
    endc

	lm	r0,nr.regs	restore all regs
	lpsw	nr.psw		back to previous mode

* save floating point registers (also called from trap.c)

savfp	equ	*
  ifnz	FPREGS!DPREGS
	l	r0,u+fsaved		f.p. regs saved?
	bnzr	rf
	lis	r0,1
	st	r0,u+fsaved
    ifnz	FPREGS
	stme	r0,u+fsav	save single-precision regs
    endc
    ifnz	DPREGS
	stmd	r0,u+dsav	save double-precision regs
    endc
  endc
	br	rf

	title	addrsw -- switch address space

* input: r1 = &(new segmentation register values)
*        r6 = return address

* must be called with mac & interrupts disabled

addrsw	equ	*
	lhi	r4,15*adc	start at last reg
seglp	equ	*
	l	r0,0(r1,r4)	next seg value
	st	r0,mac.r0(r4)	store in mac reg
	sis	r4,adc		back up
	bnm	seglp		repeat for all seg regs
	br	r6

    ifnz	PIC
	title   strclk and stpclk -- start and stop pic
stclk.a	dc	x'2fff'	10us precision and max interval (.04 sec)
stclk.b	dc	x'e000'	disarm pic interrupts and start 
sytim	dac	y'0'	accumulated system time
utime	dac	y'0'	user time
itime	dac	y'0'	idle time
    endc

	end
