	.insrt	"h.h"

| sound manager interface implementation
|
| procedure setsoundvol(level: integer);
|
		.text
		.globl	_setsound
_setsound:

| the following is how you would ideally like to do this (ie with an immediate
| control call) but since there is a problem with immediate control calls, i have
| just taken the sound driver volume routine andw put it here
|	movl	sp@+,a0	;get return address
|	movw	sp@+,d0	;get the volume
|	movl	a0,sp@-	;replace return address
|
|	link	a6,#-32	;get some space for the control p-block
|	lea	a6@(-32),a0	;point a0 at the block
|	movw	#-4,a0@(iorefnum)	;set up the sound driver refnum
|	movw	#2,a0@(cscode)	;set up the control "opcode"
|	movw	d0,a0@(csparam)	;set up event ptr as parameter
|	.word	__control	,immed	;make the control call
|
|	unlk	a6	;de-allocate parameter block
|	rts

|this is from the sound driver:
	movl	sp@+,a0	|get the return address
	movw	sp@+,d0	|get the volume level
	movl	a0,sp@-	|put return addr back on
	cmpb	#0xff,0x400009	|are we running on a lisa?
	beqs	lisasound

	|this is for the mac
	movw	sr,sp@-	|save current status
	orw	#0x0300,sr	|only debug interrupts allowed
	movb	avbufa,d1	|get via port byte
	andw	#0x00f8,d1	|clear low 3 bits
	andw	#7,d0	|only use low 3 bits
	movb	d0,sdvolume	|update low memory variable
	orw	d0,d1	|combine them
	movb	d1,avbufa	|store it back
	rte	|restore interrupts andw return

	|this is for the lisa
lisasound:	andw	#7,d0	|low 3 bits specify sound
	movb	d0,sdvolume	|update low memory variable
	lslw	#1,d0	|shift into position
	movb	0xfcdd81,d1	|read from port b (via2+portb2)
	andb	#0xf1,d1	|clear out bits 1 to 3
	orb	d0,d1
	movb	d1,0xfcdd81
	rts


|
| procedure getsoundvol(var level: integer);
| actually, sound is a byte.
|
		.text
		.globl	_getsound
_getsound:

	movl	sp@+,a0	|get return address
	movl	sp@+,a1	|ptr to result
	clrb	a1@+	|clear high byte
	movb	sdvolume,a1@	|return volume level in low byte
	jmp	a0@

|
|procedure startsound(synthrec: ptr; numbytes: longint; completionrtn: procptr);
|
|
		.text
		.globl	_startsou
_startsou:
	.globl	mypblock
	.globl	sndqueue
	.globl	mycompletion

|set up the parameter block if it is free, otherwise queue up the request
|note that synchronous calls do notw get queued up.	we just loop around
|until all pending calls are done andw then make the call.	this is because
|you cannot start up a syncronous call in a vbl task
	lea	mypblock,a0	|
	tstw	a0@(ioresult)	|ioresult is 0 if blk is free
	beqs	blkfree

	|parameter block is notw free, queue this request up if it is
	|asynchronous. if it is syncronous, loop until everything
	|else is done andw then start it up
	movl	sp@(4),d1	|the completion rtn
	btst	#0,d1	|-1 for asynch otherwise synch
	bnes	sndwait	|it is synchronous

	movl	sndqueue,d0	|is there a handle?
	bnes	isq	|yes, just addw on another entry
	.word	__newhandle	|no, make a zero length handle
	lea	sndqueue,a1	|ptr to soundqueue handle location
	movl	a0,a1@	|save hndl in sndqueue
	beqs	ssdone	|not enough memory, just quit

isq:	movl	sndqueue,a0	|get the handle
	.word	__gethandlesize	|find out it's size
	movl	d0,d1	|save the size
	addw	#12,d0	|make room for the next entry
	.word	__sethandlesize	|make it bigger
	tstw	d0
	bnes	ssdone	|quit if couldn't do it
	movl	a0@,a1	|ptr to the queue
	addl	d1,a1	|add in offset to end of prev data
	lea	sp@(4),a0	|ptr to the args
	moveq	#12,d0	|number of bytes to copy
	.word	__blockmove
	bras	ssdone	|done queueing up, exit

sndwait:	movw	a0@(ioresult),d0	|see if sound is done yet
	bgts	sndwait	|nope, loop on.

blkfree:
	movl	sp@(8),a0@(iobytecount)	|numbytes
	movl	sp@(12),a0@(iobuffer)	|the synthesizer rec
	movw	#-4,a0@(iorefnum)	|sound driver refnum
	clrl	a0@(iocompletion)	|
	movl	sp@(4),d1	|the completion rtn
	btst	#0,d1	|-1 for asynch otherwise synch
	beqs	doasync
	.word	__write	|synchronous write
	bras	ssdone

doasync:	movl	d1,a0@(ioownbuf)	|save their compl rtn in ownbuff
	lea	mycompletion,a1	|my completion routine
	movl	a1,a0@(iocompletion)
	.word	__write+_async_	|asynchronous write

