root	regfs	ra	0100		# r70 has 0120
swap	ra	01	20480
swap	ra	011	20480
swap	ra	021	20480
dump	uddump	2	10240	20480	# dump to ra21

uvqbus 0	voff 0x200

uda50	0 ub 0 reg 0772150 vec 0254
ra	0 uda50 0 unit 0
ra	1 uda50 0 unit 1
ra	2 uda50 0 unit 2
ra	3 uda50 0 unit 3
uda50	1 ub 0 reg 0760334 vec 0770
ra	4 uda50 1 unit 0
uda50	3 ub 0 reg 0772160 vec 0764
ra	6 uda50 3 unit 4
ra	7 uda50 3 unit 5
##ni1010a	0 ub 0 reg 0164000 vec 0340
dk	0 ub 0 reg 0767770 vec 0300

pg	0 ub 0 reg 0770000 vec 0710

##kdi	1
drum	0
console	0
mem	0
stdio	0

ttyld	48
mesgld	128
rmesgld	0
cmcld	0
unixpld	0
connld	0
dkpld	96

regfs	15
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
tcp	64
arp	64
ipld	0
udpld	0
bufld	4
tcpld	0
