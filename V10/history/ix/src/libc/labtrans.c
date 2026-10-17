#include "libc.h"
#include "fio.h"
#include <sys/label.h>
#include "cbit.h"

#define isset(p,n) ((p)->lb_bits[(n)/8]&bmask[(n)%8])
#define turnon(p,n) (p)->lb_bits[(n)/8]|=bmask[(n)%8]
#define turnoff(p,n) (p)->lb_bits[(n)/8]&=~bmask[(n)%8]

extern long lrand();
extern char *xs();
extern struct label labelyes;
extern struct label labelno;
extern struct label labelbot;
extern struct label labMAX();

#define DONE	2

#if DONE&FLOORMASK 
	#error floor bit overloaded
#endif


static unsigned char bmask[] = {1,2,4,8,16,32,64,128};

/*
 * convert foreign label to domestic form
 * 0 means failure to map
 */
transin(map, foreign, domestic)
struct mapping *map;
struct label *foreign, *domestic;
{
	int i, j;
	struct label zot;

	if(labEQ(foreign, &labelyes)||labEQ(foreign, &labelno)) {
		*domestic = *foreign;
		return 1;
	}
	/*
	 * her fault for telling us secrets we are not cleared for
	 */
	if(!labLE(foreign, &map->lambda)) {
		*domestic = labelno;		/* wisdom ? */
		return 0;
	}
	zot = map->dirty;
	for(i=0; i<8*LABSIZ; i++) {
		if(isset(foreign, i)) {
			j = map->shuffle[i];
			if(j==-1)
				error("map botch cannot happen!\n");
			turnon(&zot, j);
		}
	}
	*domestic = zot;
	return 1;
}

/*
 * see if safe to send label off machine
 * clear my private biba bits: they would only confuse
 */
transout(map, domestic, foreign)
struct mapping *map;
struct label *domestic;
struct label *foreign;
{
	int i;
	
	if(labLE(domestic, &map->lstar)) {
		*foreign = *domestic;
		for(i=0; i<8*LABSIZ; i++) {
			if(isset(&map->dirty, i))
				turnoff(foreign, i);
		}
		return 1;
	}
	else
		return 0;
}

enum {
	HORRID,	/* you cad! */
	CHAL,	/* myname:chal:serverend */
	RESP,	/* yourname:chal:myname:resp */
	OWNER,	/* have you any labels from authority X ? */
	ISLABEL,/* name:floor:owner::slot:myname:xsum (cbit file format) */
};

static char *word[10];
static send();
static rcv();
static char *howdy();
static scrutlab();

#define triv(s)((s)==0||*(s)==0)


/*
 * communicate with another instance at other end of fd
 * return a mapping suitable for use with transin() and transout().
 * use 'file' for my authorizations, which are issued to 'me'
 */

