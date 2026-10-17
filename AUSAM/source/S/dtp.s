//////////////////////////////////////////////////////////////////////////
//									//
//	this is a version of 'tp' modified by ian j. AGSM		//
//									//
//	enhancements or otherwise:					//
//									//
//	1) new command 's'. this is a selective dump facility.		//
//	   the number following 's' in the key is taken as a number	//
//	   of hours -- if number absent then /etc/last-dump-time	//
//	   contains time last dump  -- any file modified within/since	//
//	   that time will be dumped.					//
//									//
//	2)  new commands  implemented, new meanings, old		//
//	    unused commands are out. see 'swtab' below			//
//									//
//	3)  tape blocksize increased to tbloksiz bytes, this allows 	//
//	    more data to be kept on tape than is possible using		//
//	    a blocksize of 512. bytes.					//
//									//
//	Yet to be done:							//
//		dumps of 65536 and more blocks				//
//		virtual blocks						//
//		links							//
//									//
//////////////////////////////////////////////////////////////////////////

/	Tunable constants

tapesize =  2400.			/ default tape length in feet.
tbloksiz = 16384.			/ blocksize on tape - must be a integer power of 2
					/ 16384. is max allowable for 800 BPI drives
					/ the silly hardware times-out before write
					/ has completed !! 32768 is fine for 1600 BPI
					/ but 16384 is positive not negative like 32768.
					/ 16384. is max possible value - too much
					/ trouble otherwise.

tbloking = tbloksiz\/512.		/ sectors per tape block.
nrzreclen = [tbloksiz+534.+799.]\/800.		/ record length in inches (800bpi)
pereclen  = [tbloksiz+1067.+1599.]\/1600.	/ record length in inches (1600bpi)

/ system calls...

smdate = 30.
mknod  = 14.
lock = 56.
	readlock = 0
	writelock = 1

/ system constants

IFMT = 60000
ISDIR = 40000
ISBLK = 20000
EEXIST = 17.
/
	iot = 4

	sys	time
	mov	r0,dumptime
	mov	r1,dumptime+2		/ note program start time
	mov	(sp),rnarg
	mov	(sp)+,narg
	mov	$cmexit,command
	tst	(sp)+
	cmp	narg,$2
	bge	1f
	mov	$2,narg
	br	3f
1:
	mov	(sp)+,r0
	mov	sp,parg

	sub	$512.,sp
	clr	-(sp)
	clr	-(sp)
	mov	$1,-(sp)		/ outfd
	mov	sp,outbp		/ tty output buffer
	mov	sp,outbp1		/ for benefit of flusher

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
	jsr	pc,opentap
	sys	signal; 2; 1
	bit	$1,r0
	bne	0f
	sys	signal; 2; intrupt		/ lets remove temp file
0:
	cmp	tfilename,$tmpdirname
	bne	0f
	sys	getpid		/ generate unique temp file name
	mov	$5,r2
1:
	mov	r0,r1
	clr	r0
	div	$10.,r0
	add	$'0,r1
	movb	r1,xxxxx-1(r2)
	sob	r2,1b
0:
	sys	indir; 0f
.data
0:	sys	creat
tfilename:	tmpdirname; 0600
.text
	bes	8f
	sys	close
	mov	tfilename,0f+2		/ get new name
	sys	indir; 0f
.data
0:	sys	open; 0; 2
.text
	bes	8f
	mov	r0,dirfd
	sys	fstat; statb		/ remember dev, inum
	mov	statb,tfdev
	mov	statb+2,tfino

	mov	tpsize,r0

	jmp	*command
8:
	jsr	r5,mesg
		< directory file create error\n\0>; .even
	jbr	done

opentap:
	cmp	command,$cmn
	beq	2f
	mov	magtpnm,1f+2		/ setup correct name
	sys	indir; 1f
.data
1:	sys	open; ..; 0
.text
	br	3f
2:
	mov	magtpnm,1f+2		/ setup up name
	sys	indir; 1f
.data
1:	sys	creat; ..; 0600
.text
3:
	bes	1f
	mov	r0,fio
	rts	pc
1:
	jsr	r5,mesg
		<Tape open error\n\0>; .even
	jbr	done

setcom:
	cmp	command,$cmexit
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
	jbr	done
/
swtab:
	'0; dcof	/ d r i v e    0
	'1; dcof	/ d r i v e    1
	'm; dcm		/ d e n s i t y   800 b.p.i.
	'h; dch		/ d e n s i t y   1600 b.p.i.
	'f; dcf		/ f e e t  - size of tape
	't; dct		/ t e m p  file name
	'F; dcff	/ F i l e  don't use magtape use next arg as output
/
	'x; dcx		/ e x t r a c t  named files.
	'n; dcn		/ n e w  dump tape to be created
	'l; dcl		/ l i s t   contents of dump tape
/
	's; dcs		/ i n c r e m e n t a l l y  dump files to dump tape
	'c; dcc		/ c r e a t e   as needed directories/special-files
	'u; dcu		/ u p d a t e   last dump time in /etc/last-dump-time (new dump only)
/
	'v; dcv		/ v e r b o s e   dont be silent, give all info
	'q; dcq		/ q u e s t i o n   after each file selected for user approval(y-yes n-no x-exit)
	'i; dci		/ i g n o r e  tape read/write errors
	'.; dcd		/ d u m p  all files, even those beginning with '.'
	'-; dcig	/ i g n o r e  for consistency only
	 0; 0

