#
/* send3.c */


/* This information is proprietary and is the property of Bell
   Telephone Laboratories, Incorporated.  Its reproduction or
   disclosure to others, either orally or in writing, is pro-
   hibited without written permission of Bell Laboratories. */


#define IDMOD send3

#define IDSTR "~|^`send3.c 1.10 4/22/76

#include "send.h"

doarg(ctx,n,s,eof)
 struct context *ctx;
 char *s;
 int *eof;
 {static int nsh;
  char *px,*avv,vv[LNX];
  int ndo,k,nv,vt,vx,pd[2];
  struct keywd *pk;
  register int i;
  register char *p;
  register struct context *x;
  avv=vv;
  x=ctx;
  if (abort)
   {if (x->sfd>0) unlink(x->stm);
    x->sfd=0; return (1);};
  px=(p=s)+n;
  for (i=0;p<px;)
   {if ((k=0177&(*p++))=='\\')
     {if (p<px) k=0200|(*p++);};
    ss[i]=k;
    if (i<LNL) i++;};
  if (i>=LNL) i=LNL-1;
  px=(p=avv)+(nv=i);
  for (i=0;p<px;) *p++=0177&ss[i++];
  *p=0;
  vt=type(nv,ss,&vx);
  if (vt==TDOL)
   {if (x->sfd==0)
     {prf("%q/tmp/sh",x->stm);
      x->sfd=i=mktmp(x->stm,nsh++);
      if (i>0) inibf(&tbf,i);};
    if (x->sfd>0) putl(&tbf,nv-1,avv+1);
    return (0);};
  ndo=0;
  if (x->sfd)
   {if (x->sfd<0) {p="(create)"; goto NS;};
    flush(&tbf);
    close(x->sfd);
    if (xpipe(pd)<0) {p="(pipe)"; goto NS;};
    if ((i=tfork())==(-1))
     {p="(fork)";
      NS: k=diag(x,"cannot run shell",p,587);
      goto XS;};
    if (i==0)
     {close(0); open("/dev/null",0);
      close(1); dup(pd[1]);
      execl("/bin/sh","sh",x->stm,0);
      exit(040);};
    close(pd[1]);
    ndo=|dofile(pd[0],x->stm,x,TDOL);
    k=shex(x);
    XS: if (k && (i=open(x->stm,0))>=0)
     {inibf(&tbf,i);
      while ((i=getl(&tbf,ss))>=0) putl(&dbf,i,ss);
      close(tbf.fd);};
    if (x->sfd>0) unlink(x->stm);
    x->sfd=0;};
  switch (vt)
   {case TCOM: break;
    case TEOF: *eof=1; break;
    case TINP: ndo=|dofile(0,"-",x,vt); break;
    case TTTY: ndo=|dofile(dup(getty()),"+",x,vt); break;
    case TMSG: *px='\n'; write(getty(),avv+1,nv); break;
    case TPIN:
      *px='\n';
      if (tin) {write(getty(),avv+2,nv-1); vt=TPTY;};
      *px=0;
      ndo=|dofile(0,avv,x,vt); break;
    case TPTY:
      *px='\n';
      write(getty(),avv+2,nv-1);
      *px=0;
      ndo=|dofile(dup(getty()),avv,x,vt); break;
    case TFMT:
      i=0; ss[i++]='<';
      for (p=avv;p<px;) ss[i++]=(*p++);
      ss[i++]='>';
      if (exfm(i,ss,x->dsp)<0)
        diag(x,"default spec rejected",avv,588);
      ndo=|1; break;
    case TSFL:
      for (p=avv+1;p<px;p++)
       {x->flg[i=F(*p)]=1;
        if (i==(F'f')) fold(1);
        if (i==(F'k'))
         {x->kw0=x->kw1=(x->ocx)->kw1;
          for (i=0;i<128;i++) kwx[i]=0;};};
      ndo=|1; break;
    case TRFL:
      for (p=avv+1;p<px;p++)
       {x->flg[i=F(*p)]=0;
        if (i==(F'f')) fold(0);
        if (i==(F'k'))
         index(x->kw0=(x->ocx)->kw1,x->kw1);};
      ndo=|1; break;
    case TVFL:
      for (p=avv+1;p<px;p++)
       {i=F(*p);
        x->flg[i]=k=(x->ocx)->flg[i];
        if (i==(F'f')) fold(k);
        if (i==(F'k'))
         index(x->kw0=(x->ocx)->kw0,x->kw1);};
      ndo=|1; break;
    case TPKY:
      avv=+2; nv=- 2;
      if (pk=gtky(x,nv,avv))
       {dfky(x,nv,avv,pk->nr,pk->rp); break;};
      *px='=';
      write(getty(),avv,nv+1);
      px=(p=ss)+LNL;
      while (read(getty(),p,1)==1 && *p!='\n')
       {if (p<px) p++;};
      px=p; p=ss;
      for (i=0;p<px;)
       {if ((k=0177&(*p++))=='\\')
         {if (p<px) k=0200|(*p++);};
        ss[i++]=k;};
      if ((k=hex(i,ss))>=0)
       {dfky(x,nv,avv,0,k); break;};
      px=(p=ss)+i;
      while (p<px) *p++=0177&(*p);
      dfky(x,nv,avv,i,ss); break;
    case TDHX:
      dfky(x,nv-4,avv,0,vx); break;
    case TDKY:
      i=vx+1;
      dfky(x,vx,avv,nv-i,avv+i); break;
    case TCHD:
      if (x->dfl==0)
       {svdir(x->dsv,NDR); x->dfl++;};
      if (chdir(avv+vx)<0)
       diag(x,"cannot chdir",avv+vx,589);
      break;
    case TEXC:
      avv++; nv--;
      if (xpipe(pd)<0) {p="(pipe)"; goto NC;};
      if ((i=tfork())==(-1))
       {p="(fork)";
        NC: k=diag(x,"cannot run command",p,590);
        goto XC;};
      if (i==0)
       {close(0); open("/dev/null",0);
        close(1); dup(pd[1]);
        execl("/bin/sh","sh","-c",avv,0);
        exit(040);};
      close(pd[1]);
      ndo=|dofile(pd[0],avv,x,vt);
      k=shex(x);
      XC: if (k) putl(&dbf,nv,avv); break;
    case TFIL:
      ndo=|dofile(open(avv,0),avv,x,vt);};
  return (ndo);};

