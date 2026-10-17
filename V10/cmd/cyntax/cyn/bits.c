# 1 "tables.c"




# 1 "./defs.h"














# 17 "./defs.h"







typedef	char	*setptr;























typedef unsigned char	uchar;
typedef unsigned char	pack_t;






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




extern xdopes	xdope[];
extern uchar	sutab[];





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




typedef   char	chset[16];
typedef   char	xtset[12];


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
# 6 "tables.c"
# 1 "./cnodes.h"










typedef enum
{
	ct_args,
	ct_binopfsfx,
	ct_binopft,
	ct_binopfv,
	ct_block,
	ct_booltest,
	ct_boolval,
	ct_call,
	ct_callfv,
	ct_dcast,
	ct_clear,
	ct_colon,
	ct_discard,
	ct_fake,
	ct_ftnentry,
	ct_ftnxit,
	ct_hard,
	ct_jmp,
	ct_jmpf,
	ct_jmpt,
	ct_intrin,
	ct_intrinfv,
	ct_label,
	ct_ldiff,
	ct_nop,
	ct_not,
	ct_obj,
	ct_objfv,
	ct_objref,
	ct_question,
	ct_ret,
	ct_switch,
	ct_sync,
	ct_ujmp,
	ct_unopfsfx,
	ct_unopfv,
	ct_vret,
	ct_xnd,
	ct_xndtest,

	ct_char,
	ct_double,
	ct_extern,
	ct_float,
	ct_int,
	ct_long,
	ct_short,
	ct_space,
	ct_ptr,
	ct_string,
}
	cnodes;

struct cnode
{
	


	cnode	*c_next;
	




	cnode	*c_last;
	



	data	c_value;
	



	union
	{
		cnodes	cu_what;
		cnode	*cu_label;
	}
		cu3;
	





	union
	{
		choice	*cu_choice;
		cnode	*cu_link;
		xnode	*cu_ids;
	}
		cu4;
	


	union
	{
		xnode	*cu_flow;
		cnode	*cu_switch;
	}
		cu5;
	



	short	c_short1;
	short	c_short2;
};


















































struct cfrag
{
	union
	{
		cnode	*c_cn;
		cfrag	*c_cf;
	}
		c_un;
	cnode	*c_tail;
};



typedef   char	cnset[7];


# 7 "tables.c"
# 1 "./objects.h"








typedef enum
{
	ot_arg,
	ot_auto,
	ot_extern,
	ot_static,
}
	objects;

struct object
{
	objects	obj_what;
	object	*obj_next;
	long	obj_size;
	char	*obj_name;
	short	obj_flags;
};
# 8 "tables.c"
# 1 "./stdobj.h"










typedef enum
{
	i_data,
	i_lib,
	i_src,
	i_string,
	i_type,
	i_var,
}
	obj_items;

typedef enum
{
	d_addr,
	d_bytes,
	d_end,
	d_istring,
	d_irstring,
	d_space,
	d_string,
	d_reloc,
	d_rstring,
}
	obj_datas;

typedef enum
{
	t_arrayof,
	t_basetype,
	t_bitfield,
	t_dimless,
	t_elaboration,
	t_enum,
	t_ftnreturning,
	t_ptrto,
	t_structof,
	t_unionof,

	t_types,
}
	obj_types;

typedef enum
{
	v_arglist,
	v_array_size,
	v_auto,
	v_block_static,
	v_call,
	v_formal,
	v_function,
	v_global,
	v_implicit_function,
	v_static,
	v_varargs,
}
	obj_vars;



# 9 "tables.c"
# 1 "./options.h"










typedef enum
{
	o_optimise,
	o_restricted,
	o_warnings,

	o_options,
}
	opt;

extern int	Options[];
# 10 "tables.c"
# 1 "./tokens.h"












typedef enum
{
	tk_and,		
	tk_andand,	
	tk_auto,	
	tk_bar,		
	tk_barbar,	
	tk_break,	
	tk_case,	
	tk_cchar,	
	tk_cdouble,	
	tk_cfloat,	
	tk_char,	
	tk_cint,	
	tk_clong,	
	tk_colon,	
	tk_comma,	
	tk_continue,	
	tk_default,	
	tk_do,		
	tk_dot,		
	tk_double,	
	tk_else,	
	tk_enum,	
	tk_eof,		
	tk_eq,		
	tk_eqeq,	
	tk_extern,	
	tk_float,	
	tk_for,		
	tk_goto,	
	tk_grt,		
	tk_grteq,	
	tk_grtgrt,	
	tk_if,		
	tk_int,		
	tk_less,	
	tk_lesseq,	
	tk_lessless,	
	tk_lsminus,	
	tk_long,	
	tk_maxof,	
	tk_minof,	
	tk_minus,	
	tk_minusgrt,	
	tk_minusminus,	
	tk_name,	
	tk_not,		
	tk_noteq,	
	tk_null,	
	tk_offcurly,	
	tk_offround,	
	tk_offsquare,	
	tk_oncurly,	
	tk_onround,	
	tk_onsquare,	
	tk_percent,	
	tk_plus,	
	tk_plusplus,	
	tk_question,	
	tk_register,	
	tk_return,	
	tk_semicolon,	
	tk_set,		
	tk_short,	
	tk_sizeof,	
	tk_slash,	
	tk_static,	
	tk_string,	
	tk_struct,	
	tk_switch,	
	tk_tilda,	
	tk_times,	
	tk_typedef,	
	tk_union,	
	tk_unsigned,	
	tk_uparrow,	
	tk_void,	
	tk_while,	
}
	tokens;

