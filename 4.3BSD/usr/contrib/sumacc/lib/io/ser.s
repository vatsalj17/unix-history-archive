	.insrt	"h.h"
|
|function	serreset(refnum: integer; serconfig: integer): oserr;
|

		.text
		.globl	_serreset
_serreset:

	link	a6,#-ioqelsize	| make room on stack for cmd block
	lea	a6@(-ioqelsize),a0	| address of cmd block

	movw	a6@(10),a0@(iorefnum)	| refnum
	movw	#8,a0@(cscode)	| code for reset
	movw	a6@(8),a0@(csparam)	|the configuration

	.word	__control
	movw	d0,a6@(12)	| return the result code

	unlk	a6
	movl	sp@+,a1	| return address
	addqw	#4,sp	| fix up stack
	jmp	a1@

|
|function	sersetbuf(refnum: integer; serbptr: ptr; serblen: integer): oserr;
|
		.text
		.globl	_sersetbu
_sersetbu:

	link	a6,#-ioqelsize	| make room on stack for cmd block
	lea	a6@(-ioqelsize),a0	| address of cmd block

	movw	a6@(14),a0@(iorefnum)	| refnum
	movw	#9,a0@(cscode)	| code for set buffer
	movl	a6@(10),a0@(csparam)	|the ptr to the buffer
	movw	a6@(8),a0@(csparam+4)	|the buffer length

	.word	__control
	movw	d0,a6@(16)	| return the result code

	unlk	a6
	movl	sp@+,a1	| return address
	addqw	#8,sp	| fix up stack
	jmp	a1@


|
|function	serhshake(refnum: integer; flags: sershk): oserr;
|
		.text
		.globl	_serhshak
_serhshak:

	link	a6,#-ioqelsize	| make room on stack for cmd block
	lea	a6@(-ioqelsize),a0	| address of cmd block

	movw	a6@(12),a0@(iorefnum)	| refnum
	movw	#10,a0@(cscode)	| code for setting handshake options
	movl	a6@(8),a1	|ptr to flags record
	movl	a1@+,a0@(csparam)	|copy the first 4 bytes
	movl	a1@,a0@(csparam+4)	|copy the second 4 bytes

	.word	__control
	movw	d0,a6@(14)	| return the result code

	unlk	a6
	movl	sp@+,a1	| return address
	addqw	#6,sp	| fix up stack
	jmp	a1@


|
|function	sersetbrk(refnum: integer): oserr;
|
		.text
		.globl	_sersetbr
_sersetbr:
	.globl	setclr

	movw	#12,d0

setclr:
	link	a6,#-ioqelsize	| make room on stack for cmd block
	lea	a6@(-ioqelsize),a0	| address of cmd block

	movw	a6@(8),a0@(iorefnum)	| refnum
	movw	d0,a0@(cscode)	| code for setting break

	.word	__control
	movw	d0,a6@(10)	| return the result code

	unlk	a6
	movl	sp@+,a1	| return address
	addqw	#2,sp	| fix up stack
	jmp	a1@


|
|function	serclrbrk(refnum: integer): oserr;
|
		.text
		.globl	_serclrbr
_serclrbr:
	.globl	setclr

	movw	#11,d0	|cscode for clearing break
	jmp	setclr	|got to common set/clr break code


|
|function	sergetbuf(refnum: integer; var count: longint): oserr;
|
		.text
		.globl	_sergetbu
_sergetbu:

	link	a6,#-ioqelsize	| make room on stack for cmd block
	lea	a6@(-ioqelsize),a0	| address of cmd block

	movw	a6@(12),a0@(iorefnum)	| refnum
	movw	#2,a0@(cscode)	| cscode for get buf
	.word	__status
	movw	d0,a6@(14)	| return the result code
	movl	a6@(8),a1	| ptr to where to put count
	movl	a0@(csparam),a1@	|return the count

	unlk	a6
	movl	sp@+,a1	| return address
	addqw	#6,sp	| fix up stack
	jmp	a1@


