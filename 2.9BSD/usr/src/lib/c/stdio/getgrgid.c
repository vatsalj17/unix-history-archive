/*	@(#)getgrgid.c	2.1	SCCS id keyword	*/
#include <grp.h>

struct group *
getgrgid(gid)
register gid;
{
	register struct group *p;
	struct group *getgrent();

	setgrent();
	while( (p = getgrent()) && p->gr_gid != gid );
	endgrent();
	return(p);
}
