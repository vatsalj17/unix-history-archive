/ tap1 -- dec-disk lod/dmp

.globl	_localtime, _end
namep = 0
mode = 2
uid = 4; gid = 5
size0 = 7
size1 = 8
time0 = 10.
time1 = 12.
tapea = 14.
dirsiz = 16.
mdirent = 496.

	mov	(sp),rnarg
	mov	(sp)+,narg
	mov	$cmr,command
	incb	flu
	tst	(sp)+
	cmp	narg,$2
	bge	1f
	mov	$2,narg
	br	3f
1:
	mov	(sp)+,r0
	mov	sp,parg
1:
	movb	(r0)+,r1
	beq	3f
	mov	$swtab,r2
2:
	cmp	r1,(r2)+
	beq	2f
	tst	(r2)+
	bne	2b
	br	useerr
2:
	jsr	pc,*(r2)+
	br	1b
3:
	jsr	pc,optap
	mov	$_end,r4 / string pointer
	jsr	pc,setb
	jmp	*command

optap:
	tstb	flm
	bne	2f
	mov	$616.,tapsiz
	mov	$192.,ndirent
	sys	open; tc; 2
	br	3f
2:
	mov	$-1,tapsiz
	mov	$mdirent,ndirent
	cmp	command,$cmr
	beq	2f
	sys	open; mt; 0
	br	3f
2:
	sys	open; mt; 1
3:
	bes	1f
	mov	r0,fio
	mov	ndirent,r1
	ash	$-3,r1
	mov	r1,ndentd8
	mov	ndirent,r1
	mul	$dirsiz,r1
	add	$dir,r1
	mov	r1,edir
	rts	pc
1:
	jsr	r5,mesg
		<Tape open error\n\0>; .even
	jmp	done

setcom:
	cmp	command,$cmr
	bne	useerr
	mov	(r5)+,command
	rts	r5

noflag:
	mov	(r5)+,r0
	beq	1f
	tstb	(r0)
	beq	noflag
	br	useerr
1:
	rts	r5

useerr:
	jsr	r5,mesg
		<Bad usage\n\0>; .even
	jmp	done

swtab:
	'0; dcof
	'1; dcof
	'2; dcof
	'3; dcof
	'4; dcof
	'5; dcof
	'6; dcof
	'7; dcof
	'c; dcc
	'd; dcd
	'f; dcf
	'i; dci
	'm; dcm
	'r; dcr
	't; dct
	'u; dcu
	'v; dcv
	'w; dcw
	'x; dcx
	 0; 0

dcof:
	movb	r1,tcx
	movb	r1,mtx
	rts	pc

dcc:
	incb	flc
	rts	pc

dcf:
	incb	flf
	rts	pc

dcd:
	jsr	r5,setcom; cmd
	rts	pc

dci:
	incb	fli
	rts	pc

dcm:
	incb	flm
	rts	pc

dcu:
	incb	flu
	jsr	r5,setcom; cmr
	rts	pc

dcr:
	clrb	flu
	jsr	r5,setcom; cmr
	rts	pc

dct:
	jsr	r5,setcom; cmt
	rts	pc

dcv:
	incb	flv
	rts	pc

dcw:
	incb	flw
	rts	pc

dcx:
	jsr	r5,setcom; cmx
	rts	pc

cmd:
	jsr	r5,noflag; flm; flc; flf; 0
	cmp	narg,$2
	bgt	1f
	jmp	useerr
1:
	jsr	pc,rddir
	jsr	r5,gettape; delete
	jsr	pc,wrdir
	br	check

cmr:
	jsr	r5,noflag; 0
	tstb	flc
	bne	1f
	tstb	flm
	bne	1f
	jsr	pc,rddir
	br	2f
1:
	jsr	pc,clrdir
2:
	jsr	pc,getfiles
	jsr	pc,update
	br	check

cmt:
	jsr	r5,noflag; flc; flf; flw; 0
	jsr	pc,rddir
	tstb	flv
	beq	1f
	jsr	r5,mesg
	<   mode    uid gid tapa    size   date    time name\n\0>; .even
