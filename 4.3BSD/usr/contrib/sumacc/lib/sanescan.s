|-----------------------------------------------------------
| 25may84: C version, uses macros to save my fingers. (croft)
|-----------------------------------------------------------
	.insrt	"../h/sanemacs.h"

__fp68k = /a9eb		| avoids including toolmacs.h
__elems68k = /a9ec

	.text

	.globl	_dec2str,_str2dec,p2cstr,c2pstr

	.macro	tostr,l,c
	.globl	l
l:	movw	#c,d0
	bras	tostring
	.endm
	.macro	frstr,l,c
	.globl	l
l:	movw	#c,d0
	bras	fromstring
	.endm

FL = -26		| frame length, space for struct Decimal

tostring:
	link	a6,#FL
	movl	a6@(8),sp@-	| DecForm *f;
	movl	a6@(12),sp@-	| floattype *x;
	pea	a6@(FL)		| Decimal *d;
	movw	d0,sp@-		| fp68k opcode, ?2Dec
	.word	__fp68k
	movl	a6@(8),sp@-	| DecForm *f;
	pea	a6@(FL)		| Decimal *d;
	movl	a6@(16),sp@-	| DecStr *y;  result
	jsr	_dec2str
	movl	a6@(16),sp@-	| convert to C string
	jsr	p2cstr
	unlk	a6
	rts

	tostr,S2Str,fs2dec
	tostr,D2Str,fd2dec
	tostr,C2Str,fc2dec
	tostr,X2Str,fx2dec

	.bss
decstr:	.blkb	82
	.text

fromstring:
	link	a6,#FL
	movw	d0,sp@-		| save fp68k op
	movl	a6@(8),a0	| convert C to P string
	movl	#decstr,a1
.L8:	movb	a0@+,a1@+
	bnes	.L8
	movl	#decstr,sp@-
	jsr	c2pstr
|	addql	#4,sp
|	movl	#decstr,sp@-	| DecStr *x;
	pea	a6@(FL)		| Decimal *d;
	jsr	_str2dec
	movw	sp@+,d0
	pea	a6@(FL)		| Decimal *d;
	movl	a6@(12),sp@-	| floattype *y;  result
	movw	d0,sp@-		| fp68k op
	.word	__fp68k		| convert Dec2?
	unlk	a6
	rts

	frstr,Str2S,fdec2s
	frstr,Str2D,fdec2d
	frstr,Str2C,fdec2c
	frstr,Str2X,fdec2x