dcof:
	movb	r1,magu
	rts	pc

dcm:
	movb	r1,magt
	mov	$nrzreclen,recordln
	rts	pc

dch:
	movb	r1,magt
	mov	$pereclen,recordln
	rts	pc

dcf:
	jsr	pc,argnum		/ see if tape length specified.
	bne	0f			/ --> yes so use it.
	mov	$tapesize,r1		/ assume tapesize ft. tape.
0:
	mov	r1,tpsize		/ save length of tape.
	rts	pc

dct:					/ get a name from arg list
	cmp	rnarg,$2		/ sufficient args?
	ble	0f
	mov	*parg,tfilename		/ next arg is temp file name
	add	$2,parg
	dec	rnarg
	dec	narg
0:
	rts	pc
	rts	pc

dcff:					/ get a name from arg list
	cmp	rnarg,$2		/ sufficient args?
	ble	0f
	mov	*parg,magtpnm		/ next arg is output file name
	add	$2,parg
	dec	rnarg
	dec	narg
0:
	rts	pc

dci:
	incb	fli
	rts	pc

dcd:
	incb	fld
	rts	pc

dcn:
	jsr	r5,setcom; cmn
	rts	pc

dcig:
	rts	pc

dcl:
	jsr	r5,setcom; cml
	rts	pc

dcu:
	incb 	flu			/ cause start time to be written to /etc/last-dump-time
	rts	pc

dcv:
	incb	flv
	rts	pc

dcq:
	incb	flq
	rts	pc

dcx:
	jsr	r5,setcom; cmx
	rts	pc
dcc:
	inc	flc
	rts	pc

dcs:
	incb 	fls			/ use selective file dump
	jsr	pc,argnum		/ obtain number that optionally follows
	bne	4f			/ if non-zero then use hour interval specified
	mov	r0,-(sp)		/ save arg pointer
	sys	open; ldt; 0
	bec	3f
2:	jsr	r5,mesg; < Cannot open last-dump-time\n\0>; .even
	jbr	done
3:
	mov	r0,r1
	sys	read; seltim; 4		/ dump all files modded after this time
	bes	2b
	cmp	$4,r0
	bne	2b
	mov	r1,r0
	sys	close
	mov	(sp)+,r0
	rts	pc
4:
	mov	r0,-(sp)
	mov	r1,r0
	mpy	$-3600.,r0		/ get seconds(-ve) not hours.
	add	dumptime+2,r1		/ subtract
	adc	r0			/  time
	add	dumptime,r0		/   specified.
	mov	r0,seltim
	mov	r1,seltim+2		/ all files modified after this time dumped
	mov	(sp)+,r0
	rts	pc

/
argnum:
	clr	r1
0:
	movb	(r0),r2			/ pick next char from 1st argument.
	sub	$'0,r2
	blt	1f			/ not a valid digit so branch
	cmp	$9.,r2
	blt	1f			/ not a valid digit so branch
	inc	r0
	mul	$10.,r1
	add	r2,r1			/ build no. of hours.
	br	0b
1:
	tst	r1		/ set cc to indicate whether number found
	rts	pc

//////////////////////////////////////////////////////////////////////////

cmexit:
	jsr	r5,mesg
		<no function specified\n\0>; .even
	jbr	done

/
cmn:
	jsr	r5,noflag; flc; 0
	/ determine those files to be dumped
	jsr	pc,getfiles
	/ check to see if all files will fit on tape
	mov	nfiles,r0
	mul	$dirsiz,r0
	add	$tbloksiz-1,r1
	adc	r0
	div	$tbloksiz,r0
	mov	r0,-(sp)		/ big blocks occupied by tape directory
	clr	r0
	mov	tapad,r1
	add	$tbloking-1,r1
	adc	r0
	div	$tbloking,r0		/ number big blocks wanted for files
	add	(sp)+,r0		/ total number big blocks wanted
	mul	recordln,r0		/ total number inches required
	add	$11.,r1
	adc	r0
	div	$12.,r0			/ r0 = feet required
	cmp	tpsize,r0		/ compare tape sizes (in feet)
	bhis	0f			/ --> will fit on the tape
	mov	r0,-(sp)
	jsr	r5,mesg
		<tape too small to contain all files\n\0>; .even
	mov	(sp)+,r0
	jsr	r5,numb; 5
	jsr	r5,mesg
		< foot tape required\n\0>; .even
	jbr	done
	/ will fit so lets get it all on tape
0:
	jsr	pc,fflush
	jsr	pc,make
	tst	flu
	jeq	usage		/ no update of last-dump-time reqd.
	sys	creat; ldt; 0600
	bec	2f
1:
	jsr	r5,mesg
		<can't update last-dump-time\n\0>; .even
	jbr	done
2:
	mov	r0,r1
	sys	write; dumptime; 4
	bes	1b
	cmp	r0,$4
	bne	1b
	br	usage

/
.globl	_localtime,_ctime

cml:
	jsr	r5,noflag; flc; flq; 0
	jsr	pc,rddir		/ read tape directory onto disk and format it
	jsr	r5,mesg
		<\n\n this dump initiated on   \0>; .even
	mov	dumptime+2,-(sp)
	mov	dumptime,-(sp)
	jsr	pc,_ctime
	mov	r0,r1
	jsr	pc,pstr
	tstb	flv
	beq	1f
	jsr	r5,mesg
	<\n    mode     uid   tapa      size   date    time name\n\n\0>; .even