1:
	jsr	r5,gettape; taboc
	br	check1

cmx:
	jsr	r5,noflag; flc; flf; 0
	jsr	pc,rddir
	jsr	r5,gettape; xtract
	br	done

check:

check1:
	jsr	pc,usage

done:
	jsr	r5,mesg
		<END\n\0>; .even
	sys	sync
	sys	exit

encode:
	mov	r2,-(sp)
	mov	r4,(r1)
	mov	(r5)+,r2
1:
	movb	(r2),(r4)+
	jsr	pc,setb
	tstb	(r2)+
	bne	1b
	mov	(sp)+,r2
	rts	r5

decode:
	mov	r2,-(sp)
	mov	r1,-(sp)
	mov	(r1),r1
	mov	(r5)+,r2
1:
	movb	(r1)+,(r2)+
	bne	1b
	mov	(sp)+,r1
	mov	(sp)+,r2
	rts	r5

setb:
	mov	r0,-(sp)
	mov	r4,r0
	add	$513.,r0
	cmp	r0,sp
	bhis	2f
	bic	$777,r0
	cmp	r0,0f
	beq	1f
	mov	r0,0f
	sys	break; 0:..
	bes	2f
1:
	mov	(sp)+,r0
	rts	pc

2:
	jsr	r5,mesg
		<Out of core\n\0>; .even
	jmp	done
/ tap2 -- dec-tape lod/dmp

pstr:
	movb	(r1)+,r0
	beq	1f
	jsr	pc,putc
	br	pstr
1:
	rts	pc

mesg:
	movb	(r5)+,r0
	beq	1f
	jsr	pc,putc
	br	mesg
1:
	inc	r5
	bic	$1,r5
	rts	r5

putc:
	movb	r0,ch
	mov	$1,r0
	sys	write; ch; 1
	rts	pc

getc:
	clr	r0
	sys	read; ch; 1
	bes	1f
	tst	r0
	beq	1f
	movb	ch,r0
	rts	pc
1:	jsr	r5,mesg
		<cannot read input\n\0>; .even
	jmp	done

clrdir:
	mov	$dir,r1
	mov	ndirent,r2
1:
	jsr	pc,clrent
	sob	r2,1b
	rts	pc

clrent:
	mov	r1,-(sp)
	add	$dirsiz,(sp)
1:
	clr	(r1)+
	cmp	r1,(sp)
	blo	1b
	tst	(sp)+
	rts	pc

rddir:
	clr	sum
	jsr	pc,clrdir
	clr	r0
	jsr	pc,rseek
	jsr	pc,tread
	mov	tapeb+510.,r0
	beq	1f
	tstb	flm
	beq	1f
	mov	r0,ndirent
1:
	mov	$dir,r1
	mov	ndirent,r2
1:
	bit	$7,r2
	bne	2f
	jsr	pc,tread
	mov	$tapeb,r3
2:
	mov	r1,-(sp)
	mov	r3,-(sp)
	mov	$32.,r0
	clr	-(sp)
2:
	add	(r3)+,(sp)
	sob	r0,2b
	bis	(sp),sum
	tst	(sp)+
	bne	2f
	mov	(sp),r3
	tst	(r3)
	beq	2f
	mov	r3,0f
	jsr	pc,9f
.data
9:
	jsr	r5,encode; 0:..
	rts	pc
.text
	add	$32.,r3
	mov	r1,-(sp)
	add	$dirsiz,(sp)
	tst	(r1)+		/ namep
9:
	mov	(r3)+,(r1)+
	cmp	r1,(sp)
	blo	9b
	tst	(sp)+
2:
	mov	(sp)+,r3
	add	$64.,r3
	mov	(sp)+,r1
	bic	$100000,mode(r1)
	add	$dirsiz,r1
	sob	r2,1b
	tst	sum
	beq	1f
	jsr	r5,mesg
		<Directory checksum\n\0>; .even
	tstb	fli
	bne	1f
	jmp	done
1:
	jsr	pc,bitmap
	rts	pc

