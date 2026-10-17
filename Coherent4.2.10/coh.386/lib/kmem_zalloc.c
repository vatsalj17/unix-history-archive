/* $Header: $ */

#define	_DDI_DKI	1
#define	_SYSV4		1

/*
 * This is layered on top of the fast first-fit heap allocator. Since we want
 * to keep the allocator portable, this layer deals with synchronization
 * issues. Note that we have to deal with a special chicken-and-egg problem
 * with synchronization, in that we want to use the allocation functions for
 * allocating the locks that will be used in the allocation functions...
 *
 * $Log: $
 */

/*
 *-IMPORTS:
 *	<common/ccompat.h>
 *		__USE_PROTO__
 *		__ARGS ()
 *	<sys/types.h>
 *		_VOID
 *		size_t
 *	<string.h>
 *		memset ()
 */

#include <common/ccompat.h>
#include <sys/types.h>
#include <string.h>

#include <sys/kmem.h>

/*
 *-STATUS:
 *	DDI/DKI
 *
 *-NAME:
 *	kmem_zalloc ()	Allocate and clear space from kernel free memory.
 *
 *-SYNOPSIS:
 *	#include <sys/types.h>
 *	#include <sys/kmem.h>
 *
 *	void * kmem_zalloc (size_t size, int flag);
 *
 *-ARGUMENTS:
 *	size		Number of bytes to allocate.
 *
 *	flag		Specifies whether the caller is willing to sleep
 *			waiting for memory. If "flag" is set to KM_SLEEP, the
 *			caller will sleep if necessary until the specified
 *			amount of memory is available. If "flag" is set to
 *			KM_NOSLEEP, the caller will not sleep, but
 *			kmem_zalloc () will return NULL if the specified
 *			amount of memory is not immediately available.
 *
 *-DESCRIPTION:
 *	kmem_zalloc () allocates "size" bytes of kernel memory, clears the
 *	memory by filling it with zeros, and returns a pointer to the
 *	allocated memory.
 *
 *-RETURN VALUE:
 *	Upon successful completion, kmem_zalloc () returns a pointer to the
 *	allocated memory. If KM_NOSLEEP is specified and sufficient memory is
 *	not immediately available, kmem_zalloc () returns a NULL pointer. If
 *	"size" is set to 0, kmem_zalloc () always returns NULL regardless of
 *	the value of "flag".
 *
 *-LEVEL:
 *	Base only if "flag" is set to KM_SLEEP. Base or interrupt if "flag" is
 *	set to KM_NOSLEEP.
 *
 *-NOTES:
 *	May sleep if "flag" is set to KM_SLEEP.
 *
 *	Driver-defined basic locks and read/write locks may be held across
 *	calls to this function if "flag" is KM_NOSLEEP but may not be held if
 *	"flag" is KM_SLEEP.
 *
 *	Driver-defined sleep locks may be held across calls to this function
 *	regardless of the value of "flag".
 *
 *	Kernel memory is a limited resource and should be used judiciously.
 *	Memory allocated using kmem_zalloc () should be freed as soon as
 *	possible. Drivers should not use local freelists for memory or similar
 *	schemes that cause the memory to be held for longer than necessary.
 *
 *	The address returned by a successful call to kmem_zalloc () is word-
 *	aligned.
 *
 *-SEE ALSO:
 *	kmem_alloc (), kmem_free ()
 */

#if	__USE_PROTO__
_VOID * (kmem_zalloc) (size_t size, int flag)
#else
_VOID *
kmem_zalloc __ARGS ((size, flag))
size_t		size;
int		flag;
#endif
{
	_VOID	      *	mem;

	if ((mem = kmem_alloc (size, flag)) != NULL)
		memset (mem, 0, size);
	return mem;
}

