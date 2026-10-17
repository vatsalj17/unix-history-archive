/*	@(#)getpwnam.c	2.2	SCCS id keyword	*/
#include <whoami.h>
#include <pwd.h>

struct passwd *
getpwnam(name)
char *name;
{
	register struct passwd *p;
	struct passwd *getpwent();

#ifdef UCB_PWHASH
	if( !(p = getpwmap(getmapnam(name))) ) {
#endif
	setpwent();
	while( (p = getpwent()) && strcmp(name,p->pw_name) );
#ifdef UCB_PWHASH
	}
#endif
	endpwent();
	return(p);
}
