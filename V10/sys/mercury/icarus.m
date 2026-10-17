# 630qz-a2 (wf53805487)
root	regfs	ra	0
swap	ra	001	21120
swap	ra	011	21120
dump	uddump	0	10560	21120	# dump to ra01


uvqbus 0	voff 0x200
#uda50 1	ub 0	reg 0772150	vec 0254		# rqdx3-aa
#ra 0	uda50 0	unit 0					# rx50-aa
#ra 1	uda50 0	unit 1
#ra 2	uda50 0	unit 2

#uda50 0	ub 0	reg 0760334	vec 0260		# emulex qd35
#ra 3	uda50 1	unit 0					# fuji 2351
#ra 4	uda50 1	unit 1					# fuji 2351
#ra 5	uda50 1	unit 2                                  # fuji 2351
#ra 6	uda50 1	unit 3                                  # fuji 2351

uda50 0	ub 0	reg 0772150	vec 0254		# emulex qd35
ra 0	uda50 0	unit 0					# fuji 2351
ra 1	uda50 0	unit 1					# fuji 2351
ra 2	uda50 0	unit 2                                  # fuji 2351
ra 3	uda50 0	unit 3                                  # fuji 2351

uda50 1	ub 0	reg 0760334	vec 0260		# rqdx3-aa
ra 4	uda50 1	unit 0					# rx50-aa
ra 5	uda50 1	unit 1
ra 6	uda50 1	unit 2

uda50 2 ub 0	reg 0774500	vec 0340		#emulex UC07
ta 0 	uda50 0 unit 0

dk 0	ub 0	reg 0767770	vec 0300		# drv11

deqna 0	ub 0	reg 0774440	vec 0274		# dequna-m

drum	0
console	0
mem	0						# 5 Mbytes
stdio	0
ip	4
udp	16
tcp	64
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

regfs	16
procfs	0
msfs	0
errfs	0
netbfs	0
pipefs	0
