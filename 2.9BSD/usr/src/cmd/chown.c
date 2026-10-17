/*
 * chown uid file ...
 */

#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>

struct	passwd	*pwd,*getpwnam();
struct	group	*grp,*getgrnam();
struct	stat	stbuf;
int	uid, gid;
int	status;

main(argc, argv)
char *argv[];
{
	register c;
	register char *group;

	if(argc < 3) {
		printf("usage: chown uid file ...\n");
		exit(4);
	}
	for (group = argv[1]; *group ; group++) {
		if (*group == '.') {
			*group = '\0';
			group++;
			if (isnumber(group))
				gid = atoi(group);
			else {
				if ((grp=getgrnam(group)) == -1) {
					printf("unknown group: %s\n",group);
					exit(4);
				}
				gid = grp->gr_gid;
				endgrent();
			}
			goto owner;
		}
	}
	group = NULL;
owner:
	if(isnumber(argv[1])) {
		uid = atoi(argv[1]);
	} else {
		if((pwd=getpwnam(argv[1])) == NULL) {
			printf("unknown user id: %s\n",argv[1]);
			exit(4);
		}
		uid = pwd->pw_uid;
	}

	for(c=2; c<argc; c++) {
	    if (group) {
		if(chown(argv[c], uid, gid) < 0) {
			perror(argv[c]);
			status = 1;
		}
	    } else {
		stat(argv[c], &stbuf);
		if(chown(argv[c], uid, stbuf.st_gid) < 0) {
			perror(argv[c]);
			status = 1;
		}
	    }
	}
	exit(status);
}

isnumber(s)
char *s;
{
	register c;

	while(c = *s++)
		if(!isdigit(c))
			return(0);
	return(1);
}
