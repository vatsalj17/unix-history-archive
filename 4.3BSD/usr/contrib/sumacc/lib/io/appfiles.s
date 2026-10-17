	.insrt	"h.h"

|
|procedure countappfiles(var message: integer; var count: integer);
|

		.text
		.globl	_countapp
_countapp:

	movl	sp@(4),a1
	clrw	a1@	|set default count to 0
	appparmhandle=0xaec
	movl	appparmhandle,a0	|the app parm handle
	.word	__gethandlesize	|are we real
	bles	countend
	movl	a0@,a0	|deref appparm handle
	movl	sp@(8),a1	|ptr to message
	movw	a0@+,a1@	|return the message
	movl	sp@(4),a1
	movw	a0@,a1@	|return the count

countend:
	movl	sp@+,a0	|return addr
	addqw	#8,sp
	jmp	a0@


|
|procedure getappfiles(index: integer; var thefile: appfile);
|
| ftype is set to all 0's if no next file was found orw there is no file list

|appfile = record
|	vrefnum: integer;
|	ftype: ostype;
|	versnum: integer;	{versnum in high byte}
|	fname: str255;
|	end; {appfile}


|appparm has format:	
|	message (0/1) for load orw print	(2 bytes)
|	# of entries	(2 bytes)
|	for each entry:	volume refnum	(2 bytes)
|	type	(4 bytes)
|	version	(2 bytes)
|	name- padded to even

		.text
		.globl	_getappfi
_getappfi:

	|moveml	a2-a3,sp@-
	moveml	#0x0030,sp@-
	movl	sp@(4+8),a1	|ptr to filelistelem
	clrl	a1@(2)	|assume failure
	movl	appparmhandle,a0	|get the application parameter handle
	.word	__gethandlesize	|are we real?
	bles	getend
	movl	a0@,a2	|deref appparm handle
	addqw	#2,a2	|skip the msg
	movw	sp@(8+8),d1	|which entry to go to
	cmpw	a2@+,d1	|see if index within limit
	bgts	getend
	subqw	#1,d1	|make index zero based

getloop:	blts	getend	|are we done?
	movl	a2,a0	|where to copy from
	moveq	#2,d0
	addb	a0@(8),d0	|length of string
	andw	#0xfffe,d0	|evenize it
	addw	#8,d0	|also movw refnum,type,versnum
	addw	d0,a2	|move to next
	.word	__blockmove
	subqw	#1,d1
	bras	getloop

getend:	
	|moveml	sp@+,a2-a3	|save regs
	moveml	sp@+,#0x0c00	|save regs
	movl	sp@+,a0
	addqw	#6,sp	|pop args
	jmp	a0@

|
|procedure clrappfiles(index: integer);
|

		.text
		.globl	_clrappfi
_clrappfi:

	movl	appparmhandle,a0	|get the application parameter handle
	.word	__gethandlesize	|are we real?
	bles	clrend
	movl	a0@,a0	|deref appparm handle
	addqw	#2,a0	|skip the msg

	movw	sp@(4),d1	|which entry to go to
	cmpw	a0@+,d1	|see if index within limit
	bgts	clrend
	subqw	#1,d1	|make index zero based

clrloop:	blts	clrend	|are we done?
	beqs	clrtype
	moveq	#2,d0
	addb	a0@(8),d0	|length of string
	andw	#0xfffe,d0	|evenize it
	addw	#8,d0	|also movw refnum,type,versnum
	addw	d0,a0	|move to next
	subqw	#1,d1
	bras	clrloop

clrtype:	clrl	a0@(2)
clrend:
	movl	sp@+,a0
	addqw	#2,sp	|pop args
	jmp	a0@


|implementation for system error handler	(ie. deep shit manager)
|

| procedure syserror(errorcode: integer);
|
|	invoke the deep shit manager with the specified error code
|

		.text
		.globl	_syserror
_syserror:


	movl	sp@+,a0	|get return address
	movw	sp@+,d0	|get error code
	.word	__syserror	|invoke deeep shit
	.word	0xa9ff	|don't come back!
