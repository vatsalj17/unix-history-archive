/ machine language assist
/ for PDP-11/10

/ non-UNIX instructions
wait	= 1
rti	= 2
reset	= 5

.globl	trap, call, emtrap
.globl	_trap
trap:
	mov	PS,-4(sp)
	tst	nofault
	bne	1f
emtrap:
	jsr	r0,call1; _trap
	/ no return
1:
	mov	nofault,(sp)
	rti

.globl	_runrun, _swtch
call1:
	tst	-(sp)
	bic	$340,PS
	br	1f

call:
	mov	PS,-(sp)
1:
	mov	r1,-(sp)
	mov	sp,r1	/get stack pointer at trap
	mov	r1,-(sp)
	add	$10.,(sp)
	mov	4(sp),-(sp)
	bic	$!37,(sp)	/ trap type
	cmp	10.(sp),$_u+[usize*64.]
	blo	1f	/ trap from system
	mov	2(sp),r1	/ trap from user, get user stack
	mov	$_u+[usize*64.],sp
	mov	-(r1),-(sp)	/ copy user stack to system stack
	mov	-(r1),-(sp)
	mov	-(r1),-(sp)
	mov	-(r1),-(sp)
	mov	-(r1),-(sp)
	mov	-(r1),-(sp)
	mov	-(r1),-(sp)
	jsr	pc,*(r0)+
2:
	bis	$340,PS
	tstb	_runrun
	beq	2f
	bic	$340,PS
	jsr	pc,_swtch
	br	2b
1:
	jsr	pc,*(r0)+
2:
	tst	(sp)+
	mov	(sp)+,r1	/ new user stack pointer
	cmp	6(sp),$_u+[usize*64.]
	blo	1f	/ return from system trap
	sub	$10.,r1	/ begin. of system stack to be copied to user stack
	mov	(sp)+,(r1)+	/ copy system stack to user stack
	mov	(sp)+,(r1)+
	mov	(sp)+,(r1)+
	mov	(sp)+,(r1)+
	mov	(sp)+,(r1)+
	sub	$10.,r1
	mov	r1,sp	/ switch to user stack
1:
	mov	(sp)+,r1
	tst	(sp)+
	mov	(sp)+,r0
	rti

/ Character list get/put

.globl	_getc, _putc
.globl	_cfreelist

_getc:
	mov	2(sp),r1
	mov	PS,-(sp)
	mov	r2,-(sp)
	bis	$340,PS
	bic	$100,PS		/ spl 5
	mov	2(r1),r2	/ first ptr
	beq	9f		/ empty
	movb	(r2)+,r0	/ character
	bic	$!377,r0
	mov	r2,2(r1)
	dec	(r1)+		/ count
	bne	1f
	clr	(r1)+
	clr	(r1)+		/ last block
	br	2f
1:
	bit	$7,r2
	bne	3f
	mov	-10(r2),(r1)	/ next block
	add	$2,(r1)
2:
	dec	r2
	bic	$7,r2
	mov	_cfreelist,(r2)
	mov	r2,_cfreelist
3:
	mov	(sp)+,r2
	mov	(sp)+,PS
	rts	pc
9:
	clr	4(r1)
	mov	$-1,r0
	mov	(sp)+,r2
	mov	(sp)+,PS
	rts	pc

_putc:
	mov	2(sp),r0
	mov	4(sp),r1
	mov	PS,-(sp)
	mov	r2,-(sp)
	mov	r3,-(sp)
	bis	$340,PS
	bic	$100,PS		/ spl 5
	mov	4(r1),r2	/ last ptr
	bne	1f
	mov	_cfreelist,r2
	beq	9f
	mov	(r2),_cfreelist
	clr	(r2)+
	mov	r2,2(r1)	/ first ptr
	br	2f
1:
	bit	$7,r2
	bne	2f
	mov	_cfreelist,r3
	beq	9f
	mov	(r3),_cfreelist
	mov	r3,-10(r2)
	mov	r3,r2
	clr	(r2)+
2:
	movb	r0,(r2)+
	mov	r2,4(r1)
	inc	(r1)		/ count
	clr	r0
	mov	(sp)+,r3
	mov	(sp)+,r2
	mov	(sp)+,PS
	rts	pc
9:
	mov	pc,r0
	mov	(sp)+,r3
	mov	(sp)+,r2
	mov	(sp)+,PS
	rts	pc

.globl	_fubyte, _subyte
.globl	_fuibyte, _suibyte
.globl	_fuword, _suword
.globl	_fuiword, _suiword
_fuibyte:
_fubyte:
	mov	2(sp),r1
	bic	$1,r1
	jsr	pc,gword
	cmp	r1,2(sp)
	beq	1f
	swab	r0
1:
	bic	$!377,r0
	rts	pc

_suibyte:
_subyte:
	mov	2(sp),r1
	bic	$1,r1
	jsr	pc,gword
	mov	r0,-(sp)
	cmp	r1,4(sp)
	beq	1f
	movb	6(sp),1(sp)
	br	2f
1:
	movb	6(sp),(sp)
