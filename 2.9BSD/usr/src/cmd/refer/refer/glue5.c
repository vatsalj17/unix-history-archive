# include "stdio.h"
# include "ctype.h"
/*
 * fgrep -- print all lines containing any of a set of keywords
 *
 *	status returns:
 *		0 - ok, and some matches
 *		1 - ok, but no matches
 */
#define	MAXSIZ 700
#define QSIZE 400
struct words {
	char 	inp;
	char	out;
	struct	words *nst;
	struct	words *link;
	struct	words *fail;
} 
*www, *smax, *q;

char	buf[1024];
int	nsucc;
int	need;
char	*instr;
int	inct;
int	rflag;
int	xargc;
char	**xargv;
int	numwords;
int	found;
static int flag 0;
int	grepin = 0, grepout = 1;


fgrep(argc, argv)
char **argv;
{
	instr = nsucc = need = inct = rflag = numwords = found = 0;
	flag = 0;
	if (www==0)
		www = zalloc(MAXSIZ, sizeof (*www));
	if (www==NULL)
		err("Can't get space for machines", 0);
	for (q=www; q<www+MAXSIZ; q++)
		q->inp = q->out = q->nst = q->link = q->fail =0;
	xargc = argc-1;
	xargv = argv+1;
	while (xargc>0 && xargv[0][0]=='-')
	{
		switch(xargv[0][1])
		{
		case 'r': /* return value only */
			rflag++;
			break;
		case 'n': /* number of answers needed */
			need = xargv[1];
			xargv++; 
			xargc--;
			break;
		case 'i':
			instr = xargv[1];
			inct = xargv[2]+2;
# if D2
			fprintf(stderr,"inct %d xargv.2. %o %d\n",inct, xargv[2],xargv[2]);
# endif
			xargv += 2; 
			xargc -= 2;
			break;
		}
		xargv++; 
		xargc--;
	}
	if (xargc<=0)
		err("bad fgrep call", 0);
	cgotofn();
	cfail();
	execute();
	return(nsucc == 0);
}

execute()
{
	register char *p;
	register c;
	register ch;
	register ccount;
	char *nlp;
	ccount = instr ? inct : 0;
	found=0;
	p = instr ? instr : buf;
	if (need == 0) need = numwords;
	nlp = p;
	c = www;
	for (;;) {
		if (--ccount <= 0) {
			if (instr) break;
			if (p == &buf[1024]) p = buf;
			if (p > &buf[512]) {
				if ((ccount = read(grepin, p, &buf[1024] - p)) <= 0) break;
			}
			else if ((ccount = read(grepin, p, 512)) <= 0) break;
		}
nstate:
		ch = *p;
		if (isupper(ch)) ch |= 040;
		if (c->inp == ch) {
			c = c->nst;
		}
		else if (c->link != 0) {
			c = c->link;
			goto nstate;
		}
		else {
			c = c->fail;
			if (c==0) {
				c = www;
istate:
				if (c->inp == ch) {
					c = c->nst;
				}
				else if (c->link != 0) {
					c = c->link;
					goto istate;
				}
			}
			else goto nstate;
		}
		if (c->out && new (c)) {
			if (++found >= need)
			{
				if (instr==0)
					while (*p++ != '\n') {
						if (--ccount <= 0) {
							if (p == &buf[1024]) p = buf;
							if (p > &buf[512]) {
								if ((ccount = read(grepin, p, &buf[1024] - p)) <= 0) break;
							}
							else if ((ccount = read(grepin, p, 512)) <= 0) break;
						}
					}
				nsucc = 1;
				if (rflag==0)
				{
					if (p > nlp) write(grepout, nlp, p-nlp);
					else {
						write(grepout, nlp, &buf[1024] - nlp);
						write(grepout, buf, p-&buf[0]);
					}
					if (p[-1]!= '\n') write (1, "\n", 1);
				}
				if (instr==0)
				{
					nlp = p;
					c = www;
					found=0; 
				}
			}
			else
				ccount++;
			continue;
		}
		if (instr)
			p++;
		else
			if (*p++ == '\n')
			{
				nlp = p;
				c = www;
				found=0;
			}
	}
	if (instr==0)
		close(grepin);
}

cgotofn() {
	register c;
	register s;
	s = smax = www;
nword:	
	for(;;) {
		if ((c = gch())==0) return;
		else if (c == '\n') {
			s->out = 1;
			s = www;
		}
		else {
loop:	
			if (s->inp == c) {
				s = s->nst;
				continue;
			}
			if (s->inp == 0) goto enter;
			if (s->link == 0) {
				if (smax >= &www[MAXSIZ - 1]) overflo();
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
		s->inp = c;
		if (smax >= &www[MAXSIZ - 1]) overflo();
		s->nst = ++smax;
		s = smax;
	} 
	while ((c = gch()) != '\n');
	smax->out = 1;
	s = www;
	numwords++;
	goto nword;

}

gch()
{
	static char *s;
	if (flag==0)
	{
		flag=1;
		s = *xargv++;
		if (xargc-- <=0) return(0);
	}
	if (*s) return(*s++);
	for(flag=0; flag<1024; flag++)
		buf[flag]=0;
	flag=0;
	return('\n');
}

overflo() {
	write(2,"wordlist too large\n", 19);
	exit(2);
}
cfail() {
	struct words *queue[QSIZE];
	struct words **front, **rear;
	struct words *state;
	register char c;
	register s;
	s = www;
	front = rear = queue;
init:	
	if ((s->inp) != 0) {
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
cloop:	
		if ((c = s->inp) != 0) {
			*rear = (q = s->nst);
			if (front < rear)
				if (rear >= &queue[QSIZE-1])
					if (front == queue) overflo();
					else rear = queue;
				else rear++;
			else
				if (++rear == front) overflo();
			state = s->fail;
floop:	
			if (state == 0) state = www;
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
static int seen[50];
new (x)
{
	int i;
	for(i=0; i<found; i++)
		if (seen[i]==x)
			return(0);
	seen[i]=x;
	return(1);
}
