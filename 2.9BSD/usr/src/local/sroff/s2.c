#include "sdef.h"
#include "d.h"
#include "v.h"
#include "tw.h"
#include "s.h"
#include <setjmp.h>
jmp_buf sjbuf;

/*
sroff2.c

output, cleanup
*/

extern struct s *frame, *stk, *nxf;
extern filep ip;
extern filep offset;
extern char *enda;


extern char obuf[OBUFSZ];
extern char *obufp;
extern int dilev;
extern int eschar;
extern int tlss;
extern int tflg;
extern int print;
extern char trtab[];
extern int waitf;
extern char ptname[];
extern int ptid;
extern int em;
extern int ds;
extern int mflg;
extern filep woff;
extern int nflush;
extern int lgf;
extern int app;
extern int nfo;
extern int donef;
extern int *pendw;
extern int trap;
extern struct sgttyb ttys;
extern int ttysave;
extern int quiet;
extern int pendnf;
extern int ndone;
extern int lead;
extern int ralss;
extern char *unlkp;
extern char nextf[];
extern int pipeflg;
extern int ejf;
extern int no_out;
extern int level;
extern int stopmesg;
extern int xxx;
int toolate;
int error;

pchar(c)
int c;
{
	register i, j;

	if((i=c) & MOT){pchar1(i); return;}
	switch(j = i & CMASK){
		case 0:
		case IMP:
		case RIGHT:
		case LEFT:
			return;
		case HX:
			j = (tlss>>9) | ((i&~0777)>>3);
			if(i & 040000){
				j &= ~(040000>>3);
				if(j > dip->blss)dip->blss = j;
			}else{
				if(j > dip->alss)dip->alss = j;
				ralss = dip->alss;
			}
			tlss = 0;
			return;
		case LX:
			tlss = i;
			return;
		case PRESC:
			if(dip == &d[0])j = eschar;
		default:
			i = (trtab[j] & BMASK) | (i & ~CMASK);
	}
	pchar1(i);
}
pchar1(c)
int c;
{
	register i, j, *k;
	extern int chtab[];

	j = (i = c) & CMASK;
	if(dip != &d[0]){
		wbf(i);
		dip->op = offset;
		return;
	}
	if(!tflg && !print){
		if(j == '\n')dip->alss = dip->blss = 0;
		return;
	}
	if(no_out || (j == FILLER))return;
	ptout(i);
}
oput(i)
char i;
{
	*obufp++ = i;
	if(obufp == (obuf + OBUFSZ)) flusho();
}
oputs(i)
char *i;
{
	while(*i != 0)oput(*i++);
}
flusho(){
	if(!ptid){
		while((ptid=open(ptname,1)) < 0){
			if(++waitf <=2)prstr("Waiting for printer.\n");
			sleep(15);
		}
	}
	if(no_out == 0){
#ifdef TTY
		if (!toolate) {
			toolate++;
			if(ttysave == -1) {
				gtty(ptid, &ttys);
				ttysave = ttys.sg_flags;
			}
			ttys.sg_flags &= ~t.breset;
			ttys.sg_flags |= t.bset;
			stty(ptid, &ttys);
#else
		if (!toolate) {
			toolate++;
			if (ptid != 1) {
			    ttys.sg_ispeed = ttys.sg_ospeed = t.bspeed;
			    ttys.sg_flags = t.bflags;
			    stty(ptid, &ttys);
#ifdef	TIOCEXCL
			    ioctl(ptid,TIOCEXCL,(struct sgttyb *)0);
#endif
			}
#endif
			{
			char *p = t.twinit;
			while (*p++)
				;
			write(ptid, t.twinit, p-t.twinit-1);
			}
		}
		toolate += write(ptid, obuf, obufp-obuf);
	}
	obufp = obuf;
}
done(x) int x;{
	register i;

	error |= x;
	level = 0;
	app = ds = lgf = 0;
	if(i=em){
		donef = -1;
		em = 0;
		if(control(i,0))longjmp(sjbuf,1);
	}
	if(!nfo)done3(0);
	mflg = 0;
	dip = &d[0];
	if(woff)wbt(0);
	if(pendw)getword(1);
	pendnf = 0;
	if(donef == 1)done1(0);
	donef = 1;
	ip = 0;
	frame = stk;
	nxf = frame + 1;
	if(!ejf)tbreak();
	nflush++;
	eject((struct s *)0);
	longjmp(sjbuf,1);
}
done1(x) int x; {
	error |= x;
	if(v.nl){
		trap = 0;
		eject((struct s *)0);
		longjmp(sjbuf,1);
	}
	done2(0);
}
done2(x) int x; {
	register i;

	ptlead();
	if(ptid != 1 && stopmesg)
		prstr("Sroff finished.\n");
	flusho();
	done3(x);
}
done3(x) int x;{
	error |= x;
	signal(SIGINT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
	unlink(unlkp);
	twdone();
	if(quiet){
		ttys.sg_flags |= ECHO;
		stty(0, &ttys);
	}
	exit(error);
}
edone(x) int x;{
	frame = stk;
	nxf = frame + 1;
	ip = 0;
	done(x);
}
casepi(){
	register i;
	int id[2];

	if(toolate || skip() || !getname() || (pipe(id) == -1) ||
	   ((i=fork()) == -1)){
		prstr("Pipe not created.\n");
		return;
	}
	ptid = id[1];
	if(i>0){
		close(id[0]);
		toolate++;
		pipeflg++;
		return;
	}
	close(0);
	dup(id[0]);
	close(id[1]);
	execl(nextf,nextf,0);
	prstr("Cannot exec: ");
	prstr(nextf);
	prstr("\n");
	exit(-4);
}
