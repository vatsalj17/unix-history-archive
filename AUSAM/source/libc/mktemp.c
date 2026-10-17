mktemp(as)
char *as;
{
	register char *s;
	register pid, i;
	int sign;
	int sbuf[20];

	pid = getpid();
	sign = 0;
	while (pid<0) {
		pid =- 10000;
		sign++;
	}
	s = as;
	while (*s++);
	s--;
	i = 0;
	while (*--s == 'X') {
		*s = (pid%10) + '0';
		pid =/ 10;
		if (++i == 5)
			*s =+ sign;
	}
	while (stat(as, sbuf) != -1) {
		if (i==0 || sign>=20)
			return("/");
		*s = 'a' + ++sign;
	}
	return(as);
}
