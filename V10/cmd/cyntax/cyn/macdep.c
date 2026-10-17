/*
 *	Sydney C Compiler.
 *
 *	Copyright 1984, Bruce Ellis.
 *
 *	Unauthorised possesion, sale or use prohibited.
 */
#include	"defs.h"
#include	"cnodes.h"
#include	"macdep.h"
#include	"types.h"

/*
 *	Round up location to next mutltiple of a
 */
long
roundup(offset, a)
long	offset;
{
	long t = offset%a;
	
	if(t>0)
		offset += a - t;
	return offset;
}

/*
 *	Size of a type.
 */
long
size_of_in_bits(type)
xnode	*type;
{
	switch (type->x_what)
	{
	case xt_basetype:
		switch ((int)type->x_value.i & (int)(VOID | LONG | SHORT | INT | CHAR | FLOAT))
		{
		case CHAR:
			return BITS_PER_CHAR;

		case INT|SHORT:
			return BITS_PER_SHORT;

		case INT:
			return BITS_PER_INT;

		case INT|LONG:
			return BITS_PER_LONG;

		case FLOAT:
			return BITS_PER_FLOAT;

		case FLOAT|LONG:
			return BITS_PER_DOUBLE;

		case VOID:
			error("sizeof type void");
			return BITS_PER_BYTE;
		
		default:
			internal("size_of_in_bits", "bad basic type");
		}

	case xt_enum:
		return BITS_PER_INT;

	case xt_ptrto:
		return BITS_PER_PTR;

	case xt_structof:
	case xt_unionof:
		if (type->x_value.d->id_memblist == NULL)
		{
			Ea[0].x = type;
			error("# has not been defined");
			return BITS_PER_BYTE;
		}

		return type->x_value.d->id_value.i;

	case xt_arrayof:
		if (type->x_value.i < 0)
			error("array size unknown");

		return type->x_value.i * size_of_in_bits(type->x_subtype);

	default:
		internal("size_of_in_bits", "bad type");
	}
	return 0;
}