1:
	jsr	r5,gettape; taboc
	jbr	usage2

//////////////////////////////////////////////////////////////////////////

cmx:
	jsr	r5,noflag; 0
	tstb	flc
	beq	1f
	tstb	flq
	beq	1f
	jbr	useerr			/ don't allow directory create with questions
1:
	jsr	pc,rddir		/ read tape directory into core and format it
	jsr	pc,tread		/ get first tape block
	jsr	r5,gettape; xtract
	br	done

/
usage:
	mov	nused,r0
	jsr	r5,numb; 4
	jsr	r5,mesg
		< tape blocks\n\0>; .even
usage2:
	mov	nfiles,r0	/ no of files picked up
	jsr	r5,numb; 4
	jsr	r5,mesg
		< files on tape\n\0>; .even

/////////////////////////////////////////////////////////////////////////

done:
	jsr	r5,mesg
		<END\n\0>; .even
	jsr	pc,fflush
	mov	$15.,r1
0:	mov	r1,r0
	sys	close		/ close ALL files
	sob	r1,0b
intrupt:
	mov	tfilename,0f+2	/ get dir file name
	sys	indir; 0f
.data
0:	sys	unlink; 0	/ and remove it
.text
	sys	exit

/
pstr:
	movb	(r1)+,r0
	beq	1f
	jsr	pc,putch
	br	pstr
1:
	rts	pc

/////////////////////////////////////////////////////////////////////////

mesg:
	movb	(r5)+,r0
	beq	1f
	jsr	pc,putch
	br	mesg
1:
	inc	r5
	bic	$1,r5
	rts	r5

/////////////////////////////////////////////////////////////////////////

	.data
	.globl	putc
putch:
	jsr	r5,putc
outbp:	.=.+2
	rts	pc

/////////////////////////////////////////////////////////////////////////

	.globl	flush
fflush:
	jsr	r5,flush
outbp1:	.=.+2
	rts	pc
	.text

/////////////////////////////////////////////////////////////////////////

getc:
	clr	r0
	sys	read; ch; 1
	movb	ch,r0
	rts	pc

/
rddir:
	mov	dirfd,r0
	sys	seek; 0; 0		/ position at start of disk dir.
	jsr	pc,tread		/ read first tape block
	mov	$tapeb,r0		/ 'from'
	mov	$tapeheader,r1		/ 'to'
	mov	$dirsiz\/2,r2		/ number of words to copy
0:
	mov	(r0)+,(r1)+
	sob	r2,0b			/ copy tape header.
	mov	ndirblks,r2		/ a zero means old format
	br	2f			/ write this block to disk.
1:
	jsr	pc,tread		/ read next tape block.
	bne	2f			/ a tape mark. Is it expected?
	tst	ndirblks		/ old format?
	beq	3f			/ ok
	jsr	r5,mesg
		<Unexpected tape mark in directory\n\0>; .even
	jbr	done
2:	jsr	pc,dwrite		/ write to disk for later use.
	sob	r2,1b			/ more to do?
3:
	rts	pc

/
wrdir:
	mov	dirfd,r0
	sys	seek; 0; 0		/ position at start of disk dir.
	jsr	pc,dread		/ read first disk block
	mov	dirfd,r0
	sys	fstat; statb		/ get info on directory file
	jes	drderr
	movb	statb+9.,r0		/ get high order size
	mov	statb+10.,r1		/ get low order size
	div	$tbloksiz,r0		/ calculate number of tape blocks
	tst	r1			/ any remainder ??
	beq	0f			/ --> no.
	inc	r0			/ round
0:
	mov	r0,ndirblks		/ no of dir blocks !!
	mov	$tapeheader,r0		/ 'from'
	mov	$tapeb,r1		/ 'to'
	mov	$dirsiz\/2,r2		/ number of bytes to copy
0:
	mov	(r0)+,(r1)+
	sob	r2,0b			/ copy tape header.
	br	2f
1:
	jsr	pc,dread		/ read next directory block.
	beq	3f			/ --> end of directory file.
2:	jsr	pc,twrite		/ write to tape
	br	1b			/ continue.
3:
	rts	pc

/
tread:
	mov	fio,r0
	sys	read; tapeb; tbloksiz
	bes	trderr
	tst	r0		/ set cc for eof test on return
	beq	0f
	cmp	r0,$tbloksiz
	bne	trderr
	tst	r0		/ set cc for eof test on return
0:
	rts	pc

trderr:
	jsr	r5,mesg
		<Tape read error\n\0>; .even
	tstb	fli
	jeq 	done		/ don't ignore the error, stop
	mov	$tapeb,r0
2:
	clr	(r0)+
	cmp	r0,$tapeb+tbloksiz
	blo	2b
	rts	pc

/////////////////////////////////////////////////////////////////////////

dread:
	mov	dirfd,r0
	sys	read; tapeb; tbloksiz
	bes	drderr
	bit	$dirsiz-1,r0
	bne	drderr
	tst	r0
	rts	pc

drderr:
	jsr	r5,mesg
		<Directory file read error\n\0>; .even
	jbr	done

