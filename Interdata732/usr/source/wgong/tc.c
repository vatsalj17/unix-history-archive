/*
 * Tape commands
 */

char	*tname	"/dev/rmmt0";
#define tdrive	tname[9]

int	tcom[3];

extern	errno;

char usage[]	"Usage: tc [-drive] command [count]\nCommands are:\n";
char *clist[] {
	"ff	Forward Space File",
	"bf	Back Space File",
	"wf	Write File Mark",
	"fr	Forward Space Record",
	"br	Back Space Record",
	"rw	Rewind",
	0
};

struct opt {
	char	*name;
	int	value;
} optab[] {
	"ff",		0,
	"bf",		1,
	"wf",		3,
	"weof",		3,
	"fr",		4,
	"br",		5,
	"rw",		7,
	"rew",		7,
	"rewind",		7,
	0
};
main(argc, argv)
char **argv;
{
	register tf;
	register count;
	register char **pp;

	if (argc > 1 && argv[1][0] == '-') {
		tdrive = argv[1][1];
		argc--;
		argv++;
	}
	if (argc < 2 || (tcom[0] = lookup(argv[1])) < 0) {
		printf(usage);
		for (pp = clist; *pp; pp++)
			printf("\t%s\n", *pp);
		return;
	}

	if ((tf = open(tname, 0)) < 0) {
		printf("Can't open %s\n", tname);
		exit(1);
	}
	if (argc < 3 || tcom[0] == 7 ||
	    (count = atoi(argv[2])) <= 0)
		count = 1;

	while (count--) {
		stty(tf, tcom);
		if (errno) {
			perror("Stty");
			exit(1);
		}
	}
}

lookup(arg)
register char *arg;
{
	register struct opt *op;
	register char *name, *p;

	if (*arg == '-')
		arg++;

	for (op = optab; name = op->name; op++) {
		for (p = arg; *p == *name; p++)
			if (*name++ == 0)
				return(op->value);
	}
	return(-1);
}
