#
/* send4.c */


/* This information is proprietary and is the property of Bell
   Telephone Laboratories, Incorporated.  Its reproduction or
   disclosure to others, either orally or in writing, is pro-
   hibited without written permission of Bell Laboratories. */


#define IDMOD send4

#define IDSTR "~|^`send4.c 1.5 1/15/76

#include "send.h"

struct keywd *bkw;

iniky()
 {register int t;
  register struct keywd *pk;
  t=sbrk(12);
  ikw=pk=t;
  t=+12;
  bkw=pk->nx=t;
  pk->lk=pk->nk=pk->nr=0;
  pk->kp=pk->rp="";};

gtky(ctx,n,s)
 struct context *ctx;
 char *s;
 {register int m;
  register struct keywd *pk,*qk;
  m=n;
  qk=ctx->kw1;
  for (pk=ikw;pk<qk;pk=pk->nx)
   {if (pk->nk==m && mtch(m,pk->kp,s)) return (pk);};
  return (0);};

gtkz(ctx,n,s)
 struct context *ctx;
 char *s;
 {register int m;
  register struct keywd *pk,*qk;
  m=n;
  qk=ctx->kw1;
  for (pk=ikw;pk<qk;pk=pk->nx)
   {if (pk->nk==m && mtch(m,pk->kp,s))
     {if (pk->nr>0 || pk->rp<=0377) return (pk);};};
  return (0);};

dfky(ctx,nks,ks,nrs,rs)
 struct context *ctx;
 char *ks,*rs;
 {int tb;
  char *q,*px;
  register int t;
  register char *p;
  register struct keywd *pk;
  if ((t=nks)<=0) return;
  if (pk=gtkz(ctx,t,ks))
   {if (pk->nr==(t=nrs) && mtch(t,pk->rp,rs))
     {if (t>0 || pk->rp==rs)
      {if (pk>=ctx->kw0) return;
        goto CP;};};
    if (diag(ctx,"cannot redefine keyword",0,591))
     {p=pk->kp;
      for (t=pk->nk;t>0;t--) prc(*p++);
      prc('=');
      p=pk->rp;
      if (p<=0377)
       {t=p;
        prc('^');
        t=>>4; t=& 017;
        prc((t<10? t+'0':t+'A'-10));
        t=p; t=& 017;
        prc((t<10? t+'0':t+'A'-10));}
      else for (t=pk->nr;t>0;t--) prc(*p++);
      prf("\n");};
    return;};
  CP: t=ctx->kw1;
  t=+12;
  q=t;
  t=+nks;
  t=+nrs;
  if (t&1) t++;
  pk=t;
  if (pk>bkw)
   {tb=(-DBR)&(t+DBR);
    if (brk(tb)==(-1))
     {prf("Core exceeded. (574)\n"); abt();};
    bkw=tb;};
  pk=ctx->kw1;
  ctx->kw1=t;
  pk->nx=t;
  t=ks[0];
  pk->lk=kwx[t];
  kwx[t]=pk;
  pk->nk=nks;
  pk->nr=nrs;
  pk->kp=q;
  px=(p=ks)+nks;
  while (p<px) *q++=(*p++);
  if (rs<=0377)
   {pk->rp=rs; return;};
  pk->rp=q;
  px=(p=rs)+nrs;
  while (p<px) *q++=(*p++);};

/*end*/
