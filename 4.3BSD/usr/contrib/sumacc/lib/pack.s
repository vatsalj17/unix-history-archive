|
| file packtraps.text
|
| implementation for the interface to packages
|

| math package

	.insrt	"../h/sysequ.h"
	.insrt	"../h/toolmacs.h"

|
|macro for package invocation
|
|	first arg is routine selector, second is the trap

	.macro	packcall,subnum,trapnum
	movw	subnum,sp@-
	trapnum
	.endm

|
|procedure sfputfile(where: point| prompt: str255| origname: str255|
|	dlghook: procptr| var reply: sfreply)|
|

		.text
		.globl	_sfputfil
_sfputfil:

	moveq	#1,d2	|stuff the opcode

gopack:	movl	sp@+,a0	|get user's rts
	movw	d2,sp@-	|the routine selector
	movl	a0,sp@-	|push the rts back on
	.word	/adea	|__pack3 w/autopop
	|never gets here

|procedure sfpputfile(where: point| prompt: str255| origname: str255|
|	dlghook: procptr| var reply: sfreply| dlgid: integer|
|	filterproc: procptr)|
|

		.text
		.globl	_sfpputfi
_sfpputfi:

	moveq	#3,d2
	bra	gopack
|
|procedure sfgetfile(where: point| prompt: str255| filefilter: procptr|
|	numtypes: integer| typelist: sflistptr| dlghook: procptr|
|	var reply: sfreply)|
|
		.text
		.globl	_sfgetfil
_sfgetfil:

	moveq	#2,d2
	bra	gopack
|
|procedure sfpgetfile(where: point| prompt: str255| filefilter: procptr|
|	numtypes: integer| typelist: sflistptr| dlghook: procptr|
|	var reply: sfreply| dlgid: integer| filterproc: procptr)|
|

		.text
		.globl	_sfpgetfi
_sfpgetfi:

	moveq	#4,d2
	bra	gopack


|
|procedure diload|
|

		.text
		.globl	_diload
_diload:
	moveq	#2,d2	|stuff the opcode

godi:	movl	sp@+,a0	|get user's rts
	movw	d2,sp@-	|the routine selector
	movl	a0,sp@-	|push the rts back on
	.word	/ade9	|__pack2 w/autopop
	|never gets here


|
|procedure diunload|
|
		.text
		.globl	_diunload
_diunload:
	moveq	#4,d2
	bra	godi

|
|function dibadmount(where: point| evtmessage: longint): oserr|
|
		.text
		.globl	_dibadmou
_dibadmou:
	moveq	#0,d2
	bra	godi


|
|function diformat(drvnum: integer): oserr|
|
		.text
		.globl	_diformat
_diformat:
	moveq	#6,d2
	bra	godi


|
|function diverify(drvnum: integer): oserr|
|
		.text
		.globl	_diverify
_diverify:
	movw	#8,d2
	bra	godi


|
|function dizero(drvnum: integer| volname: str255): oserr|
|
		.text
		.globl	_dizero
_dizero:
	movw	#10,d2
	bra	godi

|
|implementation of interface to international utilities
|
|
|function iugetintl(theid: integer): handle|
|

		.text
		.globl	_iugetint
_iugetint:
	moveq	#6,d2	|stuff the opcode

gointl:	movl	sp@+,a0	|get user's rts
	movw	d2,sp@-	|the routine selector
	movl	a0,sp@-	|push the rts back on
	.word	/aded	|__pack6 w/autopop
	|never gets here

|
|procedure iusetintl(refnum: integer| theid: integer| intlparam: handle)|
|

		.text
		.globl	_iusetint
_iusetint:
	moveq	#8,d2
	bra	gointl


|
|procedure iudatestr(datetime: longint| longflag: integer| var result: str255)|
|

		.text
		.globl	_iudatest
_iudatest:
	moveq	#0,d2
	bra	gointl


|
|procedure iudatepstr(datetime: longint| longflag: integer| var result: str255|
|	intlparam: handle)|
|

		.text
		.globl	_iudateps
_iudateps:
	moveq	#14,d2
	bra	gointl


|
|procedure iutimestr(datetime: longint| wantseconds: boolean| var result: str255)|
|

		.text
		.globl	_iutimest
_iutimest:
	moveq	#2,d2
	bra	gointl


|
|procedure iutimepstr(datetime: longint| wantseconds: boolean| var result: str255|
|	intlparam: handle)|
|

		.text
		.globl	_iutimeps
_iutimeps:
	moveq	#16,d2
	bra	gointl


|
|function iumetric: boolean|
|

		.text
		.globl	_iumetric
_iumetric:
	moveq	#4,d2
	bra	gointl


|
|function iumagstring(aptr,bptr: ptr| alen,blen: integer): integer|
|

		.text
		.globl	_iumagstr
_iumagstr:
	movw	#10,d2	|the routine selector
	bra	gomag

|
|function iumagidstr(aptr,bptr: ptr| alen,blen: integer):integer|
|

		.text
		.globl	_iumagids
_iumagids:
	movw	#12,d2	|the routine selector
gomag:	movl	sp@+,a0	|get user's rts
	movw	d2,sp@-	|the routine selector
	movl	a0,sp@-	|push the rts back on
	.word	/aded	|__pack6 w/autopop
	|never gets here

|
|function iucompstring(astr,bstr: str255): integer|
|
		.text
		.globl	_iucompst
_iucompst:
	movw	#10,d2	|the routine selector
	bra	gopmag

|
|function iuequalstring(astr,bstr: str255): integer|
|
		.text
		.globl	_iuequals
_iuequals:
	movw	#12,d2	|the routine selector

gopmag:	|first readjust the stack so that it follows jerome's
	|string passing convention and not pascals
	|jerome expectes

	movl	sp@(4),a1	|ptr to bstr
	movl	sp@(8),a0	|ptr to astr
	clrw	d0
	clrw	d1
	movb	a0@+,d0	|alen
	movb	a1@+,d1	|blen
	movl	a0,sp@(8)	|first arg is ptr to astr
	movl	a1,sp@(4)	|second arg is ptr to bstr
	movl	sp@+,a0	|the return addr
	movw	d0,sp@-	|third arg is alen
	movw	d1,sp@-	|fourth arg is blen
	movw	d2,sp@-	|the routine selector
	movl	a0,sp@-	|push the rts back on
	.word	/aded	|__pack6 w/autopop
	|never gets here


|
| procedure stringtonum(thestring: str255| var thenum: longint)|
|
|	convert a string into a number using jerome's routine
|
		.text
		.globl	_stringto
_stringto:
	movl	sp@(8),a0
	packcall	#1,__pack7	|ldec2bin
	movl	sp@(4),a0
	movl	d0,a0@
	bra	exit8


|
| procedure numtostring(thenum: longint| var thestring: str255)|
|
|	convert a number to a string using anonymous routine
|
		.text
		.globl	_numtostr
_numtostr:
	.globl	exit8

	movl	sp@(4),a0
	movl	sp@(8),d0
	packcall	#0,__pack7	|lbin2dec

exit8:	movl	sp@+,a0
	addqw	#8,sp
	jmp	a0@

