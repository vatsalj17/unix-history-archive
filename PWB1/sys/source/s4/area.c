#

#define STRUCT	struct
#define CHAR	char
#define BOOL	char
#define REG	register
#define INT	int
#define STRING	char *
#define PTR	int *

#define EXITSW	break

#define BEGIN	{
#define END	}

#define IF	if(
#define THEN	){
#define ELSE	} else {
#define ELIF	} else if (
#define FI	}

#define EXITFOR	break
#define FOR	for(
#define WHILE	while(
#define DO	){
#define OD	}
#define REP	do{
#define PER	}while(
#define DONE	);
#define LOOP	for(;;){
#define POOL	}

#define SKIP	;
#define DIV	/
#define REM	%
#define NEQ	^
#define ANDF	&&
#define ORF	||

#define TRUE	-1
#define FALSE	0
#define LOBYTE	0377
#define HIBYTE	0177400
#define HEXMSK	017

#define maxint	32767
#define GETBLOK	512
#define RELBLOK	256
#define FREE 32767
#define BUSY 0

#define AREA	struct areablk

AREA {
	INT word;
	INT area;
};

/*initial empty arena*/
AREA	areai;
AREA	areaj {&areai,0};
AREA	areai {&areaj,0};
AREA **areap &areai;	/*current search pointer*/
AREA **areatop &areai;	/*top of arena (last cell)*/
INT	areanum;	/*current allocation area*/
/*
 *	storage allocator
 *	(circular first fit strategy)
 */

/* accounting
INT	shellpid;
INT	getcnt;
INT	freecnt;
INT	getloop;
INT	freeloop;
INT	getbrk;
INT	gbgebrk;
*/
INT	areabrk;	/*end of store in some sense*/

getcell(nbytes)
char *nbytes;
{
	REG INT rbytes;
	REG char *p, *q;
	INT newbrk;

	IF areabrk==0
	THEN areabrk=(sbrk(0)+3)&-4;
	FI

	/*DEB getcnt++; BED*/
	rbytes = (nbytes+7)&-4;
	p=areap;
	LOOP	REP	IF (p->area)>areanum
			THEN	WHILE ((q = p->word)->area)>areanum DO p->word = q->word; OD
				IF q >= &p[rbytes]
				THEN	areap = &p[rbytes];
					IF q>areap
					THEN	areap->word = p->word;
						areap->area = FREE;
					FI
					p->word=areap;
					p->area=areanum;
					return(p+4);
				FI
			FI
			/*DEB getloop++; BED*/
			q = p; p = p->word;
		PER	q>=areap || p<areap DONE
		/*DEB getbrk++; BED*/
		IF brk(newbrk=(areabrk+rbytes+GETBLOK)&-GETBLOK) == -1 THEN return(0); FI
		areatop->word=areabrk;
		areatop->area = (((q=areabrk)!=areatop+2) ? BUSY : FREE);
		areatop = (areabrk->word = newbrk-4);
		areabrk->area = FREE; areabrk=newbrk;
		areatop->word = &areaj; areatop->area=BUSY;
	POOL
}

freecell(p)
char **p;
{
	IF p
	THEN	p =- 2;
		IF p<areap THEN areap=p; FI
		p->area=FREE;
	FI
}

freearea(a)
INT a;
{
	REG CHAR **p; REG INT from; REG PTR top;
	from=a; top=areatop;

	/*DEB freecnt++; BED*/
	p = &areai;
	REP	IF p->area >= from
		THEN	p->area=FREE;
		FI
		/*DEB freeloop++; BED*/
		p=p->word;
	PER p!= top DONE
}

copyarea(a,to)
INT a, to;
{
	REG CHAR **p; REG INT from; REG PTR top;
	from=a; top=areatop;

	p = &areai;
	REP	IF p->area == from
		THEN	p->area=to;
		FI
		p=p->word;
	PER p!= top DONE
}

garbage()
{
	REG CHAR **p, **q; REG PTR top;
	top=areatop;

	areap = p = &areai;
	REP	IF p->area>areanum
		THEN	WHILE ((q = p->word)->area)>areanum DO p->word = q->word; OD
			areap=p;
		FI
		q = p; p = p->word;
	PER p!=top DONE
	IF areatop>=q+(RELBLOK+4) DIV 2 ANDF (q->area)>areanum
	THEN	brk(areabrk=(areabrk-RELBLOK)&-RELBLOK); q->word=areatop=areabrk-4;
		areatop->word = &areaj; areatop->area = BUSY;
		/*DEB gbgebrk++; BED*/
	FI
}

setarea(p,a)
char **p; int a;
{
	*(p-1)=a;
}
