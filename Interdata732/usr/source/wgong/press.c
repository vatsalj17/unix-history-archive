#define	THRESH	4

int	ibuf[131], obuf[131];
char	cbuf[128];
int	unflag;

main(argc, argv)
char **argv;
{
	if (argc > 1 && argv[1][0] == '-') {
		unflag++;
		argc--;
		argv++;
	}
	if (argc > 1 && (ibuf[0] = open(argv[1], 0)) < 0) {
		printf("Can't find %s\n", argv[1]);
		exit(1);
	}
	if (argc <= 2)
		obuf[0] = dup(1);
	else
		if ((obuf[0] = creat(argv[2], 0666)) < 0) {
			printf("Can't create %s\n", argv[2]);
			exit(1);
		}

	if (unflag)
		unpress();
	else
		press();
	fflush(obuf);
}

press()
{
	register c, lastc, count, nsame;

	lastc = -1;
	count = 0;
	while ((c = getc(ibuf)) >= 0) {
		if (c == lastc) {
			if (++nsame == THRESH) {
				putstr(count-THRESH+1);
				count = 0;
				while ((c=getc(ibuf))==lastc && nsame < 128)
					nsame++;
				putc(0200|(nsame-1), obuf);
				putc(lastc, obuf);
				if (c < 0)
					break;
				nsame = 1;
				lastc = c;
			}
		} else {
			nsame = 1;
			lastc = c;
		}
		cbuf[count++] = c;
		if (count >= 128) {
			putstr(count);
			count = 0;
		}
	}
	putstr(count);
}

putstr(n)
{
	register i;

	if (n) {
		putc(n-1, obuf);
		for (i = 0; i<n; i++)
			putc(cbuf[i], obuf);
	}
}

unpress()
{
	register c, n;
	while ((n = getc(ibuf)) >= 0) {
		if (n&0200) {
			n =& 0177;
			if ((c = getc(ibuf)) < 0)
				inerr();
			while (n-- >= 0)
				putc(c, obuf);
			continue;
		} else {
			while (n-- >= 0) {
				if ((c = getc(ibuf)) < 0)
					inerr();
				putc(c, obuf);
			}
		}
	}
}

inerr()
{
	printf("Input file format error\n");
	exit(1);
}
