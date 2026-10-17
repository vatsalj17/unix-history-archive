/* time command */

long lmuls(), ldivs();

main(argc, argv)
char **argv;
{
	struct {
		long user;
		long sys;
		long childuser;
		long childsys;
	} buffer;
	char status[2];
	register p;
	register i;
	long before, after;

	if(argc<=1)
		return;
	time(&before);
	p = fork();
	if(p == -1) {
		prints("Try again.\n");
		return;
	}
	if(p == 0) {
		argv[argc] = 0;
		if(argv[argc-1] == -1)
			argv[argc-1] = -2;
		pexec(argv[1], &argv[1]);
		exit(1);
	}
	signal(2, 1);
	while(wait(status) != p);
	time(&after);
	if(status[0] != '\0')
		prints("Command terminated abnormally.\n");
	times(&buffer);
	prints("\n");
	printt("real", lmuls(after-before, 60));
	printt("user", buffer.childuser);
	printt("sys ", buffer.childsys);
	exit(0);
}

char quant[] { 6, 10, 10, 6, 10, 6, 10, 10, 10 };
char *pad  "000      ";
char *sep  "\0\0.\0:\0:\0\0";
char *nsep "\0\0.\0 \0 \0\0";

printt(s, a)
char *s;
long a;
{
	char digit[9];
	long b;
	register i;
	int c;
	int nonzero;

	for(i=0; i<9; i++) {
		b = ldivs(a, quant[i]);
		digit[i] = a - lmuls(b, quant[i]);
		a = b;
	}
	prints(s);
	nonzero = 0;
	while(--i>0) {
		c = digit[i]!=0 ? digit[i]+'0':
		    nonzero ? '0':
		    pad[i];
		prints(&c);
		nonzero =| digit[i];
		c = nonzero?sep[i]:nsep[i];
		prints(&c);
	}
	prints("\n");
}

prints(s)
char *s;
{
	while(*s)
	write(2, s++, 1);
}

/* long=long*short, long=long/short, to avoid floating point */
struct { int hi, lo; };

long lmuls(l, s)
long l;
{
	long p;
	if(l.lo<0) l.hi++;
	p.hi = l.hi*s;
	p.lo = l.lo*s;
	p.hi =+ hmul(l.lo, s);
	return(p);
}

long ldivs(l, s)
long l;
{
	long q;
	long d;
	long mone;
	mone.hi = 0;
	mone.lo = 32768;
	d = lmuls(mone, s);
	for(q=0; l>=d; q=+mone)
		l =- d;
	q =+ ldiv(l.hi, l.lo, s);
	return(q);
}
