/
/	this routine provides almost the same facilities as the ctime
/	supplied with unix
/	the aim being to use .... bytes less core .... and less time
/	for compatability with the old ctime.c
/		the following symbols are defined
/			_localtime	* functionally the same
/					* no time-zone
/			_dysize		+ functionally the same
/			_dmsize		+ functionally the same
/
/	ijtime()	return a character string of the form
/			Sun Sep 16 01:03:52 1977\n\0
/			this being the present system time.
/	ctime(tvec)	long tvec; return a similar string to above
/			the time used is contained in tvec.
/
/				ian johnstone	july 1975.
/
/*** use 1 for daylight saving.
/*** use 0 otherwise
timezone = 0.*60.*60.
/
secin8hr = 8.*60.*60.
day70t75 = 365.+365.+366.+365.+365.
.globl	_ctime,_localtime,_dysize,_dmsize
_localtime:
	mov	2(sp),r0
	mov	4(sp),r1
	jsr	pc,1f
	mov	$array,r0
	rts	pc
_ctime:
	mov	2(sp),r0
	mov	4(sp),r1
	br	1f
.globl	_ijtime
_ijtime:
	sys	time
1:	mov	r2,-(sp)
	mov	r3,-(sp)
	mov	r4,-(sp)

	sub	$timezone,r1
	sbc	r0		/ time zone correction.

	div	$secin8hr,r0	/ r0= no of 8 hr lots since start 1970
				/ r1= no of seconds into 8 hour lot
	mov	r0,r3
	clr	r2
	div	$3,r2		/ r2= no of days since start of 1970
				/ r3= no of 8 hours in day
	clr	r0
	div	$60.,r0		/ r0= no of minutes in 8 hr grp
				/ r1= no of seconds .......
	mov	r1,array+00
	mov	$second,r4			/ put in seconds
	jsr	pc,numbin

	mov	r0,r1
	clr	r0
	div	$60.,r0		/ r0= no of hour into 8 hr grp
				/ r1= no of minutes ..........
	mov	r1,array+02
	mov	$minute,r4			/ put in minutes
	jsr	pc,numbin

	mov	r3,r1
	mul	$8.,r1
	add	r0,r1		/ r1= no of hours.
	mov	r1,array+04
	mov	$hour,r4			/ put in hours
	jsr	pc,numbin

/	r2= no of dayes since start of 1970 (a thursday).
	clr	r0
	mov	r2,r1
	add	$4.,r1		/ adjust for starting on thursday
	div	$7.,r0		/ r1 = day of week 0->6 :: mon->sun

	mov	r1,array+14
	mov	$weekday,r3	/ put in day of week
	mov	$sun,r4		/ put in day of week
	jsr	pc,stringin

	mov	$69.,r1
0:	inc	r1
	mov	$365.,r0
	bit	$3,r1
	bne	1f
	inc	r0
1:	sub	r0,r2
	bge	0b
	add	r0,r2		/ r2= day number in current year
	mov	r2,array+16
	mov	$year+2,r4
	mov	r1,array+12
	jsr	pc,numbin	/ r1= year number

	mov	$28.,mtab+2	/ assume ordinary year
	clr	r0		/ leap year ??
	div	$4,r0
	tst	r1
	bne	2f
	mov	$29.,mtab+2	/ really a leap year
2:	mov	$mtab,r0
0:	sub	(r0)+,r2
	bge	0b
	add	-(r0),r2
	sub	$mtab,r0
	asr	r0		/ r0= month in year 0->11
	mov	r0,array+10
	inc	r2		/ r2= day in month 1->31
	mov	r2,array+6

	mov	r2,r1
	mov	$daymonth,r4			/ put in day of month
	jsr	pc,numbin

	mov	r0,r1
	mov	$month,r3	/ put in month name.
	mov	$jan,r4		/ put in month name.
	jsr	pc,stringin

	mov	(sp)+,r4
	mov	(sp)+,r3
	mov	(sp)+,r2
	mov	$date,r0
	rts	pc

.data
array:
	0	/ seconds
	0	/ minutes
	0	/ hours
	0	/ day of month (1-31)
	0	/ month (0-11)
	0	/ year - 1900
	0	/ day of week (sunday=0)
	0	/ day of year (0-365)
	0	/ always zero 
date:
weekday:	<    >
month:		<    >
daymonth:	<   >
hour:		<  :>
minute:		<  :>
second:		<   >
year:		<1975\n\0>
	.even
_dmsize:
mtab:	31.;28.;31.;30.;31.;30.;31.;31.;30.;31.;30.;31.
sun:	<SunMonTueWedThuFriSat>
jan:	<JanFebMarAprMayJunJulAugSepOctNovDec>
.text

	.even
numbin:
	mov	r0,-(sp)
	mov	r1,-(sp)
	clr	r0
	div	$10.,r0
	bis	$'0,r0
	bis	$'0,r1
	movb	r0,(r4)+
	movb	r1,(r4)
	mov	(sp)+,r1
	mov	(sp)+,r0
	rts	pc
stringin:
	mul	$3,r1
	add	r4,r1
	movb	(r1)+,(r3)+
	movb	(r1)+,(r3)+
	movb	(r1),(r3)
	rts	pc
/
/	dysize(y)
/	{
/		if( (y%4) == 0 ) return(366);
/		return(365);
/	}
/
_dysize:
	clr	r0
	mov	2(sp),r1
	div	$4,r0
	mov	$366.,r0
	neg	r1	/ c=0 if 0 remainder, c=1 if non-zero remainder
	sbc	r0
	rts	pc
