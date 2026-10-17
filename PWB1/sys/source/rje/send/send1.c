#
/* send1.c */


/* This information is proprietary and is the property of Bell
   Telephone Laboratories, Incorporated.  Its reproduction or
   disclosure to others, either orally or in writing, is pro-
   hibited without written permission of Bell Laboratories. */


#define IDMOD send1

#define IDSTR "~|^`send1.c 1.20 4/22/76

#include "send.h"

abt()
 {if (!abort) {abort=1; dotrp();};};

sig1()
 {signal(1,1); prf("Hang-up. (577)\n"); abt();};

sig2()
 {signal(2,1); prf("Interrupt. (578)\n"); abt();};

sig3()
 {signal(3,1); prf("Quit signal. (579)\n"); abt();};

sigx(x)
 {signal(x,1); prf("Signal #%d. (580)\n",x); abt();};

struct context icx,zcx;

struct format ifm;

struct trap itr;

main(ac,av)
 char *av[];
 {extern int errno;
  int idv,ino;
  int gth,eof,*aeof,ast,st[18];
  register int t;
  register char *p,*p0;
  aeof=(&eof);
  ast=st;
  tty=(-1);
  inibf(&obf,1);
  inibf(&dbf,2);
  st[0]=st[1]=st[2]=0;
  if (fstat(0,ast)<0) open("/dev/null",0);
  if ((MTY&st[2])==CTY) {tin=1; tty=0;};
  if (fstat(1,ast)<0) dup(0);
  idv=st[0]; ino=st[1];
  if ((MTY&st[2])==CTY) {obf.bb=1; tty=1;};
  if (obf.bb==0 && seek(1,0,1)<0 && errno==ESPIPE) obf.bb=1;
  if (fstat(2,ast)<0) dup(1);
  if (st[0]==idv && st[1]==ino) oed=1;
  if ((MTY&st[2])==CTY) {dbf.bb=1; tty=2;};
  code=BCD;
  maxcol=80;
  iniky();
  stfm(&ifm);
  icx.dsp=(&ifm);
  icx.ocx=(&zcx);
  zcx.kw0=icx.kw0=ikw;
  zcx.kw1=icx.kw1=ikw->nx;
  icx.nam=(ac>0? av[0]:"send");
  icx.lvl=(-1);
  p0=p=icx.nam;
  while (t=(*p++))
   {if (t=='/') p0=p;};
  gth=mtch(4,"gath",p0);
  gth=|mtch(4,"gath",p0+1);
  usage((gth? "gath":"send"));
  if (gth)
    icx.flg[F'g']=icx.flg[F'l']=icx.flg[F'q']=1;
  eof=0;
  ac--; av++;
  setrp(&itr);
  if (!signal(1,1)) signal(1,sig1);
  if (!signal(2,1)) signal(2,sig2);
  if (!signal(3,1)) signal(3,sig3);
  for (t=4;t<NSIG;t++) signal(t,sigx);
  while (!eof && !abort)
   {if (--ac<0)
     {if (!didf) doarg(&icx,1,"-",aeof);
      break;};
    p0=(*av++);
    for (p=p0;*p;p++);
    doarg(&icx,p-p0,p0,aeof);};
  if (icx.sfd) doarg(&icx,0,0);
  flush(&obf);
  if (tmpf) {flush(&qbf); close(qbf.fd);};
  if (abort) {prf("Aborted. (571)\n"); goto X;};
  if (!gth || qat) prf("%d cards.\n",cnt);
  if (!qat) {flush(&dbf); exit((error? ~0:0));};
  if (!cue) goto X;
  if (error)
   {prf("Errors detected. Send anyway? (572)  ");
    ss[0]=0;
    if (read(2,ss,1)==1)
     {ss[1]=ss[0];
      while (ss[1]!='\n' && read(2,ss+1,1)==1);};
    if ((0137&ss[0])!='Y') goto X;};
  flush(&dbf);
  for (t=1;t<NSIG;t++) signal(t,1);
  execl(qer,"qer",tmpf,home,0);
  prf("Cannot execute queuer, %s. (573)\n",qer);
  X: flush(&dbf);
  if (tmpf) unlink(tmpf);
  exit(~0);};

usage(s)
 char *s;
 {int u,*au;
  char *px;
  register int t,d;
  register char *p;
  au=(&u);
  p="/usr/pw/usg";
  t=0377&getuid();
  t=+2; t=<<1;
  if (t>=512) return;
  prf("%q%s/%s",ss,p,s);
  if ((d=open(ss,2))>=0)
   {if (seek(d,t,0)<0 || read(d,au,2)!=2) u=0;
    u++;
    if (seek(d,t,0)>=0) write(d,au,2);
    close(d); return;};
  if ((d=creat(ss,0666))<0) return;
  px=(p=tbf.bf)+512;
  time(p); p=+4;
  while (p<px) *p++=0;
  tbf.bf[t]=1;
  write(d,tbf.bf,512);
  close(d);};

int set14;

sig14()
 {signal(14,sig14);
  if (set14) alarm(1);};

getty()
 {static int x;
  register int t;
  if (x==0)
   {x++;
    sig14();
    alarm(set14=10);
    t=open("/dev/tty",2);
    alarm(set14=0);
    signal(14,sigx);
    if (t>=0) tty=t;};
  return (tty);};

/*end*/
