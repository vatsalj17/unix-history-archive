/*
 * tm: map of tape records and filemarks
 */

char	*tape	"/dev/rmt0";
#define tdrive	tape[8]
int	maxtm	2;
int	maxrec	0x7fffffff;
int	maxfil	0x7fffffff;
int	errflag;
int	done;
int	prev	-1;
int	nrec;
int	nfil;

char	buf[16*1024];

main(argc, argv)
char **argv;
{
	register	fd, len, rec;

	while (--argc) {
		if (**++argv != '-')
			usage();
		switch (argv[0][1]) {
			case 't':
				maxtm = atoi(&argv[0][2]);
				break;
			case 'r':
				maxrec = atoi(&argv[0][2]);
				break;
			case 'e':
				errflag++;
				break;
			case '0': case '1': case '2': case '3':
				tdrive = argv[0][1];
				break;
			default:
				usage();
		}
	}
	if ((fd = open(tape, 0)) < 0) {
		printf("Can't open %s\n", tape);
		exit(1);
	}
	for (rec = 0; ; rec++) {
		len = done? -2 : read(fd, buf, sizeof buf);
		if (len != prev) {
			if (prev == 0)
				printf("tm:%d\n", nrec);
			else if (prev > 0)
				printf("%d:%d\n", prev, nrec);
			prev = len;
			nrec = 0;
		}
		if (done)
			break;
		if (len == -1) {
			printf("** Read Error b%d\n", rec);
			if (!errflag)
				done++;
		}
		nrec++;
		if (len == 0) {
			rec = 0;
			if (nrec >= maxtm)
				done++;
		}
		else if (rec >= maxrec)
			done++;
	}
}

usage()
{
	printf("Usage: tm [-drive] [-t<n>] [-r<n>] [-e]\n");
	exit(1);
}
