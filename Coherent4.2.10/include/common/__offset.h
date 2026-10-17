/* (-lgl
 *	Coherent 386 release 4.2
 *	Copyright (c) 1982, 1993 by Mark Williams Company.
 *	All rights reserved. May not be copied without permission.
 *	For copying permission and licensing info, write licensing@mwc.com
 -lgl) */

#ifndef	__COMMON___OFFSET_H__
#define	__COMMON___OFFSET_H__

#include <common/__size.h>

/*
 * This internal header file is intended as the sole point of definition for
 * the internal macro definition "__offsetof ()", exactly equivalent to the
 * ISO C macro definition "offsetof ()", but given an internal name so that
 * header files may refer to this macro without exporting the user-level macro
 * unnecessarily.
 */

#define __offsetof(type,id)	((__size_t) & (((type *) 0)->id))

#endif	/* ! defined (__COMMON___OFFSET_H__) */
