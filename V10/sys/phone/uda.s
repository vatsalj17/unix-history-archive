L11:	.stabs	"uda.c",0x64,0,0,L11
	.stabs	"vaxpcc2",0xf0,0,17665,651903014
	.data
	.comm	_buf,4
	.comm	_buffers,4
	.comm	_nbuf,4
	.comm	_swsize,4
	.comm	_swpf,4
	.comm	_bfreelist,132
	.comm	_bswlist,44
	.comm	_bclnlist,4
	.comm	_udctab,24
	.align	2
	.globl	_udport
_udport:
	.long	_udinit
	.long	_udgpkt
	.long	_udmap
	.long	_udsend
	.long	_udunmap
	.text
	.align	2
	.globl	_udinit
_udinit:
	.word	L.R1
	subl2	$L.SO1,sp
	.stabs	"udinit",0x24,0,161,_udinit
	.stabs	"dev",0xa0,0,14,4
	.stabs	"type",0xa0,0,4,8
	.stabs	"force",0xa0,0,4,12
	.stabs	"cid",0xa0,0,14,16
	.stabs	"seql",0xa0,0,292,20
	.stabs	"dg",0xa0,0,292,24
	cmpl	4(ap),_udcnt
	jlssu	L55
	clrl	r0
	jbr	L53
L55:
	cmpl	16(ap),$2
	jlssu	L56
	clrl	r0
	jbr	L53
L56:
	mull3	$12,16(ap),r0
	movl	20(ap),_udctab(r0)
	mull3	$12,16(ap),r0
	movl	24(ap),_udctab+4(r0)
	mull3	$12,16(ap),r0
	movl	8(ap),_udctab+8(r0)
	mull3	$244,4(ap),r0
	addl3	$_ud,r0,r11
	addl3	4(ap),4(ap),r0
	addl2	r0,r0
	addl2	r0,r0
	cvtwl	_udaddr+6(r0),r10
	bitw	$64,236(r11)
	jeql	L57
	tstl	12(ap)
	jneq	L57
L58:
	movl	$1,r0
	jbr	L53
L57:
	addl3	4(ap),4(ap),r0
	addl2	r0,r0
	addl2	r0,r0
	addl3	$_udaddr,r0,-(sp)
	calls	$1,_ubaddr
	movl	r0,240(r11)
	jneq	L59
	clrl	r0
	jbr	L53
L59:
	pushl	$2
	addl3	$2,240(r11),-(sp)
	pushl	r10
	calls	$3,_ubbadaddr
	tstl	r0
	jeql	L61
	.data	1
L63:

	.byte	0x75,0x64,0x25,0x64,0x20,0x6e,0x6f,0x74
	.byte	0x20,0x70,0x72,0x65,0x73,0x65,0x6e,0x74
	.byte	0xa,0x0
	.text
	pushl	4(ap)
	pushl	$L63
	calls	$2,_printf
	clrl	r0
	jbr	L53
L61:
	pushl	4(ap)
	calls	$1,_udrundown
	bitw	$1,236(r11)
	jneq	L65
	calls	$0,_geteblk
	movl	r0,216(r11)
	pushl	216(r11)
	calls	$1,_clrbuf
	calls	$0,_geteblk
	movl	r0,224(r11)
	pushl	224(r11)
	calls	$1,_clrbuf
	pushl	$0
	pushl	216(r11)
	pushl	r10
	calls	$3,_ubmbuf
	movl	r0,220(r11)
	pushl	$0
	pushl	216(r11)
	pushl	r10
	calls	$3,_ubmbuf
	movl	r0,228(r11)
	movl	216(r11),r0
	movl	28(r0),(r11)
	movl	216(r11),r0
	addl3	$320,28(r0),8(r11)
	movl	224(r11),r0
	movl	28(r0),212(r11)
	bisw2	$1,236(r11)
	pushl	$900
	pushl	4(ap)
	pushl	$_udtimer
	calls	$3,_timeout
