|-------------------------------------------------------------------------------
|       this is the glue for calling the print code.
|       it is a simple overlaying caller of resources.
|-------------------------------------------------------------------------------
                .insrt  "../h/sysequ.h"
                .insrt  "../h/syserr.h"
                .insrt  "../h/toolequ.h"
                .insrt  "../h/toolmacs.h"
                .insrt  "../h/prequ.h"

                .globl    _propendoc
                .globl    _prclosedoc
                .globl    _propenpage
                .globl    _prclosepage

                .globl    _printdefault
                .globl    _prstldialog
                .globl    _prjobdialog
                .globl    _prstlinit
                .globl    _prjobinit
                .globl    _prdlgmain
                .globl    _prvalidate
                .globl    _prjobmerge

                .globl    _prpicfile
|                .globl    prpic

                .globl    _propen
                .globl    _prclose

                .globl    _prdrvropen
                .globl    _prdrvrclose
                .globl    _prctlcall

                .globl    _prcfgdialog
                .globl    _prhack

                .globl    _prerror
                .globl    _prseterror

		.text
		.globl	_uprlink
_uprlink:

|-------------------------------------------------------------------------------
|               the doc spooling & draft procs dispatch
|-------------------------------------------------------------------------------
_propendoc:
        movl    sp@(12),a0                       |get hprint
        movl    a0@,a0                         |get pprint

        moveq   #3, d0                          |mask for low two bits.
        andb   a0@(prjob+bjdocloop), d0         |get two bit printloop type from tprint

        moveq   #-4, d1                         |get mask (/fc) for:
        andb   d1, printvars+bdocloop          |clear lo 2 bits in bdocloop
        orb    d0, printvars+bdocloop          |set lo 2 bits in bdocloop

        movl    #lopendoc,d1                    |this proc's jump table offset
        bras   prstddoc                        |go to standard entry
_prclosedoc:
        movl    #lclosedoc,d1                   |this proc's jump table offset
        bras   prstddoc                        |go to standard entry

_propenpage:
        movl    #lopenpage,d1                   |this proc's jump table offset
        bras   prstddoc                        |go to standard entry

_prclosepage:
        movl    #lclosepage,d1                  |this proc's jump table offset

prstddoc:
        moveq   #3, d0                          |mask for low two bits.
        andb   printvars+bdocloop, d0          |get printloop type from globals
|its zero!        addqw  #iprdraftid, d0                 ;add as an offset to draft

        bras   prstdlink                       |go to standard linker proc

|-------------------------------------------------------------------------------
|               the hack & cfg procs dispatch
|-------------------------------------------------------------------------------
_prcfgdialog:
	movl    #lcfgdialog,d1  |this proc's jump table offset
	moveq   #icfgdlgid,d0   |dialog procs' resid
        bras   prstdlink       |go to standard linker proc

_prhack:
	movl    #lprhack,d1     |this proc's jump table offset
	moveq   #iprhackid,d0   |dialog procs' resid
        bras   prstdlink       |go to standard linker proc

|-------------------------------------------------------------------------------
|               the dialog procs dispatch
|-------------------------------------------------------------------------------
_printdefault:
        movl    #ldefault,d1    |this proc's jump table offset
        bras   prstddlg        |go to standard entry

_prstldialog:
        movl    #lstldialog,d1  |this proc's jump table offset
        bras   prstddlg        |go to standard entry

_prjobdialog:
        movl    #ljobdialog,d1  |this proc's jump table offset
        bras   prstddlg        |go to standard entry

_prstlinit:
	movl    #lstlinit,d1    |this proc's jump table offset
        bras   prstddlg        |go to standard entry

_prjobinit:
	movl    #ljobinit,d1    |this proc's jump table offset
        bras   prstddlg        |go to standard entry

_prdlgmain:
	movl    #ldlgmain,d1    |this proc's jump table offset
        bras   prstddlg        |go to standard entry

_prvalidate:
	movl    #lprvalidate,d1 |this proc's jump table offset
        bras   prstddlg        |go to standard entry

_prjobmerge:
	movl    #lprjobmerge,d1 |this proc's jump table offset

prstddlg:
        moveq   #iprdlgsid,d0   |dialog procs' resid
        bras   prstdlink       |go to standard linker proc

