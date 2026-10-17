	.insrt	"h.h"

| interface routines expecting a parameter block ptr as argument
| hence the prefix pb

	.macro	oscall,tnum
	movl	#[tnum<<16] + 0x4e75,d1	| trap, rts
	bra	_ostrap
	.endm

	.globl	_pbopen
_pbopen:
	oscall	__open

	.globl	_pbclose
_pbclose:
	oscall	__close

	.globl	_pbread
_pbread:
	oscall	__read

	.globl	_pbwrite
_pbwrite:
	oscall	__write

	.globl	_pbcontro
_pbcontro:
	oscall	__control

	.globl	_pbstatus
_pbstatus:
	oscall	__status

	.globl	_pbkillio
_pbkillio:
	oscall	__killio

	.globl	_pbgetvin
_pbgetvin:
	oscall	__getvolinfo

	.globl	_pbgetvol
_pbgetvol:
	oscall	__getvol

	.globl	_pbsetvol
_pbsetvol:
	oscall	__setvol

	.globl	_pbeject
_pbeject:
	oscall	__eject

	.globl	_pbofflin
_pbofflin:
	oscall	__offline

	.globl	_pbflushv
_pbflushv:
	oscall	__flushvol

	.globl	_pbcreate
_pbcreate:
	oscall	__create

	.globl	_pbdelete
_pbdelete:
	oscall	__delete

	.globl	_pbopenrf
_pbopenrf:
	oscall	__openrf

	.globl	_pbrename
_pbrename:
	oscall	__rename

	.globl	_pbgetfin
_pbgetfin:
	oscall	__getfileinfo

	.globl	_pbsetfin
_pbsetfin:
	oscall	__setfileinfo

	.globl	_pbsetflo
_pbsetflo:
	oscall	__setfillock

	.globl	_pbrstflo
_pbrstflo:
	oscall	__rstfillock

	.globl	_pbsetfve
_pbsetfve:
	oscall	__setfiltype

	.globl	_pballoca
_pballoca:
	oscall	__allocate

	.globl	_pbgeteof
_pbgeteof:
	oscall	__geteof

	.globl	_pbseteof
_pbseteof:
	oscall	__seteof

	.globl	_pbgetfpo
_pbgetfpo:
	oscall	__getfpos

	.globl	_pbsetfpo
_pbsetfpo:
	oscall	__setfpos

	.globl	_pbflushf
_pbflushf:
	oscall	__flushfil



_ostrap:
	movl	sp@+,a1		| save return addr
	movb	sp@+,d0		| async flag
	movl	sp@+,a0		| PB ptr
	beq	.L1		| if not async
	orl	#_async_<<16,d1
.L1:
	movl	d1,sp@-		| for reentrancy, push trap/rts on stk
	jsr	sp@		| call trap 
	addql	#4,sp
	movw	d0,sp@		| save result
	jmp	a1@



| mountvolume and unmountvolume calls can only be made synchronously
| function pbmountvol(paramblock: parmblkptr): oserr|
| function pbunmountvol(paramblock: parmblkptr): oserr|

	.globl	_pbmountv
_pbmountv:
	movl	sp@+,a1	|get return address
	movl	sp@+,a0	|get param pointer.
	.word	__mountvol	|tell os to do it synchronously
	movw	d0,sp@	|_save result
	jmp	a1@	|return to caller

	.globl	_pbunmoun
_pbunmoun:
	movl	sp@+,a1	|get return address
	movl	sp@+,a0	|get param pointer.
	.word	__unmountvol	|tell os to do it synchronously
	movw	d0,sp@	|_save result
	jmp	a1@	|return to caller

