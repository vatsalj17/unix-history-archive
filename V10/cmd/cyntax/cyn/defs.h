/*
 *	Sydney C Compiler.
 *
 *	Copyright 1984, Bruce Ellis.
 *
 *	Unauthorised possesion, sale or use prohibited.
 */

#define STD_OBJ

/*
 *	Set manipulation macros
 */

#ifndef hocus_pocus
# define hocus_pocus(x,y) typedef char y[] 	/* sets.c gets  a different defn */
#endif

#define	SETS_SHIFT	3
#define	SETS_MASK	07

#define	in(s, c)	(((s)[((int)c) >> SETS_SHIFT]) & (1 << (((int)c) & SETS_MASK)))

typedef	char	*setptr;

#define	SYSERROR	(-1)

#define	NULL		0
#define	NODE_GRABZ	64
#define	BUFFZ		4096
#define	OUTZ		4096
#define	SBUFFZ		32
#define	PUSHBACKZ	10
#define	EOF_CHAR	'\0'
#define	STDIN		0
#define	STDOUT		1
#define	NO_FID		(-1)

#define	loop		for (;;)
#define	readonly
#define	nels(x)		(sizeof x / sizeof x[0])

#define ctack(c, d)	((c)->c_tail->c_next = (d)->c_head, (c)->c_tail = (d)->c_tail)
#define cadd(c, w)	((c)->c_next = new_cnode(), (c) = (c)->c_next, (c)->c_what = (w))

#define	pack(x)		((pack_t)x)

typedef unsigned char	uchar;
typedef unsigned char	pack_t;


/*
 *	Note. Any modification to this enum should be matched by an equivalent
 *	entry in "sutab.c" and a string in "debug.c"
 */
typedef enum
{
	xt_and,
	xt_andand,
	xt_andeq,
	xt_argident,
	xt_argname,
	xt_arrayof,
	xt_bar,
	xt_barbar,
	xt_bareq,
	xt_basetype,
	xt_bitfield,
	xt_block,
	xt_caddr,
	xt_call,
	xt_cast,
	xt_cchar,
	xt_cdouble,
	xt_cfloat,
	xt_cint,
	xt_class,
	xt_clong,
	xt_cnull,
	xt_colon,
	xt_comma,
	xt_cuint,
	xt_culong,
	xt_do,
	xt_dot,
	xt_enum,
	xt_eq,
	xt_eqeq,
	xt_expr,
	xt_extf,
	xt_ftnreturning,
	xt_grgreq,
	xt_grt,
	xt_grteq,
	xt_grtgrt,
	xt_if,
	xt_insf,
	xt_less,
	xt_lesseq,
	xt_lessless,
	xt_literal,
	xt_lslseq,
	xt_maxof,
	xt_memblist,
	xt_minof,
	xt_minus,
	xt_minuseq,
	xt_name,
	xt_not,
	xt_noteq,
	xt_null,
	xt_obj,
	xt_percent,
	xt_pereq,
	xt_plus,
	xt_pluseq,
	xt_postdec,
	xt_postinc,
	xt_predec,
	xt_preinc,
	xt_ptrdiff,
	xt_ptrminus,
	xt_ptrminuseq,
	xt_ptrplus,
	xt_ptrpluseq,
	xt_ptrpostdec,
	xt_ptrpostinc,
	xt_ptrpredec,
	xt_ptrpreinc,
	xt_ptrto,
	xt_question,
	xt_sizeof,
	xt_slash,
	xt_slasheq,
	xt_sref,
	xt_string,
	xt_structof,
	xt_svalue,
	xt_tilda,
	xt_times,
	xt_timeseq,
	xt_uand,
	xt_uminus,
	xt_unionof,
	xt_uparrow,
	xt_uparroweq,
	xt_utimes,
	xt_while,
}
	xnodes;

#define	SAMESU	1
#define	MAXSU	255

