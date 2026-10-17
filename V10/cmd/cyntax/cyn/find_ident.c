/*
 *	Sydney C Compiler.
 *
 *	Copyright 1984, Bruce Ellis.
 *
 *	Unauthorised possesion, sale or use prohibited.
 */

#include	"defs.h"
#include	"types.h"
#include	"parse.h"

/*
 *	Map identifier type to name for use in diagnostics.
 */
static char	*
ident_name(id)
ident	*id;
{
	switch ((int)(id->id_flags & (ID_NAMES | ID_TAG | ID_MEMBER)))
	{
	case IDIS_STRUCTNAME:
	    return "struct tag";

	case IDIS_UNIONNAME:
	    return "union tag";

	case IDIS_ENUMNAME:
	    return "enum tag";

	case IDIS_LABEL:
	    return "label";

	case IDIS_MOENUM:
	    return "member of an enum";

	case IDIS_TYPENAME:
	    return "type name";

	case IDIS_NAME:
	    return NULL;

	case IDIS_MOAGG:
	    internal("ident_name", "got moagg");

	default:
	    internal("ident_name", "unknown tag");
	    return 0;
	}
}

/*
 *	Return an identifier in the current context which matches (or conflicts
 *	with) that specified by init_flags and parent.
 *	WARNING: real differences between old C & ANSI C here. See K&R2, A11.1
 */
ident	*
find_ident(stp, init_flags, parent)
register st_node	*stp;
register long		init_flags;
register ident		*parent;
{
    register ident	*id;
    register int	f;

    /*
     *	Choose the class of identifier from the flags.
     */
    if ((init_flags & ID_NAMES) != 0)
	f = ID_NAMES;
    else if ((init_flags & ID_TAG) != 0)
	f = ID_TAG;
    else if ((init_flags & ID_MEMBER) != 0)
	f = ID_MEMBER;
    else
	internal("find_ident", "bad flags");
    /*
     *	Look for a match on the idlist.
     */
    for (id = stp->st_idlist; id != NULL; id = id->id_next)
    {
	if
	(
	    levnum == id->id_levnum
	    &&
	    (id->id_flags & f) != 0
	    &&
	    id->id_parent == parent
	)
	{
	    register char	*s;

	    /*
	     *	We have matched on level number, flags and parent.
	     */

	    /*
	     *	Exact match.
	     */
	    if ((id->id_flags & init_flags & f) != 0)
		return id;

	    /*
	     *	Undefined name being defined.
	     */
	    if ((id->id_flags & (IDIS_UNDEF | IDIS_NAME)) == (IDIS_UNDEF | IDIS_NAME))
	    {
		id->id_flags = init_flags;
		return id;
	    }

	    /*
	     *	Clash.
	     *  Only complain the first time.
	     */
	    if ((id->id_flags | init_flags) & IDIS_BAD)
		continue;
	    
	    Ea[0].m = id->id_name;
	    Ea[1].m = levnum > 0 ? " in this block" : "";

	    s = ident_name(id);

	    if (s == NULL)
	    {
		Ea[2].m = "";
		Ea[3].m = "";
	    }
	    else
	    {
		Ea[2].m = " as a ";
		Ea[3].m = s;
	    }

	    error("'%' already declared%%%");

	    init_flags |= IDIS_BAD;
	    break;
	    /* return NULL; */
	}
    }

    /*
     *	New definition.
     */
    id = talloc(ident);
    id->id_last = &stp->st_idlist;
    id->id_name = stp->st_name;
    id->id_flags = init_flags;
    id->id_parent = parent;
    id->id_token = NULL;
    id->id_type = NULL;
    id->id_memblist = NULL;
    id->id_object = NULL;
    id->id_levnum = levnum;
    id->id_index = 0;
    id->id_value.i = 0;

    if (levnum != 0)
    {
	id->id_levelp = levelp->lv_idents;
	levelp->lv_idents = id;
    }

    if ((id->id_next = stp->st_idlist) != NULL)
	stp->st_idlist->id_last = &id->id_next;

    stp->st_idlist = id;

    return id;
}

