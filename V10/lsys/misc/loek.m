root	regfs	ra	0	# ra00, 1k filesystem
swap	ra	01	20480
swap	ra	011	20480
swap	ra	021	20480
swap	ra	031	20480
dump	uddump	1	10240	20480	# dump to ra11

uvqbus 0	voff 0x200
uda50 0	ub 0	reg 0772150 vec 0254
ra 0	uda50 0 unit 0
ra 1	uda50 0 unit 1
ra 2	uda50 0 unit 2
ra 3	uda50 0 unit 3
deqna 0	ub 0	reg 0774440 vec 0300
uda50 1 ub 0	reg 0774500 vec 0270	# tk50
ta 0	uda50 1 unit 0
dhv11 0 ub 0	reg 0760440 vec 0310
dhv11 1	ub 0	reg 0760460 vec 0320
pg 0	ub 0	reg 0774000
pg 1	ub 0	reg 0774040
pg 2	ub 0	reg 0774100

drum 0
console 0
mem 0
stdio 0

ttyld 48
nttyld 24

regfs	10
procfs	0
mesgld	64
netafs	0
errfs	0
netbfs	0
pipefs	0

connld	0
msfs	0

ip	32
udp	32
tcp	128
arp	128
ipld	0
udpld	0
bufld	4
tcpld	0

blkbuf	40	# eh?
