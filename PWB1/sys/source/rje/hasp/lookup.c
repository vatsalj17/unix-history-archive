#define NLIN 6
#define NLIX 300

struct lins
 {char *hst,*sys,*dir,*pfx,*dev,*pno;};

lookup(pf,dr,dv,pn)
 char *pf,*dr,*dv,*pn;
 {struct lins *q,*qx,line[NLIN];
  char *p,bf[40],linx[NLIX];
  register int t;
  t=rjetab("lines",line,NLIN*6,linx,NLIX);
  qx=(q=line)+(t/6);
  p=rjesys();
  for (;q<qx;q++)
   {if (scan(pf,q->pfx)==0 || scan(q->pfx,pf)==0) continue;
    if (*p==(*q->sys)) break;
    if (scan(p,q->sys)==0) continue;
    cat(bf,q->dir,"/pool/...");
    if ((t=creat(bf,0666))<0) continue;
    close(t); break;};
  if (q>=qx) return (-1);
  t=nargs();
  if (t>1) cat(dr,q->dir);
  if (t>2) cat(dv,q->dev);
  if (t>3) cat(pn,q->pno);
  return (0);};

