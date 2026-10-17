# 11750-ca (83037152a)
# fp750

root	regfs	ra	0100
swap	hp	001	21120
swap	hp	011	21120
dump	hpdump	1	10560	21120	# eag48 hp11

mba 0	bus 0	tr 4
hp 0	mb 0	drive 0
hp 1	mb 0	drive 1
hp 2	mb 0	drive 2
hp 3	mb 0	drive 3
hp 8	mb 0	drive 4
hp 9	mb 0	drive 5
hp 10	mb 0	drive 6
hp 11	mb 0	drive 7

mba 1	bus 0	tr 5
tm78 0	mb 1	drive 0					# tu78-ab
tu78 0	ctl 0	unit 0					# rh750-f

mba 2	bus 0	tr 6
hp 4	mb 2	drive 0
hp 5	mb 2	drive 1
hp 6	mb 2	drive 2
hp 7	mb 2	drive 3

dw750 0	bus 0	tr 8	voff 0x200

uda50 0	ub 0	reg 0172150	vec 0254		# uda50-a
ra 0	uda50 0	unit 0					# ra81-a (cx-90276)
ra 1	uda50 0	unit 1					# ra81-a (cx-20600)
ra 2	uda50 0	unit 2					# ra81-a (cx-70233)
ra 3	uda50 0	unit 3					# ra81-a (cx-28880)

uda50 1	ub 0	reg 0172160	vec 0260		# uda50-a
ra 4	uda50 1	unit 0					# ra81-a (cx-20439)
ra 5	uda50 1	unit 1					# ra81-a (cx-24879)
ra 6	uda50 1	unit 2					# ra81-a (cx-a28414)
ra 7	uda50 1	unit 3					# ra81-a (cx-09074)


dz11 0	ub 0	reg 0160100	vec 0300		# dz11-a

kmc11b 0	ub 0	reg 0160300	vec 0600	# kmc11-b

ni1010a 0	ub 0	reg 0164000	vec 0340	# ethernet controller

kdi	1
drum	0
console	0						# la100-ba (pnc9573)
mem	0						# ms750-ca 2 Mbytes + 6 Mbytes
stdio	0
ip	4
udp	16
tcp	32
arp	64

ttyld	48
nttyld	32
mesgld	64
rmesgld	0
unixpld	0
connld	0
ipld	0
udpld	0
tcpld	0
bufld	4

regfs	40
procfs	0
msfs	0
netafs	0
errfs	0
netbfs	0
pipefs	0

blkbuf	20
queue	2000
stream	512
block	1000
