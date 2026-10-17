/*
 * write to another user
 */

struct
{
	char	name[8];
	char	ttyno;
};
struct
{
	char	pad[4];
	int	flag;
};
char	me[10]	"???";
char	tty[]	"/dev/ttyX";
char	*him;
int	ttyno;
int	mytty;
int	histty;
int	logcnt;
char	buf[512];
int	tf;
int	eof();
int	timout();

main(argc, argv)
char *argv[];
{
	register i;
	int c1, c2, uf;

	if(argc < 2) {
		prs(1, "usage: write user [ttyno]\n");
		exit(1);
	}
	him = argv[1];
	if(argc > 2)
		ttyno = argv[2][0];
	uf = open("/etc/utmp", 0);
	if(uf < 0) {
		prs(1, "cannot open /etc/utmp\n");
		goto cont;
	}
	mytty = ttyn(1);
	while(read(uf, buf, 16) == 16) {
		if(mytty == buf->ttyno)
			for(i=0; i<8; i++) {
				c1 = buf->name[i];
				if(c1 == ' ')
					c1 = 0;
				me[i] = c1;
				if(c1 == 0)
					break;
			}
		if(him[0] != '-' || him[1] != 0)
		for(i=0; i<8; i++) {
			c1 = him[i];
			c2 = buf->name[i];
			if(c1 == 0)
				if(c2 == 0 || c2 == ' ')
					break;
			if(c1 != c2)
				goto nomat;
		}
		logcnt++;
		c1 = buf->ttyno;
		if(ttyno != 0 && ttyno != c1)
			continue;
		if(histty == 0)
			histty = c1;
	nomat:
		;
	}
cont:
	close(uf);
	if(ttyno == 0 && logcnt > 1) {
		prs(1, him);
		prs(1, " logged in more than once\n");
		prs(1, "writing to tty ");
		prs(1, &histty);
		prs(1, "\n");
	}
	if(histty == 0) {
		prs(1, him);
		if(logcnt)
			prs(1, " not on that tty\n"); else
			prs(1, " not logged in\n");
		exit(1);
	}
	tty[8] = histty;
	signal(14, timout);
	alarm(5);
	tf = open(tty, 1);
	alarm(0);
	if(tf < 0)
		goto perm;
	if(fstat(tf, buf) < 0)
		goto perm;
	if((buf->flag & 2) == 0)
		goto perm;
	sigs(eof);
	prs(tf, "\n Message from ");
	prs(tf, me);
	prs(tf, " ");
	prs(tf, &mytty);
	prs(tf, "...\n");
	for(;;) {
		i = read(0, buf, 512);
		if(i <= 0)
			eof();
		if(buf[0] == '!') {
			buf[i] = 0;
			ex();
			continue;
		}
		write(tf, buf, i);
	}

perm:
	prs(1, "Permission denied\n");
	exit(1);
}

timout()
{

	prs(1, "Timeout opening his tty\n");
	exit(1);
}

eof()
{

	prs(tf, "EOF\n");
	exit(0);
}

prs(f, s)
char *s;
{
	register n;

	for(n=0; s[n]; n++)
		;
	write(f, s, n);
}

ex()
{
	register i;

	sigs(1);
	i = fork();
	if(i < 0) {
		prs(1, "Try again\n");
		goto out;
	}
	if(i == 0) {
		sigs(0);
		execl("/bin/sh", "sh", "-c", buf+1, 0);
		exit(0);
	}
	while(wait(buf) != i)
		;
	prs(1, "!\n");
out:
	sigs(eof);
}

sigs(sig)
{
	register i;

	for(i=1; i<4; i++)
		signal(i, sig);
}
