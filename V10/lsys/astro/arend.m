root	regfs	ra	0100		# 4k filesys
swap	ra	01	20480
swap	ra	021	20480
dump	uddump	1	10240	20480	# dump to ra11

dw750	0 bus 0	tr 8 voff 0x200
uda50	0 ub 0 reg 0772150 vec 0774
ra	0 uda50 0 unit 0
ra	1 uda50 0 unit 1
ra	2 uda50 0 unit 2
ra	3 uda50 0 unit 3
uda50 1	ub 0	reg 0772154	vec 0770	# us design
ra	4 uda50 1 unit 4
dz11	0 ub 0 reg 0760100 vec 0300
kmc11b	0 ub 0 reg 0760200 vec 0400
# om	0 ub 0 reg 0772410 vec 0124
im	0 ub 0 reg 0772470 vec 0174
mg	0 ub 0 reg 0767720 vec 0340
ni1010a	0 ub 0 reg 0764000	vec 0350

kdi	1
drum	0
console	0
mem	0
stdio	0

ttyld	32
nttyld	16
mesgld	256
rmesgld	0
cmcld	0
unixpld	0
connld	0

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
udp	32
tcp	64
arp	64
ipld	0
udpld	0
bufld	4
tcpld	0
#
