/*
 *	Sydney C Compiler.
 *
 *	Copyright 1984, Bruce Ellis.
 *
 *	Unauthorised possesion, sale or use prohibited.
 */

#define	option(x)	(Options[(int)(x)])

typedef enum
{
	o_optimise,
	o_restricted,
	o_warnings,

	o_options,
}
	opt;

extern int	Options[];
