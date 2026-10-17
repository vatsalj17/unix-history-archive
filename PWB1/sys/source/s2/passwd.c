/*
 * enter a password in the password file
 * this program should be suid with owner
 * with an owner with write permission on /etc/passwd
 */
char	*tfile	{ "/etc/ptmp" };
char	*pfile	{ "/etc/passwd" };
char	*pw;
char	pwbuf[10];
int	ttybuf[3];
int	tbuf[259];
int	pbuf[259];

main(argc, argv)
char *argv[];
{
	register u, c;
	register char *p;

	if(argc < 2) {
		write(2, "Usage: passwd user password\n", 28);
		goto bex;
	}
	if(argc == 2) {
		gtty(0, ttybuf);
		ttybuf[2] =& ~010;
		stty(0, ttybuf);
		write(2, "Password: ", 10);
		p = pwbuf;
		for(;;) {
			if(read(0, p, 1) != 1)
				break;
			if(*p == '\n')
				break;
			if(p < pwbuf+9)
				p++;
		}
		*p = 0;
		ttybuf[2] =| 010;
		stty(0, ttybuf);
		write(2, "\n", 1);
		pw = pwbuf;
	} else
		pw = argv[2];
	signal(1, 1);
	signal(2, 1);
	signal(3, 1);

	if(stat(tfile, tbuf+20) >= 0) {
		write(2, "Temporary file busy -- try again\n", 33);
		goto bex;
	}
	tbuf[0] = creat(tfile, 0600);
	if(tbuf[0] < 0) {
		write(2, "Cannot create temporary file\n", 29);
		goto bex;
	}
	pbuf[0] = open(pfile, 0);
	if(pbuf[0] < 0) {
		write(2, "Cannot open /etc/passwd\n", 24);
		goto out;
	}
	goto l1;

/*
 * skip to beginning of next line
 */

skip:
	while(c != '\n') {
		if(c < 0)
			goto ill;
		c = getc(pbuf);
		putc(c, tbuf);
	}

/*
 * compare user names
 */

l1:
	c = getc(pbuf);
	putc(c, tbuf);
	if(c < 0) {
		write(2, "User name not found in password file\n", 37);
		goto out;
	}
	p = argv[1];
	while(c != ':') {
		if(*p++ != c)
			goto skip;
		c = getc(pbuf);
		putc(c, tbuf);
	}
	if(*p)
		goto skip;
/*
 * skip old password
 */
	do {
		c = getc(pbuf);
		if(c < 0)
			goto ill;
	} while(c != ':');

/*
 * copy in new password
 */
	p = pw;
	for(c=0; c<9; c++)
		if(*p++ == 0)
			break;
	*--p = 0;
	if(p != pw)
		p = crypt(pw);
	while(*p)
		putc(*p++, tbuf);
	putc(':', tbuf);

/*
 * validate uid
 */

	u = 0;
	do {
		c = getc(pbuf);
		putc(c, tbuf);
		if(c >= '0' && c <= '9')
			u = u*10 + c-'0';
		if(c < 0)
			goto ill;
	} while(c != ':');
	c = getuid() & 0377;
	if(c != 0 && c != u) {
		write(2, "Permission denied\n", 18);
		goto out;
	}

/*
 * copy out and back
 */

	for(;;) {
		c = getc(pbuf);
		if(c < 0) {
			fflush(tbuf);
			close(pbuf[0]);
			close(tbuf[0]);
			tbuf[0] = open(tfile, 0);
			if(tbuf[0] < 0) {
				write(2, "Urk\n", 4);
				goto out;
			}
			pbuf[0] = creat(pfile, 0644);
			if(pbuf[0] < 0) {
				write(2, "Cannot create /etc/passwd\n", 26);
				goto out;
			}
			while((c = read(tbuf[0], tbuf+1, 512)) > 0)
				write(pbuf[0], tbuf+1, c);
			unlink(tfile);
			c = open("/usr/adm/pwchng", 1);
			seek(c, 0, 2);
			while(*pw)
				write(c, pw++, 1);
			*pw = ':';
			write(c, pw, 1);
			decml(c, getuid()&0377);
			write(c, pw, 1);
			decml(c, u);
			*pw = '\n';
			write(c, pw, 1);
			exit(0);
		}
		putc(c, tbuf);
	}

ill:
	write(2, "Password file illformed\n", 24);

out:
	unlink(tfile);

bex:
	exit(1);
}

decml(f, n)
{
	int d;

	if (d = n/10)
		decml(f, d);
	n =% 10;
	n =+ '0';
	write(f, &n, 1);
}
