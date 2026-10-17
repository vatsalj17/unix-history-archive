|
| getpargs - Get Pascal arguments into C subroutine/function.
|
| In using this Mac/C environment, usually calls are made from C programs
|  into the Pascal toolbox;  this argument conversion is handled auto-
|  maticly.  On rare occasions, a Pascal toolbox function (such as
|  TrackControl) wants to call a C routine.  Getpargs is used in such cases
|  to make the Pascal arguments accessable to C.  In addition, getpargs
|  adjusts the stack for a proper Pascal return.
|
| Called as the 1st line of a C subroutine that has no arguments:
|  funretptr = getpargs(&pascalargstruct, sizeof pascalargstruct);
|
| "pascalargstruct" is a structure declared local to the C subroutine and is
|  filled in with the Pascal arguments, in stack order (reversed) and with
|  the proper Pascal calling convention argument types.  
|
| "funretptr" gets the address of the Pascal return value.  This is only
|  useful if you are being called as a function;  if you store into this
|  location, be sure to use the proper expected Pascal type & length.
|
| I know this reeks of kludgery;  if you find a better way to do this
| (short of altering the C language calling conventions to be that of Pascal),
| please let me know...
|
|
| Copyright (C) 1984, Stanford Univ. SUMEX project.
| May be used but not sold without permission.
|
| history
| 05/29/84	Croft	Created.
| 05/30/84	Croft	Added funretptr stuff.
|
	.data
	.text
	.globl	getpargs

| first copy the pascalargstruct from stack into the C struct.

getpargs:
	movl	sp@(4),a1	| dst struct addr
	movl	sp@(8),d0	| count
	beqs	.L3		| nothing to do, just return funretptr
	movl	a6,a0
	addql	#8,a0		| src stack addr
	subql	#1,d0
.L1:	movb	a0@+,a1@+
	dbra	d0,.L1

| now remove the Pascal arguments from the stack by shifting it up.

	movl	sp@(8),d1	| stack shift distance
	movl	a6,a0
	addql	#7,a0		| src
	movl	a0,a1
	addl	d1,a1		| dst
	movl	a0,d0
	subl	sp,d0		| sizeof stack - 1
.L2:	movb	a0@,a1@
	subql	#1,a0
	subql	#1,a1
	dbra	d0,.L2

| update sp and fp

	addl	d1,a6
	addl	d1,sp
.L3:	movl	a6,d0		| return funretptr
	addql	#8,d0
	rts





