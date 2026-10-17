root	regfs	ra	0100
swap	ra	01	20480
# swap	ra	011	20480
dump	uddump	1	10240	20480	# dump to ra11

dw750 0	bus 0	tr 8	voff 0x200
uda50 0	ub 0	reg 0772150	vec 0254
ra 0	uda50 0	unit 0
ra 1	uda50 0	unit 1
ra 2	uda50 0	unit 2
ra 3	uda50 0 unit 3
dz11 0	ub 0	reg 0760100	vec 0300
kmc11b 0 ub 0	reg 0760200	vec 0600
ilan 0	ub 0	reg 0764000	vec 0750

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

#
# tcp/ip stuff
#
ip	32
udp	16
tcp	32
arp	128
ipld	0
udpld	0
tcpld	0

regfs	24
procfs	0
msfs	0
netafs	0
netbfs	0
pipefs	0
errfs	0

blkbuf	40
