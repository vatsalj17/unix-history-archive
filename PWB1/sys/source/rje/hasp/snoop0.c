/* snoop */


/* This information is proprietary and is the property of Bell
   Telephone Laboratories, Incorporated.  Its reproduction or
   disclosure to others, either orally or in writing, is pro-
   hibited without written permission of Bell Laboratories. */


int dv 0;	/* minor device */

int tx,tr[6];

char ln[200],*lnp;

int nlx[12] {'_d','qs','tr',0,0,0,0,0,0,0};

main()
 {int dm,x,ox;
  char *px;
  register int t,c;
  register char *p;
  nlist("/unix",nlx);
  if (nlx[4]<=0)
   {write(2,"/unix: _dqstr=?\n",16); exit(~0);};
  ox=nlx[5];
  if ((dm=open("/dev/mem",0))<0)
   {write(2,"/dev/mem?\n",10); exit(~0);};
  for (x=0;;)
   {seek(dm,ox,0);
    read(dm,&tx,2);
    if (x==tx) {sleep(1); continue;};
    seek(dm,2*x,1);
    read(dm,tr,12);
    if ((x=+6)>=60) x=0;
    if ((0377&tr[0])!=dv) continue;
    lnp=ln;
    t=tr[1];
    c=0377&t;
    if (0200&c) c=|0177400;
    prf("%d ",c);
    c=t>>8;
    if (c<1 || c>20) prf("C%d",c);
    else {if ((t=c)>10) t =- 10;
      *lnp++=(t<=6? 'R':'W');
      if (t!=c) *lnp++='T';
      if (t>6) t =- 6; t--;
      *lnp++='0'+t;};
    prf("\t0%o\t",tr[2]);
    t=tr[5];
    c='0';
    p=lnp;
    *p++=c+(3&(t>>6));
    *p++=c+(7&(t>>3));
    *p++=c+(7&t);
    *p++=' ';
    *p++=c+(3&(t>>14));
    *p++=c+(7&(t>>11));
    *p++=c+(7&(t>>8));
    lnp=p;
    prf("\t  %d\t%o\t",tr[3],tr[4]);
    c=0377&(tr[0]>>8);
    time(tr); p=tr+1;
    t=0377&(*p); *p++=c;
    if (t<c)
     {if (*p==0) tr[0]--; (*p)--;};
    p=ctime(tr)+11;
    *(px=p+8)='\n';
    while (p<=px) *lnp++=(*p++);
    write(1,ln,lnp-ln);};};

prf(s,a)
 char *s;
 {register int c,*aa;
  register char *p;
  p=s;
  aa=(&a);
  while (c=(*p++))
   {if (c!='%') {*lnp++=c; continue;};
    if ((c=(*p++))==0) break;
    if (c=='c') *lnp++=(*aa++);
    if (c=='d') prd(*aa++);
    if (c=='o') pro(*aa++);
    if (c=='s') prs(*aa++);};};

prd(n)
 {register int m,k;
  if ((m=n)<0) {*lnp++='-'; m=(-m);};
  if (k=m/10) prd(k);
  *lnp++='0'+m%10;};

pro(n)
 {register int k;
  if (k=017777&(n>>3)) pro(k);
  *lnp++='0'+(7&n);};

prs(s)
 char *s;
 {register int i;
  register char *p;
  p=s;
  while (i=(*p++)) *lnp++=i;};

/*end*/
