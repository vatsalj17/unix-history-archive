/*
 * Concatenate files.
 *
 */

char	ibuf[512];
char	obuf[512];

main(argc, argv)
char **argv;
{
	register char *cpi, *cpo;
	register n;
	int ubf;
	int silent;
	int fi;
	int fflg;

	fflg = 0;
	ubf = 0;
	silent = 0;
	cpi = argv[1];
	while (argc > 1 && cpi[0] == '-' && cpi[1] != '\0' && cpi[2] == '\0') {
		switch (cpi[1]) {
		case 'u':
			argv++;
			argc--;
			ubf++;
			break;
		case 's':
			argv++;
			argc--;
			silent++;
			break;
		default:
			argv++;
			argc--;
			badflag(cpi);
		}
		cpi = argv[1];
	}
	if (argc<2) {
		argc = 2;
		fflg++;
	}
	cpo = obuf;
	while (--argc > 0) {
		if ((*++argv)[0]=='-' && (*argv)[1]=='\0' || fflg)
			fi = 0;
		else {
			if ((fi = open(*argv, 0)) < 0) {
				if ( !silent )
					error(*argv);
				continue;
			}
		}
		while ((n = read(fi, ibuf, 512)) > 0) {
			if(ubf) {
				write(1, ibuf, n);
				continue;
			}
			cpi = ibuf;
			do {
				if (cpo >= &obuf[512]) {
					write(1, obuf, 512);
					cpo = obuf;
				}
				*cpo++ = *cpi++;
			} while (--n);
		}
		if (fi>0)
			close(fi);
	}
	if (cpo > obuf)
		write(1, obuf, cpo-obuf);
	exit(0);
}

error(s)
{
	register char *cp;

	write(2, "cat: can't open ", 16);
	cp = s;
	while (*cp) {
		write(2, cp, 1);
		cp++;
	}
	write(2, "\n", 1);
}

badflag(s)
{
	register char *cp;

	write(2, "cat: bad flag ", 14);
	cp = s;
	while (*cp) {
		write(2, cp, 1);
		cp++;
	}
	write(2, "\n", 1);
}