eread:
	mov	dirfd,r0
	sys	read; cde; dirsiz
	bes	drderr
	bit	$!dirsiz,r0
	bne	drderr
	tst	r0
	rts	pc

/
twrite:
	inc	nused		/ keep track of number blocks written to tape
	mov	fio,r0
	sys	write; tapeb; tbloksiz
	bes	twrerr
	cmp	r0,$tbloksiz
	bne	twrerr
	rts	pc

twrerr:
	jsr	r5,mesg
		<Tape write error\n\0>; .even
	jbr	done

/////////////////////////////////////////////////////////////////////////

dwrite:
	mov	dirfd,r0
	sys	write; tapeb; tbloksiz
	bes	dwrerr
	cmp	r0,$tbloksiz
	bne	dwrerr
	rts	pc

dwrerr:
	jsr	r5,mesg
		<Directory file write error\n\0>; .even
	jbr	done

ewrite:
	clr	dirlast
ewrited:
	mov	ebufpt,r0
	cmp	r0,$tapeb+tbloksiz
	blo	0f
	mov	dirfd,r0
	sys	write; tapeb; tbloksiz
	bes	dwrerr
	cmp	r0,$tbloksiz
	bne	dwrerr
	mov	$tapeb,r0
0:
	mov	$cde,r1
	mov	$dirsiz\/2,r2
0:
	mov	(r1)+,(r0)+
	sob	r2,0b
	mov	r0,ebufpt
	rts	pc

/
verify:
	movb	(r5)+,0f
	inc	r5
	tstb	flq
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
	mov	$cpathname,r1
	jsr	pc,pstr
	mov	(sp)+,r1
	tstb	flq
	beq	1f
	jsr	r5,mesg
		< \0>
	jsr	pc,fflush
	jsr	pc,getc
	cmp	r0,$'x
	bne	3f
	jsr	pc,getc
	jbr	done
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
	br	3f
1:
	jsr	r5,mesg
		<\n\0>
2:
	tst	(r5)+
3:
	rts	r5

/
getfiles:
	cmp	narg,$2
	bne	1f
	mov	$".\0,cpathname
	jsr	pc,callout
1:
	cmp	narg,$2
	ble	1f
	dec	narg
	mov	*parg,r1
	add	$2,parg
	mov	$cpathname,r2
2:
	movb	(r1)+,(r2)+
	bne	2b
	jsr	pc,callout
	mov	$-1,dirlast
	br	1b
1:
	mov	ebufpt,r0
	sub	$tapeb,r0
	beq	1f
	mov	r0,2f+4
	mov	dirfd,r0
	sys	0; 2f
.data
2:	sys	write; tapeb; 0
.text
	bes	0f
	cmp	r0,2b+4
	beq	1f
0:
	jmp	dwrerr
1:
	rts	pc

/
expand:
	sys	open; cpathname; 0
	bes	fserr
	mov	r0,-(sp)
1:
	mov	(sp),r0
	sys	read; catlb; 16.
	bec	4f
	tst	(sp)+
	jbr	fserr
4:
	tst	r0
	beq	1f
	tst	catlb
	beq	1b
	mov	$catlb+2,r1
	tstb	fld			/ dump files starting with '.'?
	bne	2f
	cmpb	(r1),$'.
	beq	1b
	br	3f
2:
	cmp	(r1),$'.
	beq	1b
	cmp	(r1),$"..
	bne	3f
	tstb	2(r1)
	beq	1b
3:
	mov	$cpathname,r0
	mov	$catlb+2,r1
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
/
callout:
	sys	stat; cpathname; statb
	bec	0f
fserr:
	mov	$cpathname,r1
	jsr	pc,pstr
	jsr	r5,mesg
		< -- Can't access file\n\0>; .even
	rts	pc
0:
	cmp	statb,tfdev	/ check for temp file
	bne	0f
	cmp	statb+2,tfino
	bne	0f
	rts	pc		/ don't dump it
0:
	tstb	fls		/ selective file dump ??
	beq	0f		/ --> no.
	cmp	seltim,statb+32.
	blo	0f		/ will dump
	bhi	1f		/ dont dump
	cmp	seltim+2,statb+34.
	blo	0f		/ will dump
1:
	mov	statb+4,r0
	bic	$!IFMT,r0
	cmp	$ISDIR,r0
	jne	3f
0:
	jsr	r5,verify; 'a
		rts pc
	inc	nfiles
	clr	csize
	clr	csize+2
	movb	statb+7,cuid
	movb	statb+8,cuid+1
	mov	statb+32.,ctime
	mov	statb+34.,ctime+2
	mov	tapad,ctapea	/ store tape address in tape dir entry
	mov	statb+4,r0
	mov	r0,cmode
	bic	$!IFMT,r0
	bne	1f		/ --> not a plain file
	movb	statb+9.,csize	/ only 24 bits for the moment
	mov	csize,r2
	mov	statb+10.,r3
	mov	r3,csize+2
	add	$511.,r3
	adc	r2
	ashc	$-9,r2		/ file size (next highest multiple of 512 bytes)
	add	r3,tapad	/ increment by size of file - ready for next file
	jsr	pc,ewrite	/ add to disk dir file
	rts	pc
1:
	cmp	r0,$ISDIR
	jeq	2f		/ directory so see whats in it
	mov	statb+12.,cmajmin	/ identify the special file
	jsr	pc,ewrite	/ add to disk dir file
	rts	pc
