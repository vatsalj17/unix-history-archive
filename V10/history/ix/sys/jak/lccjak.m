root	regfs	ra	0100
swap	ra	01	20480
swap	ra	011	20480
dump	uddump	1	10240	20480	# dump to ra11

dw750 0	bus 0	tr 8	voff 0x200
uda50 0	ub 0	reg 0772150	vec 0254
ra 0	uda50 0	unit 0
ra 1	uda50 0	unit 1
uda50 1	ub 0	reg 0772154	vec 0770	# us design
ra 4	uda50 1	unit 4		# usdesign this should be a 4
ra 5	uda50 1	unit 5		# usdesign this should be a 5
kmc11b 0 ub 0	reg 0160200 vec 0600
kdi	1
drum	0
console	0
mem	0
stdio	0
lg	16

ttyld	32
mesgld	300
rmesgld	0
cmcld	0
connld	0

regfs	10
procfs	0
msfs	0
errfs	0
netbfs	0
pipefs	0

fildes	1000
jlabel	1000
slabel	350
slabix	701
