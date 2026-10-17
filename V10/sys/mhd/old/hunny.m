# 855bb-ce (88a79317x)
# ba11-aw

root	regfs	ra	0100
swap	ra	003	249848
swap	ra	013	249848
swap	ra	023	249848
dump	bddump	0x3403	280568	249848	# ra33 == crash dumps

bia 2	bi 2	node 2	voff 0x400

dwbua 0	bi 2	node 4	vec 0x30	voff 0xa00	# klesi-b
uda50 0	ub 0	reg 0774500	vec 0310		# uda50-aa	
ta 0	uda50 0	unit 0					# tu81-e-ba (pi81115411)

debna 0	bi 2	node 7	vec 0x40			# debna??

#ZZ 0	bi 2	node 3	vec 0xxx	voff 0xxx	# dmb32??

kdb50 0	bi 2	node 3	vec 0x10			# kdb50-b
ra 5	kdb50 0	unit 0					# ra81-aa (cx-90294)
ra 6	kdb50 0	unit 1					# ra81-aa (cx-10431)
ra 7	kdb50 0	unit 2

bia 3	bi 3	node 2	voff 0x600

kdb50 1	bi 3	node 4	vec 0x10			# kdb50-b
ra 0	kdb50 1	unit 0					# ra81-aa (cx-a37480)
ra 1	kdb50 1	unit 1					# ra81-aa (cx-26837)
ra 2	kdb50 1	unit 2					# ra81-aa (cx-27089)
ra 3	kdb50 1	unit 3					# ra81-aa (cx-26828)

dwbua 1	bi 3	node 0	vec 0x20	voff 0x200	# dwbua-ca

kmc11b 0	ub 1	reg 0760130	vec 0320	# kmc11-b

om 0	ub 1	reg 0760300	vec 0330		# ??

uda50 1	ub 1	reg 0772154	vec 0420		# worm controller
ra 4	uda50 1	unit 4

dz11 0	ub 1	reg 0760100	vec 0300		# dz11-a
dz11 1	ub 1	reg 0760110	vec 0310		# dz11-a

kdi	1
drum	0
console	0
mem	0						# 138 Mbytes
stdio	0
ip	4
udp	16
tcp	128
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
netbfs	0
errfs	0
pipefs	0