L65:
	pushl	4(ap)
	calls	$1,_udreset
	jbr	L53
	.stabs	"ubno",0x40,0,4,10
	.stabs	"up",0x40,0,40,11
L53:
	ret
	.set	L.R1,0xc00
	.set	L.SO1,0x0
L69:	.data
	.text
	.align	2
	.globl	_udreset
_udreset:
	.word	L.R2
	subl2	$L.SO2,sp
	.stabs	"udreset",0x24,0,215,_udreset
	.stabs	"dev",0xa0,0,4,4
	mull3	$244,4(ap),r0
	addl3	$_ud,r0,r11
	movl	240(r11),r10
	bicw2	$2,236(r11)
	bisw2	$64,236(r11)
	clrw	(r10)
	pushl	220(r11)
	pushl	216(r11)
	addl3	4(ap),4(ap),r0
	addl2	r0,r0
	addl2	r0,r0
	cvtwl	_udaddr+6(r0),-(sp)
	calls	$3,_ubadbuf
	movl	r0,r9
	addl3	$8,r9,4(r11)
	addl3	$320,r9,12(r11)
	clrw	16(r11)
	clrw	18(r11)
	clrw	238(r11)
	clrl	r8
	jbr	L73
L74:
	incl	r8
L73:
	bitw	$2048,2(r10)
	jneq	L75
	cmpl	r8,$200000
	jlss	L74
L75:
L72:
	bitw	$2048,2(r10)
	jneq	L76
	bicw2	$64,236(r11)
	.data	1
L77:

	.byte	0x75,0x64,0x25,0x64,0x20,0x77,0x6f,0x6e
	.byte	0x27,0x74,0x20,0x69,0x6e,0x69,0x74,0xa
	.byte	0x0
	.text
	pushl	4(ap)
	pushl	$L77
	calls	$2,_printf
	clrl	r0
	jbr	L70
L76:
	addl3	4(ap),4(ap),r0
	addl2	r0,r0
	addl2	r0,r0
	cvtwl	_udaddr+4(r0),r0
	extzv	$2,$30,r0,r1
	movw	r1,r0
	bisw3	$-21120,r0,2(r10)
	movl	$1,r0
	jbr	L70
	.stabs	"i",0x40,0,4,8
	.stabs	"pa",0x40,0,4,9
	.stabs	"rp",0x40,0,40,10
	.stabs	"up",0x40,0,40,11
L70:
	ret
	.set	L.R2,0xf00
	.set	L.SO2,0x0
L78:	.data
	.text
	.align	2
	.globl	_udinintr
_udinintr:
	.word	L.R3
	subl2	$L.SO3,sp
	.stabs	"udinintr",0x24,0,250,_udinintr
	.stabs	"dev",0xa0,0,4,4
	mull3	$244,4(ap),r0
	addl3	$_ud,r0,r11
	movl	240(r11),r10
	bitw	$2,236(r11)
	jeql	L81
	.data	1
L82:

	.byte	0x75,0x64,0x25,0x64,0x3a,0x20,0x75,0x6e
	.byte	0x65,0x78,0x70,0x65,0x63,0x74,0x65,0x64
	.byte	0x20,0x69,0x6e,0x69,0x74,0x3a,0x20,0x73
	.byte	0x61,0x20,0x25,0x6f,0xa,0x0
	.text
	cvtwl	2(r10),-(sp)
	pushl	4(ap)
	pushl	$L82
	calls	$3,_printf
	jbr	L80
L81:
	cvtwl	2(r10),r0
	bicl2	$-30721,r0
	jbr	L84
L85:
	pushl	4(ap)
	calls	$1,_udreset
	jbr	L80
L86:
	bicw3	$0,4(r11),2(r10)
	jbr	L80
L87:
	extzv	$16,$16,4(r11),r1
	movw	r1,r0
	bicw3	$0,r0,2(r10)
	jbr	L80
L88:
	movw	$1,2(r10)
	clrl	r9
	jbr	L91
