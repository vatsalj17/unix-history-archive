#include "ins.h"

verify(t)
char *t;
{
	char *pid, *av[10], *xp, targ[40], targ2[40], pendxs[40], *lp;
	int lineno=0, fd;

	targ[0] = targ[2] = pendxs[0] = 0;
	fd = open(logfile, 0);
	while(lp = Frdline(fd)) {
		lineno++;
		strcpy(cpbuf, lp);
		getmfields(lp, av, 10);
		pid = strchr(av[0], '.');
		if(pid!=0)
			*pid++ = 0;
		switch(*lp) {
		case '\n':
		case 0:
			break;
		case '!':
			break;
		case 'n':
			if(strcmp(av[0], "nq")==0)
				;
			else
				goto bomb;
			break;
		case 's':
			if(strcmp(av[0], "stash")==0)
				;
			else if(strcmp(av[0], "setlab")==0)
				;
			else if(strcmp(av[0], "setpriv")==0)
				;
			else
				goto bomb;
			break;
		case 'j':
			if(strcmp(av[0], "jobxs")==0)
				addjob(av[1], av[2], av[3], av[4]);
			else if(strcmp(av[0], "jobok")==0)
				rmjob(av[1], av[2], 1);
			else
				goto bomb;
			break;
		case 'r':
			if(strcmp(av[0], "requeue")==0)
				;
			else if(strcmp(av[0], "rm")==0)
				;
			else
				goto bomb;
			break;
		case 'd':
			if(strcmp(av[0], "deljob")==0)
				rmjob(av[1], av[2], 0);
			else if(strcmp(av[0], "date")==0)
				;
			else if(strcmp(av[0], "defer")==0)
				;
			else
				goto bomb;
			break;
		case 'c':
			if(strcmp(av[0], "chmod")==0)
				;
			else if(strcmp(av[0], "chown")==0)
				;
			else if(strcmp(av[0], "copy")==0)
				;
			else
				goto bomb;
			break;
		case 'i':
			if(strcmp(av[0], "ins")==0)
				;
			else
				goto bomb;
			break;
		case 'x':
			if(strcmp(av[0],"xss")==0) {
				if(strcmp(av[1], pending)==0)
					strcpy(pendxs, av[2]);
				if(strcmp(av[1], t)==0)
					strcpy(targ2, av[2]);
			} else if(strcmp(av[0], "xs")==0) {
				if(strcmp(av[1], t)==0)
					strcpy(targ, av[2]);
			}
			else 
				goto bomb;
			break;
		case 'q':
			if(strcmp(av[0], "qdo")==0)
				;
			else
				goto bomb;
			break;
		default:
		bomb:
			fprint(2, "unknown log entry, line %d: %s", lineno, cpbuf);
		}
	}
	close(fd);
	if(targ[0]!=0 && strcmp(xs(t,0,0), targ) !=0 ) {
		error("forged %s", t);
	}
	if(targ2[0]!=0 && strcmp(xs(t,1,0), targ2) !=0 )
		error("forged %s", t);
	if(strcmp(xp=xs(pending,1,0), pendxs) !=0 ) {
		print("%s:%s\n",pendxs, xp);
		error("forged %s", pending);
	}
	jqshow();
}
struct jq {
	char *name;
	struct jq *hnext;
	char *xs;
	struct jq *q1, *q2;
	struct jq *job;
	struct jq *next, *prev;
};
#define HASH 10169
struct jq *jq[HASH];
jqshow() {
	int h;
	struct jq *q, *x, *j;

	for(h=0; h<HASH; h++)
		for(x=jq[h];x;x=x->hnext)
			for(q=x->next;q;q=q->next) {
				if(j = q->job)
					print("Q: %s	%s %s\n", 
						x->name,j->name, j->xs);
			}
}
struct jq *addq();
addjob(jname, targ, queue, xp)
char *jname;
char *targ;
char *queue;
char *xp;
{
	int h;
	struct jq *j, *q;

	j = (struct jq *)calloc(1, sizeof(*j));
	j->name = strdup(jname);
	j->xs = strdup(xp);
	h = hash(jname);
	j->hnext = jq[h];
	jq[h] = j;
	j->q1 = q = addq(targ);
	q->job = j;
	if(strcmp(targ,queue)!=0) {
		j->q2 = q = addq(queue);
		q->job = j;
	}
}

hash(s)
char *s;
{
	unsigned int x, c;
	x = 0;
	while(c=*s++)
		x = 3*x + c;
	return x%HASH;
}
struct jq *
addq(s)
char *s;
{
	struct jq *x, *q;
	int h = hash(s);

	for(x=jq[h]; x; x=x->hnext)
		if(strcmp(s,x->name)==0)
			break;
	if(x==0) {
		x = (struct jq *)calloc(1, sizeof(*x));
		x->name = strdup(s);
		x->hnext = jq[h];
		jq[h] = x;
	}
	q = (struct jq *)calloc(1, sizeof(*q));
	if(q->next = x->next)
		q->next->prev = q;
	q->prev = x;
	x->next = q;
	q->name = x->name;

	return q;
}

rmjob(jname, xp, fromhead)
char *jname;
char *xp;
{
	int h = hash(jname);
	struct jq *q, *j;
	for(j=jq[h]; j; j=j->hnext)
		if(j->xs!=0 && 
			strcmp(j->name,jname)==0 && 
			strcmp(j->xs,xp)==0) {
				break;
		}
	if(j==0) {
		print("WARNING: UNKNOWN JOB %s %s REMOVED\n",
			jname, xp);
		return;
	}
	if(q=j->q1) 
		unhook(q, fromhead);
	if(q=j->q2) 
		unhook(q, fromhead);
	j->q1 = j->q2 = 0;
	j->name = "";
	j->xs = "";
}
unhook(q, fromhead)
struct jq *q;
{
	struct jq *p, *n;
	if(q->next != 0 && fromhead != 0) {
		print("WARNING: JOB %s to update %s served OUT OF ORDER\n",
			q->job->name, q->name);
	}
	p = q->prev;
	n = q->next;
	if(p) p->next = n;
	if(n) n->prev = p;
}
