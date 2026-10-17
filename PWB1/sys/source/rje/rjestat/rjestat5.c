#
/* rjestat5.c */


/* This information is proprietary and is the property of Bell
   Telephone Laboratories, Incorporated.  Its reproduction or
   disclosure to others, either orally or in writing, is pro-
   hibited without written permission of Bell Laboratories. */


#define IDMOD rstat5

#define IDSTR "~|^`rjestat5.c 1.6 1/29/76

#include "rjestat.h"

struct msgs
 {struct msgs *mnxt;
  int mcnt;
  char *mdat;};

#define NMSG 35
#define NMSX 600

struct mstr
 {struct msgs *msg0,*msg1;
  struct msgs msgtab[NMSG];
  char msx[NMSX];};

struct mstr *mctx[NLIN];

inimsg()
 {register struct mstr **r;
  register struct msgs *m;
  register int t;
  struct mstr **rx;
  char *p;
  int w,k;
  w=sizeof(**r)+1;
  if (w&1) w++;
  p=sbrk(nlin*w);
  if (p&1) p++;
  rx=(r=mctx)+nlin;
  for (;r<rx;r++)
   {k=p;
    p=+w;
    *r=k;
    m=k->msgtab+(NMSG-1);
    for (t=0;t<NMSG;t++)
     {m=m->mnxt=k->msgtab+t;};
    k->msg0=k->msg1=m;};};

char *getmsg(z,n)
 {register struct msgs *m,*m0;
  register char *p;
  struct mstr *r;
  r=mctx[z];
  m=r->msg1; m0=r->msg0;
  if (m==m0) m->mdat=r->msx;
  p=m->mdat+n;
  if (m->mdat>=m0->mdat)
   {if (p<=r->msx+NMSX) return (m->mdat);
    p=(m->mdat=r->msx)+n;};
  while (m0!=m && p>m0->mdat) m0=m0->mnxt;
  r->msg0=m0;
  return (m->mdat);};

putmsg(z,n)
 {register struct msgs *m;
  register struct mstr *r;
  register char *p;
  r=mctx[z];
  m=r->msg1;
  m->mcnt=n;
  p=m->mdat;
  m=m->mnxt;
  r->msg1=m;
  if (m==r->msg0) r->msg0=m->mnxt;
  m->mdat=p+n;};

mesg(z)
 {register struct mstr *r;
  register char *p,*q;
  char *px,*qx,f[40],bf[200];
  int t,ds,uvac,sb[18];
  r=mctx[z];
  if (r->msg0==r->msg1) return;
  prf("%q%s/%smesg",f,line[z]->dir,line[z]->pfx);
  if ((ds=creat(f,0))<0)
   {if (stat(f,sb)>=0 && (0777&sb[2])==0 && del2(sb+16,rtim)>20)
     unlink(f);
    enq(mesg,z,7); return;};
  px=(p=bf)+200;
  uvac=scan("1110",line[z]->hst);
  q=(uvac? "@@":"/*");
  p=prf("%q%s$UX%s INVALID",p+1,q,pid);
  if (!uvac)
   {qx=bf+72;
    while (p<qx) *p++=' ';
    *p++='N';};
  *bf=p-(bf+1);
  for (;r->msg0!=r->msg1;r->msg0=r->msg0->mnxt)
   {t=r->msg0->mcnt;
    if ((p+t)>px) break;
    *p++=t;
    q=r->msg0->mdat;
    qx=q+t;
    for (;q<qx;q++) *p++=(*q);};
  px=p;
  if (!uvac)
   {for (p=bf;p<px;)
     {t=(*p++);
      q=p+t;
      for (;p<q;p++) *p=trt[0377&(*p)];};}
  write(ds,bf,px-bf);
  close(ds);
  chmod(f,0444);
  enq(mesg,z,15);};

cull()
 {extern sigi(),mesg(),backsp();
  register char *p;
  register int t,z;
  char *p0,*p1,*px,*q;
  int f,x,bad,uvac;
  signal(SIGI,1);
  signal(14,1);
  x=0;
  prf("\n");
  for (z=0;z<nlin;z++)
   {uvac=scan("1110",line[z]->hst);
    for (f=0;f==0;)
     {p=getmsg(z,80);
      px=(p0=p)+80;
      q=(uvac? "@@":"/*$");
      p=prf("%q%s",p,q);
      if (!plus) *p++='d';
      *p=0; p1=p;
      prf("%s: %s",line[z]->hst,p0);
      bad=0;
      while ((t=read(1,&x,1))>0 && x!='\n')
       {if (trt[x=& 0177]==0) bad=1;
        if (p<px) *p++=x;};
      if (t<=0) exit(0);
      if (bad) goto E;
      while (*(p-1)==' ') p--;
      if (p<=p1) break;
      if (*(p-1)=='\\') p--; else f=1;
      *p=0;
      if (!uvac && !plus && !permit(p1))
       {E: prf("Illegal entry.\n");
        f=0; continue;};
      putmsg(z,p-p0);
      enq(mesg,z,0);};};
  signal(SIGI,sigi);
  sig14();
  flag=& ~FINT;
  enq(backsp,0,-1);};

permit(s)
 char *s;
 {return (1);};

/*end*/
