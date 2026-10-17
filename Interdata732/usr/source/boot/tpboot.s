*
* Unix bootstrap loader
*	- load from 'tp' tape
*
*	N.B.	Must be assembled with SQUEZ to make it relocatable
*

* Format of 'tp' block

tapea	equ	44
size1	equ	38
dirsiz	equ	64
MDIR	equ	62

bootaddr	equ	y'20000'	must be >top of loaded program
*
t.addr	equ	x'78'	85	tape address
i.addr	equ	x'7e'	10	console input address
i.cmd2	equ	x'7f'	a3	console init cmd
*
MAGIC	equ	x'107'	a.out magic number
EOL	equ	x'0d'	carriage return
PROMPT	equ	c'?'
PAUSE	equ	c'-'	flag to go into wait state after loading
WAITPSW	equ	x'8000'	wait-state psw
*
ret	equ	15
ret1	equ	14
work	equ	13
tape	equ	12
count	equ	11
bno	equ	10
dir	equ	09
char	equ	08
dev	equ	05
core	equ	01
*
low	equ	*
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
	lm	14,t.addr	save lowcore data
	stm	14,lowsave
	lb	tape,t.addr
*
restart	equ	*
	bal	ret1,rewind	rewind tape
	lhi	char,PROMPT	ask user for filename
	bal	ret1,putc
	lis	count,0
nxtchr1	bal	ret,getc	read a character from console
	bz	nxtchr2		returns 0 at end of line
	chi	char,PAUSE	flag to pause after loading?
	bne	nopause		no - continue
	li	work,WAITPSW	set wait bit
	st	work,ps.start		in start psw
	b	nxtchr1
nopause	equ	*
	stb	char,names(count)	store in filename
	ais	count,1
	b	nxtchr1		note no test for overflow
nxtchr2	lis	char,0		delimit end of filename
	stb	char,names(count)
*
lookup	equ	*
	bal	ret1,skip	skip boot block
find	equ	*
	ci	bno,MDIR	all directory read?
	be	restart		  yes - start again
	la	core,dirbuf	set to read into directory block
	bal	ret1,rtape	read next block
	la	dir,dirbuf-dirsiz
find.1	ai	dir,dirsiz	next dir entry
	la	work,dirbuf+512	last entry?
	cr	dir,work
	be	find		  yes - read next dir block
	lis	count,0
find.2	lb	char,0(dir,count)	next char
	clb	char,names(count)	match?
	bne	find.1		  no - next dir entry
	ais	count,1
	lr	char,char	end of filename?
	bne	find.2		  no - next char
	lhl	count,tapea(dir)	file address
	exbr	count,count	pdp11 swapped bytes
pos	cr	bno,count	right block?
	be	pos.1		  yes - got it
	bal	ret1,skip	skip forward block
	b	pos
pos.1	equ	*
	lhl	count,size1(dir)	size of file
	exbr	count,count	pdp11 swapped bytes
	lis	core,0		start reading into low core
load	bal	ret1,rtape	read next block
	shi	count,512	decrement size
	bp	load		  more - read next block
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
	stm	14,t.addr
	lh	work,i.addr	console addr & cmd 2
	sth	work,t.addr	put where unix wants them
	lpsw	ps.start	go to start address in loaded file
*
* Console read and write routines
*
getc	equ	*
	lb	dev,i.addr	console input address
	oc	dev,i.cmd2	initialize
	oc	dev,i.read	start terminal read
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
	oc	dev,i.write	start terminal writing
	wdr	dev,char	write the character
putwait	ssr	dev,work	wait for busy == 0
	bc	putwait
	br	ret1
*
* Magtape I/O Routines
*

rewind	equ	*		rewind tape
	bal	ret,nmtn
	oc	tape,t.clear	clear controller
	oc	tape,t.rew	rewind
	lis	bno,0		reset block address
	br	ret1

skip	equ	*		skip forward block
	bal	ret,nmtn
	oc	tape,t.read	start reading
skip.1	ssr	tape,work	tape status
	bfc	T.EOM,skip.1	wait for EOM = 1
	ais	bno,1		increment block addr
	br	ret1

rtape	equ	*		read tape block into *(core)
	bal	ret,nmtn
	lr	ret,core	save core address for retry
	oc	tape,t.read	start reading
rtape.1	ssr	tape,work	tape status
	btc	T.BSY,rtape.1	wait for BSY = 0
	btc	T.EOM,rtape.2	  EOM - end of block
	rdr	tape,char	read byte
	stb	char,0(core)	store in memory
	ais	core,1		increment memory ptr
	b	rtape.1		next char
rtape.2	equ	*		end of block
	ais	bno,1		increment block addr
	thi	work,T.ERR	error ?
	bzr	ret1		  no - return
	lr	core,ret	restore core addr
	bal	ret,nmtn
	oc	tape,t.bsp	backspace tape
	sis	bno,1		decrement block addr
	b	rtape		retry

nmtn	equ	*		wait for end of motion
	ssr	tape,work	tape status
	thi	work,T.NMTN	no motion?
	bz	nmtn		  no - wait
	br	ret

T.ERR	equ	x'80'
T.NMTN	equ	x'10'
T.BSY	equ	x'08'
T.EOM	equ	x'02'

	align	8
ps.start dc	0,y'60'		psw to start loaded program
i.write	db	x'a3'
i.read	db	x'a1'
t.clear	db	x'e0'
t.rew	db	x'f8'
t.read	db	x'e1'
t.bsp	db	x'd1'
	align	adc
high	equ	*	highest location for relocate
*
lowsave	equ	*
dirbuf	equ	8+lowsave
names	equ	512+dirbuf
	end