2:
/ check directory for files to dump
/ add directory entry to disk directory
/ but delete it if no files dumped from the directory
	tst	dirlast		/if last thing put in was directory
	bgt	1f
	cmp	ebufpt,$tapeb+diruplim
	blos	1f
	mov	ebufpt,r0
	sub	$tapeb,r0
	mov	r0,2f+4
	mov	dirfd,r0
	sys	0;2f
	.data
2:	sys	write; tapeb; 0
	.text
	jes	dwrerr
	cmp	r0,2b+4
	jne	dwrerr
	mov	$tapeb,ebufpt
1:
	jsr	pc,ewrited	/ add to disk dir file
	inc	dirlast
	jsr	pc,expand
	tst	dirlast		/ any file dumped in this dir?
	beq	3f		/ --> yes.
	dec	nfiles		/     no. delete reference to directory
	dec	dirlast		/ remember how many
	sub	$dirsiz,ebufpt
	mov	flq,-(sp)
	clr	flq		/ no questions !!
	jsr	r5,verify; 'd
		0		/ won't happen ever
	mov	(sp)+,flq
3:
	rts	pc
/
gettape:
	clr	exta			/ clr top 16 bits of tape addr
	clr	ctapea			/ clr bottom 16 bits of tape addr
	mov	nfiles,r4
	bne	0f
	jsr	r5,mesg
		<Tape empty?\n\0>; .even
	jbr	done
0:
	mov	dirfd,r0
	sys	seek; dirsiz; 0		/ skip tape header
	sub	$2,narg			/ full extract or list?
	bgt	0f
	mov	$1,narg
	mov	$1f,parg		/ dummy arg list
	.data
1:	1f
1:	0
	.text
0:
	mov	narg,r0
0:
	clr	-(sp)
	sob	r0,0b			/ table to record argument matches
0:
	mov	ctapea,r3		/ save address last file
	jsr	pc,eread		/ get next dir entry
	jeq	drderr
	cmp	r3,ctapea		/ overflow into next 16bits ??
	blos	1f			/ if not ignore
	inc	exta			/ full 32bit address
1:
	mov	parg,-(sp)
	mov	narg,-(sp)
1:					/ search for it amongst args
	mov	*2(sp),r3
	cmp	rnarg,$2		/ global?
	beq	3f
	mov	$cpathname,r2
2:	tstb	(r3)			/ end of arg?
	beq	2f			/ a match!
	cmpb	(r2)+,(r3)+
	beq	2b
	br	4f			/ no match
2:
	tstb	(r2)			/ end of entry name?
	beq	3f
	cmpb	(r2),$'/		/ or path segment?
	bne	4f
3:
	mov	r5,-(sp)
	jsr	pc,*(r5)		/ call passed routine to do work
	mov	(sp)+,r5
	mov	2(sp),r0
	sub	parg,r0
	add	sp,r0
	inc	4(r0)			/ record a match
	br	2f
4:
	add	$2,2(sp)
	dec	(sp)
	bne	1b
2:
	cmp	(sp)+,(sp)+		/ pop off junk
	sob	r4,0b
					/ search complete. now check for fails
	mov	narg,r2
	mov	parg,r3
0:
	mov	(r3)+,r1
	mov	(sp)+,r0		/ count of references
	bne	1f			/ it was matched
	jsr	pc,pstr
	jsr	r5,mesg
		<: not found\n\0>; .even
	br	2f
1:
	add	r0,r4
2:
	sob	r2,0b
	cmp	command,$cmx
	bne	0f
	mov	r4,r0
	jsr	r5,numb; 6
	jsr	r5,mesg
		< files extracted\n\0>; .even
0:
	tst	(r5)+			/ chuck away arg
	rts	r5

/
numb:
	mov	r1,-(sp)
	mov	r0,-(sp)
	mov	r0,r1
	clr	r0	/ 32bit number in r0,r1
	br	1f

numbx:
	mov	r1,-(sp)
	mov	r0,-(sp)
	mov	6(sp),r0
	mov	8(sp),r1
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
	jsr	pc,numb2
	clrb	(r2)
	sub	(r5)+,r2
	mov	r2,r1
	jsr	pc,pstr
	mov	(sp)+,r0
	mov	(sp)+,r1
	rts	r5

numb1:
	clr	r0
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

numb2:
	div	$10000.,r0
	beq	numb1		/ --> if zero quotient
	mov	r1,-(sp)
	mov	r0,r1
	jsr	pc,numb1
	mov	(sp)+,r1
	clr	r0		/ remainder is always 4 digits !!
	div	$1000.,r0
	add	$'0,r0
	movb	r0,(r2)+
	clr 	r0
	div	$100.,r0
	add	$'0,r0
	movb	r0,(r2)+
	clr 	r0
	div	$10.,r0
	add	$'0,r0
	movb	r0,(r2)+
	add	$'0,r1
	movb	r1,(r2)+	/ greg rose responsible for this code
	rts	pc		/ ian j responsible for this comment

/
make:
	jsr	pc,wrdir

make1:
	mov	dirfd,r0
	sys	seek; dirsiz; 0	/ start at first dir entry.
