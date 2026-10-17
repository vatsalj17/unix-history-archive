#
/* rjestat4.c */


/* This information is proprietary and is the property of Bell
   Telephone Laboratories, Incorporated.  Its reproduction or
   disclosure to others, either orally or in writing, is pro-
   hibited without written permission of Bell Laboratories. */


#define IDMOD rstat4

#define IDSTR "~|^`rjestat4.c 1.5 4/22/76

#include "rjestat.h"

struct dstr
 {int df,dd,dc;
  char *dp,db[512];};

struct dstr *dctx[NLIN];

inidsp()
 {register struct dstr **r;
  struct dstr **rx;
  char *p;
  int w,k;
  w=sizeof(**r)+1;
  if (w&1) w++;
  p=sbrk(nlin*w);
  if (p&1) p++;
  rx=(r=dctx)+nlin;
  for (;r<rx;r++)
   {k=p;
    p=+w;
    *r=k;
    k->df=0;
    k->dd=(-1);
    k->dc=0;};};

#define NDSP 120

display(z)
 {register struct dstr *r;
  register char *p;
  register int t;
  char *p0,*p1,*px,f[40];
  r=dctx[z];
  if (flag&(FINT|FBKS)) return;
  if ((flag&(FDSP|FSTA))!=0 && r->df<2)
   {enq(display,z,8); return;};
  p=getopl(NDSP+10);
  px=(p0=p)+NDSP;
  if (r->df<2) *p++='\n';
  p=prf("%q%s: ",p,line[z]->hst);
  for (p1=p;;)
   {if (r->dd<0)
     {prf("%q%s/resp",f,line[z]->dir);
      if ((r->dd=open(f,0))<0)
       {if (r->df<1) r->df=1; goto X;};
      if (r->df<1) seek(r->dd,-1,2);};
    while (--r->dc>=0)
     {t=(*r->dp++);
      if (r->df<1)
       {if (t=='\n') r->df=1; continue;};
      if (p>px) p=px;
      if (t!='\t') *p++=t; else
       {for (t=7&(p-p1);t<8;t++) *p++=' ';};
      if (t!='\n') continue;
      *p=0;
      if (p==(p1+1)) r->df=1;
      if (mtch(2,"??",p1) || scan(" STARTED ",p1)) { p=p1; continue;};
      if (scan(" ON R",p1) && scan(".RD1 ",p1)) {p=p1; continue;};
      if (scan("$UX",p1) && scan("INVALID",p1))
       {t=r->df;
        r->df=(scan(pid,p1)? 2:1);
        if (t==2 && r->df==1) goto X;
        p=p1; continue;};
      if (r->df<2) {p=p1; continue;};
      putopl(t=p-p0);
      enq(display,z,(t+20)>>5);
      goto Z;};
    while ((r->dc=read(r->dd,r->dp=r->db,512))<=0)
     {fstat(r->dd,f);
      if (r->dc<0 || f[6]==0)
       {r->df=(r->dc<0? 0:1);
        close(r->dd);
        r->dd=(-1); goto X;};
      if (p!=p1) {sleep(1); continue;};
      t=(r->df>1? 1:5);
      enq(display,z,t);
      goto Z;};};
 X: enq(display,z,4);
 Z: if (r->df<2) flag=& ~FDSP; else flag=|FDSP;};

/*end*/
