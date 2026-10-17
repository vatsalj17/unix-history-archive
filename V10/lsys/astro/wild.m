root	regfs	ra	0100
swap	ra	005	110808
swap	ra	021	20480
swap	ra	01	20480
dump	uddump	2	10240	20480	# dump to ra21

dw750 0	bus 0	tr 8	voff 0x200
uda50 0	ub 0	reg 0772150	vec 0774
#uda50 1	ub 0	reg 0772154	vec 0770	# us design
uda50 1	ub 0	reg 0760334	vec 0770	# td viking
uda50 2	ub 0	reg 0772160	vec 0764
ra 0	uda50 0	unit 0
ra 1	uda50 0	unit 1
ra 2	uda50 0	unit 2
ra 3	uda50 0	unit 3
ra 4	uda50 1	unit 0		# usdesign this should be a 4
ra 5	uda50 1	unit 1		# usdesign this should be a 5
ra 6	uda50 2	unit 6
ra 7	uda50 2 unit 7
dz11 0	ub 0	reg 0760100	vec 0300
dz11 1	ub 0	reg 0760110	vec 0310
ni1010a 0 ub 0	reg 0164000	vec 0340
vplot 0 ub 0	reg 0777514	vec 0200
kmc11b 0 ub 0	reg 0160200	vec 0600
scsi 0 ub 0	reg 0763100	vec 0700
kdi	1
drum	0
console	0
mem	0
stdio	0

ttyld	32
mesgld	256
rmesgld	0
cmcld	0
connld	0

regfs	15
procfs	0
msfs	0
netafs	0
errfs	0
netbfs	0
pipefs	0

#
# tcp/ip stuff
#
ip	32
udp	16
tcp	32
arp	128
ipld	0
udpld	0
bufld	4
tcpld	0
blkbuf	20