1:
	jsr	pc,eread	/ read next entry.
	beq	1f		/ --> end of directory
	bit	$IFMT,cmode
	bne	1b		/ --> not a plain file.
	mov	csize,r2
	mov	csize+2,r3
	add	$511.,r3
	adc	r2
	ashc	$-9,r2		/ file size (next highest multiple of 512 bytes)
	sys	open; cpathname; 0	/ open it for reading.
	bes	phserro
	mov	r0,r2		/ save file descriptor for reads

.if	lock
FLOCKED = 02000
	bit	$FLOCKED,cmode	/ is file one of those special locking types?
	beq	6f
	sys	lock; readlock
.endif

6:
	mov	r2,r0
	jsr	pc,0f
.data
0:
	sys	read;  7: tapeb  ;  8:  tbloksiz
	rts	pc
.text
	bes	phserrr
	add	r0,7b		/ point next available location in tape buffer
	mov	r0,r5		/ save count of bytes read
	cmp	$tapeb+tbloksiz,7b	/ file finished ??
	bhi	5f		/ --> yes.
	mov	8b,r0		/ bytes to be read last read.
	ash	$-9,r0		/ get as block count
	sub	r0,r3		/ adjust file size counter
	bmi	phserrb		/ file has got bigger
	beq	3f		/ file has not changed
	mov	$tapeb,7b
	mov	$tbloksiz,8b
	jsr	pc,twrite
	br	6b
5:
	add	$511.,r0
	ash	$-9,r0		/ blocks just read
	sub	r0,r3		/ adjust file size
	bmi	phserrb		/ file is now bigger?
	bgt	phserrs		/ file is now smaller?
	cmpb	r5,csize+2	/ check that file same length now
	bne	phserrs
	sub	$tapeb,7b	/ make relative
	add	$511.,7b
	bic	$511.,7b	/ round up next read start address.
	add	$tapeb,7b	/ make absolute.
	mov	$tapeb+tbloksiz,8b
	sub	7b,8b		/ max length next record
	cmp	7b,$tapeb+tbloksiz	/ any room left ??
	bne	4f		/ --> yes, so continue
3:
	jsr	pc,twrite
	mov	$tapeb,7b	/ re-init
	mov	$tbloksiz,8b	/ re-init
4:
	mov	r2,r0
	sys	close
2:
	br	1b		/ next file
1:
	cmp	7b,$tapeb
	beq	0f		/ only write last block if have to
	jsr	pc,twrite
	mov	$tapeb,7b	/ re-init
	mov	$tbloksiz,8b	/ re-init
0:
	rts	pc

/ //
//	Recover from phase errors.
//
//	O - Open error.
//	R - Read error.
//	B - File is now bigger.
//	S - File is now smaller.
//

phserro:
	movb	$'O,phsedes
	br	phserr
phserrr:
	movb	$'R,phsedes
	br	phserr
phserrb:
	movb	$'B,phsedes
	mov	7b,r1
	mov	r5,r0
	bic	$!511.,r0
	beq	3f
	add	$-512.,r0
	sub	r0,r1		/ r1 rounded up to next block
3:
	mov	r3,r0
	ash	$9.,r0
	add	r0,r1
	br	5f

phserrs:
	movb	$'S,phsedes
	mov	r5,r0		/ recover length really read
	mov	7b,r1		/ next address
	bic	$!511.,r0
	beq	3f		/ already at a block address
0:
	clrb	(r1)+
	inc	r0
	bit	$512.,r0	/ end of this block?
	beq	0b
	cmp	r1,$tapeb+tbloksiz
	blo	3f
	jsr	pc,twrite
	mov	$tapeb,r1
	br	3f

phserr:
	mov	7b,r1
3:
	tst	r3		/ all done ??
	ble	5f		/ --> yes !!
	mov	$256.,r0	/ clear next block
0:	clr	(r1)+
	sob	r0,0b
	cmp	r1,$tapeb+tbloksiz
	blo	1f
	jsr	pc,twrite
	mov	$tapeb,r1
1:	dec	r3
	br	3b		/ go for next block
5:
	mov	r1,7b
	neg	r1
	add	$tapeb+tbloksiz,r1
	mov	r1,8b

	/ the phase error file has been sortof dumped
	/ now recover...

	mov	$cpathname,r1
	jsr	pc,pstr
	mov	$3f,r1
	jsr	pc,pstr
.data
3:
		< -- Phase error >
phsedes:	< \n\0>; .even
.text
	cmp	phsedes,$'O
	beq	2b
	mov	r2,r0
	sys	close
	br	2b

/
/	called from 'gettape' to list a directory entry.

taboc:
	tstb	flv
	jeq	4f
	mov	cmode,r0
	mov	r0,-(sp)
	movb	$'-,r5
	mov	$tabpf,mss
	bic	$!IFMT,r0	/ look at file type bits
	beq	0f		/ --> plain file
	movb	$'d,r5
	mov	$tabdf,mss
	cmp	$ISDIR,r0
	beq	0f		/ --> directory
	movb	$'c,r5
	mov	$tabsf,mss
	cmp	$ISBLK,r0
	beq	0f		/ --> character special
	movb	$'b,r5		/     block special
