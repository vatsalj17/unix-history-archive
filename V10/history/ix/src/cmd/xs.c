#include <libc.h>
#include <fio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/label.h>

unsigned int *xsegment();
unsigned long xsum();
unsigned long htoi();

#define BS 4096
char buf[BS];

int errcnt = 0;
int statgoo = 0;
char *key = 0;

main(ac, av)
char **av;
{
	int i;

	for(i=1; i<ac; i++) {
		if(strcmp(av[i], "-k")==0) key = av[++i];
		else if(strcmp(av[i], "-f")==0) indir(av[++i]);
		else if(strcmp(av[i], "-s")==0) statgoo++;
		else check(av[i], (unsigned int *)0);
	}
	return errcnt;
}

check(f, y)
char *f;
unsigned int *y;
{
	unsigned int *x;
	int i, fd;

	fd = open(f, 0);
	if(fd == -1) {
		errcnt++;
		perror(f);
		return;
		
	}
	x = xsegment(fd);
	close(fd);
	if(y == 0) {
		print("%s\t", f);
		for(i=0; i<4; i++) print("%4.4ux%c", x[i], (i==3)?'\n':' ');
	}
	else if(notsame(x,y)) {
		print("%s\t", f);
		for(i=0; i<4; i++) print("%4.4ux ", x[i]);
		print("should be ");
		for(i=0; i<4; i++) print("%4.4ux%c", y[i], (i==3)?'\n':' ');
		errcnt++;
	}
}
notsame(x,y)
register unsigned int *x, *y;
{
	if(*x++ != *y++) return 1;
	if(*x++ != *y++) return 1;
	if(*x++ != *y++) return 1;
	if(*x   != *y  ) return 1;
	return 0;
}

/*
 * checksum file format: lines of form
 *  filename hexval hexval hexval hexval
 */

indir(s) 
char *s;
{
	unsigned int old[4];
	char *lp, *t, *fields[10];
	int nf, f;

	if((f=open(s,0)) == -1) {
		errcnt++;
		perror(s);
		return;
	}
	while(lp = Frdline(f)) {
		t = strchr(lp, '#');
		if(t)*t = 0;
		nf = getmfields(lp, fields, 6);
		if(nf == 5) {
			old[0] = htoi(fields[1]);
			old[1] = htoi(fields[2]);
			old[2] = htoi(fields[3]);
			old[3] = htoi(fields[4]);
			check(fields[0], old);
		}
	}
	close(f);
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

unsigned long
htoi(s)
char *s;
{
	char c;
	unsigned long x = 0;
	while(c = *s++) {
		x *= 16;
		if(c>= '0' && c <= '9') x += c - '0';
		else
		if(c>= 'a' && c <= 'f') x += c - 'a' + 10;
		else
		if(c>= 'A' && c <= 'F') x += c - 'A' + 10;
		else {
			errcnt++;
			print("illegal hex constant\n");
			return 0;
		}
	}
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

