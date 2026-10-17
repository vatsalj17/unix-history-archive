/*
 * dfa stuff mostly stolen from egrep
 *
 *	status returns:
 *		0 - ok, and some matches
 *		1 - ok, but no matches
 *		2 - some error; matches irrelevant
 */
%token CHAR DOT CCL NCCL OR CAT STAR PLUS QUEST
%left OR
%left CHAR DOT CCL NCCL '('
%left CAT
%left STAR PLUS QUEST

%{
#include "priv.h"

#define MAXLIN 1000
#define MAXPOS 10000
#define NCHARS 128
#define NSTATES 512
#define FINAL -1
#define LEFT '\177'	/* serves as ^ */
#define RIGHT '\n'	/* serves as record separator and as $ */

int nstate = 1;
int line  = 1;
short gotofn[NSTATES][NCHARS];
int blkstart;
short state[NSTATES];
char out[NSTATES];
short name[MAXLIN];
short left[MAXLIN];
short right[MAXLIN];
short parent[MAXLIN];
short foll[MAXLIN];
short positions[MAXPOS];
char chars[MAXLIN];
int nxtpos = 0;
int nxtchar = 0;
char tmpstat[MAXLIN];
char initstat[MAXLIN];
int istat;
int xstate;
int count;
int icount;
char *input;


int blkend;
%}

%%
s:	t
		={ unary(FINAL, $1);
		  line--;
		}
	;
t:	b r
		={ $$ = node(CAT, $1, $2); }
	| OR b r OR
		={ $$ = node(CAT, $2, $3); }
	| OR b r
		={ $$ = node(CAT, $2, $3); }
	| b r OR
		={ $$ = node(CAT, $1, $2); }
	;
b:
		={ $$ = enter(DOT);
		   $$ = unary(STAR, $$); }
	;
r:	CHAR
		={ $$ = enter($1); }
	| DOT
		={ $$ = enter(DOT); }
	| CCL
		={ $$ = cclenter(CCL); }
	| NCCL
		={ $$ = cclenter(NCCL); }
	;

r:	r OR r
		={ $$ = node(OR, $1, $3); }
	| r r %prec CAT
		={ $$ = node(CAT, $1, $2); }
	| r STAR
		={ $$ = unary(STAR, $1); }
	| r PLUS
		={ $$ = unary(PLUS, $1); }
	| r QUEST
		={ $$ = unary(QUEST, $1); }
	| '(' r ')'
		={ $$ = $2; }
	| error
	;

%%
#define nextch()	( *input++ )
char *retext;
yylex()
{
	extern int yylval;
	int cclcnt, x;
	register char c, d;
	switch(c = nextch()) {
		case '^': c = LEFT;
			goto defchar;
		case '$': c = RIGHT;
			goto defchar;
		case '|': return (OR);
		case '*': return (STAR);
		case '+': return (PLUS);
		case '?': return (QUEST);
		case '(': return (c);
		case ')': return (c);
		case '.': return (DOT);
		case '\0': return (0);
		case RIGHT: return (OR);
		case '[':
			x = CCL;
			cclcnt = 0;
			count = nxtchar++;
			if ((c = nextch()) == '^') {
				x = NCCL;
				c = nextch();
			}
			do {
				if (c == '\0')
					error("unmatched [ in RE: %s", retext);
				if (c == '-' && cclcnt > 0 && chars[nxtchar-1] != 0) {
					if ((d = nextch()) != 0) {
						c = chars[nxtchar-1];
						while (c < d) {
							if (nxtchar >= MAXLIN) overflo();
							chars[nxtchar++] = ++c;
							cclcnt++;
						}
						continue;
					}
				}
				if (nxtchar >= MAXLIN) overflo();
				chars[nxtchar++] = c;
				cclcnt++;
			} while ((c = nextch()) != ']');
			chars[count] = cclcnt;
			sigmaset(&chars[count+1], cclcnt);
			return (x);
		case '\\':
			if ((c = nextch()) == '\0')
				error("terminal \\ in RE: %s", retext);
		defchar:
		default: yylval = c;
			{char cc = c; sigmaset(&cc, 1);}
		return (CHAR);
	}
}
/*
nextch() {
	return *input++;
}
*/