typedef   char	tkset[10];

struct token
{
	tokens	t_what;
	int	t_lineno;
	st_node	*t_filename;
	data	t_value;
	token	*t_next;
};

extern token	*tok_list;







# 11 "tables.c"

 xtset	yield_truth	=

	
	
	
	
	
	
{0x0, 0x0, 0x0, 0x40, 0x18, 0x3, 0x10, };
 tkset	local_decl_head_tokens =

	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
{0x4, 0x4, 0x28, 0x6, 0x42, 0x0, 0x0, 0x44, 0x8A, 0xB, };




 tkset	decl_head_tokens =

	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
{0x4, 0x4, 0x28, 0x6, 0x42, 0x10, 0x30, 0x44, 0xCA, 0xB, };

 xtset	agg_types =

	
	
	
{0x0, 0x0, 0x0, 0x10, 0x0, 0x0, 0x0, 0x0, 0x0, 0x80, 0x40, };

 tkset	dimension_anchors =

	
	
	
	
	
	
	
	
{0x0, 0x40, 0xC0, 0x0, 0x0, 0x0, 0x2D, 0x10, };

 tkset	field_anchors =

	
	
	
	
	
{0x0, 0x40, 0x40, 0x0, 0x0, 0x0, 0x9, 0x10, };

 tkset	enum_anchors =




{0x0, 0x40, 0x40, 0x0, 0x0, 0x0, 0x1, };

 xtset	assops	=

    
    
    
    
    
    
    
    
    
    
{0x4, 0x1, 0x0, 0x0, 0x4, 0x10, 0x2, 0x5, 0x0, 0x10, 0x8, 0x1, };

 tkset	expr_ops =

    
    
    
    
    
    
    
    
    
    
    
    
    
{0x9, 0x40, 0x80, 0x80, 0x10, 0x2, 0xC0, 0x2, 0x41, 0x4, };

 tkset	assop_starts =

    
    
    
    
    
    
    
    
    
    
{0x9, 0x0, 0x0, 0x80, 0x10, 0x2, 0xC0, 0x0, 0x41, 0x4, };

 tkset	enum_warn =

    
    
    
    
    
    
    
{0x0, 0x0, 0x0, 0x60, 0x18, 0x3, 0x10, };

 cnset	expressions	=

    
    
    
    
{0x0, 0x0, 0x0, 0xC0, 0x60, };


 tkset	base_anchors =

    
    
    
    
{0x0, 0x40, 0x40, 0x0, 0x0, 0x0, 0x1, 0x10, };

 tkset	init_anchors =

    
    
{0x0, 0x0, 0x40, 0x0, 0x0, 0x0, 0x0, 0x10, };

cnset	executables	=

    
    
    
    
    
{0x0, 0x0, 0x0, 0xC0, 0x70, };




cnset	mip_branches	=

    
    
    
{0x0, 0x0, 0xE, };




cnset	mip_dead_ends	=

    
    
    
    
{0x0, 0x0, 0x2, 0xC0, 0x10, };

 chset	alpha_nums	=    {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xFF, 0x3, 0xFE, 0xFF, 0xFF, 0x87, 0xFE, 0xFF, 0xFF, 0x7, };
 chset	uppers		= {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xFE, 0xFF, 0xFF, 0x7, };
 chset	octals		= {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xFF, };
 chset	decimals	= {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xFF, 0x3, };
 chset	hexadecimals	=   {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xFF, 0x3, 0x7E, 0x0, 0x0, 0x0, 0x7E, };
 chset	float_cont	=   {0x0, 0x0, 0x0, 0x0, 0x0, 0x40, 0x0, 0x0, 0x20, 0x0, 0x0, 0x0, 0x20, };
 chset	digits_and_signs =   {0x0, 0x0, 0x0, 0x0, 0x0, 0x28, 0xFF, 0x3, };


xtset	binary_ops =

	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
{0xC7, 0x1, 0xC0, 0x60, 0xBC, 0x17, 0x93, 0x87, 0xF, 0x1A, 0x8C, 0x1, };

xtset	unary_ops =

	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
{0x0, 0x60, 0x0, 0x8, 0x1, 0x0, 0x8, 0x78, 0xF0, 0x0, 0x32, 0x2, };

xtset	constants =

	
	
	
	
	
	
	
	
{0x0, 0x10, 0x35, 0x3, 0x0, 0x0, 0x0, 0x0, 0x0, 0x40, };

xtset	int_constants =

	
	
	
	
{0x0, 0x0, 0x14, 0x3, };

xtset	base_constants =

	
	
	
	
	
{0x0, 0x0, 0x15, 0x3, };

xtset	commutatives	=

	
	
	
	
	
{0x41, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2, 0x0, 0x0, 0x84, };

 tkset	statement_anchors =

    
    
    
    
{0x0, 0x0, 0x40, 0x0, 0x0, 0x0, 0x9, 0x10, };

 tkset	case_anchors	=

    
    
    
    
{0x0, 0x20, 0x0, 0x0, 0x0, 0x0, 0x9, 0x10, };

 tkset	compound_anchors =

    
    
    
{0x0, 0x0, 0x40, 0x0, 0x0, 0x0, 0x8, 0x10, };

 tkset	factor_heads =

    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
{0x81, 0x1B, 0x0, 0x0, 0x80, 0x3B, 0x10, 0x81, 0x64, };

