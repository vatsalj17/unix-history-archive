#
/* send5.c */


/* This information is proprietary and is the property of Bell
   Telephone Laboratories, Incorporated.  Its reproduction or
   disclosure to others, either orally or in writing, is pro-
   hibited without written permission of Bell Laboratories. */


#define IDMOD send5

#define IDSTR "~|^`send5.c 1.15 1/16/76

#include "send.h"

qout(nn,s)
 char *s;
 {char *tnm,*px,*qx;
  static char *who;
  register int i;
  register char *p,*q;
 int n;
  qat=1;
  n = nn;
  if (who==0)
   {if (n<4) goto E;
    p=s; q=ss;
    for(i=0;i<4;i++) *q++=0137&rtr[0377&*p++];
    if (mtch(4,"@RUN",ss))
     {px=(p=s)+n;
      while (p<px) *p++=rtr[0377&*p];
      ascii(0);
      maxcol=132;
      for (p=s;p<px;) *p++=trt[*p];
      who="uvac";};
    if (s[0]==SLS) who="hasp";
    p=s; q=ss;
    *q++=rtr[0377&*p++];
    for (i=1;i<4;i++) *q++=0137&rtr[0377&*p++];
    for (;i<80;i++) *q++=0377&rtr[0377&*p++];
	q = &ss[4];
    if (mtch(4,"!JOB",ss)) {
	while(*q++ != ',') ;
	while(*q++ != ',') ;
	while(*q != ',' && *q != ' ' && *q != '\0') q++;
	if (*q != ',') {who = "??"; goto E;};
     px=(p=s)+n;
      while (p<px) *p++=rtr[0377&*p];
      ascii(1);
      maxcol=80;
      for (p=s;p<px;) *p++=trt[*p];
      who = "sig5";
    }
    if (who==0) goto E;
    if ((tnm=site(who))==0) goto C;
    if ((i=mktmp(tnm,0))<0)
     {prf("Cannot create temporary %s. (575)\n",tnm); goto C;};
    tmpf=tnm;
    inibf(&qbf,i);};
  C: p=s;
  if ((*(p+2)==DOL && *p==SLS && *(p+1)==STR && code==BCD && n>=3)
/*
     || (*(p+2)=='B' && *p==']' && *(p+1)=='R' && code==ASC && n>=3)
*/
)
   {E: return (-1);};
  if (tmpf)
   {cnt++; cue=1;
    p=s+n;
    while (n>0 && *--p==trt[' ']) n--;
    px=(p=s)+n;
    q=qbf.bp; qx=qbf.bf+512;
    *q++=n;
    while (p<px)
     {if (q>=qx)
       {qbf.bp=q; q=flush(&qbf);};
      *q++=(*p++);};
    qbf.bp=q;
    if (q>=qx) flush(&qbf);};
  return (0);};

struct lins
 {char *hst,*sys,*dir,*pfx,*dev,*phne;};

site(s)
 char *s;
 {static char qnm[40],tnm[40],hnm[40];
  struct lins *linz,line[NLIN];
  char *u,linx[NLIX];
  int n,t;
  register int i;
  register char *p;
  register struct lins *q;
  for (p=s;*p;p++);
  n=p-s;
  q=line;
  i=rjetab("lines",q,NLIN*6,linx,NLIX);
  linz=q+(i/6);
  u=rjesys();
  for (;q<linz;q++)
   {if (!mtch(n,s,q->pfx)) continue;
    for (i=0;i<NHST;i++)
     {if (nhst>=0 && i!=nhst) continue;
      if (mtch(host[i].hcnt,host[i].hnam,q->hst)) break;};
    if (i>=NHST) continue;
    p=q->sys;
    if (*u==0 || *u!=(*p))
     {while (*++p!=0) {if (*u==(*p)) break;};
      if (*u!=(*p)) continue;
      prf("%q%s/pool/...",tnm,q->dir);
      if ((t=creat(tnm,0666))<0) continue;
      close(t);};
    if (space(q->dir)<0) abt();
    qer=qnm;
    prf("%q%s/%sqer",qnm,q->dir,q->pfx);
	prf("%q%s",hnm,q->dir);
	home = hnm;
    prf("%q%s/pool/stm",tnm,q->dir);
    return (tnm);};
  if(mtch(4,"sig5",s)) p = "SIGMA";
  else p=(mtch(4,"hasp",s) ? "IBM":"UNIVAC");
  prf("UNIX System %s is not configured for %s RJE",u,p);
  if (nhst>=0) prf(" to %s",host[nhst].hnam);
  prf(". (570)\n");
  return (0);};

