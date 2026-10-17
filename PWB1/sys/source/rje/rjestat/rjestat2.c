#
/* rjestat2.c */


/* This information is proprietary and is the property of Bell
   Telephone Laboratories, Incorporated.  Its reproduction or
   disclosure to others, either orally or in writing, is pro-
   hibited without written permission of Bell Laboratories. */


#define IDMOD rstat2

#define IDSTR "~|^`rjestat2.c 1.4 1/29/76

#include "rjestat.h"

struct fcns
 {int (*fnam)();
  int farg;
  int fdel;};

struct fcns *fcn0,*fcn1,*fcnx;

#define NFCN 20

struct fcns fcntab[NFCN];

inifcn()
 {fcn0=fcn1=fcntab;
  fcnx=fcntab+NFCN;};

enq(fcn,arg,del)
 int (*fcn)();
 {register struct fcns *r;
  register int fnd,zdl;
  zdl=(del<0? 0:del);
  fnd=0;
  for (r=fcn0;r<fcn1;r++)
   {if (fnd)
     {if (r->fnam==fcn && r->farg==arg) deq(r--);
      continue;};
    if (del<r->fdel)
     {if (r->fnam!=fcn || r->farg!=arg) upq(r);
      r->fnam=fcn; r->farg=arg; r->fdel=zdl;};
    if (r->fnam==fcn && r->farg==arg) fnd=1;};
  if (fnd==0 && r<fcnx)
   {fcn1++;
    r->fnam=fcn; r->farg=arg; r->fdel=zdl;};};

deq(rr)
 struct fcns *rr;
 {register struct fcns *r;
  fcn1--;
  for (r=rr;r<fcn1;r++)
   {r->fnam=(r+1)->fnam;
    r->farg=(r+1)->farg;
    r->fdel=(r+1)->fdel;};};

upq(rr)
 struct fcns *rr;
 {register struct fcns *r;
  if (fcn1<fcnx) fcn1++;
  for (r=fcn1-1;r>rr;r--)
   {r->fnam=(r-1)->fnam;
    r->farg=(r-1)->farg;
    r->fdel=(r-1)->fdel;};};

tmq(d)
 {register struct fcns *r;
  register int t;
  for (r=fcn0;r<fcn1;r++)
   {t=r->fdel-d;
    if (t<0) t=0;
    r->fdel=t;};};

xeq()
 {register struct fcns *r;
  register int (*fcn)(),arg;
  r=fcn0;
  if (r>=fcn1) exit(0);
  if (r->fdel<=0)
   {fcn=r->fnam;
    arg=r->farg;
    deq(r);
    (*fcn)(arg);};
  return (r->fdel);};


struct opls
 {struct opls *onxt;
  int ocnt;
  char *odat;
  int otim[2];};

struct opls *opl0,*opl1;

#define NOPL 7

struct opls opltab[NOPL];

#define NOPX 500

char opx[NOPX];

iniopl()
 {register struct opls *r;
  register int t;
  r=opltab+(NOPL-1);
  for (t=0;t<NOPL;t++)
   {r=r->onxt=opltab+t;};
  opl0=opl1=r;};

char *getopl(n)
 {register struct opls *r,*r0;
  register char *p;
  r=opl1; r0=opl0;
  if (r==r0) r->odat=opx;
  p=r->odat+n;
  if (r->odat>=r0->odat)
   {if (p<=opx+NOPX) return (r->odat);
    p=(r->odat=opx)+n;};
  while (r0!=r && p>r0->odat) r0=r0->onxt;
  opl0=r0;
  return (r->odat);};

putopl(n)
 {register struct opls *r;
  register char *p;
  register int *v;
  r=opl1;
  r->ocnt=n;
  p=r->odat;
  v=r->otim;
  r=r->onxt;
  opl1=r;
  if (r==opl0) opl0=r->onxt;
  r->odat=p+n;
  if ((flag&FINT)==0) write(1,p,n);
  time(v);};

backsp()
 {extern display();
  static struct opls *bk;
  register struct opls *r;
  register int t;
  if ((flag&FBKS)==0)
   {flag=|FBKS;
    for (r=opl0;r!=opl1 && del2(r->otim,itim)>((r->ocnt+60)>>5);) r=r->onxt;
    opl0=bk=r;};
  if (flag&FINT)
   {bk=opl0; return;};
  r=bk;
  bk=r->onxt;
  if (r==opl1)
   {flag=& ~FBKS;
    for (t=0;t<nlin;t++) enq(display,t,0);
    return;};
  write(1,r->odat,r->ocnt);
  time(r->otim);
  enq(backsp,0,(r->ocnt+20)>>5);};

/*end*/
