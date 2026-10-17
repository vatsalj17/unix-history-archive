/*	@(#)isapipe.c	2.1	SCCS id keyword	*/
/*
 * Returns 1 if file is a pipe or other non-seekable device
 */

#include <errno.h>

extern	errno;
long lseek();

isapipe(f)
int f;
{
	return(lseek(f, 0L, 1) < 0 && errno == ESPIPE);
}