L92:
	addl3	$136,(r11),r0
	addl3	r9,r9,r1
	addl2	r1,r1
	addl2	r1,r0
	clrl	(r0)
	ashl	$6,r9,r0
	addl2	8(r11),r0
	movw	$60,(r0)
	addl3	$148,r11,r0
	addl2	r9,r0
	clrb	(r0)
	addl3	$180,r11,r0
	addl2	r9,r0
	movb	$-1,(r0)
	incl	r9
L91:
	cmpl	r9,$32
	jlss	L92
L90:
	pushl	228(r11)
	pushl	224(r11)
	addl3	4(ap),4(ap),r0
	addl2	r0,r0
	addl2	r0,r0
	cvtwl	_udaddr+6(r0),-(sp)
	calls	$3,_ubadbuf
	movl	r0,r8
	addl2	$4,r8
	clrl	r9
	jbr	L95
L96:
	addl3	$8,(r11),r0
	addl3	r9,r9,r1
	addl2	r1,r1
	addl2	r1,r0
	bisl3	$1073741824,r8,r1
	bisl3	$-2147483648,r1,(r0)
	ashl	$6,r9,r0
	addl2	212(r11),r0
	movw	$60,(r0)
	incl	r9
	addl2	$64,r8
L95:
	cmpl	r9,$32
	jlss	L96
L94:
	bisw2	$10,236(r11)
	pushl	r11
	calls	$1,_wakeup
	jbr	L80
L98:
	.data	1
L99:

	.byte	0x75,0x64,0x25,0x64,0x20,0x69,0x6e,0x69
	.byte	0x74,0x20,0x62,0x61,0x64,0x3a,0x20,0x73
	.byte	0x61,0x20,0x25,0x6f,0xa,0x0
	.text
	cvtwl	2(r10),-(sp)
	pushl	4(ap)
	pushl	$L99
	calls	$3,_printf
	jbr	L80
L84:
	cmpl	r0,$2048
	jeql	L85
	cmpl	r0,$4096
	jeql	L86
	cmpl	r0,$8192
	jeql	L87
	cmpl	r0,$16384
	jeql	L88
	jbr	L98
L83:
	.stabs	"pa",0x40,0,4,8
	.stabs	"i",0x40,0,4,9
	.stabs	"rp",0x40,0,40,10
	.stabs	"up",0x40,0,40,11
L80:
	ret
	.set	L.R3,0xf00
	.set	L.SO3,0x0
L100:	.data
	.text
	.align	2
	.globl	_udrundown
_udrundown:
	.word	L.R4
	subl2	$L.SO4,sp
	.stabs	"udrundown",0x24,0,308,_udrundown
	.stabs	"dev",0xa0,0,4,4
	.lcomm	L102,48
	movw	$255,L102+10
	clrl	r11
	jbr	L105
L106:
	mull3	$12,r11,r0
	tstl	_udctab(r0)
	jeql	L107
	pushl	$L102
	mull3	$12,r11,r0
	pushl	_udctab+8(r0)
	pushl	4(ap)
	mull3	$12,r11,r0
	calls	$3,*_udctab(r0)
L107:
	incl	r11
L105:
	cmpl	r11,$2
	jlss	L106
L104:
	.stabs	"i",0x40,0,4,11
	.stabs	"me",0x28,0,8,L102
L101:
	ret
	.set	L.R4,0x800
	.set	L.SO4,0x0
L108:	.data
	.text
	.align	2
	.globl	_udgpkt
_udgpkt:
	.word	L.R5
	subl2	$L.SO5,sp
	.stabs	"udgpkt",0x24,0,331,_udgpkt
	.stabs	"dev",0xa0,0,4,4
	mull3	$244,4(ap),r0
	addl3	$_ud,r0,r11
	movl	240(r11),r0
	bitw	$2048,2(r0)
	jeql	L110
	pushl	4(ap)
	calls	$1,_udreset
L110:
	calls	$0,_spl6
	movl	r0,-4(fp)
	jbr	L113
L114:
	pushl	$22
	pushl	r11
	calls	$2,_sleep
L113:
	bitw	$2,236(r11)
	jeql	L114
L112:
	jbr	L117
