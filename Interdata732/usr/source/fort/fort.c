/*
 *		Fortran Command
 */

char usage[]	"Usage: fort [-c] [-l] [-S] [-o output] file [...]\n";

int	lflag;			/* make listing	*/
int	cflag;			/* compile & assemble only */
int	vflag;			/* debug */
int	sflag;			/* compile only */
int	nerror;			/* no. of errors encountered */

char	*clist[64];		/* source filenames */
int	nclist;
char	*llist[64];		/* arguments for ld */
int	nllist;
char	*args[64];		/* argument list for exec */
char	*ofname;		/* output file name */

char	sbuf[512];		/* string area for making new names */
char	*str	sbuf;

main(argc, argv)
char **argv;
{
	register i, j, f;
	register char *p;

	if (--argc <= 0) {
		printf(usage);
		exit(1);
	}
	while (argc--) {
		if (*(p = *++argv) == '-')
			switch(*++p) {

			case 'c':
				cflag = 1;
				break;

			case 'v':
				vflag = 1;
				break;
			case 'S':
				sflag = 1;
				cflag = 1;
				break;

			case 'o':
				ofname = *++argv;
				if (--argc <= 0 || ofname[0] == '-') {
					printf(usage);
					exit(1);
				}
				break;

			case 'l':
				if (p[1] == '\0') {
					lflag = 1;
					break;
				}

			default:
				llist[nllist++] = p-1;
			}
		else {
			if (isfor(p)) {
				clist[nclist++] = p;
				llist[nllist++] = setsuf(p, 'o');
			}
			else
				llist[nllist++] = p;
		}
	}
	nice(10);

	p = str;
	for (i=0; i<nclist; i++) {
		str = p;
		if (nclist > 1)
			printf("%s:\n", clist[i]);

		/* compile */
		args[0] = "fortv";
		args[1] = clist[i];
		args[2] = setsuf(clist[i], 's');
		j = 3;
		if (lflag)
			args[j++] = "l";
		args[j] = 0;
		if (call("/usr/bin/fortv", args)) {
			cflag = 1;
			continue;
		}
		if (sflag)
			continue;

		/* assemble */
		args[0] = "as";
		args[1] = "-u";
		args[4] = args[2];
		args[2] = "-o";
		args[3] = setsuf(clist[i], 'o');
		args[5] = 0;
		if (call("/bin/as", args)) {
			cflag++;
			continue;
		}
		unlink(args[4]);

	}

	/* load */
	if (cflag)
		exit(nerror);
	args[0] = "ld";
	args[1] = "-x";
	args[2] = "/lib/frt0.o";
	j = 3;
	if (ofname) {
		args[j++] = "-o";
		args[j++] = ofname;
	}
	for (i=0; i<nllist; i++)
		args[j++] = llist[i];
	args[j++] = "-lf";
	args[j++] = 0;
	call("/bin/ld", args);
	if (nclist == 1)
		unlink(setsuf(clist[0], 'o'));
	exit(nerror);
}

/*
 * Check whether suffix of filename is ".for" or ".f"
 */
isfor(name)
char *name;
{
	register char *p;

	if ((p = suffix(name)) == 0 )
		return(0);
	if (*p == 'f')
		if (p[1] == '\0' || p[1] == 'o' && p[2] == 'r' && p[3] == '\0')
			return(1);
	return(0);
}

/*
 * Return a pointer to the suffix part of a filename
 */
suffix(name)
char *name;
{
	register char *p, *s;
	register c;

	s = 0;
	for (p = name; c = *p; p++)
		if (c == '/')
			s = 0;
		else if (c == '.')
			s = p+1;
	return(s);
}

/*
 * Make a new filename from <name> with suffix <suf>
 */
setsuf(name, suf)
char *name;
{
	register char *p, *q;

	q = name;
	for (p = str; *p = *q++; p++)
		;
	if ((q = suffix(str)) == 0) {
		q = p;
		*q++ = '.';
	}
	*q++ = suf;
	*q++ = '\0';
	p = str;
	str = q;
	return(p);
}

/*
 * Fork and execute a command with arguments in args[]
 */
call(command)
char *command;
{
	int status;
	register i;

	if (vflag) {
		for (i=0; args[i]; i++)
			printf("%s ", args[i]);
		putchar('\n');
	}
	if (fork() == 0) {
		execv(command, args);
		printf("Can't exec %s\n", command);
		exit(1);
	}
	wait(&status);
	if (i = (status&0377)) {
		if (i != 2)
			printf("Fatal error in %s\n", command);
	}
	else
		i = status>>8;
	if (i)
		nerror++;
	return(i);
}
