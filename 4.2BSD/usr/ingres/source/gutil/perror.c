static	char	Sccsid[]= "@(#)perror.c	1.1 (Ingres) 4/30/82";
/*
 * Print the error indicated
 * in the cerror cell.
 * ----
 * this code stolen from the system perror, the only change is that we print
 * on 1, instead of 2 (which is usally closed).
 */

int	errno;
int	sys_nerr;
char	*sys_errlist[];
perror(s)
char *s;
{
	register char *c;
	register n;

	c = "Unknown error";
	if(errno < sys_nerr)
		c = sys_errlist[errno];
	n = strlen(s);
	if(n) {
		write(1, s, n);
		write(1, ": ", 2);
	}
	write(1, c, strlen(c));
	write(1, "\n", 1);
}