L118:
	pushl	$1
	pushl	4(ap)
	calls	$2,_udpkscan
	tstl	r0
	jeql	L120
	jbr	L117
L120:
	bisw2	$32,236(r11)
	pushl	$24
	addl3	$238,r11,-(sp)
	calls	$2,_sleep
L117:
	cmpw	238(r11),$2
	jgeq	L121
	bitw	$8,236(r11)
	jeql	L118
L121:
L116:
	bitw	$8,236(r11)
	jneq	L122
	decw	238(r11)
L122:
L125:
	clrl	r10
	jbr	L128
L129:
	addl3	$148,r11,r0
	addl2	r10,r0
	tstb	(r0)
	jneq	L130
	jbr	L127
L130:
	incl	r10
L128:
	cmpl	r10,$32
	jlss	L129
L127:
	cmpl	r10,$32
	jgeq	L131
	jbr	L124
L131:
	pushl	$1
	pushl	4(ap)
	calls	$2,_udpkscan
	tstl	r0
	jneq	L133
	pushl	4(ap)
	calls	$1,_udcmdscan
	tstl	r0
	jneq	L133
L134:
	bisw2	$4,236(r11)
	pushl	$23
	addl3	$148,r11,-(sp)
	calls	$2,_sleep
L133:
	jbr	L125
L124:
	addl3	$148,r11,r0
	addl2	r10,r0
	movb	$1,(r0)
	pushl	-4(fp)
	calls	$1,_splx
	ashl	$6,r10,r0
	addl2	8(r11),r0
	addl2	$4,r0
	jbr	L109
	.stabs	"s",0x80,0,4,4
	.stabs	"i",0x40,0,4,10
	.stabs	"up",0x40,0,40,11
L109:
	ret
	.set	L.R5,0xc00
	.set	L.SO5,0x4
L136:	.data
	.text
	.align	2
	.globl	_udmap
_udmap:
	.word	L.R6
	subl2	$L.SO6,sp
	.stabs	"udmap",0x24,0,376,_udmap
	.stabs	"dev",0xa0,0,4,4
	.stabs	"mp",0xa0,0,40,8
	.stabs	"bp",0xa0,0,40,12
	movl	12(ap),r11
	mull3	$244,4(ap),r0
	addl3	$_ud,r0,r10
	subl3	$4,8(ap),r0
	subl2	8(r10),r0
	divl3	$64,r0,r9
	addl3	$20,r10,r0
	addl3	r9,r9,r1
	addl2	r1,r1
	addl2	r1,r0
	tstl	(r0)
	jneq	L138
	pushl	$3
	pushl	r11
	addl3	4(ap),4(ap),r0
	addl2	r0,r0
	addl2	r0,r0
	cvtwl	_udaddr+6(r0),-(sp)
	calls	$3,_ubmbuf
	movl	r0,r8
	addl3	$20,r10,r0
	addl3	r9,r9,r1
	addl2	r1,r1
	addl2	r1,r0
	movl	r8,(r0)
L138:
	addl3	$148,r10,r0
	addl2	r9,r0
	bisb2	$4,(r0)
	addl3	$20,r10,r0
	addl3	r9,r9,r1
	addl2	r1,r1
	addl2	r1,r0
	pushl	(r0)
	pushl	r11
	addl3	4(ap),4(ap),r0
	addl2	r0,r0
	addl2	r0,r0
	cvtwl	_udaddr+6(r0),-(sp)
	calls	$3,_ubadbuf
	movl	r0,r8
	movl	8(ap),r0
	movl	r8,16(r0)
	movl	8(ap),r0
	addl3	$20,r10,r1
	addl3	r9,r9,r2
	addl2	r2,r2
	addl2	r2,r1
	extzv	$20,$12,(r1),r2
	bicl2	$-16,r2
	ashl	$24,r2,r2
	bisl2	r2,16(r0)
	.stabs	"i",0x40,0,4,9
	.stabs	"up",0x40,0,40,10
	.stabs	"bp",0x40,0,40,11
