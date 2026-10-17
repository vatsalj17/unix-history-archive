#include "pwserv.h"


char *strset();

#define HASH	1009

struct entry *htab[HASH];


unsigned
hash(s)
register char *s;
{
	register unsigned h = 0, c;

	while(c = *s++) 
		h += (h<<6)+c;
	return h%HASH;
}


struct entry *
getent(name)
register char *name;
{
	register int h = hash(name);
	register struct entry *p;

	for(p=htab[h]; p; p=p->next) {
		if(p->name!=0 && strcmp(name,p->name)==0)
			return p;
	}
	p = (struct entry*)malloc(sizeof(*p));
	p->name = strdup(name);
	p->pw = p->atalla = 0;
	p->next = htab[h];
	htab[h] = p;
	return p;
}


pwslurp(fd)
{
	register int k;
	register struct entry *p;
	char *lp, *fields[10];
	

	for(k=0; k<HASH; k++)
		for(p=htab[k]; p ; p=p->next) {
			p->pw = strset(p->pw, (char*)0);
			p->atalla = strset(p->atalla, (char*)0);
		}

	setfields(":");
	Finit(fd, (char *)0);
	while(lp=Frdline(fd)) {
		if(getfields(lp, fields, 5)==5) {
			p = getent(fields[0]);
			p->pw = strset(p->pw, fields[1]);
			p->atalla = strset(p->atalla, fields[2]);
		}
	}
}

char *
strset(old, new)
register char *old, *new;
{
	if(old!=0 && new!=0 && strcmp(old,new)==0)return old;
	if(old!=0)free(old);
	return new?strdup(new):new;
}
