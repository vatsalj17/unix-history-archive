# 11750-fa (82f37313x)
# fp750

root	regfs	hp	0
swap	hp	001	21120
swap	hp	011	21120
dump	hpdump	1	10560	21120	# eag48	hp11

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
tm78 0	mb 1	drive 0					# rh750
tu78 0	ctl 0	unit 0

dw750 0	bus 0	tr 8	voff 0x200

#uda50 0	ub 0	reg 0172150	vec 0254		# uda50-a
#ra 0	uda50 0	unit 0

dz11 0	ub 0	reg 0160100	vec 0300		# dz11-a

kmc11b 0	ub 0	reg 0160200	vec 0400	# kmc-11b
#kmc11b 0	ub 0	reg 0160300	vec 0600	# kmc-11b

ni1010a 0	ub 0	reg 0164000	vec 0340	# ethernet controller

kdi	1
drum	0
console	0						# la38 (pna0936)
mem	0						# ms750-ca 5 Mbytes + 3 Mbytes
stdio	0
ip	4
udp	16
tcp	64
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

regfs	30
procfs	0
msfs	0
errfs	0
netbfs	0
pipefs	0

block	1200
blkbuf	30
