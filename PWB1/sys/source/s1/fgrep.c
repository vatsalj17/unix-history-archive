#
/*
 * fgrep -- print all lines containing any of a set of keywords
 *
 *	status returns:
 *		0 - ok, and some matches
 *		1 - ok, but no matches
 *		2 - some error
 */
#define	MAXSIZ 6000
#define QSIZE 400
struct words {
	char 	inp;
	char	out;
	struct	words *nst;
	struct	words *link;
	struct	words *fail;
} w[MAXSIZ], *smax, *q;

char	buf[1024];
long	lnum;
int	aflag, bflag, nflag, cflag, vflag, fflag;
int	nfile;
int	blkno;
int	nsucc;
long	tln;
int	xargc;
char	**xargv;
int	numchars;
int	numwords;
int	numprefs;
main(argc, argv)
char **argv;
{

	xargc = argc;
	xargv = argv;
	while (--xargc > 0 && (++xargv)[0][0]=='-')
		switch (xargv[0][1]) {

		case 'a':
			aflag++;
			continue;

		case 'v':
			vflag++;
			continue;

		case 'b':
			bflag++;
			continue;

		case 'c':
			cflag++;
			continue;

		case 'n':
			nflag++;
			continue;

		case 'f':
			fflag++;
			continue;

		default:
			printf2("Unknown flag\n");
			continue;
		}
	if (xargc<=0)
		exit(2);
	cgotofn();
	cfail();
	if (aflag) {
		printf("no. of entries = %d\n", smax - w + 1);
		printf("no. of characters = %d\n", numchars+numprefs);
		printf("no. of distinct words = %d\n", numwords);
		printf("no. of prefixes = %d\n", numprefs);
	}
	nfile = xargc;
	if (xargc<=0)
		execute(0);
	else while (--xargc >= 0) {
		execute(*xargv);
		xargv++;
	}
	flush();
	exit(nsucc == 0);
}

execute(file)
char *file;
{
	register char *p;
	register c;
	register ccount;
	int f;
	char *nlp;
	int istat;
	if (file) {
		if ((f = open(file, 0)) < 0) {
			printf2("Can't open %s\n", file);
		}
	}
	else f = 0;
	ccount = 0;
	lnum = 1;
	tln = 0;
	blkno = -1;
	p = buf;
	nlp = p;
	c = w;
	for (;;) {
		if (--ccount <= 0) {
			if (p == &buf[1024]) p = buf;
			if (p > &buf[512]) {
				if ((ccount = read(f, p, &buf[1024] - p)) <= 0) break;
			}
			else if ((ccount = read(f, p, 512)) <= 0) break;
			blkno++;
		}
		nstate:
			if (c->inp == *p) {
				c = c->nst;
			}
			else if (c->link != 0) {
				c = c->link;
				goto nstate;
			}
			else {
				c = c->fail;
				if (c==0) {
					c = w;
					istate:
					if (c->inp == *p) {
						c = c->nst;
					}
					else if (c->link != 0) {
						c = c->link;
						goto istate;
					}
				}
				else goto nstate;
			}
		if (c->out) {
		found:	while (*p++ != '\n') {
				   if (--ccount <= 0) {
					if (p == &buf[1024]) p = buf;
					if (p > &buf[512]) {
						if ((ccount = read(f, p, &buf[1024] - p)) <= 0) break;
					}
					else if ((ccount = read(f, p, 512)) <= 0) break;
					blkno++;
				   }
			}
			if (vflag == 0) {
		succeed:	nsucc = 1;
				if (cflag) tln++;
				else {
					if (nfile > 1) printf("%s:", file);
					if (bflag) printf("%d:", blkno);
					if (nflag) printf("%ld:", lnum);
					if (p > nlp) write(1, nlp, p-nlp);
					else {
						write(1, nlp, &buf[1024] - nlp);
						write(1, buf, p-&buf[0]);
					}
				}
			}
			lnum++;
			nlp = p;
			c = w;
			continue;
		}
		if (*p++ == '\n')
			if (vflag) goto succeed;
			else {
				lnum++;
				nlp = p;
				c = w;
			}
	}
	close(f);
	if (cflag) {
		if (nfile > 1)
			printf("%s:", file);
		printf("%ld\n", tln);
	}
}

