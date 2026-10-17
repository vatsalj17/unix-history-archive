#include "ins.h"

unsigned int *xsegment();
unsigned long xsum();

#define BS 4096
static char buf[BS];

static int statgoo = 0;
static char *key = 0;

static char xxx[50];

char *
xs(f,sflag, logit)
char *f;
{
	unsigned int *x;
	int fd;
	fd = open(f, 0);
	if(fd == -1)
		return "";
	statgoo = sflag;
	x = xsegment(fd);
	close(fd);
	sprint(xxx, "%4.4ux.%4.4ux.%4.4ux.%4.4ux", x[0], x[1], x[2], x[3]);
	if(logit)log(sflag?"xss":"xs", "%s %s\n", f, xxx);
	return xxx;
}


unsigned int *
xsegment(fd)
{
	unsigned long t;
	static unsigned int x[4];
	char *labtoa();
	struct stat sb;
	struct label flab;
	int n, j;

	xinit();
	xstring(key);
	xstring("$$$");
	if(statgoo) {
		fstat(fd, &sb);
		fgetflab(fd, &flab);
		sprint(buf, "%o.%d.%d.", sb.st_mode, sb.st_uid, sb.st_gid);
		xstring("stat<");
		xstring(buf);
		xstring(labtoa(&flab));
		xstring(">goo");
	}
	xstring("alpha");
	while((n=read(fd, buf, BS))>0) {
		for(j=0; j<n; j++)
			xcrank(buf[j]);
	}
	xstring("omega");
	t = xsum();
	x[0] = t & 0xffff;
	x[1] = (t>>16) & 0xffff;
	xstring("digamma");
	t = xsum();
	x[2] = t & 0xffff;
	x[3] = (t>>16) & 0xffff;
	return x;
}

#define SRLEN 5

static unsigned long sr[SRLEN], *sa, *sb;

xinit() 
{
	int i;
	for(i=0; i<SRLEN; i++) sr[i] = 0;
	sa = &sr[0];
	sb = &sr[1];
}

unsigned long xsum() {
	return *sa ^ *sb;
}

xstring(s)
register char *s;
{
	register char c;
	if(s)
		while(c =*s++)
			xcrank(c);
}

unsigned long
phi(x)
register unsigned long x;
{
	do {
		x += 14921776 + x*(17761492 + x);
		x ^= 0x5827066;
		x += 0x2c357 + x*(10661688 ^ x);
		x ^= x>>16;
	} while((x&0x80000002) == 2);
	return x;
}

xcrank(x)
register x;
{
	*sb++ += phi((x&0xff) + *sa++);
	if(sa >= &sr[SRLEN]) sa = &sr[0];
	if(sb >= &sr[SRLEN]) sb = &sr[0];
}