wrdir:
	clr	r0
	jsr	pc,wseek
	tstb	flm
	bne	1f
	sys	open; tboot; 0
	bes	3f
	br	2f
1:
	sys	open; mboot; 0
	bes	3f
2:
	mov	r0,r1
	sys	read; tapeb; 512.
	mov	r1,r0
	sys	close
	mov	ndirent,tapeb+510.
3:
	jsr	pc,twrite
	mov	$dir,r1
	mov	ndirent,r2
1:
	bit	$7,r2
	bne	2f
	mov	$256.,r0
	mov	$tapeb,r3
3:
	clr	(r3)+
	sob	r0,3b
	mov	$tapeb,r3
2:
	mov	r3,-(sp)
	tst	(r1)
	beq	2f
	mov	r3,0f
	jsr	pc,9f
.data
9:
	jsr	r5,decode; 0:..
	rts	pc
.text
2:
	add	$32.,r3
	mov	r1,-(sp)
	add	$dirsiz,(sp)
	tst	(r1)+
9:
	mov	(r1)+,(r3)+
	cmp	r1,(sp)
	blo	9b
	tst	(sp)+
	mov	(sp)+,r3
	clr	-(sp)
	mov	$31.,r0
2:
	sub	(r3)+,(sp)
	sob	r0,2b
	mov	(sp)+,(r3)+
	dec	r2
	bit	$7,r2
	bne	1b
	jsr	pc,twrite
	tst	r2
	bne	1b
	rts	pc

tread:
	mov	fio,r0
	sys	read; tapeb; 512.
	bes	trderr
	cmp	r0,$512.
	bne	trderr
1:
	inc	rseeka
	rts	pc

trderr:
	jsr	r5,mesg
		<Tape read error\n\0>; .even
	tstb	fli
	beq	1f
	mov	$tapeb,r0
2:
	clr	(r0)+
	cmp	r0,$tapeb+512.
	blo	2b
	br	1b
1:
	jmp	done

twrite:
	mov	fio,r0
	sys	write; tapeb; 512.
	bes	twrerr
	cmp	r0,$512.
	bne	twrerr
	inc	wseeka
	rts	pc

twrerr:
	jsr	r5,mesg
		<Tape write error\n\0>; .even
	jmp	done

rseek:
	mov	r0,rseeka
	mov	r0,0f
	mov	fio,r0
	sys	0; 9f
.data
9:
	sys	seek; 0:..; 3
.text
	bes	seekerr
	rts	pc

wseek:
	mov	r0,-(sp)
	sub	wseeka,r0
	bge	1f
	neg	r0
1:
	cmp	r0,$25.			/ longest write seek
	ble	1f
	mov	(sp),0f
	beq	2f
	dec	0f
2:
	mov	fio,r0
	sys	0; 9f
.data
9:
	sys	seek; 0:..; 3
.text
	mov	fio,r0
	sys	read; wseeka; 1
1:
	mov	(sp),wseeka
	mov	(sp)+,0f
	mov	fio,r0
	sys	0; 9f
.data
9:
	sys	seek; 0:..; 3
.text
	bes	seekerr
	rts	pc

seekerr:
	jsr	r5,mesg
<Tape seek error\n\0>; .even
	jmp	done

verify:
	movb	(r5)+,0f
	inc	r5
	tstb	flw
	bne	1f
	tstb	flv
	beq	2f
1:
	jsr	pc,9f
.data
9:
	jsr	r5,mesg
		0:<x \0>; .even
	rts	pc
.text
	mov	r1,-(sp)
	mov	$name,r1
	jsr	pc,pstr
	mov	(sp)+,r1
	tstb	flw
	beq	1f
	jsr	r5,mesg
		< \0>
	jsr	pc,getc
	cmp	r0,$'x
	bne	3f
	jsr	pc,getc
	jmp	done
3:
	cmp	r0,$'\n
	beq	3f
	cmp	r0,$'y
	bne	4f
	jsr	pc,getc
	cmp	r0,$'\n
	beq	2f
4:
	jsr	pc,getc
	cmp	r0,$'\n
	bne	4b
	br	1b
