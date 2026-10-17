#
/* send7.c */


/* This information is proprietary and is the property of Bell
   Telephone Laboratories, Incorporated.  Its reproduction or
   disclosure to others, either orally or in writing, is pro-
   hibited without written permission of Bell Laboratories. */


#define IDMOD send7

#define IDSTR "~|^`send7.c 1.5 1/15/76

#include "send.h"

inibf(xbf,descr)
 struct iobf *xbf;
 {register struct iobf *r;
  r=xbf;
  r->fd=descr;
  r->bc=r->bb=0;
  r->bp=r->bf;};

bsp0(xbf)
 struct iobf *xbf;
 {register int n;
  register struct iobf *r;
  r=xbf;
  n=r->bp-r->bf;
  n=r->bc-n;
  if (n>0 && seek(r->fd,-n,1)==(-1))
   {prf("Overrun on standard input. (596)\n");
    abt();};
  r->bc=0;
  r->bp=r->bf;
  return (0);};

getl(xbf,s)
 struct iobf *xbf;
 char *s;
 {char *px,*qx;
  register char *p,*q;
  register struct iobf *r;
  r=xbf;
  px=(p=s)+LNL;
  q=r->bp; qx=r->bf+r->bc;
  for (;;)
   {if (q>=qx)
     {q=r->bf;
      qx=q+(r->bc=read(r->fd,q,(r->bb?1:512)));
      if (q>=qx) {r->bp=q; return (-1);};};
    while (q<qx)
     {if ((*p=0177&(*q++))=='\n')
       {r->bp=q; return (p-s);};
      if (p<px) p++;};};};

get80(xbf,s)
 struct iobf *xbf;
 char *s;
 {char *px,*qx;
  register char *p,*q;
  register struct iobf *r;
  r=xbf;
  px=(p=s)+80;
  q=r->bp; qx=r->bf+r->bc;
  while (p<px)
   {if (q>=qx)
     {q=r->bf;
      qx=q+(r->bc=read(r->fd,q,512));
      if (q>=qx) break;};
    while (q<qx && p<px) *p++=(*q++);};
  r->bp=q;
  return (p-s);};

putl(xbf,n,s)
 struct iobf *xbf;
 char *s;
 {char *px,*qx;
  register char *p,*q;
  register struct iobf *r;
  r=xbf;
  p=s+n;
  while (n>0 && *--p==' ') n--;
  px=(p=s)+n;
  q=r->bp; qx=r->bf+512;
  while (p<px)
   {*q++=(*p++);
    if (q>=qx)
     {r->bp=q; q=flush(r);};};
  *q++='\n';
  r->bp=q;
  if (q>=qx || r->bb) flush(r);};

flush(xbf)
 struct iobf *xbf;
 {extern int errno;
  register int n;
  register char *q;
  register struct iobf *r;
  r=xbf;
  q=r->bf;
  n=r->bp-q;
  r->bp=q;
  if (n>0 && write(r->fd,q,n)!=n)
   {if (r->fd!=2)
     {prf("Write error - ");
      if (errno==27) prf("file too large"); else
      if (errno==28) prf("no space left"); else
      prf("number %d",errno);
      prf(". (576)\n");};
    abt();};
  return (q);};

char *prfq;

prf(s,a)
 char *s;
 {register int c,*aa;
  register char *p;
  p=s;
  aa=(&a);
  while (c=(*p++))
   {if (c!='%') {prc(c); continue;};
    if ((c=(*p++))==0) break;
    if (c=='c') prc(*aa++);
    if (c=='d') prd(*aa++);
    if (c=='o') pro(*aa++);
    if (c=='s') prs(*aa++);
    if (c=='q') prfq=(*aa++);};
  if (p=prfq) *p=prfq=0;
  if (dbf.bb) flush(&dbf);
  return (p);};

prc(c)
 {register char *q;
  if (q=prfq)
   {*q++=c; prfq=q; return;};
  q=dbf.bp;
  *q++=c;
  dbf.bp=q;
  if (q>=dbf.bf+512) flush(&dbf);};

prd(n)
 {register int m,k;
  if ((m=n)<0) {prc('-'); m=(-m);};
  if (k=m/10) prd(k);
  prc('0'+m%10);};

pro(n)
 {register int k;
  if (k=017777&(n>>3)) pro(k);
  prc('0'+(7&n));};

prs(s)
 char *s;
 {register int i;
  register char *p;
  p=s;
  while (i=(*p++)) prc(i);};

/*end*/
