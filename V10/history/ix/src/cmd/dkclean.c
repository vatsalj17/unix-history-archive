/*
 * dkclean: run from /etc/rc.nosh with capability T_NOCHK|T_UAREA.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <ndir.h>

int errs = 0;

main(ac, av)
char **av;
{
	struct stat buf;
	struct direct *p;
	DIR *dirp;
	char *name;
	int i;

	for(i=1; i<ac; i++) {
		if(chdir(av[i]) == -1) {
			error(av[i]);
			continue;
		}
		dirp = opendir(av[i]);
		if(dirp == 0) {
			error(av[i]);
			continue;
		}
		while(p = readdir(dirp)) {
			name = p->d_name;
			if(strcmp(".", name) == 0) continue;
			if(strcmp("..", name) == 0) continue;
			if(stat(name, &buf) == -1) {
				error(name);
				continue;
			}
			if((buf.st_mode & S_IFMT) != S_IFCHR)
				continue;
			if(chown(name, 0, 0) == -1 || chmod(name, 0600) == -1) 
				error(name);
		}
	}
	return(errs);
}
error(s) {
	write(2, "dkclean: ", 9);
	perror(s);
	errs++;
}
