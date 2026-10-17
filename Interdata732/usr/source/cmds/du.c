/*
 * Quick and dirty version of du
 *
 *	- no inumber check is made, so links are counted twice
 */

char *dargv[] {
	"."
};

struct {
	int	dev;
	int	ino;
	int	flags;
	char	nlinks;
	char	uid;
	char	gid;
	char	size0;
	int	size1;
	int	addr[8];
	int	actime[2];
	int	mtime[2];
} stbuf;

char	buff[16];
char	file[128];
char	*fend file;

int	sflag;
int	aflag;

int	total;

main(argc, argv)
char **argv;
{
	register char *p, *q;

	while (--argc > 0 && **++argv == '-') {
		if (argv[0][1] == 's')
			sflag++;
		else if (argv[0][1] == 'a')
			aflag++;
	}
	if (argc < 1) {
		argv = dargv;
		argc = 1;
	}
	while (argc--) {
		p = *argv++;
		for (q = file; *q = *p++; *q++)
			;
		fend = q;
		du();
	}
	printf("Total %d\n", total);
}

du()
{
	register fi, len, tot;

	if (stat(file, &stbuf) < 0) {
		msg("can't find");
		return(0);
	}
	switch (stbuf.flags & 060000) {

	/* ordinary file */
	case 0000000:
		len = (stbuf.size1+511)/512;
		total =+ len;
		if (aflag)
			printf("%4d %s\n", len, file);
		return(len);

	/* special file */
	case 020000:
	case 060000:
		return(0);

	/* directory */
	case 040000:
		len = (stbuf.size1+511)/512;
		total =+ len;
		if (aflag)
			printf("%4d %s\n", len, file);
		if ((fi = open(file, 0)) < 0)
			msg("can't open");
		else {
			tot = 0;
			while (read(fi, buff, 16) == 16)
				if ((buff[0] | buff[1]) != 0
				    && (buff[2] != '.'
					|| (buff[3] != '\0'
					&& (buff[3] != '.'
					    || buff[4] != '\0')))
				    && nxtpath(&buff[2])) {
					tot =+ du();
					lstpath();
				}
			close(fi);
			if (!(aflag | sflag))
				printf("%4d %s\n", tot, file);
		}
		return(len);
	}
}

nxtpath(fn)
char *fn;
{
	register char *f, *t, *p;

	f = fend;
	if (f[-1] != '/')
		*f++ = '/';

	for (p = fn; p < &fn[14] && (*f = *p++); ++f)
		if (f>=&file[sizeof(file)-1]) {
			msg("name too long");
			return(0);
		}

	fend = f;
	return(1);
}


lstpath()
{
	register char *f, *t;

	f = fend;
	while (--f >= file && *f != '/')
		;
	*f = '\0';
	fend = f;
}

msg(s)
char *s;
{
	puts(file);
	puts(" -- ");
	puts(s);
	putchar('\n');
}

puts(s)
char *s;
{
	register char *p;

	for (p = s; putchar(*p); p++)
		;
}