struct mapping *
buildmap(fd, file, me, mypass, serverend)
char *file;
char *me, *mypass;
{
	int i;
	char *she;
	struct mapping *map;
	struct cbit *p, *q, *r, *cb;
	struct label lt;

	map = (struct mapping*)calloc(1, sizeof(*map));
	map->lambda = labelbot;
	map->lstar = labelbot;
	map->dirty = labelbot;
	for(i=0; i<LABSIZ*8; i++)map->shuffle[i] = -1;
	if(map == 0)
		return 0;
	setfields(":");

	if((she = howdy(fd, me, mypass, serverend))==0)
		goto abort;

	map->partner = she;
	cb = cbitread(file);
	if(cb == 0)
		goto abort;
	for(p=cb; p->name; p++) {
		if(p->floor&FLOORMASK)
			turnon(&map->dirty, p->bitslot);
	}

	/*
	 * lets play go fish!
	 */
	for(i=0; i<2; i++) {
		for(p=cb; p->name; p++)
			p->floor &= FLOORMASK;
		/*
		 * i loop over owner names
		 * she sends all cbits for that owner.
		 */
		if(i==serverend) {
			for(p=cb;p->name;p++){
				if(p->floor & DONE) continue;
				send(fd, OWNER, "%s", p->owner);
				for(q=p;q->name;q++)
					if(strcmp(p->owner, q->owner)==0)
						q->floor |= DONE;
				for(;;) {
					if(rcv(fd)!=ISLABEL) goto abort;
					if(triv(word[1]))break;
					q = cbitparse(&word[1], 
						(struct cbit*)0);
					r = cbitlookup(q->name, cb);
					if(r)
						scrutlab(r, q, map);
				}
			}
			send(fd, OWNER, ":");
		}
		/*
		 * she prompts me for owners
		 * i send all corresponding cbits
		 */
		else {
			for(;;) {
				if(rcv(fd)!=OWNER)goto abort;
				if(triv(word[1]))break;
				for(p=cb; p->name; p++) {
					if(p->floor&DONE)
						continue;
					if(strcmp(p->owner, word[1])!=0)
						continue;
					send(fd, ISLABEL, "%s:%d:%s::%d:%s:%s",
						p->name, 
						p->floor&FLOORMASK,
						p->owner, 
						p->bitslot,
						me,
						p->certificate);
					p->floor |= DONE;
				}
				send(fd, ISLABEL, "::::::");
			}
		}
	}
	/*
	 * finally, endow her with all my dirty private biba bits
	 */
	lt = labMAX(&map->lstar, &map->dirty);
	map->dirty = lt;
	return map;
abort:
	free((char*)map);
	send(fd, HORRID, "");
	return 0;
}

static
scrutlab(mine, hers, map)
struct cbit *mine, *hers;
struct mapping *map;
{
	char *s;

	if(strcmp(hers->exerciser, map->partner)!=0) return;

	if(strcmp(hers->name, mine->name)!=0) return;
	if(strcmp(hers->owner, mine->owner)!=0) return;
	if((hers->floor&FLOORMASK) != (mine->floor&FLOORMASK)) return;

	s = cbitcert(hers);

	if(rverify(hers->owner, hers->certificate, s, strlen(s))) {
		map->shuffle[hers->bitslot] = mine->bitslot;
		turnon(&map->lambda, hers->bitslot);
		turnon(&map->lstar, mine->bitslot);
		turnoff(&map->dirty, mine->bitslot);
	}
}

/*
 * CHAL with me:chal:serverend
 * RESP with me:chal:she:resp
 *
 * fold with similar dialogue in opposite direction
 */
static
char *
howdy(fd, me, mypass, serverend)
char *me, *mypass;
{
	char *she, chal[100];
	int i;
	srand(getpid());
	for(i=0; i<30; i++) chal[i] = nrand(26)+'a';
	chal[30] = 0;
	send(fd, CHAL, "%s:%s:%d", me, chal, serverend);
	if(rcv(fd) != CHAL || triv(word[1]) || triv(word[2]))
		return 0;
	if(triv(word[3]) || serverend == atoi(word[3])) return 0;
	
	she = strdup(word[1]);
	send(fd, RESP, "%s:%s:%s:%s", 
		word[1], word[2], 
		me, xs(mypass, word[2], strlen(word[2])));
	if(rcv(fd) != RESP)
		return 0;
	if(triv(word[1])||strcmp(word[1],me)!=0) return 0;
	if(triv(word[2])||strcmp(word[2],chal)!=0) return 0;
	if(triv(word[3])||strcmp(word[3],she)!=0) return 0;

	if(rverify(she, word[4], chal, strlen(chal)))
		return she;
	return 0;
}



static char buf[4096];

static
send(fd, code, f, x1, x2, x3, x4, x5, x6, x7)
register char *f;
{
	sprint(buf, f, x1, x2, x3, x4, x5, x6, x7);
	fprint(fd, "%d:%s\n", code, buf); 
/* fprint(2, "%d->	%d:%s\n", getpid(), code, buf);  */
}

static
rcv(fd)
{
	register char *s;

	s = Frdline(fd);
/* fprint(2, "%d<-	%s\n", getpid(), s); */
	if(s==0)return HORRID;
	(void)setfields(":");
	(void)getfields(s, word, 10);
	return atoi(word[0]);
}
