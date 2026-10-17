/* su -- set user id */

char	password[100];
char	pwbuf[100];
int	ttybuf[3];
int	uid;

struct {
	char logname[8];
	char logdir[22];
	char logtty[2];
} u;

main(argc, argv)
char **argv;
{
	register char *p, *q;
	char *nptr;
	int badsw, ttyb2;
	extern fin;

	badsw = 0;
	if (argc>1) 
		nptr = *++argv;
	else
		nptr = "root";
	if ((uid = getpwid(nptr, pwbuf)) == -1)
		badsw++;
	(&fin)[1] = 0;
	p = pwbuf;
	while(*p != ':')
		if(*p++ == '\0')
			goto badpw;
	if(*++p == ':')
		goto ok;
	if((getuid()&0377) == 0)
		goto ok;
	gtty(0, ttybuf);
	ttyb2 = ttybuf[2];
	ttybuf[2] =& ~010;
	stty(0, ttybuf);
	printf("password: ");
	q = password;
	while((*q = getchar()) != '\n')
		if(*q++ == '\0') {
			badsw++;
			break;
		}
	*q = '\0';
	ttybuf[2] = ttyb2;
	stty(0, ttybuf);
	printf("\n");
	if (badsw)
		goto error;
	q = crypt(password);
	while(*q++ == *p++);
	if(*--q == '\0' && *--p == ':')
		goto ok;
	goto error;

badpw:
	printf("bad password file\n");
	exit(2);
ok:
	copyn(logname(), u.logname, sizeof u.logname);
	getlogdir(pwbuf);
	copyn(logtty(), u.logtty, sizeof u.logtty);
	logpost(&u);
	setuid(uid);
	execl("/bin/sh", "-", 0);
	printf("cannot execute shell\n");
error:
	printf("sorry\n");
	exit(1);
}

/*
	getpwid -- return user id from user name
*/

getpwid(name, buf)
char *name;
char buf[];
{
	static pbuf[259], pwf;
	register char c;
	register char *bp, *aptr;
	int n, m;

	if (pwf == 0) {
		if (fopen("/etc/passwd", pbuf) < 0)
			return(-1);
		pwf = pbuf[0];
	} else {
		pbuf[1] = 0;
		pbuf[2] = 0;
		seek(pwf, 0, 0);
	}
	m = 0;
	n = -1;

	for (;;) {
		bp = buf;
		while((c=getc(pbuf)) != '\n') {
			if(c <= '\0')
				goto out;
			*bp++ = c;
		}
		*bp++ = '\0';
		bp = buf;
		aptr = name;
		while ((c = *bp++) != ':')
			if (c != *aptr++)
				goto next;
		if (*aptr!='\0' && *aptr!=' ')
			goto next;
		while ((c = *bp++) != ':')
			if (c == '\0')
				goto out;
		n = 0;
		while ((c = *bp++) != ':')
			n = n*10 + c - '0';
		/*
		 * following makes no sense 
		while ((c = *bp++) != ':')
			m = m*10 + c - '0';
		 */
out:
		return(m<<8 | n);
next:
	continue;
	}
}

/*
 *	getlogdir: scan to end of buffer, then backwards for login directory.
 *	safe regardless of presense of gid field or not.
 */
getlogdir(pstart)
register char *pstart;
{
	register char *p, *pendcolon;
	for (p = pstart; *++p;);
	while (*--p != ':')
		if (p <= pstart)
			return;
	pendcolon = p;
	while (*--p != ':')
		if (p <= pstart)
			return;
	p++;
	copyn(p, u.logdir, pendcolon - p);
}
copyn(source, sink, nbytes)
register char *source, *sink;
int nbytes;
{
	register i;
	for (i = 0; i < nbytes; i++)
		*sink++ = *source++;
}