/*
 *	Return a visible identifier which matches (or conflicts
 *	with) that specified by init_flags and parent.
 */
ident	*
refind_ident(stp, init_flags, parent)
register st_node	*stp;
register long		init_flags;
register ident		*parent;
{
    register ident	*id;
    register int	f;

    /*
     *	Choose the class of identifier from the flags.
     */
    if ((init_flags & ID_NAMES) != 0)
	f = ID_NAMES;
    else if ((init_flags & ID_TAG) != 0)
	f = ID_TAG;
    else if ((init_flags & ID_MEMBER) != 0)
	f = ID_MEMBER;
    else
	internal("refind_ident", "bad flags");

    /*
     *	Look for a match on the idlist.
     */
    for (id = stp->st_idlist; id != NULL; id = id->id_next)
    {
	if
	(
	    (id->id_flags & f) != 0
	    &&
	    (
		(id->id_flags & IDIS_MOAGG) == 0
		||
		id->id_parent == parent
	    )
	)
	{
	    register char	*s;

	    /*
	     *	We have matched on flags and parent.
	     */

	    /*
	     *	Exact match.
	     */
	    if ((id->id_flags & init_flags & f) != 0)
		return id;

	    if ((id->id_flags | init_flags) & IDIS_BAD)
		continue;
	    
	    init_flags |= IDIS_BAD;
	    /*
	     *	The name is defined as something that conflicts.
	     */
	    Ea[0].m = id->id_name;
	    s = ident_name(id);

	    switch ((int)(init_flags & (ID_NAMES | ID_TAG)))
	    {
	    case IDIS_LABEL:
		if (s == NULL && (init_flags & IDIS_UNDEF) != 0)
		{
		    Ea[1].x = id->id_type;
		    error("implicit declaration of label '%' conflicts with declaration as #");
		    return NULL;
		}

		Ea[1].m = (init_flags & IDIS_UNDEF) == 0 ? " already" : "";
		Ea[2].m = " in this block";

		if (s == NULL)
		{
		    Ea[3].x = id->id_type;
		    error("'%'% defined% as #");
		    return NULL;
		}

		Ea[3].m = " as a ";
		Ea[4].m = s;

		break;

	    case ID_FACTOR:
	    case IDIS_NAME:
	    case IDIS_ENUMNAME:
	    case IDIS_STRUCTNAME:
	    case IDIS_UNIONNAME:
		Ea[1].m = "";
		Ea[2].m = levnum > 0 ? " in this block" : "";
		Ea[3].m = " as a ";
		Ea[4].m = s == NULL ? "unknown" : s;
		break;

	    default:
		internal("refind_ident", "bad id_flags");
	    }

	    error("'%'% defined%%%");

	    break;
	    /* return NULL; */
	}
    }

    /*
     *	Search for member failed.
     */
    if ((init_flags & IDIS_MOAGG) != 0)
	return NULL;

    /*
     *	Define factor as name.
     */
    if ((init_flags & ID_FACTOR) == ID_FACTOR)
	init_flags = IDIS_NAME;

    /*
     *	New definition.
     */
    id = talloc(ident);
    id->id_last = &stp->st_idlist;
    id->id_name = stp->st_name;
    id->id_flags = init_flags;
    id->id_parent = parent;
    id->id_token = NULL;
    id->id_type = NULL;
    id->id_memblist = NULL;
    id->id_object = NULL;
    id->id_levnum = levnum;
    id->id_index = 0;

    if ((init_flags & IDIS_LABEL) != 0)
    {
	id->id_levelp = label_thread;
	label_thread = id;
	id->id_value.c = NULL;
    }
    else
    {
	id->id_value.i = 0;

	if (levnum != 0)
	{
	    id->id_levelp = levelp->lv_idents;
	    levelp->lv_idents = id;
	}
    }

    if ((id->id_next = stp->st_idlist) != NULL)
	stp->st_idlist->id_last = &id->id_next;

    stp->st_idlist = id;
    return id;
}
