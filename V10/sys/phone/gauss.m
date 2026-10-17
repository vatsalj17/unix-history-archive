root	regfs	ra	0100
swap	ra	003	249848
swap	ra	013	249848
dump	bddump	0x2501	530416	249848	# ra14 == crash dumps

bia 2	bi 2	node 2	voff 0x400
bia 3	bi 3	node 2	voff 0x600
debna 0 bi 2	node 7 vec 0x40

kdb50 0	bi 2	node 5	vec 0x10
kdb50 1 bi 3	node 5	vec 0x18
kdb50 2 bi 3	node 9	vec 0x20
ra 0	kdb50 0	unit 0
ra 1	kdb50 0	unit 1
ra 2	kdb50 1	unit 2
ra 3	kdb50 1	unit 3
ra 4	kdb50 1 unit 4
ra 5	kdb50 1 unit 5
ra 6	kdb50 0 unit 6
ra 7	kdb50 0 unit 7
ra 20	kdb50 2 unit 20
ra 21	kdb50 2 unit 21
ra 22	kdb50 2 unit 22
ra 23	kdb50 2 unit 23
dwbua 1	bi 3	node 6 vec 0x30 voff 0x800	# klesi-b
uda50 0	ub 1	reg 0774500 vec 0300
ta 0	uda50 0	unit 0
dwbua 0	bi 2	node 0	vec 0x20 voff 0x200	
dk 0	ub 0	reg 0167770 vec 0400
kmc11b 0 ub 0	reg 0160200 vec 0600
kmc11b 1 ub 0	reg 0160210 vec 0610
uda50 1	ub 0	reg 0172150 vec 0270		# td viking
ra 16	uda50 1 unit 0
ra 17	uda50 1 unit 2
uda50 3 ub 0	reg 0160334 vec 0260		# td viking
ra 18	uda50 3 unit 0
ra 19	uda50 3 unit 2
#uda50 1	ub 0	reg 0172154 vec 0300	# us design
#ra 16	uda50 1 unit 4
uda50 2	ub 0	reg 0160404 vec 0310
ta 1	uda50 2	unit 0
ta 2	uda50 2	unit 1
uda50 4	ub 0	reg 0160444 vec 0250
ta 3	uda50 4	unit 0
ta 4	uda50 4	unit 1

nautcons 0
kdi	2
drum	0
console	0
mem	0
stdio	0
ip	32
udp	16
tcp	128
arp	128

ttyld	480
mesgld	512
rmesgld	0
cmcld	0
unixpld	96
connld	0
ipld  0
udpld 0
bufld	8
tcpld 0

regfs	24
procfs	0
msfs	0
netafs	0
netbfs	0
pipefs	0
errfs	0

blkbuf	40
proc	1000
stream	1200
file	1536
queue	4096
inode	2000
maxdsize 819200
maxssize 819200
