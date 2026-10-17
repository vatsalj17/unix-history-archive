/*
 * attach/detach ttyid [...]
 *
 * Turn terminals on and off by changing ttys file and signalling init
 */

char	tfile[]	"/etc/ttys";
char	tbuf[512];
int	 len;

main(argc, argv)
char **argv;
{
	register char *p;
	register set, fd, tty;
	char *find();

	if (argc <= 1)
		error("Usage: %s tty-id ...\n", *argv);
	if ((fd = open(tfile, 2)) < 0)
		error("Can't open %s\n", tfile);
	if ((len = read(fd, tbuf, sizeof tbuf)) == sizeof tbuf)
		error("?? %s too big ??", tfile);
	if (len <= 0)
		error("Can't read %s", tfile);

	/* Determine which name command was called by */
	for (p = *argv; *p; p++)
		;
	while (p >= *argv && *p != '/')
		p--;
	set = p[1]=='d'? '0' : '1';

	while (--argc) {
		if ((*++argv)[1] != '\0')
			error("Use single-character tty-ids");
		tty = (*argv)[0];
		if ((p = find(tty)) == 0)
			error("tty%c not found", tty);
		*p = set;
	}

	seek(fd, 0, 0);
	if (write(fd, tbuf, len) != len)
		error("Can't write %s", tfile);

	if (kill(1, 1) < 0)
		error("Can't signal init process");
}

/*
 * Find entry for given tty
 */
char *
find(tty)
{
	register char *p;

	for (p = tbuf; p < &tbuf[len]; ) {
		if (p[1] == tty)
			return(p);
		while (*p++ != '\n')
			;
	}
	return(0);
}

error(s, x)
{
	printf(s, x);
	putchar('\n');
	exit(1);
}
