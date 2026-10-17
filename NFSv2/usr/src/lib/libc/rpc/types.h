/*      @(#)types.h 1.1 85/05/30 SMI      */

/*
 * Rpc additions to <sys/types.h>
 */

#define	bool_t	int
#define	enum_t	int
#define	FALSE	(0==1)
#define	TRUE	(1==1)
#define __dontcare__	-1

#ifndef KERNEL
#define mem_alloc(bsize)	malloc(bsize)
#define mem_free(ptr, bsize)	free(ptr)
#ifndef major		/* ouch! */
#include <sys/types.h>
#endif
#else
#define mem_alloc(bsize)	kmem_alloc(bsize)
#define mem_free(ptr, bsize)	kmem_free((ptr), (bsize))
#endif
