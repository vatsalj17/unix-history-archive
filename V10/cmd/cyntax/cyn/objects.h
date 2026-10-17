/*
 *	Sydney C Compiler.
 *
 *	Copyright 1984, Bruce Ellis.
 *
 *	Unauthorised possesion, sale or use prohibited.
 */

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
