root	regfs	ra	0100	# guess
swap	ra	01	20480
swap	ra	011	20480
swap	ra	021	20480
swap	ra	031	20480
dump	uddump	0	10240	20480	# dump to ra01

mba 0	bus 0	tr 8
hp 0	mb 0	drive 0

mba 1	bus 0	tr 9
tm78 0	mb 1	drive 0
tu78 0	ctl 0	unit 0

dw780 0	bus 0	tr 3	voff 0x200
uda50 0	ub 0	reg 0172150	vec 0450
ra 0	uda50 0	unit 0
ra 1	uda50 0	unit 1
ra 2	uda50 0	unit 2
ra 3	uda50 0	unit 3
dz11 0	ub 0	reg 0160120	vec 0320
dz11 1	ub 0	reg 0160130	vec 0330
kmc11b 0	ub 0	reg 0160600	vec 0600
dk 0	ub 0	reg 0767770	vec 0540

kdi	1
drum	0
console	0
starcons 0
mem	0
stdio	0

ttyld	32
mesgld	256
rmesgld	0
cmcld	0
unixpld	0
dkpld 128
connld	0
bufld	32

regfs	10
procfs	0
msfs	0
netafs	0
errfs	0
pipefs	0
netbfs	0
