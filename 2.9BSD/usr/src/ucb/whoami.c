#include <stdio.h>
#include <pwd.h>
#include <grp.h>
struct passwd *getpwuid();
struct group  *getgrgid();

main()
{
	struct passwd *pp;
	struct group  *gn;
	int uid, gid;

	pp=getpwuid(uid = getuid());
	gn=getgrgid(gid = getgid());

	if (pp == NULL)
		printf("%d", uid);
	else
		printf("%s", pp->pw_name);

	if (gn == NULL)
		printf(".%d\n", gid);
	else
		printf(".%s\n", gn->gr_name);

}