rjetab(s,l,n,lx,nx)
 char *s,*l[],lx[];
 {register char *p,**q;
  register int t;
  char *px,**qx,f[40];
  prf("%q/usr/rje/%s",f,s);
  t=open(f,0);
  nx=read(t,lx,nx-1);
  close(t);
  if (nx<=0) return (0);
  px=(p=lx)+nx;
  qx=(q=l)+n;
  while (p<px && q<qx)
   {*q++=p;
    while (p<px && *p!='\t' && *p!='\n') p++;
    *p++=0;};
  return (q-l);};

rjesys()
 {static char u[8];
  register int t;
  u[0]=0;
  t=open("/usr/rje/sys",0);
  read(t,u,1);
  close(t);
  if (u[0]==0) u[0]='X';
  t=0377&u[0];
  if (t>=0140) t=- 040;
  if (t<'A' || t>'Z') t=0;
  u[0]=t; u[1]=0;
  return (u);};

svdir(a,n)
 int a[];
 {int sb[18];
  register int k,t,*p;
  if ((k=n-2)<=0) return (-1);
  stat(".",sb); t=sb[1];
  p=a; *p++=sb[0];
  while (t!=1)
   {k--; *p++=t;
    stat("..",sb); t=sb[1];
    if (t==1) break;
    if (k<=0 || chdir("..")<0) break;};
  if (t!=1) t=0; *p=t;
  return ((t-1)|rsdir(a));};

rsdir(a)
 int a[];
 {int *p0,db[9],sb[18];
  register int f,d,*p;
  db[8]=0;
  for (p=a+1;*p!=0 && *p!=1;p++); p0=p;
  stat(".",sb);
  if (sb[0]!=a[0])
   {if (*p0!=1)  goto E;
    f=0;
    if ((d=open("/",0))<0) goto E;
    while (read(d,db,16)==16)
     {if (db[0]==0) continue;
      db[0]='//';
      stat(db,sb);
      if (sb[0]==a[0]) {f=1; break;};};
    close(d);
    if (f==0 || chdir(db)<0) goto E;};
  for (;;)
   {for (p=p0;p>a;p--)
     {if (*p==sb[1]) break;};
    if (p>a) break;
    if (sb[1]==1 || chdir("..")<0) goto E;
    stat(".",sb);};
  for (p--;p>a;p--)
   {f=0;
    if ((d=open(".",0))<0) goto E;
    while (read(d,db,16)==16)
     {if (db[0]==0) continue;
      stat(db+1,sb);
      if (sb[1]==(*p)) {f=1; break;};};
    close(d);
    if (f==0 || chdir(db+1)<0) goto E;};
  return (0);
  E: chdir ("/"); return (-1);};

space(s)
 char *s;
 {int sb[18];
  register int t;
  register char *p;
  if (stat(s,sb)<0 || fsfree(sb[0],sb)<0) return (0);
  t=0;
  if (sb[0]<1500) t=|1;
  if (sb[1]<50) t=|2;
  if (t==0) return (0);
  prf("Cannot send - only ");
  if (t&1) prf("%d blocks",sb[0]);
  if (t==3) prf(", ");
  if (t&2) prf("%d inodes",sb[1]);
  prf(" free on file system ");
  for (p=s;*p;prc(*p++))
   {if (p>s && *p=='/') break;};
  prf(". (597)\n");
  return (-1);};

fsfree(d,v)
 int v[];
 {int fsb[8];
  register int t;
  t=ustat(d,fsb);
  if (fsb[0]<0) fsb[0]=10000;
  if (fsb[1]<0) fsb[1]=10000;
  *v++=fsb[0];
  *v++=fsb[1];
  return (t);};

/*end*/