typedef enum
{
	su_arrayof,
	su_basetype,
	su_bitfield,
	su_enum,
	su_ftnreturning,
	su_ptrto,
	su_structof,
	su_unionof,
	su_types,

	su_null,

	su_and,
	su_bar,
	su_plus,
	su_times,
	su_uparrow,
}
	suequivs;

typedef struct
{
	char	*xd_name;
	pack_t	xd_suequiv;
}
	xdopes;

#define	xunpack(t)	((xnodes)(t))
#define	sunpack(t)	((suequivs)(t))

extern xdopes	xdope[];
extern uchar	sutab[];

/*
 *	Any mod to this enum should be matched by modification of
 *	'where_name' in 'readonly.c'.
 */
typedef enum
{
	at_outer,
	in_arguments,
	in_block,
	in_cast,
	in_enum,
	in_struct,
	in_union,
}
	places;

/*
 *	Forward declarations of general purpose data types.
 */
hocus_pocus(char,chset);
hocus_pocus(xnodes,xtset);


typedef struct cfrag	cfrag;
typedef struct choice	choice;
typedef struct cnode	cnode;
typedef struct decl	decl;
typedef struct ident	ident;
typedef struct level	level;
typedef struct object	object;
typedef struct operand	operand;
typedef struct optinfo	optinfo;
typedef struct st_node	st_node;
typedef struct strdata	strdata;
typedef struct tkdope	tkdope;
typedef struct token	token;
typedef struct xnode	xnode;
typedef union data	data;

struct strdata
{
	char	*str_value;
	int	str_size;
};

union data
{
	cfrag	*k;
	cnode	*c;
	double	f;
	ident	*d;
	long	i;
	object	*obj;
	st_node	*n;
	token	*t;
	xnode	*x;
	struct
	{
		long	from;
		long	to;
	}
		r;
	struct
	{
		st_node	*no_name;
		long	no_offset;
	}
		no;
	strdata	s;
};

struct xnode
{
	union
	{
		xnode	*xu_xnd;
		ident	*xu_id;
	}
		x_u0;

	union
	{
		struct
		{
			xnode	*x_x0;
			xnode	*x_x1;
		}
			x_s0;

		long	x_l;
	}
		x_u1;

	xnodes	x_what;
	data	x_value;

	union
	{
		struct
		{
			uchar	x_c0;
			uchar	x_c1;
		}
			x_s1;

		short	x_s;
	}
		x_u2;
};

#define	x_left		x_u0.xu_xnd
#define	x_subtype	x_u0.xu_xnd
#define	x_var		x_u0.xu_id

#define	x_right		x_u1.x_s0.x_x0
#define	x_type		x_u1.x_s0.x_x1
#define	x_index		x_u1.x_l
#define	x_stroff	x_u1.x_l

#define	x_flags		x_u2.x_s
#define	x_su		x_u2.x_s1.x_c1

#define	XIS_HARDASSOP	0x0008

#define	XIS_LVAL	0x0010
#define	XWAS_LVAL	0x0020
#define	XIS_LTRAV	0x0040
#define	XIS_RTRAV	0x0080

#define	XIS_IMPLICIT	0x0100

struct ident
{
	ident	*id_next;
	ident	**id_last;
	char	*id_name;
	xnode	*id_type;
	token	*id_token;
	data	id_value;
	ident	*id_parent;
	ident	*id_levelp;
	object	*id_object;
	long	id_flags;
	ident	*id_memblist;
	int	id_levnum;
	long	id_index;
};

#define	ID_NAMES	(IDIS_NAME | IDIS_LABEL | IDIS_MOENUM | IDIS_TYPENAME)
#define	ID_FACTOR	(IDIS_NAME | IDIS_MOENUM)
#define	ID_TAG		(IDIS_STRUCTNAME | IDIS_UNIONNAME | IDIS_ENUMNAME)
#define	ID_MEMBER	(IDIS_MOAGG)

