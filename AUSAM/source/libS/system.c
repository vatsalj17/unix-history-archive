#define	INTR	2
#define	QUIT	3

system(s)
char *s;
{
	int status;
	register int *istat, *qstat;

	if (fork() == 0) {
		execl("/bin/sh", "sh", "-c", s, 0);
		_exit(127);
	}
	istat = signal(INTR, 1);
	qstat = signal(QUIT, 1);
	if (wait(&status) == -1)
		status = -1;
	signal(INTR, istat);
	signal(QUIT, qstat);
	return(status);
}
