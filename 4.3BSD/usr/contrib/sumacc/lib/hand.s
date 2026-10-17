|tooltraps.text
		.text
|
|function handtohand(var thehndl: handle): oserr|
|
| clones handle in a0 returning the new handle in a0

		.globl	_handtoha
_handtoha:
                movl    sp@(4),a0                |ptr to handle
                movl    a0@,a0                 |the handle
                .word	/a9e1
                movl    sp@(4),a1                |ptr to handle to be returned
                movl    a0,a1@                 |return the new handle
                movw    d0,sp@(8)                |return error
                movl    sp@+,sp@              |pop off the return addr
                rts


|
|function ptrtoxhand(srcptr: ptr| dsthndl: handle| size: longint): oserr|
|
| copies size bytes from the srcptr into the existing handle

		.globl	_ptrtoxha
_ptrtoxha:
                movl    sp@+,d1                |save return address
                movl    sp@+,d0                |the size
                movl    sp@+,a1                |existing handle
                movl    sp@+,a1                |srcptr
                movl    d1,sp@-                |restore return address
                .word	/a9e2
                movw    d0,sp@(4)                |error code
                rts                             |return
|
|function ptrtohand(srcptr: ptr| var dsthndl: handle| size: longint): oserr|
|
| makes a handle and copies size bytes of the handle into it

		.globl	_ptrtohan
_ptrtohan:
                movl    sp@(12),a0               |src pointer
                movl    sp@(4),d0                |size
                .word	/a9e3                    |make new hand and copy ptr in
                movw    d0,sp@(16)               |error code
                movl    sp@(8),a1                |address of handle
                movl    a0,a1@                 |return new handle
                movl    sp@+,a0                |return addr
                addw     #12,sp                  |pop off args
                jmp     a0@                    |return
|
|function handandhand(hand1,hand2: handle): oserr|
|
| concatenates hand1 onto the end of hand2

		.globl	_handandh
_handandh:
                movl    sp@(8),a0                |source handle
                movl    sp@(4),a1                |other source handle
                .word	/a9e4                    |concats hndl in a0 to h in a1
                movw    d0,sp@(12)
                movl    sp@+,a0                |return addr
                addql    #8,sp                   |pop off args
                jmp     a0@                    |return
|
|function ptrandhand(ptr1: ptr| hand2: handle| size: longint): oserr|
|
| concatenates ptr1 onto the end of hand2

		.globl	_ptrandha
_ptrandha:
                movl    sp@(4),d0                |length of ptr
                movl    sp@(8),a1                |handle
                movl    sp@(12),a0               |ptr
                .word	/a9ef                       |concat ptr to handle
                movw    d0,sp@(16)               |return error
                movl    sp@+,a0                |return addr
                addw     #12,sp
                jmp     a0@
