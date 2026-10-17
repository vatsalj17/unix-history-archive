/ C library -- connection

/ types of connection
tdisconnect = 0
tconlog     = 1
tconview    = 2
tcongrab    = 3
tconshare   = 4

/  disconnect(fd)	where fd is either master-fd or slave-fd
/
/  disconnect == 0 means success
/  disconnect == -1 means error

.globl	_disconnect, cerror

_disconnect:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),r0
	mov	r0,r1		/ same fd (it will be checked)
	clr	0f		/ indicate disconnect
	sys	0; 9f
	bec	1f
	jmp	cerror
1:
	clr	r0
	mov	(sp)+,r5
	rts	pc

/  connect(master-fd, slave-fd, master-cntrl-char);
/
/  connect == 0 means success
/  connect == -1 means error

.globl	_connect

_connect:
	mov	$tconlog,0f
	jbr	2f

/  ttyview(master-fd, slave-fd, master-cntrl-char);
/
/  ttyview ==  0 means success
/  ttyview == -1 means error
/
.globl	_ttyview

_ttyview:
	mov	$tconview,0f
	jbr	2f

/  ttygrab(master-fd, slave-fd, master-cntrl-char);
/
/  ttygrab ==  0 means success
/  ttygrab == -1 means error
/
.globl	_ttygrab

_ttygrab:
	mov	$tcongrab,0f
	jbr	2f

/  ttyshare(master-fd, slave-fd, master-cntrl-char);
/
/  ttyshare ==  0 means success
/  ttygrab  == -1 means error
/
.globl	_ttyshare

_ttyshare:
	mov	$tconshare,0f
	jbr	2f

/  common code for log, view, grab, share
/
2:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),r0
	mov	6(r5),r1
	mov	8(r5),0f+2
	sys	0; 9f
	bec	1f
	jmp	cerror
1:
	clr	r0
	mov	(sp)+,r5
	rts	pc


.data
9:
	sys	59.; 0: 0; 0