0:	mov	r5,r0
	jsr	pc,putch
	mov	(sp),r0
	ash	$-6,r0
	bit	$40,r0
	jsr	pc,pmod		/ print access bits for owner
	mov	(sp),r0
	ash	$-3,r0
	bit	$200,r0
	jsr	pc,pmod		/ print access bits for group
	mov	(sp)+,r0
	bit	$1000,r0
	jsr	pc,pmod		/ print access bits for others
	mov	cuid,r0
	jsr	r5,numb; 6	/ user id.
	mov	ctapea,-(sp)	/ tape address
	mov	exta,-(sp)	/ tape address
	jsr	r5,numbx; 7
	cmp	(sp)+,(sp)+
	jmp	*mss		/ jump on file type
tabpf:	mov	csize+2,-(sp)	/ file size
	mov	csize,-(sp)	/ file size
	jsr	r5,numbx; 10.
	cmp	(sp)+,(sp)+
	br 	0f
tabdf:	jsr	r5,mesg
		<          \0>; .even
	br	0f
tabsf:	movb	cmajmin+1,r0			/ major dev no
	bic	$177400,r0			/ lo byte only
	jsr	r5,numb; 6
	movb	$',,r0
	jsr	pc,putch
	movb	cmajmin,r0			/ minor dev no
	bic	$177400,r0			/ lo byte only
	jsr	r5,numb; 3
0:	mov	ctime+2,-(sp)
	mov	ctime,-(sp)
	jsr	pc,_localtime
	tst	(sp)+
	mov	r0,(sp)
	mov	6(r0),r0		/ day of the month(1-31)
	jsr	r5,numb; 3
	mov	$'/,r0
	jsr	pc,putch
	mov	(sp),r0
	mov	8.(r0),r0		/  month(0-11)
	inc	r0
	jsr	r5,numb; 2
	mov	$'/,r0
	jsr	pc,putch
	mov	(sp),r0
	mov	10.(r0),r0		/ year - 1900
	jsr	r5,numb; 2
	mov	(sp),r0
	mov	4(r0),r0		/ hours(0-23)
	jsr	r5,numb; 3
	mov	$':,r0
	jsr	pc,putch
	mov	(sp)+,r0
	mov	2(r0),r0		/ minutes(0-59)
	jsr	r5,numb; 2
	mov	$' ,r0
	jsr	pc,putch
4:
	mov	$cpathname,r1
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
	jsr	pc,putch
	mov	(sp)+,r0
	jsr	pc,putch
	mov	(sp)+,r0
	jsr	pc,putch
	rts	pc

/
/	if the file exists delete it iff not a directory.
/	this is necessary otherwise a filesys mess will result

xunlnk:
	sys	stat; cpathname; statb	/ get info on the file.
	bes	0f			/ --> file don't exist
	bic	$!IFMT,statb+4		/ isolate file type bits
	cmp	$ISDIR,statb+4		/ directory ??
	beq	0f
	sys	unlink; cpathname		/ remove it
0:
	rts	pc

/////////////////////////////////////////////////////////////////////////

/	called from 'gettape' to process a directory entry

xtract:
	tstb	flc		/ create directory mode ??
	beq	0f		/ --> no.
	mov	cmode,r1
	mov	r1,9f
	bic	$!IFMT,r1
	beq	0f		/ --> plain file
	clr	8f		/ assume directory.
	cmp	r1,$ISDIR
	beq	4f		/ --> directory.
	mov	cmajmin,8f	/ special file.
4:
	jsr	pc,xunlnk
	jsr	r5,verify; 'c
		rts	pc
	bic	$!60777,9f
	sys	0; 7f
.data
7:
	sys	mknod; cpathname; 9: 0; 8: 0	/ create directory or special file
.text
	bec	3f
	cmp	r0,$EEXIST			/ file already exists?
	jne	crterr
	cmp	r1,$ISDIR
	jne	crterr
	/ if directory already exists then ok
	/ dont clutter up terminal with crap about create errors

	sys	stat; cpathname; statb
	bic	$!IFMT,statb+4
	cmp	$ISDIR,statb+4
	jne	crterr
	jbr	5f

3:
	cmp	r1,$ISDIR
	jne	5f
	mov	$cpathname,r0
	mov	$cpathname+1,r5
	mov	$namewk,r1
3:
	movb	(r0),(r1)+
	beq	3f
	cmpb	(r0)+,$'/
	bne	3b
	mov	r0,r5
	br	3b
3:
	movb	$'/,-1(r1)
	movb	$'.,(r1)+
	clrb	(r1)
	sys	link; cpathname; namewk
	movb	$'.,(r1)+
	clrb	(r1)
	movb	-(r5),r1
	clrb	(r5)			/name of parent directory
	sys	link; cpathname; namewk
	movb	r1,(r5)
	br	5f
0:
	bit	$IFMT,cmode
	bne	1f		/ --> not a plain file(ignore since flc=0)
	jsr	r5,verify; 'x
		rts pc
	mov	r4,-(sp)
	mov	exta,r4			/ position of file
	mov	ctapea,r5		/ position of file
	bic	$tbloking-1,r5		/ block address
	sub	tapad,r5		/ seek distance
	sbc	r4			/ seek distance
	sub	tapad+2,r4		/ seek distance
	ashc	$0,r4			/ test seek distance (32-bits)
	beq	0f			/ tape position correct
	bgt	4f			/ move tape forward
	iot				/ this cannot happen.
					/ directory is searched in order
					/ It must be mangled to get here
4:
	div	$tbloking,r4		/ convert to tape blocks
	mov	r4,r5			/ the number of tape blocks.