1:
	jsr	r5,mesg
		<\n\0>
2:
	tst	(r5)+
3:
	rts	r5

getfiles:
	cmp	narg,$2
	bne	1f
	mov	$".\0,name
	jsr	pc,callout
1:
	cmp	narg,$2
	ble	1f
	dec	narg
	mov	*parg,r1
	add	$2,parg
	mov	$name,r2
2:
	movb	(r1)+,(r2)+
	bne	2b
	jsr	pc,callout
	br	1b
1:
	rts	pc

expand:
	sys	open; name; 0
	bes	fserr
	mov	r0,-(sp)
1:
	mov	(sp),r0
	sys	read; catlb; 16.
	bes	fserr
	tst	r0
	beq	1f
	tst	catlb
	beq	1b
	mov	$name,r0
	mov	$catlb+2,r1
	cmpb	(r1),$'.
	beq	1b
2:
	tstb	(r0)+
	bne	2b
	dec	r0
	mov	r0,-(sp)
	cmpb	-1(r0),$'/
	beq	2f
	movb	$'/,(r0)+
2:
	movb	(r1)+,(r0)+
	bne	2b
	jsr	pc,callout
	clrb	*(sp)+
	br	1b
1:
	mov	(sp)+,r0
	sys	close
	rts	pc

fserr:
	mov	$name,r1
	jsr	pc,pstr
	jsr	r5,mesg
		< -- Cannot open file\n\0>; .even
	jmp	done

callout:
	sys	stat; name; statb
	bes	fserr
	mov	statb+4,r0
	bic	$!60000,r0
	beq	1f
	cmp	r0,$40000
	beq	expand
	rts	pc
1:
	mov	$dir,r1
	clr	-(sp)
1:
	tst	(r1)
	bne	3f
	tst	(sp)
	bne	2f
	mov	r1,(sp)
2:
	add	$dirsiz,r1
	cmp	r1,edir
	blo	1b
	mov	(sp)+,r1
	bne	4f
	jsr	r5,mesg
		<Directory overflow\n\0>; .even
	jmp	done
4:
	jsr	r5,verify; 'a
		rts pc
	jsr	r5,encode; name
	br	2f
3:
	jsr	r5,decode; name1
	mov	$name,r2
	mov	$name1,r3
3:
	cmpb	(r2)+,(r3)
	bne	2b
	tstb	(r3)+
	bne	3b
	tst	(sp)+
	tstb	flu
	beq	3f
	cmp	time0(r1),statb+32.
	blo	3f
	bhi	1f
	cmp	time1(r1),statb+34.
	bhis	1f
3:
	jsr	r5,verify; 'r
		rts pc
2:
	mov	statb+4,mode(r1)
	bis	$100000,mode(r1)
	movb	statb+7,uid(r1)
	movb	statb+8,gid(r1)
	tstb	flf
	beq	2f
	clrb	statb+9.
	clr	statb+10.
2:
	movb	statb+9.,size0(r1)
	mov	statb+10.,size1(r1)
	mov	statb+32.,time0(r1)
	mov	statb+34.,time1(r1)
1:
	rts	pc
/ tap3 -- dec-tape lod/dmp

gettape:
	mov	$dir,r1
	clr	-(sp)
1:
	tst	(r1)
	beq	2f
	jsr	r5,decode; name
	cmp	rnarg,$2
	ble	4f
	mov	$name,r2
	mov	*parg,r3
3:
	tstb	(r3)
	beq	3f
	cmpb	(r2)+,(r3)+
	beq	3b
	br	2f
3:
	tstb	(r2)
	beq	4f
	cmpb	(r2),$'/
	bne	2f
4:
	mov	r1,-(sp)
	jsr	pc,*(r5)
	mov	(sp)+,r1
	inc	(sp)
2:
	add	$dirsiz,r1
	cmp	r1,edir
	blo	1b
	tst	(sp)+
	bne	2f
	cmp	rnarg,$2
	ble	2f
	mov	*parg,r1
	jsr	pc,pstr
	jsr	r5,mesg
		< not found\n\0>; .even
