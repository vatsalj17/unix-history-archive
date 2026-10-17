/*	library:gnamef.c	1.6	*/
#include "sccsid.h"
VERSION(@(#)library:gnamef.c	1.6)

/*	@(#)uucp:gnamef.c	2.1	*/

#include <stdio.h>
#include "gnamef.h"
struct stat __s_;

/*
 * get next file name from directory
 *	p	 -> file description of directory file to read
 *	filename -> address of buffer to return filename in
 *		    must be of size DIRSIZ+1
 * returns:
 *	FALSE	-> end of directory read
 *	TRUE	-> returned name
 */
gnamef(p, filename)
DIR *p;
register char *filename;
{
#if  ( defined SVR4 ) || ( defined mips )
	struct dirent *dp;
#else
	struct direct dentry;
	register struct direct *dp = &dentry;
#endif	/* SVR4 */

	while (1) {
#if  ( defined BSD4_2 ) || ( defined SVR4 ) || ( defined MSDOS ) || ( defined mips )
		if ((dp = readdir(p)) == NULL)
#else	/* ! (BSD4_2 || SVR4) */
		if (fread((char *)dp,  sizeof(dentry), 1, p) != 1)
#endif	/* BSD4_2 || SVR4 */
			return(FALSE);
		if (dp->d_ino != 0 && dp->d_name[0] != '.')
			break;
	}

	(void) strncpy(filename, dp->d_name, MAXBASENAME);
	filename[MAXBASENAME] = '\0';
	return(TRUE);
}

/*
 * get next directory name from directory
 *	p	 -> file description of directory file to read
 *	filename -> address of buffer to return filename in
 *		    must be of size DIRSIZ+1
 * returns:
 *	FALSE	-> end of directory read
 *	TRUE	-> returned dir
 */
gdirf(p, filename, dir)
register char *filename;
DIR *p;
char *dir;
{
	char statname[MAXNAMESIZE];

	while (1) {
		if(gnamef(p, filename) == FALSE)
			return(FALSE);
		(void) sprintf(statname, "%s/%s", dir, filename);
		if (DIRECTORY(statname))
		    break;
	}

	return(TRUE);
}
