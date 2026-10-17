/* $Header: /ker/coh.386/RCS/alloc.c,v 2.5 93/10/29 00:54:53 nigel Exp Locker: nigel $ */
/* (lgl-
 *	The information contained herein is a trade secret of Mark Williams
 *	Company, and  is confidential information.  It is provided  under a
 *	license agreement,  and may be  copied or disclosed  only under the
 *	terms of  that agreement.  Any  reproduction or disclosure  of this
 *	material without the express written authorization of Mark Williams
 *	Company or persuant to the license agreement is unlawful.
 *
 *	COHERENT Version 2.3.37
 *	Copyright (c) 1982, 1983, 1984.
 *	An unpublished work by Mark Williams Company, Chicago.
 *	All rights reserved.
 -lgl) */
/*
 * $Log:	alloc.c,v $
 * Revision 2.5  93/10/29  00:54:53  nigel
 * R98 (aka 4.2 Beta) prior to removing System Global memory
 * 
 * Revision 2.4  93/08/19  03:26:19  nigel
 * Nigel's r83 (Stylistic cleanup)
 * 
 * Revision 2.2  93/07/26  14:28:19  nigel
 * Nigel's R80
 */

#include <common/ccompat.h>
#include <sys/cmn_err.h>
#include <sys/debug.h>
#include <sys/kmem.h>
#include <sys/errno.h>

#define	_KERNEL		1

#include <kernel/trace.h>
#include <sys/proc.h>

#include <kernel/alloc.h>


/*
 * Allocate `l' bytes of memory.
 */

#if	__USE_PROTO__
__VOID__ * kalloc (size_t size)
#else
__VOID__ *
kalloc (size)
size_t		size;
#endif
{
	size_t	      *	temp;

	if (size == 0) {
		cmn_err (CE_PANIC, "attempt to allocate 0 bytes of memory");
	}

	if ((temp = (size_t *) kmem_alloc (size + sizeof (size_t),
					   KM_NOSLEEP)) != NULL) {
		* temp = size + sizeof (size_t);
		return temp + 1;
	}

	SET_U_ERROR (ENOSPC, "no memory in kalloc ()");
	return NULL;
}


/*
 * Free memory.
 */

#if	__USE_PROTO__
void kfree (__VOID__ * ptr)
#else
void
kfree (ptr)
__VOID__      *	ptr;
#endif
{
	size_t	      *	temp = (size_t *) ptr - 1;

	kmem_free (temp, * temp);
}

