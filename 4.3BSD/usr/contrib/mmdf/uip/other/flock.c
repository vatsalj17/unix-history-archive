#include <stdio.h>
#include <sys/file.h>

main(argc, argv)
int argc;
char *argv[];
{
	int	fd;

	if (argc < 3) {
		fputs("usage: flock file command\n", stderr);
		exit(1);
	}
	if ((fd = open (argv[1], 0)) < 0
	  || flock (fd, LOCK_EX | LOCK_NB)) {
		perror (argv[1]);
	  	exit (1);
	}

	execvp(argv[2], &argv[2]);
	perror(argv[2]);
	exit(1);
}
