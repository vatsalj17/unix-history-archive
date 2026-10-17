root	regfs	ra	0100
swap	ra	01	20480
swap	ra	011	20480
swap	ra	021	20480
swap	ra	031	20480
dump	uddump	1	10240	20480	# dump to ra11

mba 0	bus 0	tr 4
tm78 0	mb 0 drive 0
tu78 0	ctl 0 unit 0
dw750 0	bus 0	tr 8	voff 0x200
uda50 0	ub 0	reg 0772150	vec 0254
ra 0	uda50 0	unit 0
ra 1	uda50 0	unit 1
ra 2	uda50 0	unit 2
ra 3	uda50 0 unit 3
dz11 0	ub 0	reg 0760100	vec 0300
kmc11b 0 ub 0	reg 0160200	vec 0600
kdi	1
drum	0
console	0
mem	0
stdio	0

ttyld	64
mesgld	256
rmesgld	0
cmcld	0
connld	0

regfs	30
procfs	0
msfs	0
netafs	0
errfs	0
netbfs	0
pipefs	0
#
# internet stuff
#
ip	32
udp	32
tcp	64
arp	64
ipld	0
udpld	0
tcpld	0
blkbuf	20
