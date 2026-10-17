/ format RK06 disk packs
/ written since you can't trust DEC shitty one
/
/		ian j
/		UNSW	may '79
/

hkcs1 = 177440	/ control & status 1
hkwc  = 177442	/ word count
hkba  = 177444	/ bus address
hkda  = 177446	/ track and sector
hkcs2 = 177450	/ control & status 2
hkdc  = 177460	/ desired cylinder

zero  = 2000	/ a word of zeros
table = 2002	/ build sector header here

ack   = 003	/ pack acknowledge
whd   = 027	/ write header
write = 023	/ write data
clear = 040	/ subsystem clear
bai   = 020	/ bus increment inhibit

	5			/ reset
	mov	$140000,sp	/ decent stack
	clr	*$zero		/ really a zero now
	jsr	pc,mesg
		<\n\n    RK06 disk formatter\n\n\0>; .even
	jsr	pc,mesg
		<ready drive 0 and type y\n\0>; .even
	jsr	pc,getc
	mov	r0,-(sp)
	mov	$'\n,r0
	jsr	pc,putc
	cmp	(sp)+,$'y
	beq	1f
	jsr	pc,mesg
		<\nNo formatting done\n\n\0>; .even
	rts	pc
1:
	mov	$clear,*$hkcs2
	mov	$ack,*$hkcs1
1:
	tstb	*$hkcs1
	bge	1b		/ wait for acknowledge

	mov	$1233.,r4	/ total tracks on an rp06 pack
2:

/ calculate cylinder and track address of next track to be done

	mov	r4,r3
	dec	r3		/ track address
	clr	r2
	div	$3,r2		/ r2 = cyl r3 = trk

/ set up table of sector headers

	mov	$22.,r0
	mov	$table,r1
	mov	r3,r5
	ash	$5,r5		/ position track no for header
	bis	$140000,r5	/ good sectors all
0:
	mov	r2,(r1)+	/ cylinder is first word
	mov	r5,(r1)+	/ sector/trk is second word
	mov	r5,(r1)
	xor	r2,(r1)+	/ check sum
	inc	r5		/ next sector
	sob	r0,0b		/ all 22 sectors thankyou

/ do the track formatting

	swab	r3		/ postion track no in hibyte
	mov	r3,*$hkda	/ want this track
	mov	r2,*$hkdc	/ want this cyl
	mov	$table,*$hkba	/ 22 sector headers live here
	mov	$-66.,*$hkwc	/ 66 words for 22 sector headers
	clr	*$hkcs2		/ drive 0 no BAI
	mov	$whd,*$hkcs1	/ will format 1 track
1:
	tstb	*$hkcs1		/ wait for format to complete
	bge	1b
	tst	*$hkcs1
	blt	2f

/ init the track with all zeros

	mov	$-5632.,*$hkwc
	mov	$zero,*$hkba
	mov	$bai,*$hkcs2
	mov	r3,*$hkda
	mov	r2,*$hkdc
	mov	$write,*$hkcs1
1:
	tstb	*$hkcs1		/ wait for write to complete
	bge	1b
	tst	*$hkcs1
	blt	2f

	dec	r4
	jne	2b		/ do them all

/ formatting complete return to loader

	jsr	pc,mesg
		<\n\nFormatting complete\n\nmachine halted\n\0>; .even
	0
2:
	jsr	pc,mesg
		<hkf: error\n\0>; .even
	rts	pc





/ read and echo character from tty.
/ perform normal cr/lf uc/lc mapping.



tks = 177560
tkb = 177562
getc:
	tstb	*$tks
	bge	getc
	mov	tkb,r0
	bic	$!177,r0
	cmp	r0,$'A
	blo	1f
	cmp	r0,$'Z
	bhi	1f
	add	$'a-'A,r0
1:
	cmp	r0,$'\r
	bne	putc
	mov	$'\n,r0

/ put a character on the tty.
/ also performs delay.
tps = 177564
tpb = 177566
putc:
	cmp	r0,$'\n
	bne	1f
	mov	$'\r,r0
	jsr	pc,putc
	mov	$'\n,r0
1:
	tstb	tps
	bpl	1b
	mov	r0,tpb
	rts	pc

/ write a string to tty
/ jsr pc, mesg; <string\0>; .even
mesg:
	movb	*(sp),r0
	beq	1f
	jsr	pc,putc
	inc	(sp)
	br	mesg
1:
	add	$2,(sp)
	bic	$1,(sp)
	rts	pc