2:
	dec	narg
	add	$2,parg
	cmp	narg,$2
	bgt	gettape
	tst	(r5)+
	rts	r5

delete:
	jsr	r5,verify; 'd
		rts pc
	jsr	pc,clrent
	rts	pc

numb:
	mov	r1,-(sp)
	mov	r0,-(sp)
	clr	r0
	br	1f

numbx:
	mov	r1,-(sp)
	mov	r0,-(sp)
	movb	size0(r1),r0
1:
	mov	$catlb,r2
1:
	mov	$"  ,(r2)+
	cmp	r2,$catlb+12.
	blo	1b
	cmp	(r5),$2
	bne	1f
	mov	$"00,-2(r2)
1:
	mov	(sp)+,r1
	jsr	pc,numb2
	mov	(r5)+,r0
	sub	r0,r2
	mov	r2,0f
	mov	r0,0f+2
	mov	$1,r0
	sys	0; 9f
.data
9:
	sys	write; 0:..; ..
.text
	mov	(sp)+,r1
	rts	r5

numb1:
	clr	r0
numb2:
	div	$10.,r0
	mov	r1,-(sp)
	mov	r0,r1
	beq	1f
	jsr	pc,numb1
1:
	mov	(sp)+,r0
	add	$'0,r0
	movb	r0,(r2)+
	rts	pc

update:
	jsr	pc,bitmap
	mov	$dir,r1
1:
	tst	(r1)
	beq	2f
	bit	$100000,mode(r1)
	beq	2f
	tstb	size0(r1)
	bne	9f
	tst	size1(r1)
	beq	2f
9:
	mov	ndentd8,-(sp)
	inc	(sp)
	movb	size0(r1),r2
	mov	size1(r1),r3
	add	$511.,r3
	adc	r2
	ashc	$-9,r2
	mov	r3,size
3:
	mov	(sp),r2
	mov	size,r3
4:
	jsr	pc,bitcalc
	inc	r2
	bitb	(sp)+,map(r0)
	bne	4f
	sob	r3,4b
	mov	(sp)+,tapea(r1)
	jsr	pc,setmap
	br	2f
4:
	inc	(sp)
	br	3b
2:
	add	$dirsiz,r1
	cmp	r1,edir
	blo	1b
	jsr	pc,wrdir

update1:
	mov	$dir,r1
	clr	-(sp)
	mov	$-1,-(sp)
1:
	tst	(r1)
	beq	2f
	bit	$100000,mode(r1)
	beq	2f
	cmp	tapea(r1),(sp)
	bhis	2f
	mov	tapea(r1),(sp)
	mov	r1,2(sp)
2:
	add	$dirsiz,r1
	cmp	r1,edir
	blo	1b
	tst	(sp)+
	mov	(sp)+,r1
	bne	1f
	rts	pc
1:
	bic	$100000,mode(r1)
	movb	size0(r1),mss
	mov	size1(r1),r2
	bne	4f
	tst	mss
	beq	update1
4:
	jsr	r5,decode; name
	mov	tapea(r1),r0
	jsr	pc,wseek
	clr	r3
	sys	open; name; 0
	bes	phserr
	mov	r0,r3
3:
	tst	mss
	bne	4f
	cmp	r2,$512.
	blo	3f
4:
	mov	r3,r0
	sys	read; tapeb; 512.
	bes	phserr
	cmp	r0,$512.
	bne	phserr
	jsr	pc,twrite
	sub	$512.,r2
	sbc	mss
	br	3b
3:
	mov	r2,0f
	beq	3f
	mov	r3,r0
	sys	0; 9f
.data
9:
	sys	read; tapeb; 0:..
.text
	bes	phserr
	cmp	r0,0b
	bne	phserr
	jsr	pc,twrite
3:
	mov	r3,r0
	sys	read; tapeb; 512.
	bes	phserr
	tst	r0
	bne	phserr
	mov	r3,r0
	sys	close
2:
	jmp	update1

