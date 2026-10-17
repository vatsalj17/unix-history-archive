#

/*
#define		INTERDATA
*/
#define		PDP11

/*
 *   getsym symbol descriptors.
 */
#define	NUL	0		/* null descriptor */
#define	LBL	1		/* label */
#define	CON	2		/* constant */
#define	SPA	3		/* spaces */
#define	DEL	4		/* delimiter */
#define	EOL	5		/* end of line */
#define	EOF	6		/* end of file */
#define	COM	7		/* comment */
#define	OPR	8		/* operator */
#define	STR	9		/* string */
#define	MCH	10		/* global match */

#define	EXP	20		/* for mactab only - will NEVER appear */
#define	SKP	21		/* no match symbol */

#define	LIT	22		/* literal */
#define	CHR	23		/* extra character */


/*
 *   pass1 action table descriptors
 */
#define	NOOP	0		/* no-op */
#define	ALBL	1		/* add label to ifield */
#define	DOPV	2		/* decode op & value */
#define	OLBL	3		/* out label to i_opr */
#define	OOPR	4		/* out opr */
#define	ODEL	5		/* out del ',' */
#define	OCON	6		/* out constant */
#define	OCHR	7		/* out character */
#define	OSTR	8		/* output string */
#define	ENDP	9		/* end pass */
#define	SELC	10		/* select options */
#define	EXPR	11		/* call expr parser */
#define	RETN	12		/* return from call */
#define	GOTO	13		/* fail return */
#define	OERR	14		/* error on scan */
#define	OREC	15		/* out i_rec (ok) */

/*
 *   symbol table mode descriptors.
 */
#define	REFR	0x0001
#define	DEFN	0x0002
#define	LITR	0x0008
#define	GLOB	0x0010

#define	REL	0x0100
#define	ABS	0x0200

#define	PMODE	0x1000
#define	RMODE	0x2000

#define	RSTOR	0x0000
#define	RABS	0x4000
#define	RREL	0x8000
#define	RLBL	0xc000


/*
 *   misc. descriptors.
 */
#ifdef	PDP11

#define	MWORD	(011223)
#define	WORDSIZ	16		/* word size of host in bits */

#endif


#ifdef	INTERDATA

#define	MWORD	(0371242)
#define	WORDSIZ	32

#endif

#define	ERR	-1		/* error or undef */
#define	MAXBUF	256		/* general usage buffer size */
#define	LCOUNT	8		/* number of locn counters */
#define	TRUE	1		/* some truth value */
#define	FALSE	0		/* zero => false */
#define	DEF	1		/* define symbol */
#define	LKP	2		/* lookup flag */
#define	SELFMT	0x01		/* select format */
#define	SELOPC	0x02		/* select opcode class */

#define	OP	0x0001		/* opcode bit */
#define	PS	0x0002		/* pseudo bit */

#define	OPTION(c)	flags[(c - 'a')]
#define	BITMASK(m)	(bitmask[(m)])

/*
 *   struct definition
 */
struct	st	{			/* symbol table */
	char	s_name[8];		/* symbol name */
	int	s_value;		/* actual value */
	int	s_mode;			/* symbol usage */
	struct	st	*s_next;	/* next in chain */
	};

struct	it	{			/* intermediate code */
	int	i_flags;		/* tag bits - op, pseudo */
	int	i_label;		/* label index to symtab */
	int	i_op;			/* opcode table entry */
	int	i_loc;			/* current loc cntr # */
	char	i_selc[4];		/* action selection */
	char	i_opr[32];		/* operand description */
	};

struct	lt	{			/* location counter table */
	int	l_value;		/* lcntr value */
	int	l_start;		/* start addr of code */
	int	l_limit;		/* limit at eo pass1 */
	int	*l_next;		/* next object word */
	int	*l_rel_f;		/* relocation factor */
	int	*l_reloc;		/* relocation info */
	int	*l_rel_n;		/* pointer to rel info */
	};

struct	fd	{			/* format descriptor */
	char	f_class;		/* instr class */
	char	f_len;			/* len of instr in bu's */
	int	f_desc[8];		/* descriptor type */
	int	f_value[8];		/* values */
	int	f_width[8];		/* widths of bits */
	};

struct	bt	{			/* i/o buffer */
	int	b_fdesc;		/* file desc */
	int	b_left;			/* nleft */
	char	*b_next;		/* next */
	char	b_buf[512];		/* char buffer */
	};

struct	ht	{			/* header record */
	int	h_literals;		/* # of literals */
	int	h_labels;		/* # pre-def labs */
	int	h_formats;		/* # of f desc */
	int	h_ops;			/* # of ops */
	struct	fd	dctype[5];
	int	h_pc_post;		/* pc incr */
	int	h_bu_len;		/* len of bu in bits */
	int	h_w_len;		/* len of wrd in bu */
	int	h_o_len;		/* opcode struct len */
	int	h_p_len;		/* len of parser table */
	int	h_p_start;		/* start of parser */
	int	h_page;			/* page len */
	int	h_ii;			/* selected ill. instr op */
	char	h_mac[32];		/* title */
	};

struct	od	{			/* opcode descriptors */
	char	o_name[8];		/* opcode symbolic */
	struct	{
		int	o_value;	/* real value */
		int	o_format;	/* format to use */
		}	o_code[1];
	};

struct	tbl	{			/* parser table */
	int	tb_sym;			/* symbol type */
	int	tb_mem;			/* member type */
	int 	tb_next;		/* next entry */
	int	tb_act;			/* action before next case */
	char	tb_arg[4];		/* args for action */
	};


#ifdef		PDP11

/*
 *   WARNING - - these definitions are very dirty
 *		 done this way to get MAC up on a
 *		 dumb C compiler.
 *		 Should be fixed pronto !!
 */


#define	ST	14
#define	IT	44
#define	LT	14
#define	FD	50
#define	BT	518
#define	HT	(56+5*FD)
#define	TBL	12
#define	INT	2

#endif


#ifdef		INTERDATA

#define	ST	(sizeof (struct st))
#define	IT	(sizeof (struct it))
#define	LT	(sizeof (struct lt))
#define	FD	(sizeof (struct fd))
#define	BT	(sizeof (struct bt))
#define	HT	(sizeof (struct ht))
#define	TBL	(sizeof (struct tbl))
#define	INT	(sizeof (int))

#endif
