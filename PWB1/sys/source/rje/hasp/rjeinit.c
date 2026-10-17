#define NSGNON 28
#define LOG "log"
#define SLOG "slog"
int acu;
char signon[] {
002,
0141,0134,0342,0311,0307,0325,0326,0325,0100,0100,0100,0100,0100,0100,0100,
0331,0305,0324,0326,0343,0305,0365,0360,0100,0100,
036,003
};
char *xm();
int *order();
char who[10],dqs[20],home[30],pfld[30],phone[20];
char strx[200];
int hpid,lk;
char xbeg[4],xend[4];
char lockf[20],statf[20],stopf[20],deadf[20],mainf[20],dispf[20];
main(ac,av) char *av[];
 {int rbt,t,lds,sds,pd[2],tv[2],sb[18];
  char *p,*q,bf[40];
  close(0);
  cat(xbeg,"000");
  cat(xend,"000");
  if (alias(av[0],who)<0)
   {if (ac<2 || alias(av[1],who)<0)
     {prf("%q%s?\n",strx,av[0]);
      writes(1,strx); exit(~0);};};
  cat(lockf,who,"lock");
  cat(statf,who,"stat");
  cat(stopf,who,"stop");
  cat(deadf,who,"dead");
  cat(mainf,who,"main");
  cat(dispf,who,"disp");
  for (t=1;t<15;t++) signal(t,1);
  close(2); close(3); close(4);
  rbt=hpid=lk=0;
  if (ac>=1 && *av[0]=='+')
   {rbt=1; while (wait(&t)!=(-1));};
  if (dmov(1,0)<0 && open("/dev/tty8",1)!=0 && open("/dev/tty",1)!=0)
   fail("can't grab tty");
  prf("%qcan't locate %s",bf,who);
  if (lookup(who,home,strx,pfld)<0 || chdir(home)<0 || stat(".",sb)<0) fail(bf);
  cat(dqs,"/dev/",strx);
  t=0377&(sb[3]>>8);
  if ((0377&(t-(getuid()>>8)))!=0 && setuid(t)<0) fail("wrong uid");
  if (stat(stopf,sb)>=0)
   {unlink(stopf);
    if (rbt) exit(0);};
  if (rbt) sleep(60);
  if (pipe(pd)<0) fail("can't create pipe");
	p = pfld;
	if (*p == 'i') *p++;
       if ((!rbt) && (*p)) {
	 q = &signon[22];
	 while(*p >= '0' && *p <= '9')
		*q++ = *p++ + 0300;
	 *q = 0100; p++;
	 q = phone;
	 while(*q++ = *p++);
	q--; *q++ = '-'; *q++ = '='; *q = '\0';
	if((phone[0] >= '0') && (phone[0] <= '9')) {
		dup(1); dup(1); dup(1);
		if((acu=open("/dev/dn0",1))<0)
			fail("dn open error");
		if (write(acu,phone,size(phone)-1)<0)
			fail("dn write error");
		close(3); close(4); close(5);
		close(acu);
		writes(0,"Number dialed and dn closed.\n");
	}
       }
  prf("%qcan't open %s",bf,dqs);
   sleep(8);
  if (open(dqs,2)!=3) fail(bf);
  if (stat(".",sb)>=0 && fsfree(sb[0],sb)>=0)
   {if (sb[0]<100 || sb[1]<10) fail("file system exhausted");
    if (sb[0]<2000 || sb[1]<100)
     {prf("%q%c\n\n%s: warning - only %d blocks, %d inodes free\n\n%c",
          strx,7,who,sb[0],sb[1],7);
      if (!rbt) writes(0,strx);};};
  if (!rbt) {
	 unlink(SLOG);
	 link(LOG,SLOG);
	 unlink(LOG);
  }
  if ((lds=open("log",1))>=0) seek(lds,0,2);
   else lds=creat("log",0645);
  if (lds>=0)
   {if ((sds=open(statf,0))>=0)
     {if ((t=read(sds,bf,30))<0) t=0;
      bf[t]=0; close(sds);
      for (p=q=bf;t=(*p);p++)
       {if (t<'0'||t>'9') break;};
      if (t==':' && p==(bf+3))
       {*p++=0; cat(xbeg,bf); cat(xend,bf);};
      while (*p && *p!='\n') p++;
      if (*p++) while (*q++=(*p++));
      if (q>bf+1)
       {*(q-1)='\n'; *q=0; writes(lds,bf);};};
    time(tv);
    p=ctime(tv)+11; p[8]='\n';
    write(lds,p,9); close(lds);};
  lock(); requeue();
  if ((sds=creat(statf,0644))<0) fail("can't create stat file");
  if (write(sds,xend,3)!=3) fail("can't rewrite stat file");
  close(sds); unlock();
  if (open(statf,2)!=4) fail("can't reopen stat file");
/*  signon remote terminal  */
 if (!rbt) {
  if (((phone[0] >= '0') && (phone[0] <= '9')) || (phone[0] == '+')) {
		if (write(3,signon,NSGNON) != NSGNON) fail("phone connect not made");
		write(3,0,0);
  }
 }
  for (t=1;(hpid=fork())==(-1) && t<20;t++) sleep(t);
  if (hpid==(-1)) fail("can't fork (main)");
  if (hpid==0)
   {unlink(deadf);
    if (dmov(3,0)>=0 && dmov(pd[1],3)>=0 && dmov(3,1)>=0 && dmov(4,2)>=0)
     {sleep(5); execl(mainf,xbeg,who,"main",pfld,0);};
    creat(deadf,0644); exit(~0);};
  seek(4,0,2); prf("%q:%d\n",bf,hpid); writes(4,bf);
  t=(rbt? 0:fork());
  if (t==(-1)) fail("can't fork (disp)");
  if (t!=0) exit(0);
  if (dmov(0,3)>=0 && dmov(pd[0],0)>=0 && dmov(3,1)>=0)
   {close(2); close(4); execl(dispf,xbeg,who,"disp",home,pfld,0);};
  creat(stopf,0);
  prf("%q%c\n\n%s: couldn't execute %s\n\n%c",strx,7,who,dispf,7);
  writes(1,strx);
  while ((t=read(0,bf,40))>0) write(1,bf,t);
  exit(~0);};

