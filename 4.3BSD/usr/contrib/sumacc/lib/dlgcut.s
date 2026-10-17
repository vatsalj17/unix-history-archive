|
|	dmgr utility routines
|
|	these routines provide cut and paste capabiulities to edittable
|	text fields in dialogs

	.insrt	"../h/toolmacs.h"
	.insrt	"../h/toolequ.h"

	.text

	.globl	_dlgcut
	.globl	_dlgcopy
	.globl	_dlgpaste
	.globl	_dlgdelet


_dlgpaste:
	bsr	stdcheck
	.word	__tepaste
adios:
	movl	sp@+,sp@	| pop return/argument
	rts
_dlgcut:
	bsr	stdcheck
	.word	__tecut
	bra	adios
_dlgcopy:
	bsr	stdcheck
	.word	__tecopy
	bra	adios

_dlgdelet:
	bsr	stdcheck
	.word	__tedelete
	bra	adios

stdcheck:
	movl	sp@(8),a0	| get dialog pointer
	tstw	a0@(editfield)	| see if edit is open
	bmis	nodice	| see if edit field defined
	movl	sp@+,a1	| pop return for stdcheck
	movl	a0@(tehandle),sp@-	| pass text edit handle
	jmp	a1@
nodice:
	moveml	sp@+,#D0+A0+A1	| get real return address
	jmp	a0@	| return
