/* su -- become super-user */

char	password[100];
char	pwbuf[100];
int	ttybuf[3];
main()
{
	register char *p, *q;

	if(getpw("root", pwbuf))
		goto badpw;
	p = pwbuf;
	while(*p != ':')
		if(*p++ == '\0')
			goto badpw;
	if(*++p == ':')
		goto ok;
	gtty(0, ttybuf);
	ttybuf[2] =& ~010;
	stty(0, ttybuf);
	printf("password: ");
	q = password;
	while((*q = getchar()) != '\n')
		if(*q++ == '\0')
			return;
	*q = '\0';
	ttybuf[2] =| 010;
	stty(0, ttybuf);
	printf("\n");
	q = crypt(password);
	while(*q++ == *p++);
	if(*--q == '\0' && *--p == ':')
		goto ok;
	goto error;

badpw:
	printf("bad password file\n");
ok:
	setuid(0);
	execl("/bin/sh", "-", 0);
	printf("cannot execute shell\n");
error:
	printf("sorry\n");
}
getpw(name, buf)
char *name, *buf;
{
	static int pwbuff[131];
	int r, c;
	register char *gnp, *rnp;

	r = 1;
	if((pwbuff[0] = open("/etc/passwd", 0)) < 0)
		return(1);
loop:
	gnp = name;
	rnp = buf;
	while((c=getc(pwbuff)) != '\n') {
		if(c <= 0)
			goto ret;
		*rnp++ = c;
	}
	*rnp++ = '\0';
	rnp = buf;
	while (*gnp++ == *rnp++);
	if (*--gnp != '\0' || *--rnp != ':')
		goto loop;
	r = 0;
ret:
	close(pwbuff[0]);
	pwbuff[1] = 0;
	pwbuff[2] = 0;
	return(r);
}
