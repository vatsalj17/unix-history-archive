#include <sys/label.h>
#include <stdio.h>

main(ac, av)
char **av;
{
	struct label x, y, *p, *atolab();

	getplab(&x, (struct label*)0);

	y = x;
	while(ac>1 && av[1][0] == '-') {
		if(strcmp(av[1], "-l") == 0) {
			p = atolab(av[2]);
			if(p == 0) {
				fprintf(stderr, "garbled label '%s'\n", av[1]);
				exit(1);
			}
			y = *p;
			ac -= 2;
			av += 2;
		} else {
			break;
		}
	}

	if(setplab((struct label*)0, &y) == -1) {
		perror("setplab");
		exit(1);
	}
	if(ac > 1) {
		execvp(av[1], av+1);
		perror(av[1]);
	} else {
		execl("/bin/sh", "sh", (char *)0);
		perror("/bin/sh");
	}
	exit(1);
}