2:
	mov	(sp)+,r0
	jsr	pc,pword
	clr	r0
	rts	pc

_fuiword:
_fuword:
	mov	2(sp),r1
fuword:
	jsr	pc,gword
	rts	pc

gword:
	mov	PS,-(sp)
	bis	$340,PS
	mov	nofault,-(sp)
	mov	$err,nofault
	mov	(r1),r0
	br	1f

_suiword:
_suword:
	mov	2(sp),r1
	mov	4(sp),r0
suword:
	jsr	pc,pword
	rts	pc

pword:
	mov	PS,-(sp)
	bis	$340,PS
	mov	nofault,-(sp)
	mov	$err,nofault
	mov	r0,(r1)
1:
	mov	(sp)+,nofault
	mov	(sp)+,PS
	rts	pc

err:
	mov	(sp)+,nofault
	mov	(sp)+,PS
	tst	(sp)+
	mov	$-1,r0
	rts	pc

.globl	_copyin, _copyout
_copyin:
	jsr	pc,copsu
1:
	mov	(r0)+,(r1)+
	dec	r2
	bne	1b
	br	2f

_copyout:
	jsr	pc,copsu
1:
	mov	(r0)+,(r1)+
	dec	r2
	bne	1b
2:
	mov	(sp)+,nofault
	mov	(sp)+,r2
	clr	r0
	rts	pc

copsu:
	mov	(sp)+,r0
	mov	r2,-(sp)
	mov	nofault,-(sp)
	mov	r0,-(sp)
	mov	10(sp),r0
	mov	12(sp),r1
	mov	14(sp),r2
	asr	r2
	mov	$1f,nofault
	rts	pc

1:
	mov	(sp)+,nofault
	mov	(sp)+,r2
	mov	$-1,r0
	rts	pc

.globl	_idle
_idle:
	mov	PS,-(sp)
	bic	$340,PS
	wait
	mov	(sp)+,PS
	rts	pc

.globl	_savu, _retu
_savu:
	bis	$340,PS
	mov	(sp)+,r1
	mov	(sp),r0
	mov	sp,(r0)+
	mov	r5,(r0)+
	bic	$340,PS
	jmp	(r1)

_retu:
	bis	$340,PS
	mov	(sp)+,r1
	mov	(sp),r0
	mov	(r0)+,sp
	mov	(r0)+,r5
	bic	$340,PS
	jmp	(r1)

.globl	_spl0, _spl1, _spl4, _spl5, _spl6, _spl7
_spl0:
	bic	$340,PS
	rts	pc

_spl1:
	bis	$40,PS
	bic	$300,PS
	rts	pc

_spl4:
_spl5:
	bis	$340,PS
	bic	$100,PS
	rts	pc

_spl6:
	bis	$340,PS
	bic	$40,PS
	rts	pc

_spl7:
	bis	$340,PS
	rts	pc

.globl	_dpadd
_dpadd:
	mov	2(sp),r0
	add	4(sp),2(r0)
	adc	(r0)
	rts	pc

.globl	_dpcmp
_dpcmp:
	mov	2(sp),r0
	mov	4(sp),r1
	sub	6(sp),r0
	sub	8(sp),r1
	sbc	r0
	bge	1f
	cmp	r0,$-1
	bne	2f
	cmp	r1,$-512.
	bhi	3f
2:
	mov	$-512.,r0
	rts	pc
1:
	bne	2f
	cmp	r1,$512.
	blo	3f
2:
	mov	$512.,r1
3:
	mov	r1,r0
	rts	pc

.globl	start, _end, _edata, _main
start:

/ clear bss and user block

	mov	$_edata,r0
1:
	clr	(r0)+
	cmp	r0,$_u+[usize*64.]
	blo	1b

/ set up stack pointer

	mov	r0,sp

/ and pointer to system stack

	mov	$_u-2,_u


/ set up previous mode and call main
/ on return, enter user mode at 060000

	jsr	pc,_main
	mov	$_u+[usize*64.]+8192.,sp	/ set stack at first 4K of user space
	clr	-(sp)
	mov	$_u+[usize*64.],-(sp)
	rti

.globl	_ldiv
_ldiv:
	clr	r0
	mov	2(sp),r1
	div	4(sp),r0
	rts	pc

.globl	_lrem
_lrem:
	clr	r0
	mov	2(sp),r1
	div	4(sp),r0
	mov	r1,r0
	rts	pc

.globl	_lshift
_lshift:
	mov	2(sp),r1
	mov	(r1)+,r0
	mov	(r1),r1
	ashc	4(sp),r0
	mov	r1,r0
	rts	pc

.globl	csv
csv:
	mov	r5,r0
	mov	sp,r5
	mov	r4,-(sp)
	mov	r3,-(sp)
	mov	r2,-(sp)
	jsr	pc,(r0)

.globl cret
cret:
	mov	r5,r1
	mov	-(r1),r4
	mov	-(r1),r3
	mov	-(r1),r2
	mov	r5,sp
	mov	(sp)+,r5
	rts	pc

.globl	_u
_u	= 56000
usize	= 16.

PS	= 177776

.bss
.globl	nofault
nofault:.=.+2
