*
* Bootstrap program to read block 0 from disc
*
bootaddr	equ	y'20000'	core address for bootstrap
*
	squez
selch	equ	1
cntrl	equ	2
disc	equ	3
cylsect	equ	4
work	equ	5
s.addr	equ	x'7d'	selch address
c.addr	equ	x'7c'	controller address
d.addr	equ	x'7a'	disc address
*
start	equ	*
	lb	selch,s.addr	set device addresses
	lb	cntrl,c.addr	... from low core locations
	lb	disc,d.addr
	lis	cylsect,0	cyl 0, sector 0
	oc	selch,s.stop	stop selch
	wd	selch,bufs+1	core start address
	wh	selch,bufs+2
	wd	selch,bufe+1	core end address
	wh	selch,bufe+2
	wdr	disc,cylsect	cyl no. to disc
	wdr	cntrl,cylsect	sector no. to controller
	oc	cntrl,c.read	start controller
	oc	selch,s.read	start selch
wselch	ssr	selch,work	wait for selch busy==0
	bc	wselch
	oc	selch,s.stop	stop selch
	b	bootaddr	branch to loaded bootstrap
*
c.read	db	x'01'		read
s.stop	db	x'48'		stop + extended address read
s.read	db	x'30'		read + go
	align	adc
bufs	dc	bootaddr
bufe	dc	bootaddr+511
	end
