|-----------------------------------------------------------
| 28aug82: written by jerome coonen
| 13oct82: remove uses of d0 across fp calls (jtc).
| 12dec82: fix bugs in class fcns (jtc).
| 28dec82: split nan class to snan, qnan; scalb, logb (jtc).
| 29dec82: i2x, l2x pass by value here. (jtc)
| 27apr83: neg as core function added. (jtc)
| 25may84: C version, uses macros to save my fingers. (croft)
|-----------------------------------------------------------
	.nlist
	.insrt	"../h/sanemacs.h"
	.list

__fp68k = /a9eb		| avoids including toolmacs.h
__elems68k = /a9ec

	.text

	.macro cfp,c
	movw	#c,sp@-
	.word	__fp68k
	.endm
	.macro cel,c
	movw	#c,sp@-
	.word	__elems68k
	.endm
	.macro c1,l,c
	.globl l
l:	movw	#c,d0
	bra	call1
	.endm
	.macro c2,l,c
	.globl l
l:	movw	#c,d0
	bra	call2
	.endm
	.macro c3,l,c
	.globl l
l:	movw	#c,d0
	bra	call3
	.endm

	c2,AddS,fadds
	c2,AddD,faddd
	c2,AddC,faddc
	c2,AddX,faddx
	c2,SubS,fsubs
	c2,SubD,fsubd
	c2,SubC,fsubc
	c2,SubX,fsubx
	c2,MulS,fmuls
	c2,MulD,fmuld
	c2,MulC,fmulc
	c2,MulX,fmulx
	c2,DivS,fdivs
	c2,DivD,fdivd
	c2,DivC,fdivc
	c2,DivX,fdivx

	c2,I2X,fi2x
	c2,L2X,fl2x
	c2,S2X,fs2x
	c2,D2X,fd2x
	c2,C2X,fc2x
	c2,X2X,fx2x
	c2,X2I,fx2i
	c2,X2L,fx2l
	c2,X2S,fx2s
	c2,X2D,fx2d
	c2,X2C,fx2c
	c2,Dec2S,fdec2s
	c2,Dec2D,fdec2d
	c2,Dec2C,fdec2c
	c2,Dec2X,fdec2x
	c3,S2Dec,fs2dec
	c3,D2Dec,fd2dec
	c3,C2Dec,fc2dec
	c3,X2Dec,fx2dec

	c1,SqrtX,fsqrtx
	c1,RintX,frintx
	c1,NegX,fnegx
	c1,AbsX,fabsx
	c2,CpySgnX,fcpysgnx
	c2,ScalbX,fscalbx
	c1,LogbX,flogbx
	c2,NextS,fnexts
	c2,NextD,fnextd
	c2,NextX,fnextx

	c1,SetEnv,fsetenv
	c1,GetEnv,fgetenv
	c1,ProcEntry,fprocentry
	c1,ProcExit,fprocexit

call1:	movl	sp@+,a0
	movw	d0,sp@-
	.word	__fp68k
	subql	#4,sp
	jmp	a0@

call2:	movl	sp@+,a0
	movl	sp@,d1
	movl	sp@(4),sp@
	movl	d1,sp@(4)
	movw	d0,sp@-
	.word	__fp68k
	subql	#8,sp
	jmp	a0@

call3:	movl	sp@+,a0
	movl	sp@,d1
	movl	sp@(8),sp@
	movl	d1,sp@(8)
	movw	d0,sp@-
	.word	__fp68k
	subw	#12,sp
	jmp	a0@



        .globl    CmpX
CmpX:
        movl   sp@+,a0        | ret address
        movl    sp@(4),d1        | relop to be tested
        addqb  #1,d1           | handy to increment

        movl    sp@,d0         | swap addresses (C already swapped)
        addql  #2,sp           | kill slot for relop
	movl	d0,sp@

        cmpb  #4,d1           | equal
        beqs   .L1
        cmpb  #8,d1           | unordered
        beqs   .L1

        cfp fcpxx
        bras   .L10
.L1:
        cfp fcmpx
.L10:
        fbne    .L12
        btst    #2,d1           | #2 is equal bit
        bras   .L50
.L12:
        fbo     .L14
        btst    #3,d1           | #3 is unordered bit
        bras   .L50
.L14:
        fbgt    .L16
        btst    #1,d1           | #1 is less bit
        bras   .L50
.L16:
        btst    #0,d1           | #0 is greater bit
.L50:
	clrl	d0
        sne     d0
        negb   d0
	subw	#12,sp		| C wants to pop his own args
        jmp     a0@



        .globl    RelX
RelX:				| C args already swapped
        movl   sp@+,a0

        moveq   #7,d1           | assume unordered
        cfp fcmpx
        fbu     .L9              | unordered
        fbne    .L7
        moveq   #3,d1           | equal
        bras   .L9
.L7:
        fbgt    .L3
        moveq   #1,d1           | less
        bras   .L9
.L3:
        moveq   #0,d1           | greater
.L9:
	movl	d1,d0
	subw	#8,sp
        jmp     a0@


        .globl    RemX
