#
/*
 * enter a password in the password file
 * this program should be suid with owner
 * with an owner with write permission on/etc/passwd
 */
char	*tfile	{ "/tmp/ptmp" };
char	*pfile	{ "/etc/passwd" };
int	tbuf[131];		/***/
int	pbuf[131];		/***/

char	*uname;		/*** user name ***/
char	*upass;		/*** new password ***/
int	echoflg;	/*** turn off echo while prompting ***/

main(argc, argv)
char **argv;
{
	register u, c;
	register char *p;

	if (argc > 1 && argv[1][0] == '-' && argv[1][1] == 'e') {
		echoflg++;
		argc--;
		argv++;
	}
	if (argc == 1)
		getpass();
	else if(argc != 3) {
		write(2, "Usage: passwd [user password]\n", 30);
		goto bex;
	}
	else {
		uname = argv[1];
		upass = argv[2];
	}
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
	p = uname;
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
	p = upass;
	for(c=0; c<9; c++)
		if(*p++ == 0)
			break;
	*--p = 0;
	if(p != upass)
		p = crypt(upass);
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
	if(c != 0 &&c != u) {
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

/*
 * Prompt for current user's password
 */
char npass[128];
struct {
	char	name[8];
	char	tty;
	char junk[15];
} utbuf;

#define ECHO	010

getpass()
{
	register char *p;
	register fd, ttysave, tty;
	int ttybuf[3];

	/* First find out who it is */
	if ((tty = ttyn(0)) == 'x' ||
	  (fd = open("/etc/utmp", 0)) < 0)
		return(1);
	while (read(fd, &utbuf, sizeof(utbuf)) == sizeof(utbuf))
		if (utbuf.tty == tty) {
			close(fd);
			uname = utbuf.name;
			for (p = uname; p < &uname[8] && *p != ' '; p++)
				;
			*p = '\0';
			/* Prompt for new password (noecho if echoflg set) */
			if (echoflg) {
				gtty(0, &ttybuf);
				ttysave = ttybuf[2];
				ttybuf[2] = ttysave & ~ECHO;
				stty(0, &ttybuf);
			}
			write(0, "New Password: ", 14);
			read(0, npass, sizeof npass);
			upass = npass;
			for (p = npass;
			  p < &npass[sizeof npass -1] && *p != '\n'; p++)
				;
			*p = '\0';
			if (echoflg) {
				ttybuf[2] = ttysave;
				stty(0, &ttybuf);
				write(0, "\n", 1);
			}
			return(0);
		}
	close(fd);
	return(1);
}