enter(x) int x; {
	if(line >= MAXLIN) overflo();
	name[line] = x;
	left[line] = 0;
	right[line] = 0;
	return(line++);
}

cclenter(x) int x; {
	register linno;
	linno = enter(x);
	right[linno] = count;
	return (linno);
}

node(x, l, r) {
	if(line >= MAXLIN) overflo();
	name[line] = x;
	left[line] = l;
	right[line] = r;
	parent[l] = line;
	parent[r] = line;
	return(line++);
}

unary(x, d) {
	if(line >= MAXLIN) overflo();
	name[line] = x;
	left[line] = d;
	right[line] = 0;
	parent[d] = line;
	return(line++);
}
overflo() {
	error("RE too long: %s", retext);
}
#define add(p,n) p[n]=nxtpos;Add()
cfoll(v)
register v;
{
	register char *tp;
	register i;
again:
	if (left[v] == 0) {
		count = 0;
		tp = &tmpstat[1];
		i = line+1;
		while(--i)
			*tp++ = 0;
		follow(v);
		add(foll, v);
	}
	else if (right[v] == 0) {
		v = left[v];
		goto again;
	}
	else {
		cfoll(left[v]);
		v = right[v];
		goto again;
	}
}
cgotofn() {
	register i;
	count = 0;
	for (i=3; i<=line; i++) tmpstat[i] = 0;
	if (cstate(line-1)==0) {
		tmpstat[line] = 1;
		count++;
		out[1] = 1;
	}
	for (i=3; i<=line; i++) initstat[i] = tmpstat[i];
	count--;		/*leave out position 1 */
	icount = count;
	tmpstat[1] = 0;
	add(state, 1);
	istat = nxtst(1,LEFT);
}


nxtst(s,c)
register char c;
{
	register short *pp;
	register num;

	pp = &positions[state[s]];
	num = *pp++;
	count = icount;
	if(line>2)
		memcpy((char*)&tmpstat[3], (char*)&initstat[3], sizeof(tmpstat[0])*(line-2));
	while(num--) {
		register curpos, k;

		curpos = *pp++;
		if ((k = name[curpos]) >= 0)
			if (
				(k == c)
				|| (k==DOT && c!=LEFT && c!=RIGHT)
				|| (k==CCL && member(c, right[curpos]))
				|| (k==NCCL && c!=LEFT && c!=RIGHT && !member(c, right[curpos]))
			) {
				register short *np = &positions[foll[curpos]];

				k = 1+ *np++;
				while(--k) {
					if (!tmpstat[*np]) {
						tmpstat[*np] = 1;
						count++;
					}
					np++;
				}
			}
	}
	if (notin()) {
		if (++nstate >= NSTATES) {
			error("internal error: NSTATES too small");
		}
		add(state, nstate);
		if (tmpstat[line]) out[nstate] = 1;
		gotofn[s][c] = nstate;
		return nstate;
	}
	else {
		gotofn[s][c] = xstate;
		return xstate;
	}
}


cstate(v)
register v;
{
	register b;
	if (left[v] == 0) {
		if (!tmpstat[v] ) {
			tmpstat[v] = 1;
			count++;
		}
		return(1);
	}
	else if (right[v] == 0) {
		if (cstate(left[v]) == 0) return (0);
		else if (name[v] == PLUS) return (1);
		else return (0);
	}
	else if (name[v] == CAT) {
		if (cstate(left[v]) == 0 && cstate(right[v]) == 0) return (0);
		else return (1);
	}
	else { /* name[v] == OR */
		b = cstate(right[v]);
		if (cstate(left[v]) == 0 || b == 0) return (0);
		else return (1);
	}
}


