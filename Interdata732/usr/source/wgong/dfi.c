/*
 *	dfi [ -l ] dev [ pattern ]
 */
char filename[14];
char *pattern	{ "*" };
int buffer[64];

int lflag;
int match();


main(argc, argv)
char **argv;
{

	register disc, c;
	register char *p;

	if (argc > 1 && *(p=argv[1]) == '-') {
		++argv;
		--argc;
		while (c = *++p) switch(c) {
			case 'l':
			case 'L':
				++lflag;
				break;
		}
	}
	if (argc < 2) {
		printf("Usage: dfi [ -l ] dev [ pattern ]\n");
		exit(1);
	}
	if ((disc = open(argv[1], 0)) < 0) {
		printf("Can't open %s\n", argv[1]);
		exit(1);
	}


	if (argc > 2)
		pattern = argv[2];

	mfd(disc, match);
}
/*
 * read the directory on the disc assigned to fd <disc>, passing
 * each directory block to function <func>.
 * if <func> returns nonzero, stop reading the directory.
 *	returns -1: disc error
 *	         0: all directory blocks read
 *	         1: <func> requested termination
 */
mfd(disc, func)
int (*func)();
{
	register int dirp, *bp;

	seek(disc, 0, 0);
	if (read(disc, buffer, sizeof buffer) < 0)
		return(-1);

	dirp = buffer[2];
	while (dirp) {
		seek(disc, dirp<<8, 0);
		if (read(disc, buffer, sizeof buffer) < 0)
			return(-1);
		for (bp = &buffer[1]; bp < &buffer[5*12+1]; bp =+ 12)
			if (bp[9] & (1<<28))
				if ((*func)(bp))
					return(1);
		dirp = buffer[0];
	}
	return(0);
}

char *mdtab[] {
	"co",
	"ch",
	"in"
};
struct { int *intp; };

/*
 * check whether filename in directory matches given pattern
 */
match(fn)
char *fn;
{
	register i;
	register char *p;
	register mode;

	p = filename;
	for (i=0; i<8; p++)
		if ((*p = fn[i++]) == ' ')
			break;
	*p++ = '.';
	for (i=8; i<11; p++)
		if ((*p = fn[i++]) == ' ')
			break;
	*p++ = '\0';

	if (!amatch(filename, pattern))
		return(0);

	printf("%.8s %.3s", fn, &fn[8]);
	if (lflag) {
		mode = fn[36]>>5;
		printf("  %s", mdtab[mode]);
		printf("%6d", (fn[22]<<8) | fn[23]);
		if (mode == 0)
			printf("%8d", fn.intp[4] - fn.intp[3] + 1);
		else
			printf("%8d", fn.intp[10]);
	}
	if (putchar('\n') <= 0)
		return(1);
	return(0);
}
amatch(as, ap)
char *as, *ap;
{
	register char *s, *p;
	register scc;
	int c, cc, ok, lc;

	s = as;
	p = ap;
	if (scc = *s++)
		if ((scc =& 0177) == 0)
			scc = 0200;
	switch (c = *p++) {

	case '[':
		ok = 0;
		lc = 077777;
		while (cc = *p++) {
			if (cc==']') {
				if (ok)
					return(amatch(s, p));
				else
					return(0);
			} else if (cc=='-') {
				if (lc<=scc && scc<=(c = *p++))
					ok++;
			} else
				if (scc == (lc=cc))
					ok++;
		}
		return(0);

	default:
		if (c!=scc)
			return(0);

	case '?':
		if (scc)
			return(amatch(s, p));
		return(0);

	case '*':
		return(umatch(--s, p));

	case '\0':
		return(!scc);
	}
}

umatch(s, p)
char
*s, *p;
{
	if(*p==0)
		return(1);
	while(*s)
		if (amatch(s++,p))
			return(1);
	return(0);
}
