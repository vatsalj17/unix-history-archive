# vs3200 (88a79327x)

root	regfs	ra	0100
swap	ra	001	20480
swap	ra	011	20480
swap	ra	021	20480
swap	ra	031	20480
dump	uddump	0	10240	20480	# dump to ra01

uvqbus 0	voff 0x200

uda50 0	ub 0	reg 0772150	vec 0254		# kda50-qa	
ra 0	uda50 0	unit 0					# ra81-aa (cx-82277)
ra 1	uda50 0	unit 1					# ra81-aa (cx-87468)
ra 2	uda50 0	unit 2
ra 3	uda50 0	unit 3

uda50 1	ub 0	reg 0760334	vec 0260		# rqdx3-aa
ra 4	uda50 1	unit 0					# rx50-aa
ra 5	uda50 1	unit 1
ra 6	uda50 1	unit 2

uda50 2	ub 0	reg 0774500	vec 0270		# tqk70-aa
ta 0	uda50 2	unit 0					# tk70-aa

dk 0	ub 0	reg 0767770	vec 0300		# drv11

deqna 0	ub 0	reg 0774440	vec 0274		# dequna-m

drum	0
console	0
mem	0						# 16 Mbytes
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
bufld	4

regfs	10
procfs	0
msfs	0
netafs	0
errfs	0
netbfs	0
pipefs	0
