////////
/ sys/io.386/mmas.s
/ Memory mapped video driver assembler assist.
/ Almost identical to vtmmas.s, they should be kept in sync until merged.
////////

//////////
/
/ This module handles special characters and escape sequences
/ as described in ANSI X3.4-1977 and ANSI X3.64-1979.
/ It recognizes only a subset of the X3.64 sequences.
/ It handles some ASCII control characters (cf. asctab), as follows:
/	BEL			audible signal
/	BS			backspace
/	CR			carriage return
/	ESC			[see tables below]
/	FF			form feed
/	HT			horizontal tab
/	LF/NL			line feed/newline
/	VT			vertical tab
/ In the default font (font 0), other control characters are ignored
/ and all other ASCII characters are printed.
/ In font 1, all characters (including control chars) except ESC are printed.
/ In font 2, all characters except ESC are printed with the high bit toggled.
/ ASCII ESC introduces an escape sequence (cf. esctab), as follows:
/	ESC 7			save cursor position
/	ESC 8			restore cursor position
/	ESC =			enter alternate keypad (cf. kb.c)
/	ESC >			exit alternate keypad (cf. kb.c)
/	ESC D		IND	index
/	ESC E		NEL	next line
/	ESC M		RI	reverse index
/	ESC [		CSI	[see table below]
/	ESC `		DMI	disable manual input
/	ESC b		EMI	enable manual input
/	ESC c		RIS	reset to initial state
/	ESC t			enter numlock (cf. kb.c)
/	ESC u			exit numlock (cf. kb.c)
/ ESC [ (a.k.a. ANSI CSI) introduces a function (cf. csitab), as follows:
/	ESC [ > 13 h		enable CRT saver
/	ESC [ > 13 l		disable CRT saver
/	ESC [ ?	4 h		set smooth scroll
/	ESC [ ?	4 l		set jump scroll
/	ESC [ ?	7 h		enable wraparound
/	ESC [ ?	7 l		disable wraparound
/	ESC [ ?	8 h		set erase in current color even if reversed
/	ESC [ ?	8 l		set erase in original foreground color
/	ESC [ ?	25 h		enable line 25
/	ESC [ ?	25 l		disable line 25
/	ESC [ <n> @	ICH	insert character
/	ESC [ <n> A	CUU	cursor up
/	ESC [ <n> B	CUD	cursor down
/	ESC [ <n> C	CUF	cursor forward
/	ESC [ <n> D	CUB	cursor backward
/	ESC [ <n> E	CNL	cursor next line
/	ESC [ <n> F	CPL	cursor preceding line
/	ESC [ <n> G	CHA	cursor horizontal absolute
/	ESC [ <n;m> H	CUP	cursor position
/	ESC [ <n> I	CHT	cursor horizontal tabulation
/	ESC [ <n> J	ED	erase in display
/	ESC [ <n> K	EL	erase in line
/	ESC [ <n> L	IL	insert line
/	ESC [ <n> M	DL	delete line
/	ESC [ <n> O	EA	erase in area
/	ESC [ <n> P	DCH	delete character
/	ESC [ <n> S	SU	scroll up
/	ESC [ <n> T	SD	scroll down
/	ESC [ <n> X	ECH	erase character
/	ESC [ <n> Z	CBT	cursor backward tabulation
/	ESC [ <n> `	HPA	horizontal position absolute
/	ESC [ <n> a	HPR	horizontal position relative
/	ESC [ <n> d	VPA	vertical position absolute
/	ESC [ <n> e	VPR	vertical position relative
/	ESC [ <n;m> f	HVP	horizontal and vertical position
/ ESC [ <n1;...;nn> m	SGR	select graphic rendition (see details below)
/	ESC [ <n;m> r		set scrolling region
/	ESC [ <n> v		select cursor rendition (0=visible, 1=invisible)
/
//////////

////////
/
/ State driven code.
/
/ Register use:
/	DS:ESI	input string
/	ES:EDI	current screen location
/	FS:EBP	terminal information
/	AH	character attributes
/	AL	character
/	BH	(usually) kept zeroed for efficiency
/	CX	input count
/	DH	current row
/	DL	current column
/
////////

	.unixorder

/ Externals referenced by this module.
	.globl	int11
	.globl	islock
	.globl	mmbeeps
	.globl	mmesc
	.globl	mmvcnt
	.globl	mmcrtsav

/ Globals defined in this module.
	.globl	VIDSLOW			/ Patchable kernel variable
	.globl	mmdata			/ in .data
	.globl	mminit
	.globl	mmgo
	.globl	mm_voff
	.globl	mm_von

/ Definitions.

ATTR	.define	%ah			/ attribute byte
ROW	.define	%dh			/ currently active vertical position
COL	.define	%dl			/ currently active horizontal position
POS	.define	%edi			/ currently active display address

/ Manifest constants.

	.set	DPL_1, 1		/ descriptor privilege level for SEG_VIDEOa/b
	.set	NCOL, 80		/ number of columns
	.set	NROW, 24		/ initial number of rows
	.set	NCR, 1			/ number of horizontal lines per char
	.set	NHB, 160		/ number of horizontal bytes per line
	.set	NRB, 160		/ number of bytes per character row(NCR*NHB)
	.set	MAXARGS, 8		/ max number of args in parameter list

	.set	MONO,		0x03B4	/ color port
	.set	COLOR,		0x03D4	/ color port
	.set	DEFATTR,	0x07	/ default attribute
	.set	CULATTR,	0x47	/ color underline attribute (wht/red)
	.set	UNDERLINE,	0x01	/ mono underline bit
	.set	INTENSE,	0x08	/ high intensity attribute bit
	.set	BLINK,		0x80	/ blinking attribute bit
	.set	TIMEOUT,	900	/ seconds before video disabled

	.set	SEG_386_UD,	0x10 	/ 32 bit user data segment descriptor
	.set	SEG_ROM,	0x40	/ ROM descriptor         @ F0000
	.set	SEG_VIDEOa,	0x48	/ 0x48: video descriptor @ B0000
	.set	SEG_VIDEOb,	0x50	/ 0x50: video descriptor @ B8000

/ Magic constants from <sys/io.h>.
	.set	IO_SEG, 0
	.set	IO_IOC, 4
	.set	IO_BASE, 12
	.set	IOSYS, 0
	.set	IOUSR, 1

/ Offsets corresponding to <sys/vt.h> VTDATA structure.
	.set	MM_FUNC,	0	/ current state
	.set	MM_PORT,	4	/ adapter base i/o port
	.set	MM_BASE,	8	/ adapter base memory address
	.set	MM_OFFSET,	12	/ offset within segment
	.set	MM_ROW,		16	/ screen row
	.set	MM_COL,		20	/ screen column
	.set	MM_POS,		24	/ screen position
	.set	MM_ATTR,	28	/ attributes
	.set	MM_N1,		32	/ numeric argument 1
	.set	MM_N2,		33	/ numeric argument 2
	.set	MM_NARGS,	40	/ arg count
	.set	MM_BROW,	44	/ base row
	.set	MM_EROW,	48	/ end row
	.set	MM_LROW,	52	/ legal row limit
	.set	MM_SROW,	56	/ saved cursor row
	.set	MM_SCOL,	60	/ saved cursor column
	.set	MM_IBROW,	64	/ initial base row
	.set	MM_IEROW,	68	/ initial end row
	.set	MM_INVIS,	72	/ cursor invisible mask
	.set	MM_SLOW,	76	/ slow [no flicker] video update
	.set	MM_WRAP,	80	/ wrap to start of next line
	.set	MM_EORIG,	81	/ erase in original foreground color
	.set	MM_OATTR,	84	/ original attribute
	.set	MM_FONT,	88	/ current font [012]
	.set	MM_ESC,		92	/ escape char. state
/ The following are unused by this source but are used in vtmmas.s.
	.set	MM_VISIBLE,	96	/ set if screen is being displayed
	.set	MM_MSEG,	100	/ heap space copy
	.set	MM_MOFF,	104	/
	.set	MM_VSEG,	108	/ video memory
	.set	MM_VOFF,	112	/

////////
/
/ Data.
/
////////

	.data

mmdata:	.long	mminit			/ FUNC
	.long	0x03B4			/ PORT
	.long	[SEG_VIDEOa|DPL_1]	/ BASE
	.long	0			/ OFFSET
	.long	0, 0			/ ROW, COL
	.long	0			/ POS
	.long	DEFATTR			/ ATTR
	.byte	0, 0, 0, 0, 0, 0, 0, 0	/ ARGS
	.long	0			/ NARGS
	.long	0, 24, 25		/ BROW, EROW, LROW
	.long	0, 0			/ SROW, SCOL
	.long	0, 24			/ IBROW, IEROW
	.long	0			/ INVIS
VIDSLOW:.long	0			/ SLOW
	.long	1			/ WRAP
	.long	DEFATTR			/ OATTR
	.long	0			/ FONT
mmesc:	.long	0			/ MM_ESC

	.text
	.align	4

////////
/
/ Constant data tables (in .text segment).
/
////////

////////
/
/ asctab - table of functions indexed by ASCII character.
/ This is used for mapping control characters only.
/
////////

asctab:	.long	eval,	eval,	eval,	eval	/* NUL  SOH  STX  ETX  */
	.long	eval,	eval,	eval,	mm_bel	/* EOT  ENQ  ACK  BEL  */
	.long	mm_bs,	mm_ht,	mm_lf,	mm_vt	/* BS   HT   LF   VT   */
	.long	mm_ff,	mm_cr,	eval,	eval	/* FF   CR   SO   SI   */
	.long	eval,	eval,	eval,	eval	/* DLE  DC1  DC2  DC3  */
	.long	eval,	eval,	eval,	eval	/* DC4  NAK  SYN  ETB  */
	.long	eval,	eval,	eval,	mm_esc	/* CAN  EM   SUB  ESC  */
	.long	eval,	eval,	eval,	eval	/* FS   GS   RS   US   */

////////
/
/ esctab - table of functions indexed by character following ESC - 0x20.
/
////////

esctab:	.long	eval,	eval,	eval,	eval	/*   ! " # \040 - \043 */
	.long	eval,	eval,	eval,	eval	/* $ % & ' \044 - \047 */
	.long	eval,	eval,	eval,	eval	/* ( ) * + \050 - \053 */
	.long	eval,	eval,	eval,	eval	/* , - . / \054 - \057 */
	.long	eval,	eval,	eval,	eval	/* 0 1 2 3 \060 - \063 */
	.long	eval,	eval,	eval,	mm_new	/* 4 5 6 7 \064 - \067 */
	.long	mm_old,	eval,	eval,	eval	/* 8 9 : ; \070 - \073 */
	.long	eval,	mm_spc,	mm_spc,	eval	/* < = > ? \074 - \077 */
	.long	eval,	eval,	eval,	eval	/* @ A B C \100 - \103 */
	.long	mm_ind,	mm_nel,	eval,	eval	/* D E F G \104 - \107 */
	.long	eval,	eval,	eval,	eval	/* H I J K \110 - \113 */
	.long	eval,	mm_ri,	eval,	eval	/* L M N O \114 - \117 */
	.long	eval,	eval,	eval,	eval	/* P Q R S \120 - \123 */
	.long	eval,	eval,	eval,	eval	/* T U V W \124 - \127 */
	.long	eval,	eval,	eval,	csi	/* X Y Z [ \130 - \133 */
	.long	eval,	eval,	eval,	eval	/* \ ] ^ _ \134 - \137 */
	.long	mm_dmi,	eval,	mm_emi,	mminit	/* ` a b c \140 - \143 */
	.long	eval,	eval,	eval,	eval	/* d e f g \144 - \147 */
	.long	eval,	eval,	eval,	eval	/* h i j k \150 - \153 */
	.long	eval,	eval,	eval,	eval	/* l m n o \154 - \157 */
	.long	eval,	eval,	eval,	eval	/* p q r s \160 - \163 */
	.long	mm_spc,	mm_spc,	eval,	eval	/* t u v w \164 - \167 */
	.long	eval,	eval,	eval,	eval	/* x y z { \170 - \173 */
	.long	eval,	eval,	eval,	eval	/* | } ~ ? \174 - \177 */

////////
/
/ csitab - table of functions indexed by character following ESC [ <params>.
/
////////

csitab:	.long	eval,	eval,	eval,	eval	/* NUL  SOH  STX  ETX  */
	.long	eval,	eval,	eval,	eval	/* EOT  ENQ  ACK  BEL  */
	.long	eval,	eval,	eval,	eval	/* BS   HT   LF   VT   */
	.long	eval,	eval,	eval,	eval	/* FF   CR   SO   SI   */
	.long	eval,	eval,	eval,	eval	/* DLE  DC1  DC2  DC3  */
	.long	eval,	eval,	eval,	eval	/* DC4  NAK  SYN  ETB  */
	.long	eval,	eval,	eval,	eval	/* CAN  EM   SUB  ESC  */
	.long	eval,	eval,	eval,	eval	/* FS   GS   RS   US   */
	.long	eval,	eval,	eval,	eval	/*   ! " # \040 - \043 */
	.long	eval,	eval,	eval,	eval	/* $ % & ' \044 - \047 */
	.long	eval,	eval,	eval,	eval	/* ( ) * + \050 - \053 */
	.long	eval,	eval,	eval,	eval	/* , - . / \054 - \057 */
	.long	eval,	eval,	eval,	eval	/* 0 1 2 3 \060 - \063 */
	.long	eval,	eval,	eval,	eval	/* 4 5 6 7 \064 - \067 */
	.long	eval,	eval,	eval,	eval	/* 8 9 : ; \070 - \073 */
	.long	eval,	eval,	csi_gt,	csi_q	/* < = > ? \074 - \077 */
	.long	mm_ich,	mm_cuu,	mm_cud,	mm_cuf	/* @ A B C \100 - \103 */
	.long	mm_cub,	mm_cnl,	mm_cpl,	mm_cha	/* D E F G \104 - \107 */
	.long	mm_cup,	mm_cht,	mm_ed,	mm_el	/* H I J K \110 - \113 */
	.long	mm_il,	mm_dl,	eval,	mm_ea	/* L M N O \114 - \117 */
	.long	mm_dch,	eval,	eval,	mm_su	/* P Q R S \120 - \123 */
	.long	mm_sd,	eval,	eval,	eval	/* T U V W \124 - \127 */
	.long	mm_ech,	eval,	mm_cbt,	eval	/* X Y Z [ \130 - \133 */
	.long	eval,	eval,	eval,	eval	/* \ ] ^ _ \134 - \137 */
	.long	mm_hpa,	mm_hpr,	eval,	eval	/* ` a b c \140 - \143 */
	.long	mm_vpa,	mm_vpr,	mm_hvp,	eval	/* d e f g \144 - \147 */
	.long	eval,	eval,	eval,	eval	/* h i j k \150 - \153 */
	.long	eval,	mm_sgr,	eval,	eval	/* l m n o \154 - \157 */
	.long	eval,	eval,	mm_ssr,	eval	/* p q r s \160 - \163 */
	.long	eval,	eval,	mm_scr,	eval	/* t u v w \164 - \167 */
	.long	eval,	eval,	eval,	eval	/* x y z { \170 - \173 */
	.long	eval,	eval,	eval,	eval	/* | } ~ ? \174 - \177 */

////////
/
/ rowtab - array of offsets to each display row.
/
////////

rowtab:	.long	NRB *  0,	NRB *  1,	NRB *  2,	NRB *  3
	.long	NRB *  4,	NRB *  5,	NRB *  6,	NRB *  7
	.long	NRB *  8,	NRB *  9,	NRB * 10,	NRB * 11
	.long	NRB * 12,	NRB * 13,	NRB * 14,	NRB * 15
	.long	NRB * 16,	NRB * 17,	NRB * 18,	NRB * 19
	.long	NRB * 20,	NRB * 21,	NRB * 22,	NRB * 23
	.long	NRB * 24,	NRB * 25,	NRB * 26,	NRB * 27
	.long	NRB * 28,	NRB * 29,	NRB * 30,	NRB * 31

////////
/
/ fcolor - foreground color
/ bcolor - background color
/
/ Indexed by ANSI color (black,red,green,brown,blue,magenta,cyan,white)
/ to get graphics color (black,blue,green,cyan,red,magenta,brown,white)
/ which is properly preshifted for installation in attribute byte.
/
////////

fcolor:	.byte	0x00, 0x04, 0x02, 0x06, 0x01, 0x05, 0x03, 0x07
bcolor:	.byte	0x00, 0x40, 0x20, 0x60, 0x10, 0x50, 0x30, 0x70

////////
/
/ Global functions.
/
////////

////////
/
/ mminit - initialize screen
/
////////
mminit:
	movb	$0x63,%fs:MM_ESC(%ebp)		/ schedule keyboard initialization
	call	int11				/ read equipment status
	andl	$0x30,%eax			/ isolate video bits
	cmpl	$0x30,%eax			/ if not monochrome
	je	?mminit1
	movl	$COLOR,%fs:MM_PORT(%ebp)	/ set color port
	movl	$[SEG_VIDEOb|DPL_1],%fs:MM_BASE(%ebp)	/ set color base
	movw	%fs:MM_BASE(%ebp),%es
?mminit1:
	movl	%fs:MM_PORT(%ebp),%edx		/ turn video off
	addl	$4,%edx
	movb	$0x21,%al
	outb	(%dx)

	movl	%fs:MM_PORT(%ebp),%edx		/ zero display offset
	movb	$12,%al
	outb	(%dx)
	incl	%edx
	subb	%al,%al
	outb	(%dx)
	decl	%edx
	movb	$13,%al
	outb	(%dx)
	incl	%edx
	subb	%al,%al
	outb	(%dx)

	movl	%fs:MM_PORT(%ebp),%edx		/ reset border to black
	addl	$5,%edx
	subb	%al,%al
	outb	(%dx)

	incl	%edx				/ reset TECMAR XMSR register
	outb	(%dx)

	movl	$0,%fs:MM_INVIS(%ebp)
	movb	$DEFATTR,ATTR
	movb	ATTR,%fs:MM_ATTR(%ebp)
	movb	ATTR,%fs:MM_OATTR(%ebp)
	movb	$1,%fs:MM_WRAP(%ebp)
	movb	%fs:MM_IBROW(%ebp),ROW
	movb	ROW,%fs:MM_BROW(%ebp)
	movb	%fs:MM_IEROW(%ebp),%bl
	movb	%bl,%fs:MM_EROW(%ebp)
	subl	%ebx,%ebx
	jmp	mm_ff

////////
/
/ mmgo( iop )
/ IO *iop;
/
////////

	.set	FRAME,32			/ ra, bx, si, di, ds, es, fs, bp

mmgo:
	push	%ebx
	push	%esi
	push	%edi
	push	%ds
	push	%es
	push	%fs
	push	%ebp
	movl	%esp,%ebp

	push	%ds				/ copy ds to fs
	pop	%fs

	cld
	movl	FRAME+0(%ebp),%ebx		/ iop
	movl	IO_BASE(%ebx),%esi		/ iop->io_base
	movl	IO_IOC(%ebx),%ecx		/ iop->io_ioc

	cmpl	$IOSYS,IO_SEG(%ebx)		/ user address space
	je	?mmgo1
	movl	$SEG_386_UD,%eax
	movw	%ax,%ds
?mmgo1:
	movl	$mmdata,%ebp
	movl	%fs:MM_PORT(%ebp),%edx		/ turn video off if color board
	cmpl	$MONO,%edx
	je	?mmgo4
	cmpb	$0,%fs:MM_SLOW(%ebp)		/ check for slow [flicker-free]
	je	?mmgo3

	movl	$0x3DA,%edx
?mmgo2:	inb	(%dx)				/ wait for vertical retrace
	testb	$8,%al
	je	?mmgo2
?mmgo3:
	movl	$0x3D8,%edx			/ disable video
	movb	$0x25,%al
	outb	(%dx)
?mmgo4:
	movb	%fs:MM_ROW(%ebp),ROW
	movb	%fs:MM_COL(%ebp),COL
	movw	%fs:MM_BASE(%ebp),%es
	movl	%fs:MM_POS(%ebp),POS
	subl	%ebx,%ebx
	movb	%fs:MM_ATTR(%ebp),ATTR
	ijmp	%fs:MM_FUNC(%ebp)

exit:	pop	%ebx
	movl	%ebx,%fs:MM_FUNC(%ebp)
	movb	ATTR,%fs:MM_ATTR(%ebp)
	movb	ROW,%fs:MM_ROW(%ebp)		/ save row,column
	movb	COL,%fs:MM_COL(%ebp)
	movl	POS,%fs:MM_POS(%ebp)		/ save position

	movl	%fs:MM_PORT(%ebp),%edx		/ adjust cursor location
	movl	POS,%ebx
	orl	%fs:MM_INVIS(%ebp),%ebx
	shrl	$1,%ebx

	movb	$14,%al
	outb	(%dx)
	incl	%edx
	movb	%bh,%al
	outb	(%dx)
	decl	%edx
	movb	$15,%al
	outb	(%dx)
	incl	%edx
	movb	%bl,%al
	outb	(%dx)

	movl	%fs:MM_PORT(%ebp),%edx		/ turn video on
	addl	$4,%edx
	movb	$0x29,%al
	outb	(%dx)
	movl	$TIMEOUT,%fs:mmvcnt		/ TIMEOUT seconds before video disabled

	movl	FRAME+0(%esp),%ebx		/ iop
	movl	%ecx,%eax
	xchg	%ecx,%fs:IO_IOC(%ebx)
	subl	%fs:IO_IOC(%ebx),%ecx
	addl	%ecx,%fs:IO_BASE(%ebx)

	/ ra, bx, si, di, ds, es, fs, bp
	pop	%ebp
	pop	%fs
	pop	%es
	pop	%ds
	pop	%edi
	pop	%esi
	pop	%ebx
	ret

////////
/
/ mm_voff()	-- turn video display off
/
////////

mm_voff:
	movb	$0x21,%al
	jmp	mm_von1

////////
/
/ mm_von()	-- turn video display on
/
////////

mm_von:
	movl	$TIMEOUT,mmvcnt			/ TIMEOUT seconds before video disabled
	movb	$0x29,%al
mm_von1:
	movl	mmdata+MM_PORT,%edx		/ enable video display
	addl	$4,%edx
	outb	(%dx)
	ret

////////
/
/ Local functions.
/
////////

////////
/
/ alx10pbl: return AL * 10 + BL in AL.
/ The result is a byte quantity, overflow is ignored.
/
////////

alx10pbl:
	movb	%al,%bh
	shlb	$2,%al			/ n1 * 4
	addb	%bh,%al			/ n1 * 5
	shlb	$1,%al			/ n1 * 10
	addb	%bl,%al			/ n1 * 10 + digit
	subb	%bh,%bh			/ clear bh
	ret

////////
/
/ blankncol: Blank $NCOL characters at the current screen location.
/ blank: Blank ECX characters at the current screen location.
/ blanklines: Blank BL lines at the current screen location.
/ The blank characters use the original attribute,
/ i.e. they are not reverse video even if the state is reversed.
/
////////

blankncol:
	movl	$NCOL,%ecx
blank:
	push	%eax				/ save current attribute
	cmpb	$0,%fs:MM_EORIG(%ebp)
	je	?blank1
	movb	%fs:MM_OATTR(%ebp),ATTR		/ get original attribute
?blank1:
	movb	$' ',%al
	rep
	stosw
	pop	%eax				/ restore current attribute
	ret

blanklines:
	push	%ecx
?blank1:
	call	blankncol
	decb	%bl
	jg	?blank1
	pop	%ecx
	ret

////////
/
/ copyf: copy display forward.
/ copyb: copy display backward.
/ Arguments:
/	EDI	destination address
/	EBX	offset
/	ECX	byte count (assumed even)
/
////////

copyf:
	push	%ds
	push	%esi
	push	%edi
	movw	%fs:MM_BASE(%ebp),%ds
	movl	%edi,%esi
	addl	%ebx,%esi
copyf1:
	shrl	$1,%ecx			/ word count
	rep
	movsw
	cld
	subl	%ebx,%ebx
	pop	%edi
	pop	%esi
	pop	%ds
	ret

copyb:
	push	%ds
	push	%esi
	push	%edi
	movw	%fs:MM_BASE(%ebp),%ds
	movl	%edi,%esi
	subl	%ebx,%esi
	addl	%ecx,%edi
	addl	%ecx,%esi
	subl	$2,%edi
	subl	$2,%esi
	std
	jmp	copyf1

////////
/
/ getn1: get first parameter to BL.
/ If the parameter is 0, adjust it to 1.
/
////////

getn1:	movb	%fs:MM_N1(%ebp),%bl
	orb	%bl,%bl
	jne	?getn1a
	incb	%bl
?getn1a:
	ret

////////
/
/ getrow: get row number from first parameter to ROW.
/ Adjust parameter n to row n-1, except 0 means row 0.
/
////////

getrow:	movb	%fs:MM_N1(%ebp),ROW
	decb	ROW
	jge	?getrow1
	subb	ROW,ROW
?getrow1:
	ret

////////
/
/ repos - reposition cursor, i.e. recompute POS from ROW and COL
/
////////

repos:	movb	COL,%bl			/ reposition to ROW and COL
	lea	(,%ebx,2),POS		/ COL * 2
	movb	ROW,%bl
	addl	%cs:rowtab(,%ebx,4),POS	/ POS = 160 * ROW + 2 * COL
	jmp	eval

////////
/
/ Ewait - wait for next input char to evaluate
/
/ eval - evaluate input character
/
////////

ewait:
	call	exit
eval:
	jcxz	ewait			/ no more characters
	decl	%ecx			/ decrement count
	lodsb				/ char to AL
	cmpb	$0,%fs:MM_FONT(%ebp)	/ font 0?
	jne	?eval2
?eval1:	cmpb	$0x20,%al		/ font 0
	jae	mmputc			/ write characters 0x20-0xFF
	movb	%al,%bl
	ijmp	%cs:asctab(,%ebx,4)	/ process controls 0x00-0x1F

?eval2:	cmpb	$0x1B,%al		/ watch for ESC
	je	?eval1			/ process ESC regardless of font
	cmpb	$1,%fs:MM_FONT(%ebp)	/ font 1?
	je	mmputc			/ font 1, write all chars as is
	cmpb	$2,%fs:MM_FONT(%ebp)	/ font 2?
	jne	?eval1			/ not font [012], treat as 0
	xorb	$0x80,%al		/ font 2, toggle high bit
/	jmp	mmputc			/ and display

////////
/
/ mmputc - put character in al on screen
/
////////

mmputc:
	stosw				/ Update display memory.
	incb	COL
	cmpb	$NCOL,COL		/ Past end of line?
	jl	eval			/ Not past
	cmpb	$0,%fs:MM_WRAP(%ebp)	/ Yes past, Wrap around?
	jne	?nextline
	subl	$2,POS			/ No wrap, adjust back to end of line.
	decb	COL
	jmp	eval

?nextline:
	subb	COL,COL			/ Wrap to next line.
next1:	incb	ROW
	cmpb	%fs:MM_EROW(%ebp),ROW	/ Past scrolling region?
	jle	eval			/ Not past
	movb	%fs:MM_EROW(%ebp),ROW	/ Yes past, scroll up 1 line.
/	jmp	scrollup

////////
/
/ scrollup - scroll display upwards
/
////////

scrollup:
	push	%ecx
	movb	%fs:MM_BROW(%ebp),%bl
	movl	%cs:rowtab(,%ebx,4),%edi	/ destination
	movb	ROW,%bl
	movl	%cs:rowtab(,%ebx,4),%ecx	/ start of current row
	push	%ecx
	subl	%edi,%ecx		/ bytes to shift
	movl	$NHB,%ebx		/ offset
	call	copyf			/ shift
	pop	%edi
	call	blankncol		/ blank current row
	pop	%ecx
	movb	COL,%bl			/ reposition to ROW and COL
	lea	(,%ebx,2),POS		/ COL * 2
	movb	ROW,%bl
	addl	%cs:rowtab(,%ebx,4),POS
	jmp	ewait

////////
/
/ Esc state - entered when last char was ESC - transient state.
/
////////

mm_esc0:
	call	exit
mm_esc:	jcxz	mm_esc0
	decl	%ecx
	lodsb				/ character to AL
	movb	%al,%bl
	subb	$0x20,%bl
	cmpb	$0x80-0x20,%bl
	jae	mmputc
	ijmp	%cs:esctab(,%ebx,4)

////////
/
/ Csi state - entered when last two chars were ESC [
/
/ Get ANSI X3.64 parameter list arguments.
/ The arguments have the format
/	p1;p2;...pn
/ where each parameter is a possibly empty string of ASCII digits [0-9].
/ Store the arguments starting at MM_N1 and the arg count at MM_NARGS;
/ empty parameters have default value 0.
/ Parameters are assumed to fit into bytes, values above 255 will not work.
/ Return the next character after the parameter list in AL.
/ If more than MAXARGS parameters are given, the extras get eaten.
/
////////

csi:
	movl	$0,%fs:MM_N1(%ebp)	/ initialize args 1 to 4
	movl	$0,%fs:MM_N1+4(%ebp)	/ initialize args 5 to 8
	movl	$1,%fs:MM_NARGS(%ebp)	/ initialize arg count
	jmp	csi1

csi0:
	call	exit
csi1:
	jcxz	csi0			/ no more characters
	decl	%ecx			/ decrement count
	lodsb				/ char to AL
	cmpb	$';',%al
	je	csi3			/ semi means another parameter follows
	movb	%al,%bl
	subb	$'0',%bl
	cmpb	$9,%bl
	jna	csi2			/ digit
csival:
	movb	%al,%bl			/ nondigit
	shlb	$1,%bl
	jc	mmputc			/ write high-bit characters
	ijmp	%cs:csitab(,%ebx,2)	/ process others via csitab

csi2:					/ add digit in BL into current parameter
	push	%edi
	lea	MM_N1-1(%ebp),%edi	/ address of 0th parameter
	addl	%fs:MM_NARGS(%ebp),%edi	/ + arg count = address of current param
	movb	%fs:(%edi),%al		/ current parameter value to AL
	call	alx10pbl
	movb	%al,%fs:(%edi)		/ pn = (pn * 10) + digit
	pop	%edi
	jmp	csi1			/ look for more digits or params

csi3:					/ prepare for next parameter after semi
	cmpl	$MAXARGS,%fs:MM_NARGS(%ebp)
	jae	csi5			/ too many parameters, oops
	incl	%fs:MM_NARGS(%ebp)	/ bump parameter count
	jmp	csi1			/ and look for more digits or params

csi4:
	call	exit
csi5:					/ too many parameters, eat remaining
	jcxz	csi4			/ no more characters
	decl	%ecx			/ decrement count
	lodsb				/ char to AL
	cmpb	$';',%al
	je	csi5			/ semi means another parameter follows
	movb	%al,%bl
	subb	$'0',%bl
	cmpb	$9,%bl
	jna	csi5			/ digit
	jmp	csival			/ nondigit, process as above

////////
/
/ Csi_gt state - entered after input sequence ESC [ >
/	
////////

csi_gt0:
	call	exit
csi_gt:	jcxz	csi_gt0
	decl	%ecx
	lodsb
	movb	%al,%bl
	subb	$'0',%bl
	cmpb	$9,%bl
	ja	?csi_gt1
	movb	%fs:MM_N1(%ebp),%al
	call	alx10pbl
	movb	%al,%fs:MM_N1(%ebp)	/ n1 = (n1 * 10) + digit
	jmp	csi_gt

?csi_gt1:
	cmpb	$'h',%al
	je	mm_cgh
	cmpb	$'l',%al
	je	mm_cgl
	jmp	eval

////////
/
/ Csi_q state - entered after input sequence ESC [ ?
/	
////////

csi_q0:
	call	exit
csi_q:	jcxz	csi_q0
	decl	%ecx
	lodsb
	movb	%al,%bl
	subb	$'0',%bl
	cmpb	$9,%bl
	ja	?csi_q1
	movb	%fs:MM_N1(%ebp),%al
	call	alx10pbl
	movb	%al,%fs:MM_N1(%ebp)	/ n1 = (n1 * 10) + digit
	jmp	csi_q

?csi_q1:
	cmpb	$'h',%al
	je	mm_cqh
	cmpb	$'l',%al
	je	mm_cql
	jmp	eval

////////
/
/ mm_bel - schedule beep
/
////////

mm_bel:	movb	$-1,%fs:mmbeeps
	jmp	eval

////////
/
/ mm_bs - backspace
/
////////

mm_bs:	movb	$1,%bl
	jmp	mm_cub1

////////
/
/ mm_cbt - cursor backward tabulation
/
/	Moves the active position horizontally in the backward direction
/	to the preceding in a series of predetermined positions.
/
////////

mm_cbt:	call	getn1
	salb	$3,%bl			/ n1 * 8 = distance to move
	addb	$7,COL
	andb	$0xF8,COL		/ calculate next tab stop
	subb	%bl,COL			/ step back
	jge	?mm_cbt1
	subb	COL,COL			/ can't step past column 0
?mm_cbt1:
	jmp	repos

////////
/
/ mm_cgh - process 'ESC [ > N1 h' escape sequence
/
/	Recognized sequences:	ESC [ > 13 h	-- Set CRT saver enabled.
/
////////

mm_cgh:	movl	$1,%ebx
mm_cgh1:
	cmpb	$13,%fs:MM_N1(%ebp)
	jne	?mm_cgh2
	movl	%ebx,%fs:mmcrtsav
?mm_cgh2:
	jmp	eval

////////
/
/ mm_cgl - process 'ESC [ > N1 l' escape sequence
/
/	Recognized sequences:	ESC [ > 13 l	-- Reset CRT saver.
/
////////

mm_cgl:	subl	%ebx,%ebx
	jmp	mm_cgh1

////////
/
/ mm_cht - cursor horizontal tabulation
/
/	Advances the active position horizontally to the next or following
/	in a series of predetermined positions.
/
////////

mm_cht:	call	getn1
mm_cht1:
	salb	$3,%bl			/ n1 * 8 = distance to move
	push	%ecx
	subl	%ecx,%ecx
	movb	COL,%cl
	andb	$7,%cl
	subb	%cl,%bl			/ distance from current position
	movb	%bl,%cl
	addb	%cl,COL
	call	blank
	pop	%ecx
	cmpb	$NCOL,COL
	jb	eval
	subb	$NCOL,COL
	jmp	next1

////////
/
/ mm_cnl - cursor next line
/
/	Moves the active position to the first position of the next display line.
/	Scrolls the active display if necessary.
/
////////

mm_cnl:	subb	COL,COL
	call	getn1
	addb	%bl,ROW
	jmp	mm_ind1

////////
/
/ mm_cpl - cursor preceding line
/
/	Moves the active position to the first position of the preceding
/	display line.
/
////////

mm_cpl:	subb	COL,COL
	call	getn1
	subb	%bl,ROW
mm_cpl1:
	cmpb	%fs:MM_BROW(%ebp),ROW
	jge	repos
	movb	%fs:MM_BROW(%ebp),ROW
	movb	$1,%bl
	jmp	mm_il1

////////
/
/ mm_cqh - process 'ESC [ ? N1 h' escape sequence
/
/	Recognized sequences:	ESC [ ? 4 h	-- Set smooth scroll.
/				ESC [ ? 7 h	-- Set wraparound.
/				ESC [ ? 8 h	-- Erase in current foreground
/				ESC [ ? 25 h	-- Enable line 25.
/
////////

mm_cqh:
	cmpb	$25,%fs:MM_N1(%ebp)	/ enable line 25
	je	mm_cqh25
	movb	$1,%bl			/ flag 1 to BL
mm_cqh1:				/ flag in BL, 0 or 1
	cmpb	$4,%fs:MM_N1(%ebp)	/ Smooth scroll.
	jne	?mm_cqh2
	movb	%bl,%fs:MM_SLOW(%ebp)
?mm_cqh2:
	cmpb	$7,%fs:MM_N1(%ebp)	/ Wraparound.
	jne	?mm_cqh3
	movb	%bl,%fs:MM_WRAP(%ebp)
?mm_cqh3:
	cmpb	$8,%fs:MM_N1(%ebp)	/ original foreground erase
	jne	?mm_cqh4
	movb	%bl,%fs:MM_EORIG(%ebp)
?mm_cqh4:
	jmp	eval
mm_cqh25:
	movb	$NROW+1,%fs:MM_LROW(%ebp)	/ set row limit 25
	movb	$NROW,%fs:MM_IEROW(%ebp)	/ set initial end row 24
	cmpb	$NROW-1,%fs:MM_EROW(%ebp)
	jne	?mm_cqh25a			/ current end row not 23, leave it
	incb	%fs:MM_EROW(%ebp)		/ set end row to 24
?mm_cqh25a:
	jmp	eval

////////
/
/ mm_cql - process 'ESC [ ? N1 l' escape sequence
/
/	Recognized sequences:	ESC [ ? 4 l	-- Set jump scroll.
/				ESC [ ? 7 l	-- Reset wraparound.
/				ESC [ ? 8 l	-- Erase in original foreground
/				ESC [ ? 25 l	-- Disable line 25.
/
////////

mm_cql:
	cmpb	$25,%fs:MM_N1(%ebp)	/ disable line 25
	je	?mm_cql25
	subb	%bl,%bl			/ flag 0 to BL
	jmp	mm_cqh1			/ use ESC ? n h code with flag reset
?mm_cql25:
	movb	$NROW,%fs:MM_LROW(%ebp)		/ set row limit 24
	movb	$NROW-1,%fs:MM_IEROW(%ebp)	/ set initial end row 23
	cmpb	$NROW,%fs:MM_EROW(%ebp)
	jne	?mm_cql25a		/ current end row not 24, leave it
	decb	%fs:MM_EROW(%ebp)	/ set end row to 23
?mm_cql25a:
	cmpb	$NROW,ROW		/ check if on last line
	jne	eval
	decb	ROW
	jmp	repos			/ reposition cursor to previous row

////////
/
/ mm_cr - carriage return
/
/	Moves the active position to first position of current display line.
/
////////

mm_cr:	subb	COL,COL
	movb	ROW,%bl
	movl	%cs:rowtab(,%ebx,4),POS
	jmp	eval

////////
/
/ mm_cub - cursor backwards
/
////////

mm_cub:	call	getn1
mm_cub1:
	cmpb	COL,%bl
	ja	?mm_cub2		/ back up to preceding line
	subb	%bl,COL
	jmp	repos

?mm_cub2:
	subb	COL,%bl			/ adjust count for current line
	decb	%bl			/ to last column of preceding line
	movb	$NCOL-1,COL
	decb	ROW
	cmpb	%fs:MM_BROW(%ebp),ROW
	jge	mm_cub1			/ still in the screen, keep trying
	subb	COL,COL
	movb	%fs:MM_BROW(%ebp),ROW
	jmp	repos

////////
/
/ mm_cud - cursor down
/
/	Moves the active position downward without altering the
/	horizontal position.
/
////////

mm_cud:	call	getn1
	addb	%bl,ROW
	cmpb	%fs:MM_EROW(%ebp),ROW
	jna	?mm_cud1
	movb	%fs:MM_EROW(%ebp),ROW
?mm_cud1:
	jmp	repos			/ reposition cursor

////////
/
/ mm_cuf - cursor forward
/
/	Moves the active position in the forward direction.
/
////////

mm_cuf:	call	getn1
	addb	%bl,COL
?mm_cuf1:
	cmpb	$NCOL,COL
	jb	?mm_cuf2
	subb	$NCOL,COL
	incb	ROW
	cmpb	%fs:MM_EROW(%ebp),ROW
	jna	?mm_cuf1
	movb	%fs:MM_EROW(%ebp),ROW
	movb	$NCOL-1,COL
?mm_cuf2:
	jmp	repos

////////
/
/ mm_cup - cursor position
/
/	Moves the active position to the position specified by two parameters.
/	The first parameter (mm_n1) specifies the vertical position (%fs:MM_ROW(%ebp)).
/	The second parameter (mm_n2) specifies the horizontal position (%fs:MM_COL(%ebp)).
/	A parameter value of 0 or 1 for the first or second parameter
/	moves the active position to the first line or column in the
/	display respectively.
/
////////

mm_cup:	call	getrow
/	addb	%fs:MM_BROW(%ebp),ROW
	cmpb	%fs:MM_EROW(%ebp),ROW
	jna	mm_cup1
	movb	%fs:MM_EROW(%ebp),ROW
mm_cup1:
	movb	%fs:MM_N2(%ebp),COL
	jmp	mm_hpa1

////////
/
/ mm_cuu - cursor up
/
/	Moves the active position upward without altering the horizontal
/	position.
/
////////

mm_cuu:	call	getn1
	subb	%bl,ROW
	cmpb	%fs:MM_BROW(%ebp),ROW
	jge	?mm_cuu1
	movb	%fs:MM_BROW(%ebp),ROW
?mm_cuu1:
	jmp	repos			/ reposition cursor

////////
/
/ mm_dch - delete characters
/
////////

mm_dch:	call	getn1
	movb	%bl,%al			/ characters to delete
	push	%ecx
	movb	ROW,%bl			/ current row
	movl	%cs:rowtab+4(,%ebx,4),%ecx	/ start of next row
	subl	POS,%ecx		/ characters left in this row * 2
	movb	%al,%bl
	shll	$1,%ebx			/ characters to delete * 2
	subl	%ebx,%ecx		/ byte count to ECX
	jle	?dch2			/ tried to delete too many, ignore
	push	%ecx
	call	copyf
	pop	%ecx
	addl	%ecx,POS		/ destination to blank
	movzxb	%al,%ecx		/ chars to blank
	call	blank
?dch2:
	pop	%ecx
	jmp	repos			/ reposition cursor

////////
/
/ mm_dl - delete line
/
/	Removes the contents of the active line.
/	The contents of all following lines are shifted in a block
/	toward the active line.
/
////////

mm_dl:
	call	getn1
	push	%ecx
	movb	%bl,%al			/ lines to delete to AL
	movb	ROW,%bl
	jmp	mm_su1

////////
/
/ mm_dmi - disable manual input
/
/	Set flag preventing keyboard input, and causing cursor to vanish.
/
////////

mm_dmi:
	movl	$1,%fs:islock
	jmp	eval

////////
/
/ mm_ea - erase in area
/
/	Erase some or all of the characters in the currently active area
/	according to the parameter:
/		0 - erase from active position to end inclusive (default)
/		1 - erase from start to active position inclusive
/		2 - erase all of active area
/
////////

mm_ea:	movb	%fs:MM_N1(%ebp),%al
	cmpb	$0,%al
	jne	?mm_ea1
	movb	%fs:MM_EROW(%ebp),%bl
	jmp	mm_el0
?mm_ea1:
	cmpb	$1,%al
	jne	mm_ea2
	movb	%fs:MM_BROW(%ebp),%bl
	jmp	mm_el1
mm_ea2:
	subb	COL,COL
	movb	%fs:MM_BROW(%ebp),ROW
	movb	ROW,%bl
	movl	%cs:rowtab(,%ebx,4),POS
	movb	%fs:MM_EROW(%ebp),%bl
	subb	ROW,%bl
	incb	%bl			/ rows to erase
	jmp	mm_ff1

////////
/
/ mm_ech - erase characters
/
////////

mm_ech:	call	getn1
	movb	%bl,%al			/ characters to erase
	movb	ROW,%bl			/ current row
	push	%ecx
	movl	%cs:rowtab+4(,%ebx,4),%ecx	/ start of next row
	subl	POS,%ecx		/ characters left in this row * 2
	shrl	$1,%ecx			/ characters left in this row
	cmpb	%cl,%al
	ja	?ech2			/ tried to erase too many, ignore
	movzxb	%al,%ecx		/ chars to erase
	call	blank			/ blank n characters
?ech2:
	pop	%ecx
	jmp	repos			/ reposition cursor

////////
/
/ mm_ed - erase in display
/
/	Erase some or all of the characters in the display according to the
/	parameter
/		0 - erase from active position to end inclusive (default)
/		1 - erase from start to active position inclusive
/		2 - erase all of display
/
////////

mm_ed:	movb	%fs:MM_N1(%ebp),%al
	cmpb	$0,%al
	jne	?mm_ed1
	movb	%fs:MM_LROW(%ebp),%bl
	decb	%bl
	jmp	mm_el0
?mm_ed1:
	cmpb	$1,%al
	jne	mm_ff
	subb	%bl,%bl
	jmp	mm_el1

////////
/
/ mm_el - erase in line
/
/	Erase some or all of the characters in the line according to the
/	parameter:
/		0 - erase from active position to end inclusive (default)
/		1 - erase from start to active position inclusive
/		2 - erase entire line
/
////////

mm_el:	movb	%fs:MM_N1(%ebp),%al
	movb	ROW,%bl
	cmpb	$0,%al
	je	mm_el0
	cmpb	$1,%al
	je	mm_el1
	movl	%cs:rowtab(,%ebx,4),POS
	subb	COL,COL
	movb	$1,%bl
	jmp	mm_ff1

mm_el1:	push	%ecx
	movl	POS,%ecx
	movl	%cs:rowtab(,%ebx,4),POS
mm_el1a:
	subl	POS,%ecx
	jl	?mm_el1b
	shrl	$1,%ecx
	call	blank
?mm_el1b:
	pop	%ecx
	jmp	repos

mm_el0:	push	%ecx
	movl	%cs:rowtab+4(,%ebx,4),%ecx
	jmp	mm_el1a

////////
/
/ mm_emi - enable manual input
/
/	Clear flag preventing keyboard input.
/
////////

mm_emi:
	movl	$0,%fs:islock
	jmp	eval

////////
/
/ mm_ff - form feed, clear and home cursor
/
////////

mm_ff:	subb	COL,COL
	movb	%fs:MM_BROW(%ebp),ROW
	subl	POS,POS
	movb	%fs:MM_LROW(%ebp),%bl
mm_ff1:
	call	blanklines
	jmp	repos

////////
/
/ mm_cha - cursor horizontal absolute
/ mm_hpa - horizontal position absolute
/
/	Moves the active position within the active line to the position
/	specified by the parameter.  A parameter value of zero or one
/	moves the active position to the first position of the active line.
/
////////

mm_cha:
mm_hpa:	movb	%fs:MM_N1(%ebp),COL
mm_hpa1:
	decb	COL
	jge	mm_hpa2
	subb	COL,COL
mm_hpa2:
	cmpb	$NCOL,COL
	jb	?mm_hpa3
	movb	$NCOL-1,COL
?mm_hpa3:
	jmp	repos

////////
/
/ mm_hpr - horizontal position relative
/
/	Moves the active position forward the number of positions specified
/	by the parameter.  A parameter value of zero or one indicates a
/	single-position move.
/
////////

mm_hpr:	call	getn1
	addb	%bl,COL
	jmp	mm_hpa2

////////
/
/ mm_ht - horizontal tabulation
/
////////

mm_ht:	movb	$1,%bl
	jmp	mm_cht1

////////
/
/ mm_hvp - horizontal and vertical position
/
/	Moves the active position to the position specified by two parameters.
/	The first parameter specifies the vertical position (%fs:MM_ROW(%ebp)).
/	The second parameter specifies the horizontal position (%fs:MM_COL(%ebp)).
/	A parameter value of zero or one moves the active position to the
/	first line or column in the display.
/
////////

mm_hvp:	call	getrow
	cmpb	%fs:MM_LROW(%ebp),ROW
	jna	?mm_hvp1
	movb	%fs:MM_LROW(%ebp),ROW
?mm_hvp1:
	jmp	mm_cup1

////////
/
/ mm_ich - insert characters
/
////////

mm_ich:	call	getn1
	movb	%bl,%al			/ characters to insert
	push	%ecx
	movb	ROW,%bl			/ current row
	movl	%cs:rowtab+4(,%ebx,4),%ecx	/ start of next row
	subl	POS,%ecx		/ characters left in this row * 2
	movb	%al,%bl
	shll	$1,%ebx			/ characters to insert * 2
	subl	%ebx,%ecx		/ characters to shift
	jl	?ich2			/ tried to insert too many, ignore
	push	%edi
	addl	%ebx,%edi		/ destination
	call	copyb
	pop	%edi			/ restore current position
	movzxb	%al,%ecx		/ chars to blank
	call	blank
?ich2:
	pop	%ecx
	jmp	repos			/ reposition cursor

////////
/
/ mm_il - insert line
/
/	Insert a erased line at the active line by shifting the contents
/	of the active line and all following lines away from the active line.
/	The contents of the last line in the scrolling region are removed.
/
////////

mm_il:	call	getn1
mm_il1:
	push	%ecx
	movb	%bl,%al			/ lines to scroll to AL
	movb	ROW,%bl			/ beginning row to scroll in BL
	jmp	mm_sd1

////////
/
/ mm_ind - index
/ mm_lf - line feed
/ mm_nel - next line
/ mm_vt - vertical tab
/
/	Moves the active position to the next display line.
/	Scrolls the active display if necessary.
/
////////

mm_ind:
mm_lf:
mm_nel:
mm_vt:	incb	ROW
mm_ind1:
	cmpb	%fs:MM_EROW(%ebp),ROW
	jna	repos
	movb	%fs:MM_EROW(%ebp),ROW
	jmp	scrollup

////////
/
/ mm_new - save cursor position
/
////////

mm_new:	movb	COL,%fs:MM_SCOL(%ebp)
	movb	ROW,%fs:MM_SROW(%ebp)
	jmp	eval

////////
/
/ mm_old - restore old cursor position
/
////////

mm_old:	movb	%fs:MM_SCOL(%ebp),COL
	movb	%fs:MM_SROW(%ebp),ROW
	jmp	repos

////////
/
/ mm_ri - reverse index
/
/	Moves the active position to the same horizontal position on the
/	preceding line.  Scrolling occurs if above scrolling region.
/
////////

mm_ri:	decb	ROW
	jmp	mm_cpl1

////////
/
/ mm_scr - select cursor rendition
/
/	Invokes the cursor rendition specified by the parameter.
/
/	Recognized renditions are:	0 - cursor visible
/					1 - cursor invisible
////////

mm_scr:	decb	 %fs:MM_N1(%ebp)
	je	?mm_scr1
	jg	?mm_scr2
	movl	$0,%fs:MM_INVIS(%ebp)
	jmp	eval

?mm_scr1:
	movl	$-1,%fs:MM_INVIS(%ebp)
?mm_scr2:
	jmp	eval

////////
/
/ mm_sd - scroll down
/
////////

mm_sd:
	call	getn1
	push	%ecx
	movb	%bl,%al			/ lines to scroll to AL
	movb	%fs:MM_BROW(%ebp),%bl
mm_sd1:
	push	%ebx
	movb	%fs:MM_EROW(%ebp),%cl
	subb	%bl,%cl
	incb	%cl			/ lines in scrolling area
	addb	%al,%bl			/ destination line
	cmpb	%fs:MM_EROW(%ebp),%bl
	ja	mm_sd3			/ too many
	movl	%cs:rowtab(,%ebx,4),%edi	/ destination
	subb	%al,%cl			/ lines to move
	jle	mm_sd3			/ scrolled more than available
	movzxb	%cl,%ecx
	imull	$NHB,%ecx,%ecx		/ bytes to move
	movzxb	%al,%ebx
	imull	$NHB,%ebx,%ebx		/ offset
	call	copyb
	pop	%ebx			/ restore first row to blank to BL
mm_sd2:
	movl	%cs:rowtab(,%ebx,4),%edi
	movzxb	%al,%ecx
	imull	$NCOL,%ecx,%ecx		/ bytes to blank
	call	blank
	pop	%ecx
	jmp	repos			/ leaves cursor unchanged

mm_sd3:
	pop	%ebx
	pop	%ecx
	jmp	mm_ea2			/ erase active area

////////
/
/ mm_sgr - select graphic rendition
/
/	Invokes the graphic rendition specified by the parameter.
/	All following characters in the data stream are rendered
/	according to the parameters until the next occurrence of
/	SGR in the data stream.
/
/	Recognized renditions are:	0	reset all
/					1	high intensity
/					4	underline
/					5	slow blink
/					7	reverse video
/					8	concealed on
/					10	select primary font
/					11	select first alternate font
/					12	select second alternate font
/					30-37	foreground color
/					40-47 	background color
/					50-57	border color
/
/	This is the only escape sequence which takes a parameter sequence
/	of unspecified length.
/
////////

mm_sgr:	movb	%fs:MM_N1(%ebp),%al
	cmpb	$10,%al
	jge	?mm_sgr10
	cmpb	$0,%al			/ reset all = 0
	jne	?mm_sgr1
	movl	$0,%fs:islock		/ clear keyboard lock state
	movb	%fs:MM_OATTR(%ebp),ATTR	/ restore original attribute
	jmp	?mm_sgrnext

?mm_sgr1:
	cmpb	$1,%al			/ bold =  1
	jne	?mm_sgr4
	orb	$INTENSE,ATTR
	jmp	?mm_sgrnext

?mm_sgr4:
	cmpb	$4,%al			/ underline = 4
	jne	?mm_sgr5
	cmpl	$COLOR,%fs:MM_PORT(%ebp)	/ color card?
	je	?mm_sgr4c		/ yes
	andb	$BLINK|INTENSE,ATTR	/ retain BLINK and INTENSE
	orb	$UNDERLINE,ATTR
	jmp	?mm_sgrnext

?mm_sgr4c:
	orb	$CULATTR,ATTR
	jmp	?mm_sgrnext

?mm_sgr5:
	cmpb	$5,%al			/ blinking = 5
	jne	?mm_sgr7
	orb	$BLINK,ATTR
	jmp	?mm_sgrnext

?mm_sgr7:
	cmpb	$7,%al			/ reverse video = 7
	jne	?mm_sgr8
	movb	$0x70,%al		/ reversed attribute for mono
	cmpl	$COLOR,%fs:MM_PORT(%ebp)	/ color card?
	jne	?mm_sgr7b
	movb	%fs:MM_OATTR(%ebp),%al	/ yes, exchange foreground/background
	rolb	$4,%al
?mm_sgr7b:
	andb	$BLINK|INTENSE,ATTR	/ retain BLINK and INTENSE
	orb	%al,ATTR
	jmp	?mm_sgrnext

?mm_sgr8:
	cmpb	$8,%al			/ concealed on = 8
	jne	?mm_sgrnext		/ n1 = 9, ignore
	cmpl	$COLOR,%fs:MM_PORT(%ebp)	/ color card?
	jne	?mm_sgr9a

	andb	$0x70,ATTR		/ Yes,	Set foreground color
	movb	ATTR,%al		/	to background color.
	rorb	$4,%al
	orb	%al,ATTR
	jmp	?mm_sgrnext

?mm_sgr9a:
	andb	$0x80,ATTR		/ No, set attributes to non-display.
	jmp	?mm_sgrnext		/	retain blink attribute.

?mm_sgr10:
	subb	$10,%al			/ map 10-57 to 0-47
	cmpb	$2,%al			/ foreground color
	jg	?mm_sgr10a		/ >12
	movb	%al,%fs:MM_FONT(%ebp)	/ set font [012]
	jmp	?mm_sgrnext

?mm_sgr10a:
	cmpl	$COLOR,%fs:MM_PORT(%ebp)	/ color card?
	jne	?mm_sgrnext		/ no, ignore remaining options

?mm_sgr30:
	subb	$20,%al			/ map 30-57 to 0-27
	jl	?mm_sgrnext		/ <30, ignore
	cmpb	$7,%al			/ foreground color
	jg	?mm_sgr40
	movb	%al,%bl
	andb	$0xf8,ATTR		/ mask out old foreground
	orb	%cs:fcolor(%ebx),ATTR	/ and replace with new
	movb	%fs:MM_OATTR(%ebp),%al	/ get original attribute
	andb	$0xf8,%al		/ mask out old foreground
	orb	%cs:fcolor(%ebx),%al	/ and replace with new
	movb	%al,%fs:MM_OATTR(%ebp)	/ and store
	jmp	?mm_sgrnext
?mm_sgr40:
	subb	$10,%al			/ map 40-57 to 0-17
	jl	?mm_sgrnext		/ <40, ignore
	cmpb	$7,%al			/ background color
	jg	?mm_sgr50
	movb	%al,%bl
	andb	$0x8f,ATTR		/ mask out old background
	orb	%cs:bcolor(%ebx),ATTR	/ and replace with new
	movb	%fs:MM_OATTR(%ebp),%al	/ get original attribute
	andb	$0x8f,%al		/ mask out old background
	orb	%cs:bcolor(%ebx),%al	/ and replace with new
	movb	%al,%fs:MM_OATTR(%ebp)	/ and store
	jmp	?mm_sgrnext
?mm_sgr50:
	subb	$10,%al			/ map 50-57 to 0-7
	jl	?mm_sgrnext		/ <50, ignore
	cmpb	$7,%al			/ border color
	jg	?mm_sgrnext
	movb	%al,%bl
	movb	%cs:fcolor(%ebx),%al
	push	%edx
	movl	%fs:MM_PORT(%ebp),%edx
	addl	$5,%edx
	outb	(%dx)
	pop	%edx
/	jmp	?mm_sgrnext

/ Parameter n1 has been processed, check if more params were specified.
?mm_sgrnext:
	decl	%fs:MM_NARGS(%ebp)	/ adjust count
	je	eval			/ done
	push	%ds
	push	%es
	push	%esi
	push	%edi
	push	%ecx
	movw	%fs,%cx
	movw	%cx,%ds
	movw	%cx,%es
	movl	$MAXARGS-1,%ecx		/ count
	lea	MM_N1(%ebp),%edi	/ destination
	movl	%edi,%esi
	incl	%esi			/ source
	rep
	movsb				/ shift remaining args down 1
	pop	%ecx
	pop	%edi
	pop	%esi
	pop	%es
	pop	%ds
	jmp	mm_sgr			/ process next parameter

////////
/
/ mm_spc - schedule special keyboard function
/
////////

mm_spc:	movb	%al,%fs:MM_ESC(%ebp)
	jmp	eval

////////
/
/ mm_ssr - set scrolling region
/
////////

mm_ssr:	movb	%fs:MM_N1(%ebp),%al
	decb	%al
	jge	?mm_ssr1
	subb	%al,%al
?mm_ssr1:
	cmpb	%fs:MM_LROW(%ebp),%al
	ja	?mm_ssr3
	movb	%fs:MM_N2(%ebp),%bl
	decb	%bl
	jge	?mm_ssr2
	subb	%bl,%bl
?mm_ssr2:
	cmpb	%fs:MM_LROW(%ebp),%bl
	ja	?mm_ssr3
	cmpb	%bl,%al
	ja	?mm_ssr3
	movb	%al,%fs:MM_BROW(%ebp)
	movb	%bl,%fs:MM_EROW(%ebp)
	movb	%al,ROW
	subb	COL,COL
?mm_ssr3:
	jmp	repos

////////
/
/ mm_su - scroll up
/
////////

mm_su:
	call	getn1
	push	%ecx
	movb	%bl,%al			/ lines to scroll to AL
	movb	%fs:MM_BROW(%ebp),%bl
mm_su1:
	movl	%cs:rowtab(,%ebx,4),%edi	/ destination
	movb	%fs:MM_EROW(%ebp),%cl
	subb	%bl,%cl
	incb	%cl			/ lines in scrolling area
	subb	%al,%cl			/ lines to move
	jle	?mm_su2			/ scrolled more than available
	movzxb	%cl,%ecx
	imull	$NHB,%ecx,%ecx		/ bytes to move
	movzxb	%al,%ebx
	imull	$NHB,%ebx,%ebx		/ offset
	call	copyf
	movb	%fs:MM_EROW(%ebp),%bl
	incb	%bl
	subb	%al,%bl			/ first row to blank
	jmp	mm_sd2

?mm_su2:
	pop	%ecx
	jmp	mm_ea2			/ erase active area

////////
/
/ mm_vpa - vertical position absolute
/
/	Moves the active position to the line specified by the parameter
/	without changing the horizontal position.
/	A parameter value of 0 or 1 moves the active position vertically
/	to the first line.
/
////////

mm_vpa:	call	getrow
mm_vpa1:
	cmpb	%fs:MM_LROW(%ebp),ROW
	jna	?mm_vpa2
	movb	%fs:MM_LROW(%ebp),ROW
?mm_vpa2:
	jmp	repos

////////
/
/ mm_vpr - vertical position relative
/
/	Moves the active position downward the number of lines specified
/	by the parameter without changing the horizontal position.
/	A parameter value of zero or one moves the active position
/	one line downward.
/
////////

mm_vpr:	call	getn1
	addb	%bl,ROW
	jmp	mm_vpa1

/ end of mmas.s
