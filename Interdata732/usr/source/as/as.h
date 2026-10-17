/*
 * 		Assembler common data
 *
 *
 *	Copyright (C) 1978, Richard Miller
 */

/*
 * Lexical tokens - returned by token()
 */
#define	GARBAGE	0	/* unknown character */
#define	PLUS	1	/*	+	*/
#define	MINUS	2	/*	-	*/
#define	OR	3	/*	!	*/
#define	AND	4	/*	&	*/
#define	STAR	5	/*	*	*/
#define	SLASH	6	/*	/	*/
#define BINOP	6	/* last binary operator */
#define	EOL	7	/* end of line */
#define	SPACE	8	/* blanks and/or tabs */
#define	COMMA	9	/*	,	*/
#define	LPAREN	10	/*	(	*/
#define	RPAREN	11	/*	)	*/
#define	APAR	12	/*	A(	*/
#define	ZPAR	13	/*	Z(	*/
#define	CON	14	/* constant - value in conbuf */
#define	HCON	15	/* halfword constant - value in conbuf */
#define	STRING	16	/* C'...' - string & length in strbuf & strlen */
#define	SYMBOL	17	/* symbol - name in symbuf */

/*
 * Segment relocation bits -- used in a.out relocation sections
 */
#define	RUNDEF	(-1)	/* undefined -- internal only */
#define	RABS	00	/* absolute */
#define	RTEXT	02	/* text = PURE */
#define	RDATA	04	/* data = IMPUR */
#define	RBSS	06	/* bss */
#define	REXT	010	/* external reference --
				bits 15-4 contain symbol number */
#define	RSEG	017	/* Mask for segment */
#define	RHI	01	/* high-order halfword of a relocatable word  --
				next halfword in relocation bits contains
				  nonzero:	fullword relocatable address
				  zero:		3-byte relocatable address */

/*
 * Symbol types
 */
#define	SUNDEF	00	/* undefined */
#define	SABS	01	/* absolute */
#define	STEXT	02	/* text */
#define	SDATA	03	/* data */
#define	SBSS	04	/* bss */
#define	SCOMN	05	/* offset in common block -- internal only
				bits 31-8 contain symbol number of block name */
#define	SSEG	07	/* mask for segment */
#define	SEXT	040	/* external (global) symbol */

/*
 * Error types
 */
#define erra	'A'	/* previously defined absolute value required */
#define errb	'B'	/* short branch out of range */
#define errc	'C'	/* illegal or missing opcode */
#define errd	'D'	/* illegal data in COMN, STRUC or BSS */
#define erre	'E'	/* eof or END within DO, COMN, STRUC or IFx */
#define errg	'G'	/* garbage character */
#define errh	'H'	/* halfword value too large */
#define erri	'I'	/* improper nesting of IFx, COMN or STRUC */
#define errl	'L'	/* missing label */
#define errm	'M'	/* multiply defined symbol */
#define errn	'N'	/* number syntax */
#define erro	'O'	/* org to illegal address */
#define errp	'P'	/* symbol value changed between passes */
#define errq	'Q'	/* missing quote */
#define errr	'R'	/* relocation error */
#define errs	'S'	/* missing symbol */
#define erru	'U'	/* undefined symbol */
#define errv	'V'	/* illegal value (e.g. register >15) */
#define errx	'X'	/* syntax error */
#define errz	'Z'	/* division by zero */

/*
 * a.out file header
 */
EXTERN struct {
	int mword;	/* 'magic word' */
	int tsize;	/* size of text (rounded to word) */
	int dsize;	/*	"  data		"	*/
	int bsize;	/*	"  bss		"	*/
	int ssize;	/* size of symbol table */
	int entrypt;	/* entry point (NOT IMPLEMENTED) */
	int unused;
	int rflag;	/* relocation bits suppressed */
} hdr;

/*
 * Parser information
 */
EXTERN int	dcflag;		/* DC-type instruction - a() constants legal */

EXTERN int	nexttoken;	/* look-ahead token */
EXTERN int	conbuf;		/* returned value of last constant */
EXTERN char	strbuf[64];	/* returned value of last string */
EXTERN int	strlen;		/* returned length of last string */
EXTERN	char	symbuf[8];	/* returned name of last symbol */

/*
 * Expression evaluation
 */
EXTERN struct exp {
	int rel;		/* relocatability of expression */
	int val;		/* value of expression */
	int half;		/* 16-bit flag (only used in DC) */
} exp;

/*
 * Symbol table information
 */
struct symbol {		/* user-defined symbol */
	struct symbol *next;	/* chain to next symbol with same hash code
				   (must be first field -- see symlook()) */
	char name[8];		/* left-justified, zero-padded */
	int type;		/* relocatability type */
	int value;
};

struct optab {			/* opcode symbol */
	struct symbol *next;
	char name[8];	
	int opval;		/* value of opcode */
};

EXTERN struct symbol *cursym;		/* most recently looked-up symbol */
EXTERN struct symbol *curlab;		/* label of current line */
EXTERN struct symbol *usymtab;		/* first user-defined symbol entry */
EXTERN struct symbol *symtop;		/* end of allocated symbol storage */
EXTERN struct symbol *nextsym;		/* next available symbol entry */
EXTERN int	symseek;		/* seek address for symbol table */

/*
 * Segment information
 */
#define	OBSIZE	512	/* size of output buffers (must be even) */

EXTERN struct segment {
	int loc;		/* location counter */
	int maxloc;		/* highest address so far */
	int nchar;		/* number of chars in buffers */
	int tseek;		/* seek address for text buffer */
	int rseek;		/* seek address for relocation bits buffer */
	char tbuf[OBSIZE];	/* text buffer */
	char rbuf[OBSIZE];	/* relocation bits buffer */
} pure, impure;

EXTERN struct vsegment {	/* virtual segment - no buffers */
	int loc;		/* location counter */
	int maxloc;		/* highest address so far */
} bss, comm;

EXTERN struct segment *curseg;		/* current segment */
EXTERN int	currel;			/* current relocatability */
EXTERN struct symbol *curcomm;		/* current COMN or STRUC name */

/*
 * Conditional assembly information
 */
EXTERN int	noasm;		/* assembly turned off by simple IF */
EXTERN int	ifnest;		/* nesting level of compoiund IF */
EXTERN int	ifcount;	/* no. of ENDC's required to resume assembly */
EXTERN int	endswitch;	/* END statement encountered */

/*
 * Global information & flags
 */
EXTERN int	ofile;		/* fd of object file */
EXTERN int	ucase;		/* translate upper-case symbols to lower-case */
EXTERN int 	mflag;		/* multiple definitions of relocatable symbols
				   are permitted */
EXTERN int	lflag;		/* make listing to std output */
EXTERN int	opt;		/* SQUEZ optimization:		[arg]
					-1:	NORX3		-s1
					 0:	NOSQUEZ		-s0
					 1:	SQUEZ (default)	-s2	*/
EXTERN int	bdata;		/* Block Data program */
EXTERN int	line;		/* input line number */
EXTERN int	errcnt;		/* no. of errors */
EXTERN int	pass;		/* pass number */
EXTERN int	passg;		/* code-generation pass flag */
EXTERN int	passl;		/* listing pass flag */
EXTERN int	docount;	/* counter for DO loops */
