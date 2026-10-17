/ db4 -- debugger

maxsym = 24000.
core:
   <core\0>
a.out:
   <a.out\0>
.even
zero:	0
.bss
regbuf:
	.=.+1024.
sigp	= regbuf+310+.pdp70+.mappedb
txtsiz	= sigp+20
datsiz	= txtsiz+2
stksiz	= datsiz+2
separate = stksiz+2	/ chm 9:3:79
/ ilj 1/3/76
uintflg = stksiz + 81.	/ chm 22/2/79
rtxtsiz: .=.+2
.data
objmagic: 407		/ilj 1/376
nobjmagic: 412		/ilj 1/376
namsiz:	nambuf
incdot: 2
nlcom: '/

	.bss

starmod:.=.+2
symbol:	.=.+10.
getoff:	.=.+2
namstrt: .=.+2
bytemod: .=.+2
savsp: .=.+2
error: .=.+2
ttyfin: .=.+2
dbfin: .=.+2
symfin:	.=.+2
curfin:	.=.+2
dbfout: .=.+2
magic:	.=.+2		/ chm 22/2/79
sytype:	.=.+2		/ chm 9:3:79
ch: .=.+2
lastop: .=.+2
addres: .=.+2
taddr: .=.+2
adrflg: .=.+2
fpsr:	.=.+2
och:	.=.+2
dot: .=.+2
count: .=.+2
syscnt: .=.+2
temp: .=.+2
temp1: .=.+2
obuf: .=.+8.
inbuf: .=.+128.
nambuf:	.=.+20

