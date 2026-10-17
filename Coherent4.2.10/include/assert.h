/* (-lgl
 *	Coherent 386 release 4.2
 *	Copyright (c) 1982, 1993 by Mark Williams Company.
 *	All rights reserved. May not be copied without permission.
 *	For copying permission and licensing info, write licensing@mwc.com
 -lgl) */

#ifndef	__ASSERT_H__
#define	__ASSERT_H__

#include <common/ccompat.h>

__EXTERN_C_BEGIN__

__NO_RETURN__	__assertfail	__PROTO ((__CONST__ char * _test,
					  __CONST__ char * _file, int _line))
				__NO_RETURN_ATTR__;

__EXTERN_C_END__

#endif	/* ! defined (__ASSERT_H__) */

#undef	assert

#if	NDEBUG
# define	assert(p)	((void) 0)
#elif _HAVE_STRINGIZE
# define	assert(p) ((p) ? (void) 0 : \
			   __assertfail (# p, __FILE__, __LINE__))
#else
# define	assert(p) ((p) ? (void) 0 : \
			   __assertfail ("p", __FILE__, __LINE__))
#endif
