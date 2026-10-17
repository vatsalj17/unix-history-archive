
#define hocus_pocus(x,y) \
typedef set x	y

#include	"defs.h"
#include	"cnodes.h"
#include	"objects.h"
#include	"stdobj.h"
#include	"options.h"
#include	"tokens.h"

readonly xtset	yield_truth	=
[[
	xt_eqeq,
	xt_grt,
	xt_grteq,
	xt_less,
	xt_lesseq,
	xt_noteq,
]];
readonly tkset	local_decl_head_tokens =
[[
	tk_auto,
	tk_char,
	tk_double,
	tk_enum,
	tk_extern,
	tk_float,
	tk_int,
	tk_long,
	tk_register,
	tk_short,
	tk_static,
	tk_struct,
	tk_typedef,
	tk_union,
	tk_unsigned,
	tk_void,
]];

/*
 *	Head tokens for a declaration.  Caters for degenerates.
 */
readonly tkset	decl_head_tokens =
[[
	tk_auto,
	tk_char,
	tk_double,
	tk_enum,
	tk_extern,
	tk_float,
	tk_int,
	tk_long,
	tk_name,
	tk_onround,
	tk_onsquare,
	tk_register,
	tk_short,
	tk_static,
	tk_struct,
	tk_times,
	tk_typedef,
	tk_union,
	tk_unsigned,
	tk_void,
]];

readonly xtset	agg_types =
[[
	xt_structof,
	xt_unionof,
	xt_enum,
]];

readonly tkset	dimension_anchors =
[[
	tk_comma,
	tk_eof,
	tk_eq,
	tk_offcurly,
	tk_offsquare,
	tk_oncurly,
	tk_onsquare,
	tk_semicolon,
]];

readonly tkset	field_anchors =
[[
	tk_comma,
	tk_eof,
	tk_offcurly,
	tk_oncurly,
	tk_semicolon,
]];

readonly tkset	enum_anchors =
[[
tk_eof,
tk_offcurly,
tk_comma,
]];

readonly xtset	assops	=
[[
    xt_andeq,
    xt_bareq,
    xt_grgreq,
    xt_lslseq,
    xt_minuseq,
    xt_pereq,
    xt_pluseq,
    xt_slasheq,
    xt_timeseq,
    xt_uparroweq,
]];

readonly tkset	expr_ops =
[[
    tk_and,
    tk_bar,
    tk_comma,
    tk_eq,
    tk_grtgrt,
    tk_lessless,
    tk_minus,
    tk_percent,
    tk_plus,
    tk_question,
    tk_slash,
    tk_times,
    tk_uparrow,
]];

readonly tkset	assop_starts =
[[
    tk_and,
    tk_bar,
    tk_grtgrt,
    tk_lessless,
    tk_minus,
    tk_percent,
    tk_plus,
    tk_slash,
    tk_times,
    tk_uparrow,
]];

readonly tkset	enum_warn =
[[
    xt_eq,
    xt_eqeq,
    xt_grt,
    xt_grteq,
    xt_less,
    xt_lesseq,
    xt_noteq,
]];

readonly cnset	expressions	=
[[
    ct_ret,
    ct_switch,
    ct_xnd,
    ct_xndtest,
]];


readonly tkset	base_anchors =
[[
    tk_comma,
    tk_eof,
    tk_offcurly,
    tk_semicolon,
]];

readonly tkset	init_anchors =
[[
    tk_eof,
    tk_semicolon,
]];

cnset	executables	=
[[
    ct_ret,
    ct_switch,
    ct_xnd,
    ct_xndtest,
    ct_vret,
]];

/*
 *	Machine independant branch cnodes.
 */
cnset	mip_branches	=
[[
    ct_jmp,
    ct_jmpf,
    ct_jmpt,
]];

/*
 *	Machine independant dead ends.
 */
cnset	mip_dead_ends	=
[[
    ct_jmp,
    ct_ret,
    ct_switch,
    ct_vret,
]];

readonly chset	alpha_nums	= [['A'..'Z', 'a'..'z', '0'..'9', '_']];
readonly chset	uppers		= [['A'..'Z']];
readonly chset	octals		= [['0'..'7']];
readonly chset	decimals	= [['0'..'9']];
readonly chset	hexadecimals	= [['0'..'9', 'a'..'f', 'A'..'F']];
readonly chset	float_cont	= [['.', 'e', 'E']];
readonly chset	digits_and_signs = [['0'..'9', '+', '-']];


xtset	binary_ops =
[[
	xt_and,
	xt_andand,
	xt_andeq,
	xt_bar,
	xt_barbar,
	xt_bareq,
	xt_colon,
	xt_comma,
	xt_eq,
	xt_eqeq,
	xt_grgreq,
	xt_grt,
	xt_grteq,
	xt_grtgrt,
	xt_insf,
	xt_less,
	xt_lesseq,
	xt_lessless,
	xt_lslseq,
	xt_minus,
	xt_minuseq,
	xt_noteq,
	xt_percent,
	xt_pereq,
	xt_plus,
	xt_pluseq,
	xt_ptrdiff,
	xt_ptrminus,
	xt_ptrminuseq,
	xt_ptrplus,
	xt_ptrpluseq,
	xt_question,
	xt_slash,
	xt_slasheq,
	xt_times,
	xt_timeseq,
	xt_uparrow,
	xt_uparroweq,
]];

xtset	unary_ops =
[[
	xt_call,
	xt_cast,
	xt_dot,
	xt_extf,
	xt_not,
	xt_postdec,
	xt_postinc,
	xt_predec,
	xt_preinc,
	xt_ptrpostdec,
	xt_ptrpostinc,
	xt_ptrpredec,
	xt_ptrpreinc,
	xt_tilda,
	xt_uand,
	xt_uminus,
	xt_utimes
]];

xtset	constants =
[[
	xt_caddr,
	xt_cdouble,
	xt_cint,
	xt_clong,
	xt_cnull,
	xt_cuint,
	xt_culong,
	xt_string,
]];

xtset	int_constants =
[[
	xt_cint,
	xt_clong,
	xt_cuint,
	xt_culong,
]];

xtset	base_constants =
[[
	xt_cdouble,
	xt_cint,
	xt_clong,
	xt_cuint,
	xt_culong,
]];

xtset	commutatives	=
[[
	xt_and,
	xt_bar,
	xt_plus,
	xt_times,
	xt_uparrow,
]];

readonly tkset	statement_anchors =
[[
    tk_eof,
    tk_offcurly,
    tk_oncurly,
    tk_semicolon,
]];

readonly tkset	case_anchors	=
[[
    tk_colon,
    tk_semicolon,
    tk_oncurly,
    tk_offcurly,
]];

readonly tkset	compound_anchors =
[[
    tk_eof,
    tk_oncurly,
    tk_semicolon,
]];

readonly tkset	factor_heads =
[[
    tk_times,
    tk_and,
    tk_minus,
    tk_not,
    tk_tilda,
    tk_onround,
    tk_sizeof,
    tk_minof,
    tk_maxof,
    tk_plusplus,
    tk_minusminus,
    tk_name,
    tk_string,
    tk_cchar,
    tk_cdouble,
    tk_cfloat,
    tk_cint,
    tk_clong,
]];