member(symb, set) {
	register i, num, pos;
	num = chars[set];
	pos = set + 1;
	for (i=0; i<num; i++)
		if (symb == chars[pos++]) return 1;
	return 0;
}

notin() {
	register short *pp, *sp;
	register n = nstate, i, j;
	sp = &state[1];
	for (i=1; i<=n; i++) {
		pp = &positions[*sp++];
		if (*pp++ == count) {
			j = 1+count;
			while(--j) {
				if (!tmpstat[*pp++ ] )
					goto nxt;
			}
			xstate = i;
			return (0);
		}
		nxt: ;
	}
	return (1);
}

/*
add(array, n)
short *array;
*/
Add()
{
	register short *pp;
	register char *tp;
	register i;

	if (nxtpos + count >= MAXPOS) overflo();
	/*array[n] = nxtpos;*/
	pp = &positions[nxtpos++];
	tp = &tmpstat[3];
	*pp++ = count;
	for (i=3; i <= line; i++)
		if ( *tp++ )
			*pp++ = i;
	nxtpos = pp - positions;
}

follow(v)
register int v;
{
	register int p;
again:
	if (v == line) return;
	p = parent[v];
	switch(name[p]) {
		case STAR:
		case PLUS:	cstate(v);
				v = p;
				goto again;
				/* follow(p);
				return; */

		case OR:
		case QUEST:
				v = p;
				goto again;
				/* follow(p);
				return; */

		case CAT:	if (v == left[p]) {
					if (cstate(right[p]) == 0) {
						v = p;
						goto again;
						/* follow(p);
						return; */
					}
				}
				else {
					v = p;
					goto again;
					/* follow(p);
					return; */
				}
				return;
		case FINAL:	if (!tmpstat[line] ) {
					tmpstat[line] = 1;
					count++;
				}
				return;
	}
}


/*
 * newstate = m->delta[currchar][oldstate]
 */
#define NG	(3*NCHARS)
unsigned char cmap[NCHARS];
short club[NCHARS], schism[NG];
short curg;


struct dfa *dfanil = 0;

struct dfa *
egrepcomp(s)
char *s;
{
	static int first = 1;
	register struct dfa *y;

	if(first) {
		first = 0;
		dfanil = egrepcomp("$");
	}
	y = (struct dfa *)calloc( sizeof(*y), 1);
	y->s = strdup(s);
	if(strpbrk(s, "+*.?[^$()|")==0) {
		y->constring = y->s;
		return y;
	}
	rere(y);
	return y;
}
rere(y)
struct dfa *y;
{
	register char *s = y->s, *t;
	register int i, j;

	memset ((char*)&blkstart, 0, &blkend - &blkstart);
	line  = 1;
	nxtpos = 0;
	nxtchar = 0;
	nstate = 1;
	t = malloc( (unsigned)strlen(s)+10);
	sprint(t, "^(%s)$", s);
	retext = input = t;
	sigma0();
	if(yyparse())
		return;
	sigma1(1);
	cfoll(line-1);
	cgotofn();
	populate();

	y->nstates = 1+nstate;
	y->out = malloc( (unsigned)y->nstates);
	y->delta = (short **)malloc(sizeof(short*)* NCHARS);
	y->pat = t;
	y->map = malloc(NCHARS);
	memcpy(y->map, (char*)cmap, NCHARS);
	for(j=0; j<NCHARS; j++) {
		if(j == cmap[j]) {
			register short *dp;

			dp = y->delta[j] = (short*)malloc(sizeof(short)* y->nstates);
			for(i=0; i<y->nstates; i++)
				*dp++ = gotofn[i][j];
		}
	}
	for(j=0; j<NCHARS; j++) {
		if(j != cmap[j])
			y->delta[j] = y->delta[cmap[j]];
	}
	for(i=0; i<y->nstates; i++)
		y->out[i] = out[i];