phserr:
	mov	r1,-(sp)
	mov	$name,r1
	jsr	pc,pstr
	jsr	r5,mesg
		< -- Phase error\n\0>; .even
	mov	(sp)+,r1
	clr	time0(r1) / time
	beq	2b
	sys	close
	br	2b

bitmap:
	mov	$map,r0
1:
	clr	(r0)+
	cmp	r0,$emap
	blo	1b
	mov	$dir,r1
1:
	tst	(r1)
	beq	2f
	bit	$100000,mode(r1)
	bne	2f
	tst	size1(r1)
	bne	3f
	tstb	size0(r1)
	beq	2f
3:
	jsr	pc,setmap
2:
	add	$dirsiz,r1
	cmp	r1,edir
	blo	1b
	rts	pc

setmap:
	movb	size0(r1),r2
	mov	size1(r1),r3
	add	$511.,r3
	adc	r2
	ashc	$-9.,r2
	mov	tapea(r1),r2
1:
	jsr	pc,bitcalc
	bitb	(sp),map(r0)
	bne	maperr
	bisb	(sp)+,map(r0)
	inc	r2
	sob	r3,1b
	rts	pc

bitcalc:
	mov	(sp),-(sp)
	cmp	r2,tapsiz
	bhis	maperr
	mov	r2,r0
	bic	$!7,r0
	mov	r0,-(sp)
	mov	$1,r0
	als	(sp)+,r0
	mov	r0,2(sp)
	mov	r2,r0
	ash	$-3,r0
	bic	$160000,r0
	rts	pc

maperr:
	jsr	r5,mesg
		<Tape overflow\n\0>; .even
	jmp	done

usage:
	jsr	pc,bitmap
	mov	$dir,r2
1:
	tst	(r2)
	beq	2f
	inc	nentr
2:
	add	$dirsiz,r2
	cmp	r2,edir
	blo	1b
	mov	ndentd8,r2
	inc	r2
	mov	tapsiz,r3
	dec	r3
	sub	ndentd8,r3
1:
	jsr	pc,bitcalc
	bitb	(sp)+,map(r0)
	beq	2f
	inc	nused
	mov	r2,lused
	br	3f
2:
	inc	nfree
	tstb	flm
	bne	1f
3:
	inc	r2
	sob	r3,1b
1:
	mov	nentr,r0
	jsr	r5,numb; 4
	jsr	r5,mesg
		< entries\n\0>; .even
	mov	nused,r0
	jsr	r5,numb; 4
	jsr	r5,mesg
		< used\n\0>; .even
	tstb	flm
	bne	1f
	mov	nfree,r0
	jsr	r5,numb; 4
	jsr	r5,mesg
		< free\n\0>; .even
1:
	mov	lused,r0
	jsr	r5,numb; 4
	jsr	r5,mesg
		< last\n\0>; .even
	rts	pc

taboc:
	tstb	flv
	beq	4f
	mov	mode(r1),r0
	mov	r0,-(sp)
	ash	$-6,r0
	bit	$40,r0
	jsr	pc,pmod
	mov	(sp),r0
	ash	$-3,r0
	bit	$200,r0
	jsr	pc,pmod
	mov	(sp)+,r0
	bit	$1000,r0
	jsr	pc,pmod
	clr	r0
	bisb	uid(r1),r0
	jsr	r5,numb; 4
	clr	r0
	bisb	gid(r1),r0
	jsr	r5,numb; 4
	mov	tapea(r1),r0
	jsr	r5,numb; 5
	mov	size1(r1),r0
	jsr	r5,numbx; 9.
	mov	r1,-(sp)
	add	$time0,(sp)
	jsr	pc,_localtime
	mov	r0,(sp)
	mov	10.(r0),r0
	jsr	r5,numb; 3
	mov	$'/,r0
	jsr	pc,putc
	mov	(sp),r0
	mov	8.(r0),r0
	inc	r0
	jsr	r5,numb; 2
	mov	$'/,r0
	jsr	pc,putc
	mov	(sp),r0
	mov	6(r0),r0
	jsr	r5,numb; 2
	mov	(sp),r0
	mov	4(r0),r0
	jsr	r5,numb; 3
	mov	$':,r0
	jsr	pc,putc
	mov	(sp)+,r0
	mov	2(r0),r0
	jsr	r5,numb; 2
	mov	$' ,r0
	jsr	pc,putc