L137:
	ret
	.set	L.R6,0xf00
	.set	L.SO6,0x0
L139:	.data
	.text
	.align	2
	.globl	_udunmap
_udunmap:
	.word	L.R7
	subl2	$L.SO7,sp
	.stabs	"udunmap",0x24,0,410,_udunmap
	.stabs	"dev",0xa0,0,4,4
	.stabs	"mp",0xa0,0,40,8
	mull3	$244,4(ap),r0
	addl3	$_ud,r0,r11
	subl3	$4,8(ap),r0
	subl2	8(r11),r0
	divl3	$64,r0,r10
	addl3	$20,r11,r0
	addl3	r10,r10,r1
	addl2	r1,r1
	addl2	r1,r0
	tstl	(r0)
	jeql	L141
	addl3	$20,r11,r0
	addl3	r10,r10,r1
	addl2	r1,r1
	addl2	r1,r0
	pushl	(r0)
	addl3	4(ap),4(ap),r0
	addl2	r0,r0
	addl2	r0,r0
	cvtwl	_udaddr+6(r0),-(sp)
	calls	$2,_ubmfree
	addl3	$20,r11,r0
	addl3	r10,r10,r1
	addl2	r1,r1
	addl2	r1,r0
	clrl	(r0)
L141:
	addl3	$148,r11,r0
	addl2	r10,r0
	clrb	(r0)
	bitw	$4,236(r11)
	jeql	L143
	bicw2	$4,236(r11)
	addl3	$148,r11,-(sp)
	calls	$1,_wakeup
L143:
	.stabs	"i",0x40,0,4,10
	.stabs	"up",0x40,0,40,11
L140:
	ret
	.set	L.R7,0xc00
	.set	L.SO7,0x0
L144:	.data
	.text
	.align	2
	.globl	_udsend
_udsend:
	.word	L.R8
	subl2	$L.SO8,sp
	.stabs	"udsend",0x24,0,442,_udsend
	.stabs	"dev",0xa0,0,4,4
	.stabs	"cid",0xa0,0,4,8
	.stabs	"mp",0xa0,0,40,12
	mull3	$244,4(ap),r0
	addl3	$_ud,r0,r11
	bicw2	$8,236(r11)
	subl3	$4,12(ap),r0
	subl2	8(r11),r0
	divl3	$64,r0,r10
	ashl	$6,r10,r0
	addl2	8(r11),r0
	movb	8(ap),3(r0)
	movw	16(r11),r0
	incw	16(r11)
	cvtwl	r0,r9
	cmpw	16(r11),$32
	jlss	L146
	clrw	16(r11)
L146:
	addl3	$136,(r11),r0
	addl3	r9,r9,r1
	addl2	r1,r1
	addl2	r1,r0
	bitl	$-2147483648,(r0)
	jeql	L147
	.data	1
L149:

	.byte	0x75,0x64,0x73,0x65,0x6e,0x64,0x0
	.text
	pushl	$L149
	calls	$1,_panic
L147:
	addl3	$180,r11,r0
	addl2	r9,r0
	tstb	(r0)
	jlss	L150
	pushl	4(ap)
	calls	$1,_udcmdscan
	addl3	$180,r11,r0
	addl2	r9,r0
	tstb	(r0)
	jlss	L151
	.data	1
L152:

	.byte	0x75,0x64,0x73,0x65,0x6e,0x64,0x0
	.text
	pushl	$L152
	calls	$1,_panic
L151:
L150:
	movl	240(r11),r8
	bitw	$32768,2(r8)
	jeql	L153
	.data	1
L154:

	.byte	0x75,0x64,0x25,0x64,0x3a,0x20,0x68,0x61
	.byte	0x72,0x64,0x20,0x65,0x72,0x72,0x6f,0x72
	.byte	0x20,0x25,0x6f,0xa,0x0
	.text
	cvtwl	2(r8),r0
	bicl3	$-65536,r0,-(sp)
	pushl	4(ap)
	pushl	$L154
	calls	$3,_printf
	pushl	4(ap)
	calls	$1,_udreset
	jbr	L145