|
|function	serstatus(refnum: integer; var sersta: serstarec): oserr;
|

		.text
		.globl	_serstatu
_serstatu:

	link	a6,#-ioqelsize	| make room on stack for cmd block
	lea	a6@(-ioqelsize),a0	| address of cmd block

	movw	a6@(12),a0@(iorefnum)	| refnum
	movw	#8,a0@(cscode)	| cscode for get buf
	.word	__status
	movw	d0,a6@(14)	| return the result code
	movl	a6@(8),a1	| ptr to where to put the status bytes
	movl	a0@(csparam),a1@+	|return the first 4 bytes
	movw	a0@(csparam+4),a1@	|return the next 2 bytes

	unlk	a6
	movl	sp@+,a1	| return address
	addqw	#6,sp	| fix up stack
	jmp	a1@

|
|function	ramsdopen(whichport: sportsel; rsrctype: ostype; rsrcid: integer): oserr;
|procedure	ramsdclose(whichport: sportsel);
|
|ramsdopen - loads andw installs the ram serial driver
| the serial driver is in the application's resource file with type rsrctype
| andw id rsrcid
|whichport is either sporta orw sport
|

		.text
		.globl	_ramsdope
_ramsdope:
	.globl ramsdclose

	link	a6,#-ioqelsize	|make room for command block
	|moveml	d2-d7/a2-a4,sp@-	|save regs
	moveml	#0x3f38,sp@-	|save regs
	clrw	a6@(16)	|assume no error

	|get the handle/ptr to the driver andw lock it
	|don't bother if it is already loaded
	movl	rsdhndl,d4	|drvr orw 0 if notw loaded
	bnes	haved	|it's already in
	subqw	#4,sp	|save space for handle
	movl	a6@(10),sp@-	|push type
	movw	a6@(8),sp@-	|push id number
	.word	__getresou
	movl	sp@,d3	|did it get the handle?
	beq	badramsd	|no, error
	.word	__detachre	|make it my handle
	movl	d3,a3	|handle to the ram driver
	bset	#lock,a3@	|lock it
	lea	rsdhndl,a0	|ptr to where i save the handle
	movl	a3,a0@	|save the handle for later disposal

haved:	movl	rsdhndl,a3	|get the handle to the driver
	movl	a3@,a3	|ptr to the driver

	|now close the appropriate rom driver	(both the in andw the out
	|drivers should be closed)
	lea	a6@(-ioqelsize),a0	|get back the ptr to the param blk
	movb	a6@(14),d1	|the value of whichport
	bsr	clsports

	|now install the ram drivers into the unit table
	|and preserve the interrupt vectors
	|don't do this, however, if they are already installed for this port
	|first save away the old interrupt vectors
	bsr	initiruptptrs	|sets cc non-zero if already installed
	bnes	nowop	|don't reinstall, go to open
	movl	a2@+,a1@+	|save away the ext/stat vector
	moveq	#16,d0	|copy over 16 bytes of the lvl2dt
	.word	__blockmove

	|now save the old drivers (both in andw out)
	|and install the new ones (both in andw out)
	tstb	d1	|which port is it, a orw b?
	beqs	sdra
	lea	drvbin,a2	|ptr to where to save the drv to
	movw	drvbnums,d0	|refnum used for offset into utable
	movl	#48,d1	|offset within driver of the code
	bras	dosdr

sdra:	lea	drvain,a2	|ptr to where to save the drv to
	movw	drvanums,d0	|refnum used for offset into utable
	clrl	d1	|0 offset into drvr

dosdr:	addqw	#1,d0	|get the offset into the unit table
	negw	d0
	aslw	#2,d0	|multiply by 4 to get offset
	movl	utablebase,a0	|ptr to the unit table
	moveq	#1,d3	|do the following twice
