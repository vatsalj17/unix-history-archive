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
    if (c=='t') prt(*aa++);
    if (c=='q') prfq=(*aa++);};
  if (p=prfq) *p=prfq=0;
  return (p);};

prc(c)
 {register char *q;
  if (q=prfq)
   {*q++=c; prfq=q; return;};
  write(1,&c,1);};

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

prt(v)
 int v[];
 {register char *p;
  p=ctime(v)+11;
  *(p+8)=0;
  prs(p);};