fail(s)
 char *s;
 {if (hpid>0)  creat(stopf,0); unlock();
  prf("%q%c\n\n%sinit failed -- %s\n\n%c",strx,7,who,s,7);
  writes(0,strx);
  exit(~0);};

dmov(d0,d1)
 {close(d1);
  if (dup(d0)==d1) {close(d0); return (0);};
  return (-1);};

writes(ds,s)
 char *s;
 {return (write(ds,s,len(s)));};

len(s)
 char *s;
 {register char *p;
  for (p=s;*p;p++); return (p-s);};

#define ENFILE 23

lock()
 {extern int errno;
  int sb[20];
  int t,sf,ds;
  for (sf=t=0;(ds=creat(lockf,0))<0;t++)
   {if (errno==ENFILE) unlink(lockf);
    if (sf==0 && stat(lockf,sb+2)>=0) sf=t+1;
    if (t<sf+10) {sleep(2); continue;};
    if (sf==0) fail("can't set lock");
    if (stat(lockf,sb)>=0 && sb[16]==sb[18] && sb[17]==sb[19])
      unlink(lockf);
    sf=t=0;};
  close(ds); lk=1;};

unlock()
 {if (lk) {lk=0; unlink(lockf);};};

struct dent {int in; char fn[14];};

requeue()
 {struct dent *bx;
  register struct dent *bp;
  register char *p,*q;
  int s,t,ds,*sp,*sp0,*sx,ss[1000];
  char x0[8],x1[8],bf[512];
  char inf0[30],inf1[30];
  sp=ss; sx=ss+1000;
  if ((ds=open(".",0))<0) fail("can't open dir");
  while ((t=read(ds,bf,512))!=0)
   {if (t<0 || (t&017)) fail("can't read dir");
    bx=(bf+t);
    for (bp=bf;bp<bx;bp++)
     {if (bp->in==0) continue;
      p=bp->fn; q="xmit";
      while (*q!=0 && *p++==(*q)) q++;
      if (*q!=0) continue;
      for (s=t=0;t<3;t++)
       {if (*p<'0' || *p>'9') break;
        s=(s*10)+((*p++)-'0');};
      if (t!=3 || *p!=0) continue;
      if (sp<sx) *sp++=s; else fail("too many xmit files");};};
  close(ds);
  if (sp==ss) return;
  sp0=sp=order(ss,sx=sp);
  cat(xbeg,xm(s=(*sp))+4);
  for (;;)
   {if (++sp>=sx) sp=ss;
    if (sp==sp0) break;
    if (++s>999) s=0;
    if (s==(*sp)) continue;
    cat(x0,xm(s)); cat(x1,xm(*sp));
    if (link(x1,x0)<0 || unlink(x1)<0) fail("can't relink xmit file");
    cat(inf0,"info/logx",x0+4);
    cat(inf1,"info/logx",x1+4);
    unlink(inf0); link(inf1,inf0); unlink(inf1);};
  cat(xend,xm(++s)+4);};
char *xm(s)
 {static char xms[8];
  char *p,*q; int t;
  prf("%q%d",strx,s); p=strx; t=len(p);
  if (t<0 || t>3) fail("bad xmit seq no");
  q=cat(xms,"xmit");
  while (t<3) {*q++='0'; t++;};
  cat(q,p); return (xms);};

error(s,pid)
{
	extern fout;

	fout = 2;
	perror(s);
	kill(pid,9);
	exit(1);
}
size(s) char *s;
{
	register char *ptr;
	register i;
	ptr = s;
	i = 1;
	while(*ptr++) i++;
	return(i);
}