printf2(s, a) {
	extern fout;
	flush();
	fout = 2;
	printf(s, a);
	flush();
	exit(2);
}
cgotofn() {
	register char *pbuf;
	register c;
	register s;
	char *ebuf;
	int f;
	s = smax = w;
	if (fflag) {
		if ((f = open(*xargv,0)) < 0)
			printf2("Can't open %s\n", *xargv);
		if ((c = read(f,buf,512)) <= 0) {
			close(f);
			printf2("File %s empty\n", *xargv);
		}
		else {
			pbuf = buf;
			ebuf = buf + c;
		}
	}
	else {
		pbuf = *xargv;
		while (c = *pbuf++) {
			s->inp = c;
			numchars++;
			if (smax >= &w[MAXSIZ - 1]) overflo();
			s->nst = ++smax;
			s = smax;
		}
		numwords++;
		smax->out = 1;
		xargc--;
		xargv++;
		return;
	}
	xargc--;
	xargv++;
nword:	for(;;) {
		if (pbuf >= ebuf) {
			if ((c = read(f, buf, 512)) <= 0)  {
				close(f);
				return;
			}
			pbuf = buf;
			ebuf = buf + c;
		}
		if ((c = *pbuf++)==0) continue;
		else if (c == '\n') {
			s->out = 1;
			s = w;
			numprefs++;
		}
		else {
			numchars++;
		loop:	if (s->inp == c) {
				s = s->nst;
				continue;
			}
			if (s->inp == 0) goto enter;
			if (s->link == 0) {
				if (smax >= &w[MAXSIZ - 1]) overflo();
				s->link = ++smax;
				s = smax;
				goto enter;
			}
			s = s->link;
			goto loop;
		}
	}

	enter:
	do {
		numchars++;
		s->inp = c;
		if (smax >= &w[MAXSIZ - 1]) overflo();
		s->nst = ++smax;
		s = smax;
		if (pbuf >= ebuf) {
			if ((c = read(f, buf, 512)) <= 0) {
				close(f);
				return;
			}
			pbuf = buf;
			ebuf = buf + c;
		}
	} while ((c = *pbuf++) != '\n');
	smax->out = 1;
	s = w;
	numwords++;
	goto nword;

}

overflo() {
	extern fout;
	flush();
	fout = 2;
	printf("smax = %d\n", smax - w + 1);
	printf("wordlist too large\n");
	flush();
	exit(2);
}
cfail() {
	struct words *queue[QSIZE];
	struct words **front, **rear;
	struct words *state;
	register char c;
	register s;
	s = w;
	front = rear = queue;
init:	if ((s->inp) != 0) {
		*rear++ = s->nst;
		if (rear >= &queue[QSIZE - 1]) overflo();
	}
	if ((s = s->link) != 0) {
		goto init;
	}

	while (rear!=front) {
		s = *front;
		if (front == &queue[QSIZE-1])
			front = queue;
		else front++;
	cloop:	if ((c = s->inp) != 0) {
			*rear = (q = s->nst);
			if (front < rear)
				if (rear >= &queue[QSIZE-1])
					if (front == queue) overflo();
					else rear = queue;
				else rear++;
			else
				if (++rear == front) overflo();
			state = s->fail;
		floop:	if (state == 0) state = w;
			if (state->inp == c) {
				q->fail = state->nst;
				if ((state->nst)->out == 1) q->out = 1;
				continue;
			}
			else if ((state = state->link) != 0)
				goto floop;
		}
		if ((s = s->link) != 0)
			goto cloop;
	}
}
