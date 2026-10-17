/*
 *	cv [-]{bcdho}{bcdhlo} n [n...]
 */

char digits[]	"0123456789abcdef";

main(argc, argv)
char *argv[];
{
	register char *p;
	register ibase, obase, i;

	if (argc < 3)
		error("Arg count");

	p = argv[1];
	if (*p = '-')
		p++;
	ibase = cvbase(*p);
	obase = cvbase(*++p);

	for (i=2; i<argc; putchar(' '))
		putnum(getnum(argv[i++], ibase), obase);
	putchar('\n');
}

cvbase(c)
{
	switch (c) {
	case 'b': case 'B':
		return(2);
	case 'c': case 'C':
		return(1);
	case 'o': case 'O':
		return(8);
	case 'l': case 'L':
		return(10);
	case 'd': case 'D':
		return(0);
	case 'h': case 'H':
		return(16);
	}
	error("Conversion type");
}

error(s)
{
	printf("Error: %s\n", s);
	exit(1);
}

getnum(s,base)
char *s;
{
	register char *p;
	register c, n;

	n = 0;
	if (base == 1) {
		for (p = s; c = *p; p++)
			n = (n<<8) + c;
		return(n);
	}
	if (base == 0)
		base = 10;
	if (*(p = s) == '-')
		p++;
	for (; c = *p; p++) {
		if (c >= 'a' && c <= 'f')
			c =+ '0'+10-'a';
		if (c >= 'A' && c <= 'F')
			c =+ '0'+10-'A';
		if ((c =- '0') < 0 || c > base)
			error("Number syntax");
		n = n*base + c;
	}
	return(*s == '-' ? -n : n);
}

putnum(num, base)
{
	register n, c, i;
	char buff[32];
	extern int ldivr;

	n = num;
	if (base == 0) {
		base = 10;
		if (n < 0) {
			n = -n;
			putchar('-');
		}
	}
	i = 31;
	if (base == 1) {
		while (c = n&0377) {
			buff[i--] = c;
			n =>> 8;
		}
	}
	else do {
		n = ldiv(0, n, base);
		buff[i--] = digits[ldivr];
	} while (n);
	while (++i < 32)
		putchar(buff[i]);
}
