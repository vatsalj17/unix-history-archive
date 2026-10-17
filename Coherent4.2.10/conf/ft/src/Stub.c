/*
 * floppy tape stub
 */

#define	__KERNEL__	1
#include <sys/con.h>

CON	      *	ftCon = 0;

/* Called from main.c */
#if	__USE_PROTO__
void ftAlloc (void)
#else
void
ftAlloc ()
#endif
{
}