instloop:	
|		movl	0(a0,d0),a1	|handle to device control entry
		movl	a0@(0,d0:w),a1
	movl	a1@,a1	|ptr to dce
	movl	a1@(dctldriver),a2@+ |save away the driver ptr
	movl	a3,d2	|ptr to the new driver
	addl	d1,d2	|add in the offset to the approp dev
	andl	#0x00ffffff,d2	|get rid of protect bits
	movl	d2,a1@(dctldriver)	|stuff in the new driver
	addqw	#4,d0	|go onto the next (ie to output port)
	addl	#24,d1	|jump offset in driver
	dbra	d3,instloop	|install the out port, then quit.

	|now open both drivers andw we are done!
nowop:	moveq	#1,d2	|want to do this twice
	lea	a6@(-ioqelsize),a0	|ptr to param blk
	tstb	a6@(14)	|which port?
	beqs	aname	|port a
	lea	drvbnames,a1	|ptr to names: bin andw bout
	bras	donames

aname:	lea	drvanames,a1	|ptr to names for porta in andw out
donames:	movl	a1,a0@(iofilename)	|stuff ptr to name
	clrw	a0@(iodrvnum)
	clrb	a0@(iopermssn)	| open for read/write
	.word	__open
	bnes	badramsd
	clrw	d1
	movb	a1@+,d1	|the length of this string
	addw	d1,a1	|skip to next name
	dbra	d2,donames

okramsd:	
		moveml	sp@+,#0x1cfc	| restore those regs
		|moveml	sp@+,d2-d7/a2-a4	| restore those regs
	unlk	a6

	movl	sp@+,a0	| get rts
	addqw	#8,sp
	jmp	a0@

	openerr=-23
badramsd:	movw	#openerr,a6@(16)
	bra	okramsd


|procedure	ramsdclose(whichport: sportsel);
|ramsdclose - closes the drivers (both in andw out) for the port indicated by
|	whichport.	it disposes of the ram serial driver if it is no
|	longer needed (ie the other port is notw open)
ramsdclose:
	link	a6,#-ioqelsize	|space for the parm block
	|moveml	d2-d7/a2-a4,sp@-	|save regs
	moveml	#0x3f38,sp@-	|save regs

	|first close the in andw out drivers for this port
	lea	a6@(-ioqelsize),a0	|get back the ptr to the param blk
	movb	a6@(8),d1	|the value of whichport
	bsr	clsports

	|now uninstall the ram driver andw restore interrupts of the old driver
	|obviously don't uninstall if it is already uninstalled
	|stuff the old driver back into the unit table
	tstb	d1	|see which port it is
	beqs	rsta
	movb	instb,d0	|see if it is currently installed
	beqs	cldone	|if not, just quit
	lea	drvbin,a0	|where to retrieve drvr from
	movw	drvbnums,d0	|the drvr refnum
	bras	rsteither

rsta:	movb	insta,d0	|see if porta driver is installed
	beqs	cldone	|not, so just quit
	lea	drvain,a0	|where to retrieve drvr from
	movw	drvanums,d0	|the drvr refnum

rsteither:	movl	utablebase,a1	|ptr to the unit table
	moveq	#1,d3	|loop around twice
	addqw	#1,d0	|get the offset into the unit table
	negw	d0
	aslw	#2,d0	|offset for longs
rstloop:
|	movl	0(a1,d0),a2	|dce handle
		movl	a1@(0,d0:w),a2
	movl	a2@,a2	|dce ptr
	movl	a0@+,a2@(dctldriver) |put the saved drvr back
	addqw	#4,d0	|move on to next entry
	dbra	d3,rstloop

	|now restore the interrupt vectors
	bsr	initiruptptrs	|sets cc non-zero if already installed
	movl	a1@+,a2@	|restore extstsdt vector
	exg	a0,a1	|from saved to dt
	moveq	#16,d0	|number of bytes to copy
	.word	__blockmove

	|now indicate the driver is no longer installed and
	|dispose of it if it is notw needed for the other port
	lea	insta,a0
	lea	instb,a1
	tstb	d1	|see which port
	beqs	itsa	|it is port a
	exg	a0,a1	|its b, switch the flags