4:
	mov	$name,r1
	jsr	pc,pstr
	jsr	r5,mesg
		<\n\0>
	rts	pc

pmod:
	beq	1f
	mov	$'s,-(sp)
	br	2f
1:
	bit	$1,r0
	beq	1f
	mov	$'x,-(sp)
	br	2f
1:
	mov	$'-,-(sp)
2:
	bit	$2,r0
	beq	1f
	mov	$'w,-(sp)
	br	2f
1:
	mov	$'-,-(sp)
2:
	bit	$4,r0
	beq	1f
	mov	$'r,r0
	br	2f
1:
	mov	$'-,r0
2:
	jsr	pc,putc
	mov	(sp)+,r0
	jsr	pc,putc
	mov	(sp)+,r0
	jsr	pc,putc
	rts	pc

xtract:
	movb	size0(r1),mss
	bne	2f
	tst	size1(r1)
	beq	1f
2:
	jsr	r5,verify; 'x
		rts pc
	mov	size1(r1),r3
	mov	tapea(r1),r0
	jsr	pc,rseek
	sys	unlink; name
	mov	mode(r1),0f
	sys	0; 9f
.data
9:
	sys	creat; name; 0:..
.text
	bes	crterr
	mov	r0,r2
2:
	tst	mss
	bne	3f
	cmp	r3,$512.
	blo	2f
3:
	jsr	pc,tread
	mov	r2,r0
	sys	write; tapeb; 512.
	bes	crterr1
	cmp	r0,$512.
	bne	crterr1
	sub	r0,r3
	sbc	mss
	br	2b
2:
	mov	r3,0f
	beq	2f
	jsr	pc,tread
	mov	r2,r0
	sys	0; 9f
.data
9:
	sys	write; tapeb; 0:..
.text
	bes	crterr1
	cmp	r0,0b
	bne	crterr1
2:
	mov	r2,r0
	sys	close
/	movb	gid(r1),0f+1
/	movb	uid(r1),0f
/	sys	0; 9f
/.data
/9:
/	sys	chown; name; 0:..
/.text
	mov	time0(r1),r0
	mov	r1,-(sp)
	mov	time1(r1),r1
/	sys	0; 9f
.data
9:
	sys	smdate; name
.text
	mov	(sp)+,r1
1:
	rts	pc

crterr1:
	clr	r0
	mov	r1,-(sp)
	clr	r1
/	sys	smdate; name
	mov	(sp)+,r1
	mov	r2,r0
	sys	close

crterr:
	mov	$name,r1
	jsr	pc,pstr
	jsr	r5,mesg
		< -- create error\n\0>; .even
	rts	pc
/ tap4 -- dec-tape lod/dmp

.data
tc:	</dev/tap>
tcx:	<x\0>
mt:	</dev/mt>
mtx:	<0\0>
tboot:	</usr/mdec/tboot\0>
mboot:	</usr/mdec/mboot\0>
.even
.bss
dir:	. = .+[mdirent*dirsiz]
tapeb:
map:	.=.+4096.
emap:
ch:	.=.+1
flc:	.=.+1
flf:	.=.+1
fli:	.=.+1
flm:	.=.+1
flu:	.=.+1
flv:	.=.+1
flw:	.=.+1
.even

command:.=.+2
sum:	.=.+2
size:	.=.+2
nentr:	.=.+2
nused:	.=.+2
nfree:	.=.+2
lused:	.=.+2
catlb:	.=.+20.
narg:	.=.+2
rnarg:	.=.+2
parg:	.=.+2
fio:	.=.+2
mss:	.=.+2
ndirent:.=.+2
ndentd8:.=.+2
edir:	.=.+2
rseeka:	.=.+2
wseeka:	.=.+2
tapsiz:	.=.+2
name:	.=.+32.
name1:	.=.+32.
statb:	.=.+40.

smdate = 30.
