root	regfs	ra	0100
swap	ra	004	249848
swap	ra	024	249848
swap	ra	034	249848
dump	bddump	0x0100	530416	249848	# ra04 == crash dumps


# bi - port 0
dwbua 0	bi 0	node 0	vec 0x20 voff 0x200
kdb50 0	bi 0	node 1	vec 0x10
bia 0	bi 0	node 2	voff 0x400
kdb50 1 bi 0	node 3	vec 0x18
dwbua 1	bi 0	node 4	vec 0x30 voff 0x800	# klesi-b
debna 0 bi 0	node 13 vec 0x40

# ub 0 - unibus
dk 0	ub 0	reg 0767770 vec 0400
uda50 1	ub 0	reg 0172150 vec 0260		# td viking
ra 16	uda50 1 unit 0
ra 17	uda50 1 unit 2
ra 18	uda50 1 unit 4
uda50 2	ub 0	reg 0160404 vec 0270		# td viking
ta 1	uda50 2	unit 0
ta 2	uda50 2	unit 1
#uda50 1	ub 0	reg 0172154 vec 0300	# us design
#ra 7	uda50 1 unit 4

# ub 1 - tu81+
uda50 0	ub 1	reg 0774500 vec 0300
ta 0	uda50 0	unit 0

# kdb50 0 - ra81/ra90
ra 0	kdb50 0	unit 0
ra 2	kdb50 0	unit 2
ra 4	kdb50 0 unit 4
# kdb50 1 - ra81/ra90
ra 1	kdb50 1	unit 1
ra 3	kdb50 1	unit 3

nautcons 0
drum	0
console	0
mem	0
stdio	0
ip	32
udp	16
tcp	64
arp	128

ttyld	128
mesgld	256
rmesgld	0
cmcld	0
unixpld	0
dkpld	96
cdkpld	0
connld	0
ipld	0
udpld	0
bufld	4
tcpld	0

regfs	24
procfs	0
msfs	0
netafs	0
netbfs	0
pipefs	0
errfs	0
file	1536
queue	3072
maxdsize 819200
maxssize 819200