L153:
	addl3	$180,r11,r0
	addl2	r9,r0
	movb	r10,(r0)
	addl3	$136,(r11),r0
	addl3	r9,r9,r1
	addl2	r1,r1
	addl2	r1,r0
	ashl	$6,r10,r1
	addl2	12(r11),r1
	addl2	$4,r1
	bisl3	$-1073741824,r1,(r0)
	addl3	$148,r11,r0
	addl2	r10,r0
	bisb2	$2,(r0)
	addl3	$148,r11,r0
	addl2	r10,r0
	bicb2	$1,(r0)
	cvtwl	(r8),r10
	.stabs	"rp",0x40,0,40,8
	.stabs	"j",0x40,0,4,9
	.stabs	"i",0x40,0,4,10
	.stabs	"up",0x40,0,40,11
L145:
	ret
	.set	L.R8,0xf00
	.set	L.SO8,0x0
L155:	.data
	.comm	_ud_spur,4
	.comm	_ud_npr,4
	.text
	.align	2
	.globl	_ud0int
_ud0int:
	.word	L.R9
	subl2	$L.SO9,sp
	.stabs	"ud0int",0x24,0,486,_ud0int
	.stabs	"dev",0xa0,0,4,4
	mull3	$244,4(ap),r0
	addl3	$_ud,r0,r11
	cmpl	4(ap),_udcnt
	jgeq	L161
	bitw	$1,236(r11)
	jneq	L160
L161:
	.data	1
L162:

	.byte	0x75,0x64,0x25,0x64,0x3a,0x20,0x73,0x74
	.byte	0x72,0x61,0x79,0x20,0x69,0x6e,0x74,0x72
	.byte	0xa,0x0
	.text
	pushl	4(ap)
	pushl	$L162
	calls	$2,_printf
	jbr	L159
L160:
	movl	240(r11),r10
	bitw	$2,236(r11)
	jneq	L163
	clrl	r9
	jbr	L166
L167:
	bitw	$63488,2(r10)
	jeql	L168
	jbr	L165
L168:
	incl	r9
L166:
	cmpl	r9,$5
	jlss	L167
L165:
	bitw	$63488,2(r10)
	jneq	L169
	.data	1
L170:

	.byte	0x75,0x64,0x25,0x64,0x3a,0x20,0x69,0x6e
	.byte	0x69,0x74,0x20,0x6c,0x6f,0x73,0x74,0x3b
	.byte	0x20,0x73,0x61,0x20,0x30,0x25,0x6f,0xa
	.byte	0x0
	.text
	cvtwl	2(r10),-(sp)
	pushl	4(ap)
	pushl	$L170
	calls	$3,_printf
	jbr	L159
L169:
L163:
	bitw	$32768,2(r10)
	jeql	L171
	.data	1
L172:

	.byte	0x75,0x64,0x25,0x64,0x3a,0x20,0x68,0x61
	.byte	0x72,0x64,0x20,0x65,0x72,0x72,0x6f,0x72
	.byte	0x20,0x25,0x6f,0xa,0x0
	.text
	cvtwl	2(r10),r0
	bicl3	$-65536,r0,-(sp)
	pushl	4(ap)
	pushl	$L172
	calls	$3,_printf
	pushl	4(ap)
	calls	$1,_udreset
	jbr	L159
L171:
	bitw	$30720,2(r10)
	jeql	L173
	pushl	4(ap)
	calls	$1,_udinintr
	jbr	L159
L173:
	movl	(r11),r0
	tstb	3(r0)
	jneq	L174
	movl	(r11),r0
	tstw	4(r0)
	jneq	L174
L176:
	movl	(r11),r0
	tstw	6(r0)
	jneq	L174
L175:
	incl	_ud_spur
L174:
	movl	(r11),r0
	tstb	3(r0)
	jeql	L177
	movl	(r11),r0
	cvtbl	3(r0),-(sp)
	addl3	4(ap),4(ap),r0
	addl2	r0,r0
	addl2	r0,r0
	cvtwl	_udaddr+6(r0),-(sp)
	calls	$2,_ubmflush
	movl	(r11),r0
	clrb	3(r0)
	clrw	2(r10)
	incl	_ud_npr
