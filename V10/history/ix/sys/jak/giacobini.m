root	regfs	ra	0100
swap	ra	01	20480
dump	uddump	0	14336	16384	# dump to tail end of ra01

dw750 0	bus 0	tr 8	voff 0x200
uda50 0	ub 0	reg 0772150	vec 0254
ra 0	uda50 0	unit 0
ra 1	uda50 0	unit 1
uda50 1 ub 0	reg 0772154	vec 0260
ra 2	uda50 1	unit 4
dk 0	ub 0	reg 0167770	vec 0400
kmc11b 0 ub 0	reg 0160200 vec 0600
kmc11b 1 ub 0	reg 0160210 vec 0610
kdi	2
drum	0
console	0
mem	0
stdio	0

ttyld	32
mesgld	300
rmesgld	0
cmcld	0
dkpld	96
connld	0
cdkpld	0

regfs	10
procfs	0
msfs	0
netafs	0
pipefs	0

proc	500
file	1000
stream	500
queue	2000
inode	500
