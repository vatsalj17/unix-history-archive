static char Sccsid[] "@(#)rename	1.1";
# include <errnos.h>

/*
	rename (unlink/link)
	Calls xlink() and xunlink().
*/

rename(oldname,newname)
char *oldname, *newname;
{
	extern int errno;

	if (unlink(newname) < 0 && errno != ENOENT)
		return(xunlink(newname));

	if (xlink(oldname,newname) == -1)
		return(-1);
	xunlink(oldname);
}
