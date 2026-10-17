int	ibuf1[259];
int	ibuf2[259];
int	eflg;
int	lflg	1;
long	line	1;
long	chr	0;
long	skip1;
long	skip2;

long	otoi();

main(argc, argv)
char **argv;
{
	register c1, c2;
	char *arg;

	if(argc < 3)
		goto narg;
	arg = argv[1];
	if(arg[0] == '-' && arg[1] == 's') {
		lflg--;
		argv++;
		argc--;
	}
	arg = argv[1];
	if(arg[0] == '-' && arg[1] == 'l') {
		lflg++;
		argv++;
		argc--;
	}
	if(argc < 3)
		goto narg;
	arg = argv[1];
	if( arg[0]=='-' && arg[1]==0 )
		ibuf1[0] = dup(0);
	else if(fopen(arg, ibuf1) < 0)
		goto barg;
	arg = argv[2];
	if(fopen(arg, ibuf2) < 0)
		goto barg;
	if (argc>3)
		skip1 = otoi(argv[3]);
	if (argc>4)
		skip2 = otoi(argv[4]);
	while (skip1) {
		if ((c1 = getc(ibuf1)) < 0) {
			arg = argv[1];
			goto earg;
		}
		skip1--;
	}
	while (skip2) {
		if ((c2 = getc(ibuf2)) < 0) {
			arg = argv[2];
			goto earg;
		}
		skip2--;
	}

loop:
	chr++;
	c1 = getc(ibuf1);
	c2 = getc(ibuf2);
	if(c1 == c2) {
		if (c1 == '\n')
			line++;
		if(c1 == -1) {
			if(eflg)
				exit(1);
			exit(0);
		}
		goto loop;
	}
	if(lflg == 0)
		exit(1);
	if(c1 == -1) {
		arg = argv[1];
		goto earg;
	}
	if(c2 == -1)
		goto earg;
	if(lflg == 1) {
		printf("%s %s differ: char %ld, line %ld\n", argv[1], arg,
			chr, line);
		exit(1);
	}
	eflg = 1;
	printf("%6ld %3o %3o\n", chr, c1, c2);
	goto loop;

narg:
	printf("arg count\n");
	exit(2);

barg:
	if (lflg)
	printf("cannot open %s\n", arg);
	exit(2);

earg:
	printf("EOF on %s\n", arg);
	exit(1);
}

putchar(c)
{

	write(2, &c, 1);
}

long otoi(s)
char *s;
{
	long v;
	int base;

	v = 0;
	base = 10;
	if (*s == '0')
		base = 8;
	while (*s>='0' && *s<='9')
		v = v*base + *s++ - '0';
	return(v);
}
