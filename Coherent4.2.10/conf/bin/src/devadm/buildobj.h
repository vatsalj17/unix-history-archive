#ifndef	BUILDOBJ_H
#define	BUILDOBJ_H

/*
 * This file declares structures and function prototypes for some routines
 * useful for building up objects a piece at a time.
 */
/*
 *-IMPORTS:
 *	<sys/compat.h>
 *		EXTERN_C_BEGIN
 *		EXTERN_C_END
 *		VOID
 *		PROTO ()
 *	<stddef.h>
 *		size_t
 */

#include <sys/compat.h>
#include <stddef.h>

/*
 * The following code can be used as the basis of a system for building up
 * objects such as symbol-table entries incrementally. It's kind of like the
 * GNU obstack system (and clearly inspired by it), but it's not free
 * software. It's also a little more clearly laid out, and easier to
 * understand and modify, and slower.
 */

#ifndef	BUILD_T
#define	BUILD_T
typedef struct builder		build_t;
#endif


/*
 * Return values from build functions.
 */

enum {
	BUILD_TOO_BIG = -11,
	BUILD_CORRUPT = -10,
	BUILD_STACK_EMPTY = -9,
	BUILD_BAD_NESTING = -8,
	BUILD_NOT_LAST = -7,
	BUILD_SIZE_OVERFLOW = -6,
	BUILD_NO_MEMORY = -5,
	BUILD_NO_OBJECT = -4,
	BUILD_OBJECT_BEGUN = -3,
	BUILD_NULL_BASE = -2,
	BUILD_NULL_HEAP = -1,
	BUILD_OK
};

/*
 * Special value for "init" parameter in functions below that causes newly
 * allocated memory to be cleared.
 */

#define	INIT_ZERO	((VOID *) 1)


#if	BUILDOBJ_DELTA_EXTENSIONS
/*
 * If the extensions for navigating within build strings are included, then
 * we need a string that defines the printable encoding characters in order to
 * be character-set-independent. Note that this string *must* *not* contain
 * repeated characters!
 */

#include <common/_tricks.h>
#include <common/_limits.h>

#if	! defined (BUILD_DELTA_CODE)
#define	BUILD_DELTA_CODE	" !\"#$%&'()*+,-./0123456789:;<=>?" \
				"@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_" \
				"`abcdefghijklmnopqrstuvwxyz{|}~"
#endif

#ifdef	BUILD_DELTA_CODE

#define	TOP_BIT(x)		(1 << __MOST_BIT_32_CONST (x))
#define	CLEAR_TOP_BIT(x)	((x) - TOP_BIT (x))

#define	DELTA_RADIX		(sizeof (BUILD_DELTA_CODE) - 1)
#define	LOG_DELTA_RADIX_256	(256 * __MOST_BIT_32_CONST (DELTA_RADIX) + \
				 (256 * CLEAR_TOP_BIT (DELTA_RADIX) / \
				  TOP_BIT (DELTA_RADIX)))

#else

#define	DELTA_RADIX		__UCHAR_MAX
#define	LOG_DELTA_RADIX_256	(__CHAR_BIT * 256)

#endif


/*
 * Some constants related to the details of the encoding of a size_t, since
 * that is the type of datum we will most likely encode. The encoding consists
 * of a single delta-digit describing the length of the remainder of the code
 * (so that the code can be safely used for interchange) plus digits for the
 * coded datum beginning with the most significant digits. If the length digit
 * is a 0, then that is currently undefined behaviour.
 *
 * We use some rough calculations to pick a reasonable default maximum number
 * of digits for coding a size_t value (our fixed-point estimate of log2 is
 * always lower than the real thing, and no worse than 94% accurate).
 */

#define	LOG_SIZET_RADIX_256	(256 * __CHAR_BIT * sizeof (size_t))

#define	SIZET_CODE_SIZE		__DIVIDE_ROUNDUP_CONST (LOG_SIZET_RADIX_256, \
							LOG_DELTA_RADIX_256)

#endif	/* BUILDOBJ_DELTA_EXTENSIONS */


EXTERN_C_BEGIN

build_t	      *	builder_alloc	PROTO ((size_t _chunksize, size_t _align));
void		builder_free	PROTO ((build_t * _heap));

VOID	      *	build_malloc	PROTO ((build_t * _heap, size_t _size));

int		build_begin	PROTO ((build_t * _heap, size_t _size,
					CONST VOID * _init));
int		build_add	PROTO ((build_t * _heap, size_t _size,
					CONST VOID * _init));
int		build_addchar	PROTO ((build_t * _heap, char _ch));
VOID	      *	build_end	PROTO ((build_t * _heap, size_t * _size));
int		build_reduce	PROTO ((build_t * _heap, size_t _size));
int		build_release	PROTO ((build_t * _heap, CONST VOID * _base));
int		build_abandon	PROTO ((build_t * _heap));
int		build_resume	PROTO ((build_t * _heap));

CONST char    *	build_error	PROTO ((int _errcode));

int		build_push	PROTO ((build_t * _heap, VOID * _data,
					size_t _size));
int		build_pop	PROTO ((build_t * _heap, VOID * _data,
					size_t * _sizep));

size_t		build_offset	PROTO ((CONST build_t * _heap));
void		build_dump	PROTO ((build_t * _heap));

#if	BUILDOBJ_DELTA_EXTENSIONS
#endif	/* BUILDOBJ_DELTA_EXTENSIONS */

EXTERN_C_END

#endif	/* ! defined (BUILDOBJ_H) */