#define	IDIS_NAME		0x000001L
#define	IDIS_STRUCTNAME		0x000002L
#define	IDIS_UNIONNAME		0x000004L
#define	IDIS_ENUMNAME		0x000008L
#define	IDIS_FAKE		0x000010L
#define	IDIS_LABEL		0x000020L
#define	IDIS_MOAGG		0x000040L
#define	IDIS_MOENUM		0x000080L
#define	IDIS_TYPENAME		0x000100L
#define	IDIS_BAD		0x000200L	/* clash message was done */

#define	IDIS_ARGUMENT		0x002000L

#define	IDIS_INIT		0x004000L	/* function: had the body */
#define	IDIS_ALLOC		0x008000L	/* optimiser generated */
#define	IDIS_UNDEF		0x010000L	/*
						 *	undefined aggragate or
						 *	identifier that has been
						 *	diagnosed as undefined
						 */

#define	IDIS_UANDED		0x100000L	/* someone used '&' on it */
						/* if function --- */
#define	IDIS_INTRIN		0x200000L	/* is an instrinic */

struct st_node
{
	st_node	*st_left;
	st_node	*st_right;
	char	*st_name;
	long	st_index;
	ident	*st_idlist;
};

struct	tkdope
{
	uchar	equiv_xt;
	char	prec_lev;
};

struct	level
{
	ident	*lv_idents;
};

struct choice
{
	cnode	*ch_def;
	cnode	*ch_case;
};

typedef union
{
	char	*m;
	double	*f;
	long	i;
	token	*t;
	xnode	*x;
}
	formarg;


extern char	*alloc_end;
extern char	*alloc_ptr;
extern cfrag	*kfree;
extern cfrag	*ktmp;
extern cnode	*cnfree;
extern cnode	*ctmp;
extern token	*tfree;
extern token	*ttmp;
extern xnode	*xfree;
extern xnode	*xtmp;

extern int	cstr_hiwater;

extern char	*alloc_fill();
extern char	*alloc_vector();
extern char	*chunk();
extern char	*cstr_alloc();
extern char	*rechunk();
extern cfrag	*kreplenish();
extern cnode	*creplenish();
extern token	*treplenish();
extern void	free_chunk();
extern xnode	*xreplenish();

#define	alloc(n)	((alloc_ptr += (n)) > alloc_end ? alloc_fill(n) : (alloc_ptr - (n)))
#define	talloc(x)	(x *)alloc(sizeof(x))
#define	vector(p, n, t)	(t *)alloc_vector((char *)p, (n) * sizeof (t))

#define	new_cnode()	(cnfree = (ctmp = cnfree == NULL ? creplenish() : cnfree)->c_next, ctmp)
#define	free_cnode(c)	(ctmp = cnfree, (cnfree = c)->c_next = ctmp)

#define	new_cfrag()	(kfree = (ktmp = kfree == NULL ? kreplenish() : kfree)->k_next, ktmp)
#define	free_cfrag(k)	(ktmp = kfree, (kfree = k)->k_next = ktmp)

#define	new_token()	(tfree = (ttmp = tfree == NULL ? treplenish() : tfree)->t_next, ttmp)
#define	free_token(p)	(ttmp = tfree, (tfree = p)->t_next = ttmp)

#define	new_xnode()	(xfree = (xtmp = xfree == NULL ? xreplenish() : xfree)->x_left, xtmp)
#define	free_xnode(x)	(xtmp = xfree, (xfree = x)->x_left = xtmp)

extern formarg	Ea[];
extern int	errored;
extern int	global_lineno;
extern st_node	*global_filename;

extern char	*str_alloc();
extern cfrag	*statement();
extern cnode	**flatten();
extern double	atof();
extern ident	*declaration();
extern ident	*find_ident();
extern ident	*is_typedefed();
extern ident	*refind_ident();
extern int	get_fill();
extern long	size_of_in_bits();
extern st_node	*find_str();
extern token	*tok_fill();
extern xnode	*bexpr();
extern xnode	*block();
extern xnode	*cast();
extern xnode	*decl_cast();
extern xnode	*decl_item();
extern xnode	*expr();
extern xnode	*extract_field();
extern xnode	*factor();
extern xnode	*find_type();
extern xnode	*get_shared_type();
extern xnode	*traverse();
