#include "libc.h"
#include "fio.h"
#include <sys/label.h>
#include "cbit.h"

/*
 * missing: versions of labtoa & atolab which use these.
 */
struct cbit *
cbitread(file)
char *file;
{
	char *fields, *s, *v[8];
	struct cbit *p, *q;
	int fd, i, gobble, nleft;

	if((fd=open(file,0))==-1) return 0;
	
	gobble = nleft = 10;
	q = p = (struct cbit*)calloc(gobble, sizeof(*p));
	if(p==0)return 0;
	fields = strdup(setfields(":"));

	for(i=0; s=Frdline(fd); i++) {
		if(getfields(s,v,8)!=7) continue;
		if(nleft<2) {
			nleft = 2+gobble/2;
			gobble += nleft;
			p = (struct cbit*)realloc((char*)p, gobble*sizeof(*p));
			q = &p[i];
			memset((char*)q, 0, sizeof(*q)*(gobble-i));
		}
		(void)cbitparse(v,q);
		q++;
		nleft--;
	}
	(void)setfields(fields);
	free(fields);
	close(fd);
	return p;
}

struct cbit*
cbitparse(v,p)
char **v;
struct cbit *p;
{
	if(p==0) p = (struct cbit*)malloc(sizeof(*p));
	if(p==0)return 0;
	p->name = strdup(v[0]);
	p->floor = atoi(v[1]);
	p->owner = strdup(v[2]);
	p->nickname = strdup(v[3]);
	p->bitslot = atoi(v[4]);
	p->exerciser = strdup(v[5]);
	p->certificate = strdup(v[6]);
	return p;
}

char *
cbitcert(p)
struct cbit *p;
{
	char *s;
	unsigned int n;
	if(p==0)return 0;
	n = strlen(p->name)+strlen(p->owner)+strlen(p->exerciser)+100;

	s = calloc(1,n);
	if(s==0)return 0;
	sprint(s, "mayholdlabel(%s:%d:%s:::%s:)",
		p->name,
		p->floor&FLOORMASK,
		p->owner,
		p->exerciser);
	return s;
}

struct cbit*
cbitlookup(name, q)
char *name;
struct cbit *q;
{
	struct cbit *r, *t;

	t = 0;
	for(r=q; r->name; r++) {
		if(strcmp(r->name,name))continue;
		if(t==0)
			t = r;
		else
			return 0;
	}
	return t;
}
