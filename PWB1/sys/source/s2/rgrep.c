#
#define	LBSIZE	256

char	ibuf[512];
int	lnum[2];
char	linebuf[LBSIZE+1];
int	bflag;
int	nflag;
int	cflag;
int	vflag;
int	nfile;
int	blkno;
int	tln[2];
char	*regp;

main(argc, argv)
char **argv;
{
	extern fout;

	fout = dup(1);
	flush();
	while (--argc > 0 && (++argv)[0][0]=='-')
		switch (argv[0][1]) {

		case 'v':
			vflag++;
			continue;

		case 'b':
			bflag++;
			continue;

		case 'c':
			cflag++;
			continue;

		case 'n':
			nflag++;
			continue;

		default:
			printf2("Unknown flag\n");
			continue;
		}
	if (argc<=0)
		exit(2);
	if ((regp = regcmp(*argv,0))==0)
		printf2("RE Error\n");
	nfile = --argc;
	if (argc<=0)
		execute(0);
	else while (--argc >= 0) {
		argv++;
		execute(*argv);
	}
	flush();
	exit(0);
}

execute(file)
{
	register char *p1, *p2;
	register c;
	int f;
	char *ebp, *cbp;

	if (file) {
		if ((f = open(file, 0)) < 0) {
			printf2("Can't open %s\n", file);
		}
	} else
		f = 0;
	ebp = ibuf;
	cbp = ibuf;
	lnum[0] = 0;
	lnum[1] = 0;
	tln[0] = 0;
	tln[1] = 0;
	blkno = -1;
	for (;;) {
		if ((++lnum[1])==0)
			lnum[1]++;
		if((lnum[1]&0377) == 0)
			flush();
		p1 = linebuf;
		p2 = cbp;
		for (;;) {
			if (p2 >= ebp) {
				if ((c = read(f, ibuf, 512)) <= 0) {
					close(f);
					if (cflag) {
						if (nfile > 1)
							printf("%s:", file);
						p1 = locv(tln[0],tln[1]);
						printf("%s\n", p1);
					}
					return;
				}
				blkno++;
				p2 = ibuf;
				ebp = ibuf+c;
			}
			if ((c = *p2++) == '\n')
				break;
			if(c)
			if (p1 < &linebuf[LBSIZE-1])
				*p1++ = c;
		}
		*p1++ = 0;
		cbp = p2;
		p1 = linebuf;
	if (regex(regp,p1))
		goto found;
	nfound:
		if (vflag)
			succeed(file);
		continue;
	found:
		if (vflag==0)
			succeed(file);
	}
}

printf2(s, a)
{
	extern fout;
	flush();
	fout = 2;
	printf(s, a);
	flush();
	exit(2);
}

succeed(f)
{
	if (cflag) {
		if (++tln[1]==0)
			tln[0]++;
		return;
	}
	if (nfile > 1)
		printf("%s:", f);
	if (bflag)
		printf("%l:", blkno);
	if (nflag)
		printf("%s:", locv(lnum[0], lnum[1]));
	printf("%s\n", linebuf);
}
