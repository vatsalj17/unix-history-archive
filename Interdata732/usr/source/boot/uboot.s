*
* Unix bootstrap loader
*
*	N.B.	Must be assembled with SQUEZ to make it relocatable
*
bootaddr	equ	y'20000'	must be >top of loaded program
*
i.addr	equ	x'78'	10	console input address
i.cmd2	equ	x'79'	a3	console init cmd
d.addr	equ	x'7a'	c7	disc address
*		x'7b'		unused
c.addr	equ	x'7c'	b6	disc controller
s.addr	equ	x'7d'	f0	selch address
os.ext	equ	x'7e'	0000	OS file extension (for LSU) or console def'n
*
LARGE	equ	19	large file bit in inode flags
MAGIC	equ	x'107'	a.out magic number
EOL	equ	x'0d'	carriage return
PROMPT	equ	c'?'
PAUSE	equ	c'-'	flag to go into wait state after loading
WAITPSW	equ	x'8000'	wait-state psw
*
ret	equ	15
ret1	equ	14
work	equ	13
offset	equ	12
count	equ	11
bno	equ	10
inode	equ	09
char	equ	08
cyl	equ	07
sector	equ	06
dev	equ	05
disc	equ	04
cntrl	equ	03
selch	equ	02
core	equ	01
*
low	equ	*	loaded into low core by LSU
	org	low+x'60'
	b	start(0,0)
	org	low+x'80'
*
start	equ	*
	li	1,bootaddr		relocation address
	lis	2,0			start at low core
rloop	lm	8,0(2)			move 8 words
	stm	8,0(1,2)
	ahi	2,8*adc			increment core address
	cli	2,high			all moved?
	bl	rloop			no - continue
	b	go-low(1)		execute relocated code
*
go	equ	*
	lh	work,os.ext		OS file extension 0?
	bz	lsu			yes - assume loaded by LSU
	sth	work,i.addr		no - OS extension is really console def
lsu	equ	*
	lm	14,i.addr	save lowcore data
	stm	14,lowsave
	lb	selch,s.addr
	lb	cntrl,c.addr
	lb	disc,d.addr
	la	work,buf	set up disc read buffer addresses
	st	work,bufs
	ahi	work,511
	st	work,bufe
	lis	char,0
	lhi	count,names-ind+12
zeroout	st	char,ind(count)	clear indirect blocks & filename
	sis	count,4
	bnm	zeroout
*
	lhi	char,PROMPT	ask user for filename
	bal	ret1,putc
	lis	count,0
nxtchr1	bal	ret,getc	read a character from console
	bz	lookup		returns 0 at end of line
	chi	char,PAUSE	flag to pause after loading?
	bne	nopause		no - continue
	li	work,WAITPSW	set wait bit
	st	work,ps.start		in start psw
	b	nxtchr1
nopause	equ	*
	stb	char,names(count)	set in filename
	ais	count,1
	b	nxtchr1		note no test for overflow
*
lookup	equ	*
	lis	inode,1		read inode for root directory
	bal	ret,iget
nxtdblk	bal	ret1,rblk	read directory block
	bz	start		start over if end of directory
	la	count,buf
cmpnext	lis	offset,12
	lh	inode,0(count)	directory entry used?
	bz	nextent		no - look at next
nxtchr2	lh	char,2(offset,count)	compare names
	clh	char,names(offset)
	bne	nextent		not equal - look at next entry
	sis	offset,2
	bnm	nxtchr2
	b	gotinod		right name - go read it in
nextent	ahi	count,16	next directory entry
	la	work,buf+512
	cr	count,work
	bl	cmpnext
	b	nxtdblk
*
gotinod	equ	*
	bal	ret,iget	read inode for file
	lis	core,0
nxtsblk	st	core,bufs	read block into core
	ai	core,511
	st	core,bufe
	bal	ret1,rblk
	bz	reloc
	ais	core,1
	b	nxtsblk
*
reloc	equ	*
	l	char,x'0'	check first word
	clhi	char,MAGIC	a.out file?
	bne	finish		no - done
	lis	count,0	yes - relocate to get rid of header
mvsys	l	char,32(count)
	st	char,0(count)
	ais	count,4
	cr	count,core
	bl	mvsys
finish	equ	*
	lm	14,lowsave	restore lowcore data
	stm	14,i.addr
	lpsw	ps.start	go to start address in loaded file
