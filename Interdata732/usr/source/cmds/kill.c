/*
 *	kill [-signo] processid [...]
 */
main(argc, argv)
char **argv;
{
	register char *p;
	register signo, pid;

	signo = 9;
	while (--argc > 0) {
		if (*(p = *++argv) == '-') {
			signo = getnum(++p);
			if (signo <= 0 || signo > 13) {
				printf("Illegal signal\n");
				exit(1);
			}
			continue;
		}
		pid = getnum(p);
		if (kill(pid, signo) < 0)
			printf("%s not found\n", *argv);
	}
}

getnum(s)
char *s;
{
	register n, c;

	n = 0;
	while (c = *s++) {
		if ((c =- '0') < 0 || c > 9) {
			printf("Non-numeric arg\n");
			exit(1);
		}
		n = n * 10 + c;
	}
	return(n);
}
