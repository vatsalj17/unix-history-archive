#

/*
 *	slp - set printer/plotter modes
 *
 *	slp [+] [0-9] [flags]
 *
 *	+ - use std output for device
 *	digit - /dev/lp?
 *	default - /dev/lp
 */

#define	WOPEN	00001
#define	TRNS	00004
#define	SKIP	00010
#define	VTAB	00020
#define	FLUSH	00040
#define	FEED	00100
#define	NOEJECT	00200
#define	SIXLPI	01000
#define	POWROFF	040000
#define	ERROR	0100000

struct {
	char	*string;
	int	set, reset;
} modes[] {
	"eject",	0, NOEJECT,
	"-eject",	NOEJECT, 0,
	"feed",		FEED, 0,
	"-feed",	0, FEED,
	"skip",		SKIP, 0,
	"cdc",		SKIP, 0,
	"dec",		0, SKIP,
	"-skip",	0, SKIP,
	"-cdc",		0, SKIP,
	"trns",		TRNS, 0,
	"half",		TRNS, 0,
	"-trns",	0, TRNS,
	"full",		0, TRNS,
	"vtab",		VTAB, 0,
	"-vtab",	0, VTAB,
	"flush",	FLUSH, 0,
	"-flush",	0, FLUSH,
	"6lpi",		SIXLPI, 0,
	"8lpi",		0, SIXLPI,
	0
};

char	*arg;
int	fd	0;
int	mode[4];
char	*lp	"/dev/lp\0";	/* note spare character */

main(argc, argv)
char *argv[];
{
	char	c;
	register int	i;
	extern fout;

	fout = 2;

	if (argc > 1) {
		c = argv[1][0];
		if (c == '+') {
			fd = 1;
			argc--;
			argv++;
		} else if (c <= '9' && c >= '0' && argv[1][1] == 0) {
			lp[7] = c;
			argc--;
			argv++;
		}
	}
	if (!fd && (fd = open(lp, 0))<0) {
		perror(lp);
		return(1);
	}
	if (gtty(fd, mode) < 0) {
		perror("Gtty");
		return(1);
	}
	if (argc == 1) {
		prmodes();
		return(0);
	}

	while (--argc > 0) {
		arg = *++argv;

		if (eq("width")) {
			mode[2] = atoi(*++argv);
			argc--;
		}
		if (eq("length")) {
			mode[1] = atoi(*++argv);
			argc--;
		}

		for (i = 0; modes[i].string != 0; i++)
			if (eq(modes[i].string)) {
				mode[0] =& ~modes[i].reset;
				mode[0] =| modes[i].set;
			}

		if (arg != 0)
			printf("Unknown mode: %s\n", arg);
	}

	if (stty(fd, mode) < 0) {
		perror("Stty");
		return(1);
	}

	return(0);
}

eq(string)
char *string;
{
	register int	i;
	register char	*s;

	if (arg == 0)
		return(0);

	s = string;
	i = 0;
	do {
		if (arg[i] != s[i])
			return(0);
	} while (arg[i++] != '\0');

	arg = 0;
	return(1);
}

prmodes()
{
	register int	m;

	m = mode[0];

	printf("Width=%d cols   Length=%d lines\n", mode[2], mode[1]);

	if (m & SIXLPI)
		printf("6lpi   ");
	else
		printf("8lpi   ");
	if (m & TRNS)
		printf("half   ");
	if (m & SKIP)
		printf("cdc    ");
	if (m & VTAB)
		printf("vtab   ");
	if (m & FLUSH)
		printf("flush  ");
	if (m & FEED)
		printf("feed   ");
	if (m & NOEJECT)
		printf("-eject ");
	if (m & WOPEN)
		printf("busy\n");
	else
		printf("idle\n");
	if (m & POWROFF)
		printf("PRINTER IS POWERED OFF\n");
	else if (m & ERROR)
		printf("ERROR SET IN STATUS REGISTER\n");
}
