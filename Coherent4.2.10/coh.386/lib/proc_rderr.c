/* $Header: /ker/coh.386/RCS/ker_data.c,v 2.4 93/08/19 03:26:31 nigel Exp Locker: nigel $ */
/*
 * This file contains definitions for the functions which support the Coherent
 * internal binary-compatibility scheme. We select _SYSV3 to get some old
 * definitions like makedev () visible.
 *
 * $Log:	ker_data.c,v $
 * Revision 2.4  93/08/19  03:26:31  nigel
 * Nigel's r83 (Stylistic cleanup)
 * 
 * Revision 2.2  93/07/26  14:55:28  nigel
 * Nigel's R80
 */

#define	_SYSV3		1

#include <common/ccompat.h>
#include <sys/errno.h>

#define	_KERNEL	1

#include <sys/uproc.h>


/*
 * Simply deal with getting and setting the user error code.
 */

#if	__USE_PROTO__
int get_user_error (void)
#else
int
get_user_error ()
#endif
{
	return u.u_error;
}


#if	__USE_PROTO__
void set_user_error (int error)
#else
void
set_user_error (error)
int		error;
#endif
{
	u.u_error = error;
}