	y->start = istat;
	return;
}

#define CRANK(c,t) y->delta[c][t]


dfamatch(y, s)
register struct dfa *y;
register char *s;
{
	register int t, c;

	if(y->constring) {
		c = !strcmp(y->constring, s);
		return c;
	}
	if(y->delta == 0) rere(y);
	t = CRANK(LEFT,y->start);
	while(c = *s++)
		t = CRANK(c,t);
	t = CRANK(RIGHT,t);
	return y->out[t];
}

char *seen;
char alpha[NCHARS], *atop;
int m1kick;

dfale(x, y)
struct dfa *x, *y;
{
	int i, j;
	char xy[NCHARS];

	if(x->constring != 0 && y->constring != 0)
		return !strcmp(x->constring, y->constring);

	if(x->delta == 0) rere(x);
	if(y->delta == 0) rere(y);


	memset(&xy[0], 0, sizeof(xy));
	for(i=0; i<NCHARS; i++) {
		char xi, yi;
		if(xy[i])continue;
		xi = x->map[i];
		yi = y->map[i];
		for(j=i+1; j<NCHARS; j++)
			if(xy[j]==0
				&& xi == x->map[j]
				&& yi == y->map[j]) xy[j] = 1;
	}
	atop = alpha;
	for(i=0; i<NCHARS; i++) if(xy[i]==0) *atop++ = i;
	m1kick = x->nstates;
	seen = calloc(x->nstates, y->nstates);
	i = dfaLE(x, x->start, y, y->start);
	free(seen);
	return i;
}

dfaLE(m1, s1, m2, s2)
struct dfa *m1, *m2;
{
	int c, t1, t2;
	char *s;

	if(m1->out[s1] && !m2->out[s2]) return 0;
	seen[ s1 + m1kick * s2] = 1;
	for(s=alpha; s<atop; s++) {
		t1 = m1->delta[c = *s][s1];
		t2 = m2->delta[  c   ][s2];
		if(seen[ t1 + m1kick * t2]) continue;
		if(!dfaLE(m1, t1, m2, t2)) return 0;
	}
	return 1;
}

populate() {
	char cbuf[NCHARS];
	register char *s, *t;
	int i, j;

	t = cbuf;
	for(j=0; j<NCHARS;j++)
		if(cmap[j]==j)
			*t++ = j;
	for(i=1; i<= nstate; i++) {
		for(s=cbuf;s<t;s++)
			nxtst(i,*s);
	}
}


/*
 * initially put all chars in the same club
 */
sigma0() {
	int i;

	for(i=0; i<NCHARS; i++) cmap[i] = i;
	memset((char*)club, 0, sizeof(club));
	memset((char*)schism, 0, sizeof(schism));
	curg = 0;
	sigmaset("\n", 1);
}

/*
 * renumber the clubs that are actually used
 */
sigma1(flag) {
	register int g, i;
	register short *sp, *cp;

	sp = &schism[0];
	cp = &club[0];
	for(i=0; i<=curg; i++) *sp++ = -1;
	for(i=0; i<NCHARS; i++) {
		g = *cp++;
		if(schism[g] == -1) curg = schism[g] = i;
	}
	cp = &club[0];
	for(i=0; i<NCHARS; i++,cp++) {
		*cp = schism[*cp];
		if(flag)
			cmap[i] = *cp;
	}
}

/*
 * create schismatic clubs for the elements of s[]
 * squish down the club names if too many are formed
 */
sigmaset(s, n)
char *s;
{
	int c, g, maxg;

	maxg = curg++;

	while(n--) {
		c = 0xff & *s++;
		g = club[c];
		if(schism[g] <= maxg)
			schism[g] = curg++;
		club[c] = schism[g];
	}
	curg++;
	if(curg > NCHARS+NCHARS/2) sigma1(0);
}