|-------------------------------------------------------------------------------
|               the pic printing procs dispatch
|-------------------------------------------------------------------------------
_prpicfile:
        movl    #lprpicfile,d1  |this proc's jump table offset
|        bras   prstdpic        ;go to standard entry
|
|prpic
|        movl    #lprpic,d1      ;this proc's jump table offset
|
|prstdpic
        moveq   #iprpicid,d0    |pic printing procs' resid
|-------------------------------------------------------------------------------
|               this is the shared linking code.
|
| it swaps in the proper resource segment and jumps to the proper
| jump table entry in that seg's header.  it is called with the stack
| frame setup properly for the target procedure in the seg.  we do two
| hacks:
|       (1) we set the swap the frame's return address with our own; saving
| the caller's in a register.  this is needed to allow us to un-lock the
| resource seg, otherwise an autopop style would work.
|       (2) we save the caller's a3/4 in the code, not the stack.  this is
| to preserve the callers stack frame for the receiving resource proc.
|
| called with:  d0 = resource id
|               d1 = funlock/jump table offset
| uses:         a3 = callers return address
|               a4 = resource handle
|               d4 = preserved funlock/jump table offset
|-------------------------------------------------------------------------------
prstdlink:

| save some regs & put link long word and callers rtn addr

        lea     la3, a0         |get reg save area address
        moveml 	#D4+A3+A4,a0@   |preserve caller's regs
        movl    d1, d4          |save funlock/jump table offset
        movl    sp@+,a3        |[pop &] save the return address.

| get the code from the printer's resource file.
| if error, simply leave! [could alert!]

        subql  #4,sp                   |make room for rsrc handle
        movl    #lpdeftype,sp@-        |get pdef rsrc type
        movw    d0,sp@-                |..and id
	.word	__getresource
        movl    sp@+,d0                |get the resource handle
        beqs   prstderr                |abort if nil [reserr not set!].
        movl    d0, a4                  |save the resource handle

|lock & jump to the proc thru a small jump table at the start of the seg.
|replace the caller's rtn addr with our own so that we can unlock the seg.

        bset    #lock,a4@      |lock the code seg.
        movl    a4@,a0         |de-reference.
        moveq   #0,d0           |get all zeros in d0 so following is valid long:
        movb    d4,d0           |get proc offset [byte!] from funlock/jump table offset
        addl   d0,a0           |add it to the seg base
|.if: fprdbgok
        movl    a0,sp@-        |clear the hi byte so pc ok for debugger!
        clrb   sp@
        movl    sp@+,a0
|.endc:
        jsr     a0@            |call the proc (note: pushes new rtn adr!)
                                |  now the params are striped & we can cleanup & rtn.

| unlock the seg, if ok.  then go home.
| we dont unlock between a docstart-docend pair because
| quickdraw is using that code in its bottle neck procs.

        tstw   d4              |check if we should unlock
        bpls   prstddone       |no: skip
        bclr    #lock,a4@      |yes: unlock the code seg. (assummed pergable)
prstddone:
        movl    a3,a1           |fetch saved rtn adr