RemX:
        movl   sp@+,a0        | return address
	movl	sp@(8),a1	| addr of quotient
	movl	sp@+,d0		| swap args
	movl	d0,sp@(4)
        cfp fremx                   | returns with quo in d0
        movl    d0,a1@         | deliver integer result
	subw	#12,sp
        jmp     a0@


|-----------------------------------------------------------
| the class functions use a common startup routine to fetch
| the leading 32 bits and store the sign in its var param.
| a set of finishup sequences set the return value to the
| appropriate integer value.
|-----------------------------------------------------------
| pascal: function class?(var x: ???????; var sgn: integer):
|                                            numberclass
| stack = ret < sgn < x < result
|-----------------------------------------------------------

	.macro	class,l,c
	.globl	l
l:	movw	#c,d0
	bras	classcom
	.endm

	class,ClassS,fclasss
	class,ClassD,fclassd
	class,ClassC,fclassc
	class,ClassX,fclassx

classcom:
	movl	sp@+,a0		| ret addrs
	movl	sp@(4),a1	| swap args and save sgn var
	movl	sp@,sp@(4)
	movl	a1,sp@
	movw	d0,sp@-
	.word	__fp68k
        movw    a1@,d0         | ret code
	extl	d0
        clrl   a1@            | 0 for +, 1 for -
        tstw   d0
        bpls   .L71

        addql  #1,a1@
        negl   d0
.L71:
        subql  #1,d0
	subw	#8,sp		| C wants to pop his own stack
        jmp     a0@



|-----------------------------------------------------------
|-----------------------------------------------------------
| the environment parameter is an integer passed by address
| for convenience in later routines, set/getenv leave a1
| unchanged but touch d0.
|-----------------------------------------------------------
|-----------------------------------------------------------

	.globl	SetRnd
SetRnd:
        movl   sp@+,a1        | ret adrs

        movl    sp@,d1         | new rnd mode
	movl	#13,d0
        roll   d0,d1           | align round bits
        andl  #/6000,d1       | mask just those

        pea     sp@            | overwrite input operand
	cfp	fgetenv
        andw  #/9fff,sp@     | kill old mode
        orw    d1,sp@         | new mode
        pea     sp@
	cfp	fsetenv
        jmp     a1@


	.globl	GetRnd
GetRnd:
        movl   sp@+,a1
	clrw	sp@-
        pea     sp@
	cfp	fgetenv

        movl    #/6000,d0       | isolate round bits
        andw   sp@+,d0
	movl	#13,d1
        rorw   d1,d0           | in hi byte
        jmp     a1@


|-----------------------------------------------------------
| pascal: procedure testxcp(x: exception): boolean;
| so stack = ret < x < result.   hi byte (0) is set of flags
| and low byte (1) is halts.
|-----------------------------------------------------------

	.globl	TestXcp
TestXcp:
        clrl   d1
        bras   gentest
	.globl	TestHlt
TestHlt:
        moveq   #1,d1

gentest:
        movl   sp@+,a1        | ret adrs, safe from getenv
	movl	sp@,d0
	movw	d0,sp@-
        pea     sp@(2)
	cfp	fgetenv
        movw    sp@+,d0        | bit index in hi byte
        extw   d0              | make it word index
        btst    d0,sp@(0,d1)
        sne     d0              | d0 = 0000 or 00ff
        negb   d0              | d0 = 0000 or ffff

        jmp     a1@

|-----------------------------------------------------------
| pascal: procedure setxcp(x: exception; onoff: boolean);
| stack = ret < onoff < x
| this procedure was originally set up to just set or clear
| bits in the control word.  nowadays, setting an exception
| flag entails a possible halt, so a special dummy floating
| point routine is used.
|-----------------------------------------------------------
	.globl	SetXcp,SetHlt
SetXcp:	link	a6,#0
	movl	a6@(8),d0
	movb	d0,sp@-
	movl	a6@(12),d0
	movb	d0,sp@-
	bsrs	sxcp
	unlk	a6
	rts

SetHlt:	link	a6,#0
	movl	a6@(8),d0
	movb	d0,sp@-
	movl	a6@(12),d0
	movb	d0,sp@-
	bsrs	shlt
	unlk	a6
	rts

sxcp:
        tstb   sp@(4)           | check boolean
        beqs   clrxcp

        movl   sp@+,a0        | save return address
        addql  #2,sp           | kill boolean
        movb    sp@,d0         | get error index
        extw   d0              | extend to word
        movw    d0,sp@         | replace as word
        pea     sp@            | save address
	cfp	fsetxcp
        addql  #2,sp           | kill index word
        jmp     a0@

clrxcp:
        clrw   d1
        bras   gensetx
shlt:
        moveq   #1,d1
gensetx:
        movl   sp@,a1         | ret address, leave slot

        pea     sp@		
	cfp	fgetenv
        movb    sp@(6),d0        | bit index in hi byte
        extw   d0              | make it word index

        tstb   sp@(4)           | test input boolean
        bnes   .L81

        bclr    d0,sp@(0,d1)
        bras   .L83
.L81:
        bset    d0,sp@(0,d1)
.L83:
        pea     sp@
	cfp	fsetenv

        addql  #8,sp           | kill ret and inputs
        jmp     a1@
