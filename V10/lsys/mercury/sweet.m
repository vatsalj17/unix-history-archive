# 11750-fa (84039526x)
# fp750

root	regfs	ra	0100
swap	ra	001	20480
swap	ra	011	20480
swap	ra	021	20480
swap	ra	031	20480
dump	uddump	0	10240	20480	# dump to ra01

dw750 0	bus 0	tr 8	voff 0x200

uda50 0	ub 0	reg 0772150	vec 0254		# uda50
ra 0	uda50 0	unit 0					# ra81-aa (cx-90237)
ra 1	uda50 0	unit 1					# ra81-aa (cx-34087)
ra 2	uda50 0	unit 2					# ra81-aa (cx-35068)
ra 3	uda50 0	unit 3					# ra81-aa (cx-34654)

uda50 1	ub 0	reg 0760334	vec 0260		# worm controller
ra 4	uda50 1	unit 0
ra 5	uda50 1	unit 1

kmc11b 0	ub 0	reg 0160300	vec 0600	# kmc-11b

ni1010a 0	ub 0	reg 0764000	vec 0340	# ethernet controller

scsi 0	ub 0	reg 0772154	vec 0700		# jukebox controller

kdi	1
drum	0
console	0
mem	0						# ms750-ca 8 Mbytes
stdio	0
ip	4
udp	16
tcp	32
arp	128

ttyld	64
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
netafs	0
errfs	0
netbfs	0
pipefs	0