type(n,s,xp)
 char *s;
 int *xp;
 {char *px;
  register int i,t;
  register char *p;
  i=s[0];
  if (n<=0) return (TCOM);
  if (n==1)
   {if (i=='.') return (TEOF);
    if (i=='-') return (TINP);
    if (i=='+') return (TTTY);};
  if (n>=2 && s[1]==':')
   {if (i=='-') return (TPIN);
    if (i=='+') return (TPTY);
    if (i=='=') return (TPKY);};
  if (i==':')
   {if (n>=2 && s[n-1]==':') return (TFMT);
    return (TMSG);};
  if (i=='!')
   {if (mtch(7,s,"!chdir "))
     {for (i=7;i<n && s[i]==' ';i++);
      for (t=i;t<n && s[t]!=';';t++);
      if (t>=n) {*xp=i; return (TCHD);};};
    return (TEXC);};
  if (i=='$') return (TDOL);
  if (i=='~') return (TCOM);
  if (i=='-' || i=='+' || i=='=')
   {px=(p=s)+n;
    while (++p<px)
     {if ((t=(*p))>=0140) t=& 0137;
      if (t<'A' || t>'Z') break;};
    if (p==px)
     {if (i=='-') return (TSFL);
      if (i=='+') return (TRFL);
      if (i=='=') return (TVFL);};};
  p=s+n;
  while (--p>=s && *p!='=');
  i=p-s;
  if (i>=0)
   {t=i+1;
    t=hex(n-t,s+t);
    if (t>=0) {*xp=t; return (TDHX);};
    *xp=i; return (TDKY);};
  for (i=0;i<NHST;i++)
   {t=host[i].hcnt;
    if (n==t && mtch(t,s,host[i].hnam)!=0)
     {if (nhst<0) nhst=i;
      return (TCOM);};};
  return (TFIL);};

hex(n,s)
 char *s;
 {register int i,h;
  register char *p;
  p=s;
  if (n!=3 || *p++!='^') return (-1);
  h=0;
  i=(*p++);
  if (i>=0140) i=& 0137;
  if (i>='0' && i<='9') h=+(i-'0');
  else if (i>='A' && i<='F') h=+(i+10-'A');
  else return (-1);
  h=<<4;
  i=(*p++);
  if (i>=0140) i=& 0137;
  if (i>='0' && i<='9') h=+(i-'0');
  else if (i>='A' && i<='F') h=+(i+10-'A');
  else return (-1);
  return (h);};

/*end*/
