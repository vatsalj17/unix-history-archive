/*
 *	banner [arg ...]
 *
 * If no arguments, reads standard input and prints it in block chars
 * If arguments present, prints arguments in block chars (1 per line)
 */

main(argc, argv)
register int	argc;
register char	**argv;
{
	static char	buf[128];

	if (argc == 1) {
		while (gets(buf))
			bprint(buf);
	}
	else
		while (--argc)
			bprint(*++argv);
}

gets(s)
register char *s;
{
	register char	*p, c;

	for (p = s; (c = getchar()) && c != '\n'; p++)
		*p = c;

	*p = '\0';
	return (c);
}
