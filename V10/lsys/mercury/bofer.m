# 630q4-ba123 (86s63510k)

root	regfs	ra	0100
swap	ra	001	20480
swap	ra	011	20480
swap	ra	021	20480
swap	ra	031	20480
dump	uddump	0	10240	20480	# dump to ra01

uvqbus 0	voff 0x200

uda50 0	ub 0	reg 0772150	vec 0254		# rqdx3-aa
ra 0	uda50 0	unit 0					# rd54-aa
ra 1	uda50 0	unit 1					# rd54-aa
ra 2	uda50 0	unit 2
ra 3	uda50 0	unit 3

uda50 1	ub 0	reg 0774500	vec 0270		# tqk50-aa
ta 0	uda50 1	unit 0					# tk50-aa

dk 0	ub 0	reg 0767770	vec 0300		# drv11

deqna 0	ub 0	reg 0774440	vec 0274		# dequna-m

dhv11 0	ub 0	reg 0760440	vec 0310		# dhv11-m
dhv11 1	ub 0	reg 0760460	vec 0320		# dhv11-m

drum	0
console	0
mem	0						# 9 Mbytes
stdio	0
ip	4
udp	16
tcp	32
arp	128

ttyld	32
mesgld	256
rmesgld	0
dkpld	96
unixpld	0
connld	0
cdkpld	0
ipld	0
udpld	0
tcpld	0
bufld	32

regfs	10
procfs	0
msfs	0
netafs	0
errfs	0
netbfs	0
pipefs	0
