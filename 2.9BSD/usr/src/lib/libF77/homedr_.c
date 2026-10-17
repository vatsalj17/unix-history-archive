/* Fortran get home directory PLWard 3/20/80 */
#include <pwd.h>
struct passwd *getpwuid();

char homedr_(name,length)
	char name[];
	long length;
{
    register struct passwd *pp;
    int i,j;

    pp = getpwuid(getuid());
    for (i=0;i < length;i++) name[i]=' ';
    for (i=0;pp->pw_dir[i] != '\0';i++);
    if  (i > length) return;
    for (j=0; j < i; j++)
        name[j]=pp->pw_dir[j];
}