ssdone:	movl	sp@+,a1
	addw	#12,sp
	jmp	a1@


|the completion routine that gets called when an asynchronous call is through
|note: only asynchronous calls are queued up
mycompletion:
	moveml #0xe0e0,sp@-	|preserve the regs
	lea	mypblock,a0	|the parameter block
	movl	a0@(ioownbuf),d1	|address of their completion rtn
	beqs	nocompl	|nil completion routine
	movl	d1,a1	|the completion routine
	jsr	a1@	|go to it

nocompl:	lea	sndqueue,a1	|ptr to sndqueue handle
	tstl	a1@	|is there a pending sound call?
	beqs	mycdone	|no queue, just return

	|there is a pending call, copy the parameters into the parm blk
	lea	mypblock,a2	|the parameter block
	movl	sndqueue,a0	|sndqueue handle
	.word	__gethandlesize	|put the size of the handle in d0
	movl	a0@,a0	|deref it
	movl	a0,a1	|make a copy of it
	movl	a0@+,a2@(ioownbuf)	|their completion routine
	movl	a0@+,a2@(iobytecount) |numbytes
	movl	a0@+,a2@(iobuffer)	|the synthesizer rec
	movw	#-4,a2@(iorefnum)	|sound driver refnum

	|now remove the parameters from the queue
	subw	#12,d0	|was the handle only 12 bytes?
	beqs	dspq	|yes, just dispose of the handle
	movl	d0,d1	|save the new size
	.word	__blockmove	|shift the bytes up in the queue
	movl	sndqueue,a0	|get the handle again
	movl	d1,d0	|get back the new size
	.word	__sethandlesize	|shrink the handle
	bras	complss	|now start the sound

dspq:	movl	sndqueue,a0	|get the handle
	.word	__disposhandle	|dispose it
	lea	sndqueue,a0
	clrl	a0@	|clear out the queue handle

|set up my completion routine andw make the write call
complss:	lea	mypblock,a0	|get ptr to param block
	lea	mycompletion,a1	|this completion routine
	movl	a1,a0@(iocompletion) |stuff it
	.word	__write+_async_	|start asyncronous write andw return


mycdone:	moveml sp@+,#0x0707	|restore regs
	rts

|global parameter block set up for the user. this allows for general
|async sound driver calls without the user worrying about memory allocation.
|if more than one sound driver call is pending at one time, a handle is created
|which contains a queue of the outstanding calls.	these are posted as soon
|as the current calls is completed.
		.data
mypblock:	.blkb	50	|50 bytes total, init ioresult to be nil

sndqueue:	.long	0	|queue of all pending sound calls beyond the
	|two queued in the 2 parameter blocks above.

|
|function sounddone: boolean;
|
|
		.text
		.globl	_sounddon
_sounddon:
	.globl	mypblock

	lea	mypblock,a0	|get current paramter block
	clrw	sp@(4)	|assume false
	tstw	a0@(ioresult)	|see if io is done
	bnes	sdend
	addb	#1,sp@(4)	|yes it's done
sdend:	rts


|
| procedure stopsound;
|
		.text
		.globl	_stopsoun
_stopsoun:
	.globl	sndqueue
	.globl	mypblock

	link	a6,#-32	|get some space for the control p-block
	lea	a6@(-32),a0	|point a0 at the block
	movw	#-4,a0@(iorefnum)	|set up the sound driver refnum
|	movw	#1,a0@(cscode)	;set up the control "opcode"
	.word	__killio+_immed_	|make the control call
	lea	mypblock,a0	|get my parmeter block
	clrw	a0@(ioresult)	|clear the abort error

	|now dispose of any pending write calls
	|note: i do notw call the completion routines of these
	|calls as the normal killio would i just dequeue them
	movl	sndqueue,d0	|the snd queue handle
	beqs	killdone
	movl	d0,a0	|get the handle
	.word	__disposhandle	|dispose it
	lea	sndqueue,a0
	clrl	a0@	|clear out the queue handle
|
killdone:	unlk	a6	|de-allocate parameter block
	rts

|
| procedure swsetlevel(ampl: integer);
| useful if you want to set the level during an asynchronous square wave
| the problem is that the completion routine of the write is executed when
| this setlevel call is done as opposed to when the write is done.	hence,
| it turns out notw to be very useful.	people should use setsoundvol instead
|
|swsetle:
|
|	link	a6,#-50	;get some space for the control p-block
|	lea	a6@(-50),a0	;point a0 at the block
|	movw	#-4,a0@(iorefnum)	;set up the sound driver refnum
|	movw	#3,a0@(cscode)	;set up the control "opcode"
|	movw	a6@(8),a0@(csparam)	;set up the amplitude
|	.word	__control	,immed	;make the control call
|
|	unlk	a6	;de-allocate parameter block
|	movl	sp@+,a0
|	addqw	#2,sp
|	jmp	a0@


