root	regfs	ra	0100
swap	ra	003	249848
swap	ra	013	249848
swap	ra	023	249848
dump	bddump	0x3403	280568	249848	# ra33 == crash dumps

bia 2	bi 2	node 2	voff 0x400

dwbua 0	bi 2	node 4	vec 0x30	voff 0xa00	# klesi-b
uda50 0	ub 0	reg 0774500	vec 0310
ta 0	uda50 0	unit 0

#debna 0	bi 2	node 7	vec 0xxx	voff 0xxx
#drb32 0	bi 2	node 3	vec 0xxx	voff 0xxx

kdb50 0	bi 2	node 3	vec 0x10
ra 5	kdb50 0	unit 0
ra 6	kdb50 0	unit 1
ra 7	kdb50 0	unit 2

bia 3	bi 3	node 2	voff 0x600

kdb50 1	bi 3	node 4	vec 0x10
ra 0	kdb50 1	unit 0
ra 1	kdb50 1	unit 1
ra 2	kdb50 1	unit 2
ra 3	kdb50 1	unit 3

dwbua 1	bi 3	node 0	vec 0x20	voff 0x200	

kmc11b 0	ub 1	reg 0760130	vec 0320

om 0	ub 1	reg 0760300	vec 0330

uda50 1	ub 1	reg 0772154	vec 0420
ra 4	uda50 1	unit 4

dz11 0	ub 1	reg 0760100	vec 0300
dz11 1	ub 1	reg 0760110	vec 0310

kdi	1
drum	0
console	0
mem	0
stdio	0
ip	32
udp	16
tcp	32
arp	128

ttyld	128
mesgld	300
rmesgld	0
unixpld	0
connld	0
ipld	0
udpld	0
tcpld	0
bufld	32

regfs	30
procfs	0
msfs	0
netafs	0
netbfs	0
errfs	0
pipefs	0
