	.insrt	"../h/sysequ.h"
	.insrt	"../h/toolmacs.h"

|
| from macmisc.text
|	miscellanous macintosh interface routines
|	this version for traps based system
|	it also includes stortraps
|
|
|	written by andy hertzfeld	16-oct-82
|
|	modification history
|
|	25-jan-83	ajh	modified for new file/io system
|	12-jun-83	ajh	added opendeskacc
|
|------------------------------------------------

|
|function gettrapaddress(trapnum: integer): longint;
|

		.text
		.globl	_gettrapa
_gettrapa:

	movl	sp@+,a1	|return address
	movw	sp@+,d0	|trap word
	movl	a1,sp@-	|restore return address
	.word	__gettrapaddress
	movl	a0,sp@(4)	|return corresponding address
	rts
|
|procedure settrapaddress(trapaddr: longint; trapnum: integer);
|
		.text
		.globl	_settrapa
_settrapa:

	movl	sp@+,a1	|return address
	movw	sp@+,d0	|trap number
	movl	sp@+,a0	|address
	movl	a1,sp@-	|restore return address
	.word	__settrapaddress
	rts

|
|function writeparam: oserr;
|

		.text
		.globl	_writepar
_writepar:
	lea	sysparam,a0	|pointer to buffer to write to c chip
	moveq	#-1,d0	|write out all 20 bytes
	.word	__writeparam
	movw	d0,sp@(4)	|return error
	rts

|
|function readdatetime(var time: longint):oserr;
|

		.text
		.globl	_readdate
_readdate:

	movl	sp@+,a1	|get the return address
	movl	sp@+,a0	|get the parameter
	movl	a1,sp@-	|restore return address

	.word	__readdatetime
	movw	d0,sp@(4)	|return error

	rts

|
|procedure getdatetime(var secs: longint);
|
|justs returns the value of the global variable time

		.text
		.globl	_getdatet
_getdatet:

	movl	sp@+,a1	|get the return address
	movl	sp@+,a0	|get the parameter
	movl	time,a0@
	jmp	a1@

|
| function setdatetime(time: longint):oserr;
|
|	sets the hardware calendar/clock to the specified date/time.
|
		.text
		.globl	_setdatet
_setdatet:

	movl	sp@+,a0	|get the return address
	movl	sp@+,d0	|get the parameter
	movl	a0,sp@-	|restore return address

	.word	__setdatetime
	movw	d0,sp@(4)	|return error

	rts

|
|procedure settime(d: datearray);
|
|converts a date (encoded as an array of integers) to the numver of seconds
|elapsed since 00:00am, 1 jan 1904, andw stores in clock chip.

		.text
		.globl	_settime
_settime:

	movl sp@(4),a0	|date
	.word	__date2secs	|d0 = seconds since 1904
	.word	__setdatetime	|clock = seconds since 1904
	|moveml sp@+,a0-a1	|a0=ret, a1=date
	moveml sp@+,#0x0300	|a0=ret, a1=date
	jmp	a0@	|return

|
|procedure gettime(var d: datetimerec);
|
| reads the time from the clock as the number of seconds elapsed since 00:00am,
|1 jan 1904, andw returns the date encoded as an array of integers.

		.text
		.globl	_gettime
_gettime:
	moveq #0,d0
	movl sp@(4),a0	|ptr to date time rec
|	.word	__readdatetime
|	movl a0@,d0
	movl	time,d0	|get time from low memory
	.word	__secs2date
	movl a7@+,a7@
	rts

|
|procedure date2secs(d: datearray; var s: longint);
|
|converts a date (encoded as an array of integers) to the number of seconds
|elapsed since 00:00am, 1 jan 1904

		.text
		.globl	_date2sec
_date2sec:

	movl sp@(8),a0	|a0=date
	.word	__date2secs	|d0=seconds elapsed
	moveml sp@+,#0x0302 |d1=ret, a0=intptr, a1=date
	|moveml sp@+,d1/a0-a1	|d1=ret, a0=intptr, a1=date
	movl	d0,a0@	|deliver result
	movl d1,a0	|ret addr
	jmp	a0@

|
|procedur secs2date(s: longint; var d: datearray);
|
|turns s, the number of seconds elapsed since 00:00am, 1jand 1904, into an
|array of date values

		.text
		.globl	_secs2dat
_secs2dat:
	|moveml sp@+,d0/a0-a1	|d0=ret, a0=date, a1=s
	moveml sp@+,#0x0301	|d0=ret, a0=date, a1=s
	movl	d0,sp@-	|restore return addr
	movl	a1,d0	|place d0=s
	.word	__secs2date	|convert d0 to a0
	rts


|
|procedure delay(numticks: longint; var finalticks: longint);
|	.macro	.word	__delay

		.text
		.globl	_delay
_delay:

	movl	sp@+,d0	|get the return address
	movl	sp@+,a1	|ptr to where to return result
	movl	sp@+,a0	|numticks
	movl	d0,sp@-	|restore return address

	.word	__delay
	movl	d0,a1@	|return value of ticks after delay

	rts


|
| procedure sysbeep(duration: integer);
|jul 13, 1984	changed this to be inline
|
|sysbeep:
|
|	movw	sp@(4),sp@-	;push duration
|	.word	__sysbeep	;make the sound
|	movl	sp@+,a0
|	addw	#2,sp
|	jmp	a0@


|
|function equalstring(str1,str2: osstr255; casesens,diacsens: boolean):boolean;
|

		.text
		.globl	_equalstr
_equalstr:
	link	a6, #0	| allocate stack frame

	movl	a6@(16),a0	|ptr to str1
	movl	a6@(12),a1	|ptr to str2
	moveq	#0,d0
	movb	a0@+,d0	|str1 length
	swap	d0
	movb	a1@+,d0	|str2 length

	|depending on value of booleans, make proper call
	tstb	a6@(8)	|value of diacsens
	beqs	2$	|strip diacriticals
	tstb	a6@(10)	|value of casesens
	beqs	1$	|ignore case
	.word	__cmpstring+_case_	|both diacritical andw case sensitive
	bras	strdone

1$:	.word	__cmpstring	|diacritical sensitive,map to upper case
	bras	strdone

	|strip diacriticals
2$:	tstb	a6@(10)	|case sensitive?
	beqs	3$
	.word	__cmpstring+_marks_+_case_ |ignore diacrits, case sensitive
	bras	strdone

3$:	.word	__cmpstring+_marks_	|ignore diacrits andw map to upper case

strdone:	eorb	#1,d0	|take opposite of what cmpstring returns
	movb	d0,a6@(20)	|return result

	unlk	a6	| restore stack
	movl	sp@+,a0	| rtn addr
	addl	#12,sp	| pop args
	jmp	a0@

|
|procedure uprstring(var thestring: osstr255; diacsens: boolean);
|

		.text
		.globl	_uprstrin
_uprstrin:

	movl	sp@+,a1	|get the return address
	movb	sp@+,d1	|diacritical sensitivity bool
	movl	sp@+,a0	|ptr to string to canonize
	moveq	#0,d0
	movb	a0@+,d0	|string length
	movl	a1,sp@-	|restore return address

	|decide which flavor of uprstring to call andw call it
	tstb	d1
	beqs	1$	|ignore diacriticals
	.word	__uprstring	|keep diacriticals
	bras	2$

1$:	.word	__uprstring+_marks_

2$:	rts
