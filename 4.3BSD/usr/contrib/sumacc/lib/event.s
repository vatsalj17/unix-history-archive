	.insrt	"../h/sysequ.h"
	.insrt	"../h/toolmacs.h"

|the following are the implementation of the event mgr routines that are
|register based as oposed to stack based
|they are included here because they are ostraps, however the interfaces for
|them are in toolintf
|
|	function postevent(eventnum: integer; eventmsg: longint): oserr;
|
		.text
		.globl	_posteven
_posteven:
|
	movl	sp@+,a1	|a1 is return address
	movl	sp@+,d0	|32-bit quantity defined by the event
	movw	sp@+,a0	|word of event number
	movl	a1,sp@-	|
	.word	__postevent
	movw	d0,sp@(4)	|return 0=event posted; 1=not posted
	rts
|
| procedure flushevents(evtmasks: longint);
|
		.text
		.globl	_flusheve
_flusheve:
	movl	sp@+,a1	|pop return address
	movl	sp@+,d0	|pop long word of event masks
	movl	a1,sp@-
	.word	__flushevents
	rts

|
| procedure seteventmask;
|
		.text
		.globl	_setevent
_setevent:
|
	movl	sp@+,a1
	movw	sp@+,sysevtmask	|get the mask
	jmp	a1@	| return to caller

|
|function oseventavail(mask: integer; var theevent: eventrecord): boolean;
|

		.text
		.globl	_oseventa
_oseventa:

	movl	sp@+,a1	|return address
	movl	sp@+,a0	|user event record
	movw	sp@+,d0	|event mask (set of events desired)
	movl	a1,sp@-	|push return address
	.word	__oseventavail
	tstw	d0
	beqs	1$	|non-null event returned

	clrb	sp@(4)	|null event returned
	bras	2$

1$:	movb	#1,sp@(4)	|non-null event returned
2$:	rts

|
|function getosevent(mask: integer; var theevent: eventrecord): boolean;
|

		.text
		.globl	_getoseve
_getoseve:

	movl	sp@+,a1	|return address
	movl	sp@+,a0	|user event record
	movw	sp@+,d0	|event mask (set of events desired)
	movl	a1,sp@-	|push return address
	.word	__getosevent
	tstw	d0
	beqs	1$	|non-null event returned

	clrb	sp@(4)	|null event returned
	bras	2$

1$:	movb	#1,sp@(4)	|non-null event returned
2$:	rts


