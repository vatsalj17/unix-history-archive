root	regfs	ra	00
swap	ra	001	20480
swap	ra	011	20480
swap	ra	021	20480
swap	ra	031	20480
dump	uddump	1	10240	20480	# dump to ra11

uvqbus 0	voff 0x200
uda50 0	ub 0	reg 0772150 vec 0154
ra 0	uda50 0	unit 0
ra 1	uda50 0	unit 1
ra 2	uda50 0	unit 2
ra 3	uda50 0 unit 3
uda50 1	ub 0	reg 0774500 vec 0270
ta 0	uda50 1 unit 0
dk 0	ub 0	reg 0767770 vec 0300
dhv11 0	ub 0	reg 0760440 vec 0310
pg 0	ub 0	reg 0774000
pg 1	ub 0	reg 0774040
pg 2	ub 0	reg 0774100
drum	0
console	0
mem	0
stdio	0

ttyld	48
mesgld	256
rmesgld	0
dkpld 96
cmcld	0
connld	0
unixpld	0
cdkpld	0

regfs	10
procfs	0
msfs	0
netafs	0
errfs	0
netbfs	0
pipefs	0
