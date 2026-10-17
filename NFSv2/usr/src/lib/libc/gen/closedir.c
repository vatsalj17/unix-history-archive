#ifndef lint
static	char sccsid[] = "@(#)closedir.c 1.1 85/05/30 SMI";
#endif

#include <sys/param.h>
#include <sys/dir.h>

/*
 * close a directory.
 */
void
closedir(dirp)
	register DIR *dirp;
{
	close(dirp->dd_fd);
	dirp->dd_fd = -1;
	dirp->dd_loc = 0;
	free(dirp->dd_buf);
	free(dirp);
}
