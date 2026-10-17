#
# alice
#

root	regfs	ra	0100
swap	ra	01	20480
swap	ra	011	20480
swap	ra	021	20480
swap	ra	031	20480
swap	ra	041	20480
swap	ra	051	20480
dump	uddump	0x1001	10240	20480	# dump to ra11: dw1, first uda addr

ms780 0	bus 0	tr 1
ms780 1	bus 0	tr 2

# mba 0	bus 0	tr 8	# rm03

mba 1	bus 0	tr 9
tm78 0	mb 1	drive 0
tu78 0	ctl 0 unit 0

dw780 0	bus 0	tr 3	voff 0x200
dw780 1	bus 0	tr 4	voff 0x400
#
# uda50 addresses are annoyingly nonstandard
#
uda50 0	ub 0	reg 0772160	vec 0154
uda50 1 ub 1	reg 0772150	vec 0160
ra 0	uda50 0	unit 0
ra 1	uda50 1	unit 1
ra 2	uda50 0	unit 2
ra 3	uda50 1 unit 3
ra 4	uda50 0 unit 4
ra 5	uda50 1 unit 5
# dz4 dz5 sic
dz11 0	ub 1	reg 0760100	vec 0320
dz11 4	ub 0	reg 0760140	vec 0360
dz11 5	ub 0	reg 0760150	vec 0370
dn11 0	ub 1	reg 0775200	vec 0430
drbit 0	ub 1	reg 0767570
ni1010a 0 ub 1	reg 0764000	vec 0350
dk 0	ub 0	reg 0767600	vec 0300
kmc11b 0 ub 0	reg 0760200	vec 0600
kdi	1
drum	0
console	0
starcons 0
mem	0
stdio	0

ttyld	128
nttyld	32
mesgld	256
rmesgld	0
cmcld	0
dkpld	256
cdkpld	0
connld	0
bufld	32

regfs	20
procfs	0
msfs	0
netafs	0
errfs	0
netbfs	0
pipefs	0

#
# internet stuff
#
ip	4
udp	16
tcp	96
arp	4
ipld	0
udpld	0
tcpld	0
