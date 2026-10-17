/ rk06 disk driver

hkcs1 = 177440	/ control & status 1
hkda  = 177446	/ desired track/sector address
hkcs2 = 177450	/ control & status 2
hkdc  = 177460	/ desired cylinder

ack = 03	/ pack acknowledge
clear = 040	/ subsystem clear

	inc	$-1	/ first time ?? dirty but saves space
	bne	1f
	mov	$clear,*$hkcs2
	mov	$ack,*$hkcs1
0:
	tstb	*$hkcs1
	bpl	0b		/ wait for acknowledge to complete
1:
	mov	dska,r1
	clr	r0
	div	$22.,r0		/ r0 = sectors r1 = blocks
	mov	r1,-(sp)
	mov	r0,r1
	clr	r0
	div	$3.,r0		/ r0 = cylinder r1 = track
	bisb	r1,1(sp)
	mov	r0,*$hkdc	/ cylinder wanted
	mov	$hkda,r1
	mov	(sp)+,(r1)	/ track & sector wanted
	mov	ba,-(r1)	/ bus address
	mov	wc,-(r1)	/ word count
	mov	$iocom,-(r1)
1:
	tstb	(r1)
	bpl	1b		/ wait for iocom to complete
	rts	pc