L177:
	jbr	L180
L181:
	movl	(r11),r0
	clrw	4(r0)
	pushl	4(ap)
	calls	$1,_udcmdscan
L180:
	movl	(r11),r0
	tstw	4(r0)
	jneq	L181
L179:
	jbr	L183
L184:
	movl	(r11),r0
	clrw	6(r0)
	pushl	$0
	pushl	4(ap)
	calls	$2,_udpkscan
	tstl	r0
	jeql	L185
	bicw2	$16,236(r11)
L185:
L183:
	movl	(r11),r0
	tstw	6(r0)
	jneq	L184
L182:
	.stabs	"i",0x40,0,4,9
	.stabs	"rp",0x40,0,40,10
	.stabs	"up",0x40,0,40,11
L159:
	ret
	.set	L.R9,0xe00
	.set	L.SO9,0x0
L186:	.data
	.text
	.align	2
	.globl	_udcmdscan
_udcmdscan:
	.word	L.R10
	subl2	$L.SO10,sp
	.stabs	"udcmdscan",0x24,0,543,_udcmdscan
	.stabs	"dev",0xa0,0,4,4
	mull3	$244,4(ap),r0
	addl3	$_ud,r0,r11
	movl	(r11),r7
	clrl	r8
	clrl	r9
	jbr	L190
L191:
	addl3	$180,r11,r0
	addl2	r9,r0
	tstb	(r0)
	jlss	L192
	addl3	$136,r7,r0
	addl3	r9,r9,r1
	addl2	r1,r1
	addl2	r1,r0
	bitl	$-2147483648,(r0)
	jneq	L192
L193:
	addl3	$180,r11,r0
	addl2	r9,r0
	cvtbl	(r0),r10
	addl3	$148,r11,r0
	addl2	r10,r0
	cvtbl	(r0),r0
	bicl2	$-7,r0
	cmpl	r0,$2
	jneq	L194
	addl3	$148,r11,r0
	addl2	r10,r0
	clrb	(r0)
	incl	r8
L194:
	addl3	$180,r11,r0
	addl2	r9,r0
	movb	$-1,(r0)
L192:
	incl	r9
L190:
	cmpl	r9,$32
	jlss	L191
L189:
	tstl	r8
	jeql	L195
	bitw	$4,236(r11)
	jeql	L195
L196:
	addl3	$148,r11,-(sp)
	calls	$1,_wakeup
L195:
	movl	r8,r0
	jbr	L187
	.stabs	"udc",0x40,0,40,7
	.stabs	"freed",0x40,0,4,8
	.stabs	"j",0x40,0,4,9
	.stabs	"i",0x40,0,4,10
	.stabs	"up",0x40,0,40,11
L187:
	ret
	.set	L.R10,0xf80
	.set	L.SO10,0x0
L197:	.data
	.text
	.align	2
	.globl	_udpkscan
_udpkscan:
	.word	L.R11
	subl2	$L.SO11,sp
	.stabs	"udpkscan",0x24,0,578,_udpkscan
	.stabs	"dev",0xa0,0,4,4
	.stabs	"doall",0xa0,0,4,8
	mull3	$244,4(ap),r0
	addl3	$_ud,r0,r11
	movl	(r11),r7
	clrl	-4(fp)
	cvtwl	18(r11),r10
L201:
	addl3	$8,r7,r0
	addl3	r10,r10,r1
	addl2	r1,r1
	addl2	r1,r0
	bitl	$-2147483648,(r0)
	jeql	L202
	movw	r10,18(r11)
	jbr	L200
L202:
	incl	-4(fp)
	ashl	$6,r10,r0
	addl3	212(r11),r0,r9
	cvtbw	2(r9),r0
	bicw2	$-16,r0
	addw2	r0,238(r11)
	bitw	$32,236(r11)
	jeql	L203
	addl3	$238,r11,-(sp)
	calls	$1,_wakeup
	bicw2	$32,236(r11)
