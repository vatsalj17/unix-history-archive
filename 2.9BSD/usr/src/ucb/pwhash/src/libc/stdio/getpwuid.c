/*	@(#)getpwuid.c	2.2	SCCS id keyword	*/
#include <whoami.h>
#include <pwd.h>

struct passwd *
getpwuid(uid)
register uid;
{
	register struct passwd *p;
	struct passwd *getpwent();

#ifdef UCB_PWHASH
	if( !(p = getpwmap(getmapuid(uid))) ) {
#endif
	setpwent();
	while( (p = getpwent()) && p->pw_uid != uid );
#ifdef UCB_PWHASH
	}
#endif
	endpwent();
	return(p);
}
