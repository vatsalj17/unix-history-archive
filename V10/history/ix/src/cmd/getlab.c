#include <errno.h>
#include <sys/label.h>
#include <stdio.h>

struct label x;
struct label y;
extern errno;
char *plab(), *labtoa(), *strchr();
int showfds;
int errs = 0;

main(ac, av)
char **av;
{
	int i, fd;

	if(ac>=2 && strcmp(av[1],"-d")==0) {
		showfds++;
		av++;
		ac--;
	}
	if(ac < 2) {
		getplab(&x, &y);
		printf("%-20s\t%s\n", "proc lab",  plab(&x));
		printf("%-20s\t%s\n", "proc ceil",  plab(&y));
		if(showfds || ac>1)
			printf("\n");
	}
	if(showfds) {
		for(i=0; i<128; i++) {
			errno = 0;
			if(fgetflab(i, &x) == 0) {
				printf("fd %-3d%14s\t%s\n", i, "", plab(&x));
				continue;
			} else if(errno == EBADF) 
				continue;
			errs++;
			fprintf(stderr,"fd %d: ", i);
			perror("");
		}
		if(ac > 1)
			printf("\n");
	}

	for(i=1; i<ac; i++) {
		int xok = 0, yok = 0;
		errno = 0;
		if(getflab(av[i], &x) == 0) {
			xok++;
		}
		if(errno ) perror(av[i]);
		fd = open(av[i], 0);
		errno = 0;
		if(fd != -1 && fgetflab(fd, &y) == 0) {
			yok++;
		}
		if(errno ) perror(av[i]);
		close(fd);
		if(xok && yok && labEQ(&x, &y)) {
			printf("%-20s\t%s\n", av[i], plab(&x));
		} else {
			if(xok)printf("%-14s[name]\t%s\n", av[i], plab(&x));
			if(yok)printf("%-14s[desc]\t%s\n", av[i], plab(&y));
		}
		if(!xok && !yok)
			errs++;
	}
	exit(errs);
}

/* insert some newlines if necessary */
char *plab(p)
struct label *p;
{
	char s[200];
	char *t;
	strcpy(s, labtoa(p));
	for(t=s; strlen(t)>54; ) {
		t = strchr(t+54,' ');
		if(t)
			*t++ = '\n';
		else
			break;
	}
	return s;
}
