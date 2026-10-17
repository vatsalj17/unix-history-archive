root	regfs	hp	0
swap	hp	001	21120
swap	hp	011	21120
dump	hpdump	0	10560	21120	# eag48 hp01

mba 0	bus 0	tr 4
hp 0	mb 0	drive 0
hp 1	mb 0	drive 1
hp 2	mb 0	drive 2
hp 3	mb 0	drive 3
hp 4	mb 0	drive 4
hp 5	mb 0	drive 5
hp 6	mb 0	drive 6
hp 7	mb 0	drive 7

mba 1	bus 0	tr 5
tm78 0	mb 1	drive 0
tu78 0	ctl 0	unit 0

dw750 0	bus 0	tr 8	voff 0x200
uda50 0	ub 0	reg 0172150	vec 0254
ra 0	uda50 0	unit 0
dz11 0	ub 0	reg 0160100	vec 0300
kmc11b 0 ub 0	reg 0160200	vec 0400
ni1010a 0 ub 0	reg 0164000	vec 0340
kdi 1
drum 0
console 0
mem 0
stdio 0

ttyld 48
nttyld 32
mesgld 64
rmesgld 0
cmcld 0
connld 0

regfs	30
procfs	0
msfs	0
netafs	0
errfs	0
netbfs	0
pipefs	0

ip	4
udp	16
tcp	32
arp	64
ipld	0
udpld	0
tcpld	0

blkbuf	20
