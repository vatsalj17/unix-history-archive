root	regfs	ra	0100
swap	ra	003	249848
swap	ra	013	249848
swap	ra	023	249848
dump	bddump	0x2401	530416	249848	# ra14 == crash dumps

bia 2	bi 2	node 2	voff 0x400

kdb50 0	bi 2	node 4	vec 0x10
kdb50 1 bi 3	node 4	vec 0x18
ra 0	kdb50 0	unit 0
ra 1	kdb50 0	unit 1
ra 2	kdb50 1	unit 2
ra 3	kdb50 1	unit 3
ra 4	kdb50 0 unit 4
ra 5	kdb50 0 unit 5
ra 6	kdb50 1 unit 6
ra 7	kdb50 1 unit 7
debna 0 bi 2	node 7	vec 0x40
dwbua 0	bi 2	node 0	vec 0x20 voff 0x200	
kmc11b 0 ub 0	reg 0160200 vec 0600
kmc11b 1 ub 0	reg 0160210 vec 0610
dz11 0	ub 0	reg 0160100 vec 0300
cure 0 ub 0	reg 0176070 vec 0210
rcure 0 ub 0	reg 0176070
dwbua 2	bi 2	node 6 vec 0x30 voff 0xa00	# klesi-b
uda50 0	ub 2	reg 0774500 vec 0300
ta 0	uda50 0	unit 0
uda50 3 ub 0	reg 0160404 vec 0310
ta 1 	uda50 3 unit 0

bia 3	bi 3	node 2	voff 0x600
dwbua 1	bi 3	node 0	vec 0x20 voff 0x800

# methei
om 0 ub 1 reg 0772410 vec 0124
om 1 ub 1 reg 0772430 vec 0134
om 2 ub 1 reg 0772450 vec 0114
om 3 ub 1 reg 0772470 vec 0104
om 4 ub 1 reg 0772510 vec 0504
om 5 ub 1 reg 0772530 vec 0514
om 6 ub 1 reg 0772550 vec 0524
om 7 ub 1 reg 0772570 vec 0534
# graphic devices
im 0 ub 1 reg 0772670 vec 0164
gpib 0 ub 1 reg 0767710 vec 0350
rco 0 ub 1 reg 0774000 vec 0150

kdi	2
drum	0
fineclock 0
console	0
mem	0
stdio	0

ttyld	128
mesgld	300
rmesgld	0
cmcld	0
unixpld	0
connld	0
dkpld	255
cdkpld	0

regfs	48
procfs	0
msfs	0
netafs	0
netbfs	0
errfs	0
pipefs	0

ip	4
udp	16
tcp	64
arp	128

ipld  0
udpld 0
bufld	4
tcpld 0
