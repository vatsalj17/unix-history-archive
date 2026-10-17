#include <sys/types.h>
#include <sys/stat.h>

/* Set the userid of the file's directory to n
   return the old owner.
*/

owner(f, n)
char *f;
{
	struct stat statb;
	char *strrchr();
	char *ep = strrchr(f, '/');
	int r;
	if(ep == f)
		*ep = '/';
	else if(ep)
		*ep = 0;
	else
		f = ".";
	r = stat(f, &statb);
	if(ep)
		*ep = '/';
	if(r < 0 || n == -2)
		return -2;
	chown(f, n, statb.st_gid);
	return statb.st_uid;
}