|        moveml la3,#D4+A3+A4    ;restore caller's regs
        lea     la3, a0         |get reg save area address [randy's find]
        moveml a0@,#D4+A3+A4   |restore caller's regs

        jmp     a1@            |go home

| this is the abort.  it gets the frame size from the link long
| and uses that to fixup the stack.

prstderr:
        movw    #resnotfound,printvars+iprerr   |post the error
        swap    d4              |get param size
        addw   d4, sp          |strip params. d4 hi word ignored.
        bras   prstddone

la3:    .long    0
la4:    .long    0
ld4:    .long    0

|-------------------------------------------------------------------------------
|       these are the non-link procs:
|-------------------------------------------------------------------------------
|        procedure prdrvropen
| opens the printer driver
|-------------------------------------------------------------------------------
_prdrvropen:
	moveq   #[ioqelsize/2]-1,d0     |generate a cleared ioqel
.L2:      clrw   sp@-
        dbra    d0,.L2

        lea     sprdrvr, a0             |get drvr name
        movl    a0, sp@(iofilename)      |stash it

        movl    sp,a0                   |point to block
        .word	__open                           |do the open

ioelrtn:
        addw     #ioqelsize,sp           |clean up stack
        movw    d0, printvars+iprerr    |save the error
        rts

sprdrvr:         .byte   6
                .ascii  '.print '

|-------------------------------------------------------------------------------
|       procedure prdrvrclose
| this could be omitted or put in the prbits link.
|-------------------------------------------------------------------------------
_prdrvrclose:
        subw     #ioqelsize,sp           |make room for write control block
        movl    sp,a0                   |point to block

        movw    #iprdrvrref,a0@(iorefnum)|set the refnum param
        .word	__close                          |close drvr
        bras   ioelrtn

|-------------------------------------------------------------------------------
|       procedure prctlcall (iwhichctl: integer; lparam1, lparam2, lparam3: longint);
| general 3 param call to the printer driver.
|-------------------------------------------------------------------------------
_prctlcall:
        movl    sp@+, a0       | rtn
        movl    sp@+, a1       | lparam3
        movl    sp@+, d0       | lparam2
        movl    sp@+, d1       | lparam1
        movw    sp@+, d2       | iwhichctl
        movl    a0, sp@-       | rts fix

        subw     #ioqelsize,sp           | make room for write control block

        movw    d2, sp@(cscode)          | iwhichctl
        movl    d1, sp@(csparam)         | lparam1
        movl    d0, sp@(csparam+4)       | lparam2
        movl    a1, sp@(csparam+8)       | lparam3
        movw    #iprdrvrref,sp@(iorefnum)| set the refnum param

        movl    sp,a0                   | point to block
        .word	__control

        bras   ioelrtn

|-------------------------------------------------------------------------------
|       procedure propen
| opens both the printer resource file and printer driver
| note: opens the drvr first so that it uses the one in sys.rsrc
| if probs with driver open, does not open the print rsrc file.
|
|       procedure prclose
| closes just the printer rsrc file, leaving the driver open.
| this is so that an ornament can use the driver w/o the app
| taking the driver away!
| note: close opens the resource simply to get the refnum.
|-------------------------------------------------------------------------------
_propen:
        bsrs   _prdrvropen              |open drvr in sys.rsrc
        bnes   openrts                 |abort if error.
        moveq   #0, d1                  |mark call open
        bras   propen1
_prclose:
        moveq   #1, d1                  |mark call close
propen1:
|this uses d1 & a1 accross rsrc calls.  this is safe; they're preserved.
|first get the name of the current printer's resource file from sys.rsrc

        subql    #4,sp                   |make room for rsrc handle
        movl    #lpstrtype,sp@-        |get pstr rsrc type
        movw    #ipstrrfil,sp@-        |..and id for rsrcfile name
	.word	__getresource
        movl    sp@+,d0                |get the rsrcfile name handle
        beqs   gotnil                  |quit if nil
        movl    d0, a1                  |save the resource handle

|open the printer resource file.
|for close, this simply is used to get the refnum

        subql    #2,sp                   |make room for rsrc refnum
        bset    #lock,a1@              |lock res file name
        movl    a1@,sp@-              |push & de-reference.
	.word	__openresfile                    |open the res file
        movw    sp@+, d0               |pop the refnum
        bclr    #lock,a1@              |unlock the res file name
        bsrs   ckerr                   |save & check error

|now return for opens, or close the resfile with the id
|found above if closing.

        tstw   d1                      |open call?
        beqs   openrts                 |yes: go home
        movw    d0,sp@-                |no: closeresfile. push the refnum param
	.word	__closeresfile
        bsrs   ckerr

openrts:
        rts

ckerr:
        movw    reserr,printvars+iprerr |save the error
        beqs   .L1                      |ok: go on
        addqw    #4, sp                  |no: pop addr to abort!
.L1:      rts

gotnil:
        movw    #resnotfound,printvars+iprerr   |post the error
        rts

|-------------------------------------------------------------------------------
|       function  prerror: integer;
|-------------------------------------------------------------------------------
_prerror:
        movw    printvars+iprerr, sp@(4) | rtn the error
        rts

|-------------------------------------------------------------------------------
|       procedure prseterror( ierr: integer );
|-------------------------------------------------------------------------------
_prseterror:
        movl    sp@+, a0               | pop rtn addr
        movw    sp@+, printvars+iprerr | set the error code
        jmp     a0@

|-------------------------------------------------------------------------------
        .end