L203:
	cmpb	3(r9),$2
	jleq	L204
	.data	1
L205:

	.byte	0x75,0x64,0x25,0x64,0x20,0x6d,0x73,0x67
	.byte	0x20,0x69,0x64,0x20,0x25,0x64,0xa,0x0
	.text
	cvtbl	3(r9),-(sp)
	pushl	4(ap)
	pushl	$L205
	calls	$3,_printf
	jbr	L206
L204:
	cvtbl	3(r9),r0
	mull2	$12,r0
	addl3	$_udctab,r0,r8
	cvtbl	2(r9),r0
	bicl2	$-241,r0
	jbr	L208
L209:
	tstl	(r8)
	jeql	L210
	addl3	$4,r9,-(sp)
	pushl	8(r8)
	pushl	4(ap)
	calls	$3,*(r8)
L210:
	jbr	L207
L211:
	tstl	4(r8)
	jeql	L212
	addl3	$4,r9,-(sp)
	pushl	8(r8)
	pushl	4(ap)
	calls	$3,*4(r8)
L212:
	jbr	L207
L208:
	casel	r0,$0,$16
L213:
	.word	L209-L213
	.word	L214-L213
	.word	L214-L213
	.word	L214-L213
	.word	L214-L213
	.word	L214-L213
	.word	L214-L213
	.word	L214-L213
	.word	L214-L213
	.word	L214-L213
	.word	L214-L213
	.word	L214-L213
	.word	L214-L213
	.word	L214-L213
	.word	L214-L213
	.word	L214-L213
	.word	L211-L213
L214:
L207:
L206:
	movw	$60,(r9)
	addl3	$8,r7,r0
	addl3	r10,r10,r1
	addl2	r1,r1
	addl2	r1,r0
	bisl2	$-1073741824,(r0)
	cmpl	r10,$31
	jgeq	L215
	movl	r10,r0
	incl	r10
	jbr	L216
L215:
	clrl	r10
	movl	r10,r0
L216:
	jbr	L201
L200:
	movl	-4(fp),r0
	jbr	L198
	.stabs	"udc",0x40,0,40,7
	.stabs	"cp",0x40,0,40,8
	.stabs	"pk",0x40,0,40,9
	.stabs	"nf",0x80,0,4,4
	.stabs	"i",0x40,0,4,10
	.stabs	"up",0x40,0,40,11
L198:
	ret
	.set	L.R11,0xf80
	.set	L.SO11,0x4
L217:	.data
	.comm	_ud_kicked,4
	.text
	.align	2
	.globl	_udtimer
_udtimer:
	.word	L.R12
	subl2	$L.SO12,sp
	.stabs	"udtimer",0x24,0,637,_udtimer
	.stabs	"i",0xa0,0,4,4
	mull3	$244,4(ap),r0
	addl3	$_ud,r0,r11
	bitw	$1,236(r11)
	jneq	L220
	jbr	L219
L220:
	bitw	$2,236(r11)
	jeql	L221
	bitw	$16,236(r11)
	jneq	L222
	bisw2	$16,236(r11)
	jbr	L223
L222:
	calls	$0,_spl6
	movl	r0,r10
	pushl	$1
	pushl	4(ap)
	calls	$2,_udpkscan
	tstl	r0
	jeql	L224
	bitw	$4,236(r11)
	jeql	L224
L225:
	addl3	$148,r11,-(sp)
	calls	$1,_wakeup
	incl	_ud_kicked
L224:
	pushl	r10
	calls	$1,_splx
	bicw2	$16,236(r11)
L223:
L221:
	pushl	$900
	pushl	4(ap)
	pushl	$_udtimer
	calls	$3,_timeout
	.stabs	"s",0x40,0,4,10
	.stabs	"up",0x40,0,40,11
L219:
	ret
	.set	L.R12,0xc00
	.set	L.SO12,0x0
L226:	.data
	.text
L227:	.stabs	"uda.c",0x94,0,659,L227
