/*
 *	Sydney C Compiler.
 *
 *	Copyright 1984, Bruce Ellis.
 *
 *	Unauthorised possesion, sale or use prohibited.
 */

#include	"defs.h"
#include	"types.h"
#include	"xtypes.h"

xnode	*type_char;
xnode	*type_chstr;
xnode	*type_double;
xnode	*type_float;
xnode	*type_ftnretint;
xnode	*type_int;
xnode	*type_long;
xnode	*type_short;
xnode	*type_uchar;
xnode	*type_uint;
xnode	*type_ulong;
xnode	*type_ushort;
xnode	*type_void;

typedef struct
{
	xnode	**fundamental;
	long	type;
}
	fund_type;

/*
 *	These are craftily ordered to produce a balanced tree when
 *	successively find_typed.
 */
static fund_type	ftypes[] =
{
	/*6*/	{&type_uchar,	UNSIGNED | CHAR},

	/*3*/	{&type_short,	SHORT | INT},

	/*2*/	{&type_int,	INT},
	/*1*/	{&type_char,	CHAR},

	/*5*/	{&type_void,	VOID},
	/*4*/	{&type_float,	FLOAT},

	/*9*/	{&type_double,	LONG | FLOAT},

	/*7*/	{&type_uint,	UNSIGNED | INT},
	/*8*/	{&type_ushort,	UNSIGNED | SHORT | INT},

	/*10*/	{&type_long,	LONG | INT},
	/*11*/	{&type_ulong,	UNSIGNED | LONG | INT},
};

void
init_type()
{
	register fund_type	*p;
	register xnode		*x;

	for (p = ftypes; p < ftypes + nels(ftypes); p++)
	{
		x = new_xnode();
		x->x_what = xt_basetype;
		x->x_value.i = p->type;
		x->x_subtype = NULL;
		*p->fundamental = find_type(x);
	}

	x = new_xnode();
	x->x_what = xt_ftnreturning;
	x->x_subtype = type_int;
	type_ftnretint = find_type(x);

	x = new_xnode();
	x->x_what = xt_ptrto;
	x->x_subtype = type_char;
	type_chstr = find_type(x);
}