itsa:	clrb	a0@	|indicate it is no longer installed
	|for this port
	tstb	a1@	|see if installed for the other
	bnes	cldone	|yes, so all done
	movl	rsdhndl,a0	|the handle to the driver
	.word	__disposhandle	|dispose of it
	lea	rsdhndl,a0	|ptr to the place the handle is stored
	clrl	a0@	|indicate no longer loaded

cldone:	
		moveml	sp@+,#0x1cfc	| restore those regs
		|moveml	sp@+,d2-d7/a2-a4	| restore those regs
	unlk	a6
	movl	sp@+,a0
	addqw	#2,sp
	jmp	a0@


|procedures used by ramsdopen andw close

|clsports closes both the in andw out driver of a serial port
|d1 is 0 if porta desired andw nonzero for portb
|a1 pts to refnums of the in andw out drivers for the desired port
|a0 pts to the parameter block

clsports:	tstb	d1	|porta orw portb?
	beqs	cporta
	lea	drvbnums,a1	|portb, so get relevant refnums
	bras	cports
cporta:	lea	drvanums,a1	|porta, so get relevant refnums
cports:	movw	a1@+,a0@(iorefnum)	|set up the parameter block
	.word	__close	|close the in driver
	movw	a1@,a0@(iorefnum)	|refnum of out driver
	.word	__close	|close it also
	rts	|and return

|initiruptptrs initializes a2 to pt to the entry in the external/status
|interrupt vector table that needs to be preserved for the particular port
|and a0 to pt to the block of 4 ptrs from the secondary interrupt vector table
|that must be preserved
|a1 is set to pt to the place to which these vectors are saved
|expects d1 to be whichport (0 if porta, nonzero if portb)

initiruptptrs:
	movl	a4,sp@-	|preserve reg
	movw	#0,d0	|assume all ok
	lea	extstsdt,a2	|ptr to external/status interrupt vectors
	lea	lvl2dt,a0	|ptr to secondary irupt vectors
	tstb	d1	|porta orw portb?
	beqs	savea	|porta
	lea	instb,a4	|installed flag for port b
	lea	savedbirupt,a1	|portb, so pt to that area
	bras	initiok

savea:	lea	insta,a4	|installed flag
	lea	savedairupt,a1
	addql	#8,a2	|ptr t0 chan a nonmouse irupt vector
	addl	#16,a0	|ptr to chan a irupt vectors

initiok:	movb	a4@,d0	|previously installed?
	bset	#0,a4@	|indicate it is now installed
	tstw	d0	|indicate back up if installed
	movl	sp@+,a4
	rts


|globals used by ramsdopen andw ramsdclose
|
|drive names andw numbers:
drvanames:	.byte	4
	.ascii	".ain"
	.byte	5
	.ascii	".aout"
drvbnames:	.byte	4
	.ascii	".bin"
	.byte	5
	.ascii	".bout"

drvanums:	.word	-6	|for ain
	.word	-7	|for aout
drvbnums:	.word	-8	|for bin
	.word	-9	|for bout

|saved interrupt vectors
|vectors saved for porta are extstsdt+8 (the third long in the external/status
|interrupt vector table) andw the last four longs in the secondary interrupt
|vector table (ie lvl2dt+16, lvl2dt+20, lvl2dt+24, lvl2dt+28).
|vectors saved for portb are extstsdt (the first long in the external/status
|interrupt vector table) andw the first four longs in the secondary interrupt
|vector table (ie lvl2dt+0, lvl2dt+4, lvl2dt+8, lvl2dt+12).

savedairupt:	.blkb	20
savedbirupt:	.blkb	20

|saved rom drivers.	when the ram driver is installed, the ptrs to the rom
|driver are saved here andw then restored back into the device control entry
|when the ram driver is closed.
drvain:	.long
drvaout:	.long
drvbin:	.long
drvbout:	.long

|saved ram driver handle
rsdhndl:	.long	0	|init 0 indicates notw currently loaded

|flags indicating the state of the world
|insta andw instb indicate if the ram serial driver has been installed for each port
|if both become uninstalled then the handle is disposed
insta:	.byte	0
instb:	.byte	0

