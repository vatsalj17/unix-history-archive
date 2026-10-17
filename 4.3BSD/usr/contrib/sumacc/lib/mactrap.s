|
| mactrap.s - C library trap linkage for Macintosh
|
| Copyright (C) 1984, Stanford Univ. SUMEX project.
| May be used but not sold without permission.
|
| history
| 05/01/84	Croft	Created.
| 05/25/84	Croft	Added PS length check; speeded up switch code
| 05/31/84	Croft	Added VS, VPS, now reentrant.
|
	.data
	.text
	.globl	_mactrap,_savea5,p2cstr

_FLEN = -256			| total frame length

_FVAR = -256			| VAR flag (pascal string or short)
_FRT = -254			| return type
_FOP = -252			| opcode (jmp or trap)
_FAD = -250			| longword address
_FJMP = -246			| JMP
_FJAD = -244			| jmp address
_FS = -240			| string conversion area
_FREG = -8			| register save area
_FSL = _FREG - _FS - 2		| string conv area length

_mactrap:
	link	a6,#_FLEN
	moveml	#A5+D2,a6@(_FREG)
	clrb	a6@(_FVAR)	| clear VAR flag
	movw	#/4ef9,a6@(_FJMP)	| setup JMP
	movl	#.L19,a6@(_FJAD)
	lea	a6@(_FS),a5	| a5 = string area
	lea	a6@(12),a0	| a0 = points to C args
	movl	a6@(4),a1
	movl	a1@+,d0		| d0 = C arg types
	movw	#/4e71,a6@(_FOP)	| nop
	movl	a1@,a6@(_FAD)	| trap or stub address
	blts	.L100		| if trap
	movw	#/4eb9,a6@(_FOP)	| jbsr
.L100:
	movl	d0,d1		| isolate C return type
	lsrl	#3,d0
	andw	#7,d1
	movw	d1,a6@(_FRT)	| save for later use
	subql	#4,sp		| reserve room for possible return value
|
| convert each argument type
|
.L1:
	movw	d0,d1
	lsrl	#3,d0		| each type packed in 3 bit nibble
	andw	#7,d1
	addw	d1,d1
	movw	pc@(6,d1:w),d1
	jmp	pc@(2,d1:w)
.L2:
	.word	.L10-.L2
	.word	.LAS-.L2
	.word	.LAL-.L2
	.word	.LAPS-.L2
	.word	.LASS-.L2
	.word	.LAB-.L2
	.word	.LAVS-.L2
	.word	.LAVPS-.L2
.LAS:
	movl	a0@+,d1		| short
	movw	d1,sp@-
	bras	.L1
.LAVPS:				| VAR pascal string
.LAVS:				| VAR short
	addqb	#1,a6@(_FVAR)
.LAL:
	movl	a0@+,sp@-	| long
	bras	.L1
.LASS:
	movl	a0@+,a1		| short struct (includes os/restype strings)
	movb	a1@+,d1		| may not be word aligned
	lsll	#8,d1
	movb	a1@+,d1
	lsll	#8,d1
	movb	a1@+,d1
	lsll	#8,d1
	movb	a1@+,d1
	movl	d1,sp@-
	bras	.L1
.LAB:
	movl	a0@+,d1		| boolean
	movb	d1,sp@-
	bras	.L1
.LAPS:
	movl	a0@+,d1
	bclr	#24,d1		| test and clear
	beqs	.L3		| if C string
	movl	d1,sp@-
	bras	.L1
.L3:
	movl	d1,a1
	clrl	d2		| count
	movl	a5,d1		| start address
	addql	#1,a5		| save byte for length
.L4:
	movb	a1@+,a5@+
	beqs	.L5
	addql	#1,d2
	cmpw	#_FSL,d2	| don't overflow conversion area
	blts	.L4
	subql	#8,a5
.L5:
	movl	d1,a1		| get string start address back
	movb	d2,a1@		| set count
	movl	d1,sp@-
	bras	.L1
|
| done with argument conversion, call toolbox
|
.L10:
	movl	_savea5,a5
	jmp	a6@(_FOP)
.L19:
|
| if an argument was a VAR short or VAR string, it needs deconversion;
| normally there are no such args, so this code is skipped.
|
	tstb	a6@(_FVAR)
	beq	.L29		| no such args (normal case)
	lea	a6@(12),a5	| a5 = points to C args
	movl	a6@(4),a1
	movl	a1@+,d2		| d2 = C arg types
	lsrl	#3,d2
	subql	#4,a5
.L11:
	addql	#4,a5		| point to next arg
	movw	d2,d1
	lsrl	#3,d2		| each type packed in 3 bit nibble
	andw	#7,d1
	beqs	.L29		| done
	cmpb	#6,d1
	blts	.L11
	beqs	.L13		| VAR short
				| else VAR pascal string
	movl	a5@,sp@-
	jsr	p2cstr
	addql	#4,sp
	bras	.L11
.L13:
	movl	a5@,a1		| convert VAR short
	movw	a1@,d0
	extl	d0
	movl	d0,a1@
	bras	.L11
|
| move function result type into d0
|
.L29:
	movw	a6@(_FRT),d1
	addw	d1,d1
	movw	pc@(6,d1:w),d1
	jmp	pc@(2,d1:w)
.L20:
	.word	.L30-.L20
	.word	.L21-.L20
	.word	.L22-.L20
	.word	.L30-.L20
	.word	.L30-.L20
	.word	.L23-.L20
	.word	.L30-.L20
	.word	.L30-.L20
.L21:
	movw	sp@+,d0
	extl	d0
	bras	.L30
.L22:
	movl	sp@+,d0
	bras	.L30
.L23:
	clrl	d0
	movb	sp@+,d0
.L30:
	moveml	a6@(_FREG),#A5+D2
	unlk	a6
	addql	#4,sp
	rts
