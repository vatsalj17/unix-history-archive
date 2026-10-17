/ rp04 disk driver

hpcs1 = 176700
hpda  = 176706
hpcs2 = 176710
hpof  = 176732
hpca  = 176734

fmt22 = 010000
preset = 021
clear = 040

		/ dirty, but i need the space
	inc	$-1
	bne	1f
	mov	$clear,*$hpcs2
	mov	$preset,*$hpcs1
	mov	$fmt22,*$hpof
1:
	mov	dska,r1
	clr	r0
	div	$22.,r0
	mov	r1,-(sp)
	mov	r0,r1
	clr	r0
	div	$19.,r0
	bisb	r1,1(sp)
	add	dskc,r0			///// adjust to start cyl of desired file system.
	mov	r0,*$hpca
	mov	$hpda,r1
	mov	(sp)+,(r1)
	mov	ba,-(r1)
	mov	wc,-(r1)
	mov	$iocom,-(r1)
1:
	tstb	(r1)
	bpl	1b
	rts	pc