4:
	jsr	pc,tread		/ read a record
	add	$tbloking,tapad		/ update tape address
	adc	tapad+2			/ update tape address
	sob	r5,4b
0:
	mov	(sp)+,r4
	jsr	pc,xunlnk
	mov	cmode,0f
	sys	0; 9f
.data
9:
	sys	creat; cpathname; 0:..
.text
	bes	crterr1		/ unable to create the file ??

	mov	r0,r2		/ save file descriptor
	mov	ctapea,r5	/ tape address of the file

.if	lock
	bit	$FLOCKED,cmode
	beq	2f
	sys	lock; writelock
.endif

2:
	bic	$![tbloking-1],r5		/ get block displacement in tape buffer
	ash	$9,r5		/ get as byte displacement
	mov	r5,9f
	add	$tapeb,9f	/ start writing from here.
	add	$-tbloksiz,r5
	neg	r5		/ bytes left in buffer
	mov	r5,8f
	sub	r5,csize+2
	sbc	csize	/ is this all the file ??
	bge	0f		/ --> no.
	add	csize+2,r5
	mov	r5,8f
	beq	2f		/ --> if null file/multiple of ....
	clr	r5
0:
	mov	r2,r0
	sys	0; 7f
.data
7:
	sys	write; 9: 0; 8: 0
.text
	bes	crterr1		/ write error
	cmp	r0,8b
	bne	crterr1		/ write error (out of space probably)
	tst	r5
	ble	2f		/ all done
	jsr	pc,tread
	add	$tbloking,tapad	/ update tape address
	adc	tapad+2		/ update tape address
	mov	tapad,r5
	br	2b		/ go again
2:
	mov	r2,r0
	sys	close
5:
	mov	cuid,0f
	sys	0; 9f
.data
9:
	sys	chown; cpathname; 0:..
.text
	mov	ctime,r0
	mov	r1,-(sp)
	mov	ctime+2,r1
	sys	0; 9f
.data
9:
	sys	smdate; cpathname
.text
	mov	(sp)+,r1
1:
	rts	pc

crterr1:
	clr	r0
	mov	r1,-(sp)
	clr	r1
	sys	smdate; cpathname
	mov	(sp)+,r1
	mov	r2,r0
	sys	close

crterr:
	mov	$cpathname,r1
	jsr	pc,pstr
	jsr	r5,mesg
		< -- create error\n\0>; .even
	rts	pc


/
.data
tapad:		0;0		/ first address in second file.
				/ 32-bits for xtract, 16-bits otherwise
				/ second 16 are the most significant.
tpsize:		tapesize	/ number of feet on tape.
recordln:	pereclen		/ size of a single record in inches.
ebufpt:		tapeb + dirsiz
dirlast:	-1		/ count of level of empty directories
ldt:
		</etc/last-dump-time\0>
dotname:
		<.\0>
tmpdirname:
		</tmp/temp.dtp.>
xxxxx:		<xxxxx\0>

magtpnm:
		magtape
magtape:
		</dev/r>
magt:
		<mt>
magu:
		<0\0>
.even
.bss
tapeb: . = . +tbloksiz
tapebend = .
dirfd: . = .+2
ch:	.=.+1
fli:	.=.+1
flu:	.=.+1
fls:	.=.+1
flc:	.=.+1
flv:	.=.+1
flq:	.=.+1
fld:	.=.+1
.even

seltim: .=.+4	/ all files modified after this time dumped
tfdev:	.=.+2	/ dev of temp file
tfino:	.=.+2	/ inode number of temp file
command:.=.+2
size:	.=.+2
nused:	.=.+2		/ no. of blocks used on tape
nfree:	.=.+2
catlb:	.=.+20.
narg:	.=.+2
rnarg:	.=.+2
parg:	.=.+2
fio:	.=.+2
exta:	.=.+2	/ used by gettape, xtract, taboc to simulate 32-bit
		/ tape addresses - used as an extension of ctapea
mss:	.=.+2
statb:	.=.+40.

/	incore directory structure

cde:
cpathname:	.=.+114.		/ file pathname (null terminated)
cmode:		.=.+2.		/ file access bits
cuid:		.=.+2.		/ the owner
csize:		.=.+4.		/ file size (bytes)
cmajmin = 	csize+2		/ for special files is maj/min number
ctime:		.=.+4.		/ last modified time (secs since 1970)
ctapea:		.=.+2.		/ multiple by 512 to get byte address on tape of file

/	tape directory structure

tpathname = 0		/ file pathname (null terminated)
pathnaml = 114.		/ pathname max length. must be even length.
tmode = 114.		/ file access bits
tuid =116.		/ the owner
tsize = 118.		/ file size (bytes)
ttime = 122.		/ last modified time (secs since 1970)
ttapea = 126.		/ multiple by 512 to get byte address on tape of file
dirsiz = 128.		/ must be a power of 2 and a factor of 512.
dirresvd = 15. * dirsiz
diruplim = tbloksiz - dirresvd

/	tape header entry -- ( DUMMY directory entry )

tapeheader:
nfiles:		.=.+2		/ files dumped this tape
dumptime:	.=.+4		/ dump commenced at this time
ndirblks:	.=.+2		/ number of directory blocks
		. = tapeheader + dirsiz

/	Name scratch area

namewk:	.=.+pathnaml