*
* Read inode and indirect blocks
*
iget	equ	*
	ahi	inode,15	find block containing inode
	lr	sector,inode
	srls	sector,3	(inode+15)/8
	bal	ret1,rblka	read block
	ni	inode,7		find offset of inode in block
	slls	inode,6		(inode+15) mod 8
	stm	6,regsave	save work regs
	lm	6,buf(inode)	copy inode info from buffer
	stm	6,inod		(up to addr[7])
	lm	6,regsave	restore work regs
*
	la	bno,addr	small file: block addresses in addr
	li	work,LARGE	test for large file
	tbt	work,inod
	bzr	ret
	la	core,ind	large file - read indirect blocks
nxtind	bal	ret1,rblk
	bz	finind
	li	count,508	move indirect block
mvind	l	char,buf(count)
	st	char,0(core,count)
	sis	count,4
	bnm	mvind
	ahi	core,512
	b	nxtind
finind	la	bno,ind		large file: block addresses in ind
	br	ret
*
* Console read and write routines
*
getc	equ	*
	lb	dev,i.addr	console input address
	oc	dev,i.cmd2	initialize
	oc	dev,t.read	start terminal read
getwait	ssr	dev,work	wait for busy == 0
	bc	getwait
	rdr	dev,char	read char from terminal buffer
	nhi	char,x'7f'	clear parity bit
	clhi	char,c'A'	check for alpha
	bl	notran
	clhi	char,c'Z'
	bp	notran
	ohi	char,x'20'	alpha - translate to lowercase
notran	bal	ret1,putc	echo character back
	clhi	char,EOL	end of line?
	bner	ret		no - return (cond. code nonzero)
	lr	ret1,ret
	lis	char,x'0a'	echo extra linefeed
*
putc	equ	*
	lb	dev,i.addr	console output address
	ais	dev,1
	oc	dev,t.write	start terminal writing
	wdr	dev,char	write the character
putwait	ssr	dev,work	wait for busy == 0
	bc	putwait
	br	ret1
*
* Read a disc block
*
rblk	equ	*
	l	sector,0(bno)	next block number
	bzr	ret1
	ais	bno,4
rblka	equ	*
	dh	sector,f24	get cylinder address
	slls	sector,1	get sector address
	clhi	sector,24	correct head?
	bl	wselch1		yes - start read
	ais	sector,8	else set head 0
wselch1	ssr	selch,work	wait for selch busy == 0
	btc	s.busy,wselch1
wcntrl1	ssr	cntrl,work	check controller status
	bfc	c.idle,wcntrl1	wait for idle == 1
	btc	c.err,*
wdisc1	ssr	disc,work	check disc status
	btc	d.nrsrw,wdisc1	wait for no arm motion
	btc	d.err,*
	whr	disc,cyl	cylinder address to disc
	oc	disc,d.seek	start seek
wcntrl2	ssr	cntrl,work	sense controller status
	bfc	c.idle,wcntrl2	wait for idle == 1
wdisc2	ssr	disc,work	sense disc status
	btc	d.nrsrw,wdisc2	wait for no arm motion
	btc	d.err,*
	oc	selch,s.stop	stop selch
	wd	selch,bufs+1	start address to selch
	wh	selch,bufs+2
	wd	selch,bufe+1	end address to selch
	wh	selch,bufe+2
	whr	disc,cyl	cylinder to disc (again)
	wdr	cntrl,sector	sector to controller
	oc	cntrl,c.read	start controller reading
	oc	selch,s.read	start selch reading
wselch2	ssr	selch,work	sense selch status
	btc	s.busy,wselch2	wait for busy == 0
	oc	selch,s.stop	stop selch
wcntrl3	ssr	cntrl,work	check controller status
	bfc	c.idle,wcntrl3	wait for idle == 0
	btc	c.err,*
	br	ret1		(cond code must be nonzero)
*
s.busy	equ	8	selector channel busy
c.idle	equ	2	disc controller idle
c.err	equ	5	disc controller error
d.nrsrw	equ	8	disc not-ready-to-seek-read-or-write
d.err	equ	7	disc error
*
	align	8
ps.start dc	0,y'60'		psw to start loaded program
f24	dc	h'24'	sectors per track
d.seek	db	x'c2'	disarm+seek
c.read	db	x'01'	read
s.stop	db	x'48'	stop + extended address read
s.read	db	x'70'	extended address read + read + go
t.write	db	x'a3'
t.read	db	x'a1'
	align	adc
high	equ	*	highest location for relocate
*
bufs	equ	*
bufe	equ	bufs+4
lowsave	equ	bufe+4
regsave	equ	8+lowsave
buf	equ	10*adc+regsave
inod	equ	buf+512
mode	equ	inod
addr	equ	3*adc+inod
ind	equ	11*adc+inod
names	equ	8*512+ind
	end
