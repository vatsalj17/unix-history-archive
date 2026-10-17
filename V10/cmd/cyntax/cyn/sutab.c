/*
 *	Sydney C Compiler.
 *
 *	Copyright 1984, Bruce Ellis.
 *
 *	Unauthorised possesion, sale or use prohibited.
 */

#include	"defs.h"

/*
 *	Sethi/Ullman numbers for VAX11.
 */
#define	THATREG	4
#define	SOMEREG	2

uchar	sutab[]	=
{
	THATREG + SOMEREG,	/* xt_and */
	SOMEREG,	/* xt_andand */
	THATREG + SOMEREG,	/* xt_andeq */
	0,		/* xt_argident */
	0,		/* xt_argname */
	0,		/* xt_arrayof */
	SOMEREG,	/* xt_bar */
	SOMEREG,	/* xt_barbar */
	SOMEREG,	/* xt_bareq */
	0,		/* xt_basetype */
	0,		/* xt_bitfield */
	0,		/* xt_bits */
	0,		/* xt_block */
	SOMEREG / 2,	/* xt_caddr */
	6 * THATREG,	/* xt_call */
	0,		/* xt_cast */
	0,		/* xt_cchar */
	0,		/* xt_cdouble */
	0,		/* xt_cfloat */
	0,		/* xt_cint */
	0,		/* xt_class */
	0,		/* xt_clong */
	0,		/* xt_cnull */
	SOMEREG,	/* xt_colon */
	0,		/* xt_comma */
	0,		/* xt_cuint */
	0,		/* xt_culong */
	0,		/* xt_do */
	SOMEREG / 2,	/* xt_dot */
	0,		/* xt_enum */
	SOMEREG / 2,	/* xt_eq */
	SOMEREG,	/* xt_eqeq */
	0,		/* xt_expr */
	THATREG,/* xt_extf */
	6 * THATREG + SOMEREG,	/* xt_ftnreturning */
	THATREG + SOMEREG,	/* xt_grgreq */
	SOMEREG,	/* xt_grt */
	SOMEREG,	/* xt_grteq */
	THATREG + SOMEREG,	/* xt_grtgrt */
	0,		/* xt_if */
	THATREG,/* xt_inf */
	SOMEREG,	/* xt_less */
	SOMEREG,	/* xt_lesseq */
	SOMEREG,	/* xt_lessless */
	0,		/* xt_literal */
	SOMEREG,	/* xt_lslseq */
	0,		/* xt_maxof */
	0,		/* xt_memblist */
	0,		/* xt_minof */
	SOMEREG,	/* xt_minus */
	SOMEREG,	/* xt_minuseq */
	0,		/* xt_name */
	0,		/* xt_not */
	SOMEREG,	/* xt_noteq */
	0,		/* xt_null */
	0,		/* xt_obj */
	2 * THATREG + SOMEREG,	/* xt_percent */
	2 * THATREG + SOMEREG,	/* xt_pereq */
	SOMEREG,	/* xt_plus */
	SOMEREG,	/* xt_pluseq */
	THATREG,	/* xt_postdec */
	THATREG,	/* xt_postinc */
	SOMEREG,	/* xt_predec */
	SOMEREG,	/* xt_preinc */
	SOMEREG,	/* xt_ptrdiff */
	SOMEREG,	/* xt_ptrminus */
	SOMEREG,	/* xt_ptrminuseq */
	SOMEREG / 2,	/* xt_ptrplus */
	SOMEREG,	/* xt_ptrpluseq */
	THATREG,	/* xt_ptrpostdec */
	THATREG,	/* xt_ptrpostinc */
	SOMEREG,	/* xt_ptrpredec */
	SOMEREG,	/* xt_ptrpreinc */
	0,		/* xt_ptrto */
	SOMEREG,	/* xt_question */
	0,		/* xt_sizeof */
	THATREG,	/* xt_slash */
	THATREG,	/* xt_slasheq */
	0,		/* xt_sref */
	0,		/* xt_string */
	0,		/* xt_structof */
	0,		/* xt_svalue */
	SOMEREG,	/* xt_tilda */
	SOMEREG,	/* xt_times */
	SOMEREG,	/* xt_timeseq */
	SOMEREG / 2,	/* xt_uand */
	SOMEREG,	/* xt_uminus */
	0,		/* xt_unionof */
	SOMEREG,	/* xt_uparrow */
	SOMEREG,	/* xt_uparroweq */
	SOMEREG / 2,	/* xt_utimes */
	0,		/* xt_while */
};
