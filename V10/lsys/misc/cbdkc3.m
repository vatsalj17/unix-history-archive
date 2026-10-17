root	regfs	ra	0100
swap	ra	01	20480
swap	ra	011	20480
swap	ra	021	20480
dump	uddump	1	10240	20480	# dump to ra11

mba 0	bus 0	tr 9
tm78 0	mb 0	drive 0
tu78 0	ctl 0	unit 0

dw780 0	bus 0	tr 3	voff 0x200
uda50 0	ub 0	reg 0772150	vec 0254
ra 0	uda50 0	unit 0
ra 1	uda50 0	unit 1
ra 2	uda50 0	unit 2
dz11 0	ub 0	reg 0760100	vec 0300
dz11 1	ub 0	reg 0760110	vec 0310
kmc11b 0 ub 0	reg 0160000	vec 0400
dk 0	ub 0	reg 0767740	vec 0530
kdi	1
drum	0
console	0
mem	0
stdio	0

ttyld	32
mesgld	256
rmesgld	0
unixpld	0
dkpld	256
connld	0

regfs	20
procfs	0
msfs	0
netafs	0
errfs	0
netbfs	0
pipefs	0
