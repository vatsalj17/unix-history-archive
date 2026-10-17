root	regfs	ra	0100
swap	ra	01	20480
swap	ra	011	20480
swap	ra	021	20480
swap	ra	031	20480
dump	uddump	0	10240	20480	# dump to ra01

mba 0	bus 0	tr 4
tm03 0	mb 0	drive 0
te16 0	ctl 0	unit 0
dw750 0	bus 0	tr 8	voff 0x200
uda50 0	ub 0	reg 0772150	vec 0154
ra 0	uda50 0	unit 0
ra 1	uda50 0	unit 1
ra 2	uda50 0	unit 2
ra 3	uda50 0 unit 3
dz11 0	ub 0	reg 0760100	vec 0300
dz11 1	ub 0	reg 0760110	vec 0310
dz11 2	ub 0	reg 0760120	vec 0320
dz11 3	ub 0	reg 0760130	vec 0330
dk 0	ub 0	reg 0767770	vec 0540
# ni1010a 0 ub 0	reg 0767360	vec 0350
kmc11b 0 ub 0	reg 0760200	vec 0400
kdi	1
drum	0
console	0
mem	0
stdio	0

ttyld	32
mesgld	256
rmesgld	0
cmcld	0
unixpld	0
dkpld 128
connld	0

regfs	10
procfs	0
msfs	0
netafs	0
errfs	0
netbfs	0
pipefs	0
