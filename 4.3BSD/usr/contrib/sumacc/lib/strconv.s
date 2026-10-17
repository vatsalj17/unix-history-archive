|
| convstr - string conversion routines.
|
| Copyright (C) 1984, Stanford Univ. SUMEX project.
| May be used but not sold without permission.
|
|
| history
| 05/25/84	Croft	Created.
| 05/31/84	Croft	Changed semantics slightly.
| 05/31/84	Croft	C-generated code was so crummy I converted to asm


| C to Pascal string.  Converts a C string "in place" to a Pascal
| string by shuffling bytes down one position and inserting a 
| byte count. 

	.text
	.globl	c2pstr,C2PStr
C2PStr:
c2pstr:
	movl	sp@(4),a0
	movw	#0,a1
	movb	a0@+,d0
.L1:	
	tstb	d0
	beqs	.L2
	movb	a0@,d1
	movb	d0,a0@+
	movb	d1,d0
	addql	#1,a1
	bras	.L1
.L2:
	movl	a1,d0
	movl	sp@(4),a0
	movb	d0,a0@
	movl	a0,d0		| function return value is string address
	rts


| Pascal to C string.  Converts a Pascal string "in place" to a C string.

	.globl	p2cstr,P2CStr
P2CStr:
p2cstr:
	movl	sp@(4),a0
	movl	a0,a1
	clrl	d0
	movb	a0@+,d0
	subql	#1,d0
	blts	.L3
.L4:
	movb	a0@+,a1@+
	bnes	.L44
	subql	#1,a1		| delete imbedded zeros
.L44:
	dbra	d0,.L4
.L3:
	clrb	a1@
	movl	sp@(4),d0	| function return value is string address
	rts


| C interpretation of Pascal string.  Called with pointer to Pascal string; 
| returns with pointer to C string.  Non-destructive.

	.globl	CofPStr
CofPStr:
	movl	sp@(4),a0
	clrl	d0
	movb	a0@,d0
	addl	d0,a0
	addql	#1,a0
	clrb	a0@
	movl	sp@(4),d0
	addql	#1,d0
	rts


| Force toolbox argument to be recognized as a Pascal string.
| Normally "mactrap.s" converts C string pointer arguments for toolbox
| routines into the Pascal strings that the toolbox ROM expects.
| This is done dynamically via a string conversion area on the stack.
| If Mactrap is passed a pointer with bit 24 set, it assumes the
| string is already in Pascal form and avoids the conversion.
| E.g.: "DrawString(isapstr(pascalstringptr))".

	.globl	isapstr,IsAPStr
isapstr:
IsAPStr:
	movl	sp@(4),d0
	bset	#24,d0
	rts
