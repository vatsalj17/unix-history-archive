long
atol(ap)
char *ap;
{
	register c;
	register char *p;
	int f;
	long	l;

	p = ap;
	l = 0;
	f = 0;
loop:
	while(*p == ' ' || *p == '	')
		p++;
	if(*p == '-') {
		f++;
		p++;
		goto loop;
	}
	while(*p >= '0' && *p <= '9')
		l = l*10 + *p++ - '0';
	if(f)
		l = -l;
	return(l);
}
