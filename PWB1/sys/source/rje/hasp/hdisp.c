#
#define SFILES 0
#define	QUEDIR	"/usr/rje/sque"
#define QLEN 200
#define QUELOG	"/usr/rje/sque/log"
/* haspdisp */



hdisp() {return ("~|^`hdisp.c 1.57 4/23/76");};

int inhib;
char *who;

char strx[200];

int pwid -1;

char pwnm[10], pwdr[50];

char usgdr[50];

int huid;

char *hdir;

struct jdat    {char *jnb;	/*job number*/
		char *jnm;	/*job name*/
		char *pnm;	/*pgmr name*/
		char *unm;	/*user name*/
		char *rje;	/*rje dir*/
		char *xmt;	/*xmit stamp*/
		int   lvl;	/*msg level*/
		int   err;	/*retry limit*/
		int   uid;	/*numeric id*/
		char *dir;	/*login dir*/
		char *tty;	/*login tty*/
		char *esbf;	/*next free slot*/
		char sbf[200];	/*string buf*/};

char jobsout[] "jobsout";

char resp[] "resp";

char trtab[]   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		' ',0,0,0,0,0,0,0,0,0,'[','.','<','(','+','|',
		'&',0,0,0,0,0,0,0,0,0,'!','$','*',')',';','^',
		'-','/',0,0,0,0,0,0,0,0,']',',','%','_','>','?',
		0,0,0,0,0,0,0,0,0,'`',':','#','@','\'','=','"',
		0,'a','b','c','d','e','f','g','h','i',0,0,0,0,0,0,
		0,'j','k','l','m','n','o','p','q','r',0,0,0,0,0,0,
		0,'~','s','t','u','v','w','x','y','z',0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		'{','A','B','C','D','E','F','G','H','I',0,0,0,0,0,0,
		'}','J','K','L','M','N','O','P','Q','R',0,0,0,0,0,0,
		'\\',0,'S','T','U','V','W','X','Y','Z',0,0,0,0,0,0,
		'0','1','2','3','4','5','6','7','8','9',0,0,0,0,0,0};

int xmit(), badf(), prnt(), pnch(), mesg(), lerr();

int (*fcn[])() {0,&xmit,&badf,&prnt,&pnch,&mesg,&lerr};

main(ac,av)
 char *av[];
 {int t; char file[36];
  sig14();
  who=av[1];
  hdir=".";
  huid=0;
  if (ac>3 && stat(av[3],file)>=0)
   {hdir=av[3]; huid=0377&file[7];};
  prf("%q%s/usg",usgdr,hdir);
  if (stat(usgdr,file)<0) usgdr[0] = 0;
  inhib = 0;
  if (ac>4)
	if (av[4][0] == 'i')
		inhib++;
  for (;;) {t=getf(file); (*fcn[t])(file);};};

int set14;

sig14()
 {signal(14,sig14);
  if (set14) alarm(1);};

getf(file)
 char *file;
 {register int t,c;
  register char *p;
  int ds; char *p0,bf[16];
 L: *(p=bf)=0; p0=p+16;
  while ((t=getp(p))==1 && *p && ++p<p0);
  c=bf[0];
  if (t==0 && c=='0' && p==(bf+1))
   {/*type(1,"halted");*/ exit(0);};
  if (t==1 && c>='1' && c<='6' && p<p0)
   {prf("%q%s",file,bf+1);
    *p++='\n'; t=p-bf;
    if ((ds=open("log",1))>=0)
     {seek(ds,0,2); write(ds,bf,t); close(ds);};
    return (c-'0');};
  if (p>bf) type(1,"pipe garbage");
  if (p>=p0) while ((t=getp(bf))==1 && *bf);
  if (t==1) goto L;
  if (t!=0) type(1,"pipe error");
/*type(1,"rebooting");*/
  prf("%q%sinit",bf,who);
  execl(bf,"+",bf,0);
  prf("%qcannot execute %s for reboot",strx,bf);
  type(1,strx);
  exit(~0);};

getp(q)
 char *q;
 {static int c;
  static char *p;
  static char bf[20];
  if (c<=0) c=read(0,p=bf,20);
  if (c<=0) return (c);
  c--; *q=(*p++); 
  if (*q=='\n') *q=0;
  return (1);};

type(ds,s)
 char *s;
 {int c,t;
  char *p,bf[200];
  c=7;
  for (t=2;t>0;t--)
   {write(ds,&c,1);
    if (t>1) sleep(2);};
  p=prf("%q\n\n",bf);
  if (ds==1) p=prf("%q%s: ",p,who);
  p=prf("%q%s\n\n%c",p,s,7);
  write(ds,bf,p-bf);};

len(s)
 char *s;
 {register char *p;
  for (p=s;*p;p++);
  return (p-s);};

copy(s0,s1)
 char *s0,*s1;
 {register char *p0,*p1;
  p0=s0; p1=s1;
  while (*p1++=(*p0++));
  return (p1-1);};

eq(s0,s1)
 char *s0,*s1;
 {register char *p0,*p1;
  p0=s0; p1=s1;
  while (*p0) if (*p0++!=(*p1++)) return (0);
  if (*p1) return (0); else return (~0);};

char *mark()
 {static char bf[30];
  int tvec[2];
  time(tvec);
  copy(ctime(tvec)+11,bf);
  bf[8]=0; return (bf);};

zdat(d)
 struct jdat *d;
 {register struct jdat *q;
  q=d; q->lvl=q->err=0; q->uid=(-1);
  q->esbf = q->sbf;
  q->jnb=q->jnm=q->pnm=q->unm=q->rje=q->xmt=q->dir=q->tty="";};

zdatj(d)
 struct jdat *d;
 {register struct jdat *q;
  q=d; q->uid=(-1);
  q->jnb=q->jnm=q->pnm=q->xmt=q->dir=q->tty="";};

zdatu(d)
 struct jdat *d;
 {register struct jdat *q;
  q=d; q->lvl=q->err=0; q->uid=(-1);
q->unm=q->rje=q->xmt=q->dir=q->tty="";};

xmit(file)
 char *file;
 {sent(0,file);};

lerr(file)
 char *file;
 {int n,fbf[259];
  char bf[150];
  n=0;
  if (fopen("log",fbf)>=0)
   {while (getl(fbf,bf))
     {if (eq(bf+1,file)) n++;};
    close(*fbf);};
  if (n<=0) n=20;
  sent(n,file);};

sent(c,file)
 char *file;
 {register int t;
  register char *p;
  int l,ds,fbf[259];
  int uflg,jflg;
  char *q,*sp[2],bf[150],info[30];
  struct jdat x,g;
  zdat(&x); zdat(&g);
  uflg = jflg = 0;
  if (fopen(file,fbf)>=0)
   {for (l=0;l<100;l++)
     {if ((t=getc(fbf))<0 || t>80) break;
      for (p=bf;t>0;t--) *p++=trtab[0377&getc(fbf)];
      *p=0;
	if (bf[0] != '/' && bf[0] != '*')
		continue;
	if (jflg == 0)
		if ((jflg = exjobc(bf,&x,uflg)) > 1)
			uflg++;
	if (uflg == 0)
		uflg = exusr(bf,&x,jflg);
	if (uflg && jflg) break;
  }
    close(*fbf);};
  if (stat(file,bf)>=0)
   {x.uid=0377&bf[7];
    t=(077600&(bf[9]<<7))+(0177&(bf[11]>>1));
    if (c==0) usage("in",x.uid,t+1);};
  prf("%qinfo/logx%s",info,file+4);
  if (fopen(info,fbf)>=0)
   {if (getl(fbf,bf))
     {sp[1]=g.sbf;
      sp[0]=bf;
      g.unm=delim('|',sp);
      p=delim('|',sp);
      if (*p==0) t=(-1); else
       {for (t=0;*p;p++) t=(10*t)+(*p-'0');};
      g.uid=(t>=0? t:-1);
      g.dir=delim('|',sp);
      g.tty=delim('|',sp);};
    close(*fbf);};
  if (x.uid==g.uid)
   {if (*x.unm==0) x.unm=g.unm;
    if (eq(x.unm,g.unm)) {x.dir=g.dir; x.tty=g.tty;};};
  if (*x.unm==0) x.unm=getnm(x.uid);
  if (*x.dir==0) x.dir=getdir(x.unm);
  p=mark();
  if (c!=0) 
   {if (x.err<=0) x.err=3;
    if (x.err>9) x.err=9;
    if (c<x.err) q="line error - resubmitting";
    else {q="line error - giving up";
          unlink(file); unlink(info);};
    l=5;}
   else {unlink(file); unlink(info);
         q="transmitted"; l=3;
         if (*x.jnm==0 || *x.pnm==0) l=7;};
  if (file[0]=='z') file[0]='x';
  prf("%q%s %s/%s %s: name=%s, pgmr=%s.",strx,p,hdir,file,q,x.jnm,x.pnm);
  nfy(l,&x,strx);
  prf("%q%s|%s|%s %s|%s|%s|%d|%d|%s|%s\n",
       strx,x.jnm,x.pnm,p,file,x.unm,x.rje,x.lvl,x.uid,x.dir,x.tty);
  if ((*x.jnm==0) || (inhib)) return;
  if ((ds=open(jobsout,1))>=0 || (ds=creat(jobsout,0645))>=0)
   {seek(ds,0,2);
    write(ds,strx,len(strx));
    close(ds);};};

badf(file)
 char *file;
 {char *p,bf[36];
  struct jdat x;
  zdat(&x);
  if (stat(file,bf)>=0) x.unm=getnm(0377&bf[7]);
  p=enter(file,&x,"badf");
  prf("%q%s %s wrong format for xmit queue.",strx,mark(),p);
  nfy(7,&x,strx);};

prnt(file)
 char *file;
 {int xp,l,fbf[259];
  int uflg,jflg;
  char *p,bf[150];
  struct jdat o,q,x;
  xp=(-1);
  zdat(&o);
  uflg = jflg = 0;
  if (fopen(file,fbf)>=0)
   {for (l=0;l<200;l++)
     {bf[8]=bf[4]=0;
      if (getl(fbf,bf)==0) break;
	if ((bf[0] == '/') || (bf[0] == '*'))
		p = &bf[0];
	else if ((bf[8] == '/') || (bf[8] == '*'))
		p = &bf[8];
	else if (bf[4] == '*')
		p = &bf[4];
	else continue;
	if (jflg == 0)
		if ((jflg = exjobc(p,&o,uflg)) > 1)
			uflg++;
	if (uflg == 0)
		uflg = exusr(p,&o,jflg);
	if (uflg && jflg) break;
 }
    seek(*fbf,-300,2);
    fbf[1]=0;
    while (getl(fbf,bf))
     {if (bf[0]!='*') continue;
      if (exterm(bf,&x)) {
        if (!eq(o.jnb,x.jnb) || !eq(o.jnm,x.jnm))
         {if (uflg == 0) zdat(&o);
          o.jnb=x.jnb; o.jnm=x.jnm; o.pnm=x.pnm;};
        break;};};
    close(*fbf);};
  p=enter(file,&o,"prnt");
  prf("%q%s %s",bf,mark(),p);
  post(2,&o,bf,xp);};

pnch(file)
 char *file;
 {char *p0,bf[100];
  register char *p,*pp;
  register int t;
  int xp,ds,c;
  struct jdat q;
  xp=(-1);
  zdat(&q);
  if ((ds=open(file,0))<0) goto Z;
  t=read(ds,bf,81);
  if (t!=81||bf[0]!=80) goto Y;
  if (bf[1]==0377)
   {p=bf+25; t=45;
    while (--t>0) *p++=trtab[0377&*p];
    pp = q.sbf;
    q.jnb = pp;
    for (p=bf+60;*p==' ';p++);
    for (p0=p;(t=(*p0))>='0' && t<='9';p0++);
    if (t!=' ') goto Y;
    if ((t=(p0-p))>4) goto Y;
    while (t<3) {*pp++='0'; t++;};
    while (p<p0) *pp++=(*p++);
    *pp++='\0';
    locase(q.jnm);
    q.jnb = pp;
    for (p=bf+29;(t=(*p++))!=' ';*pp++=t);
    *pp++ = 0; locase(q.jnb);
    q.esbf = pp;
    goto X;};
  if (bf[1]!=0152) goto Y;
  p = q.sbf; q.jnb = p;
  for (t=42;t<82;t=+10)
   {if ((c=0377&bf[t])==0152)
     {c=0160; if (t<52) continue;};
    if (c==0160) c=0; else c=- 0220;
    if (c<0 || c>9) goto Y;
    *p++=(c+'0');};
  *p++='\0';
  q.esbf = p;
X:	for(c=0;c<100;c++) {
		if (read(ds,bf,1) < 0)
			break;
		if ((t=bf[0]) < 0 || t > 80)
			break;
		read(ds,bf,t);
		p = &bf[0];
		while(t--)
			*p++ = trtab[0377 & *p];
		*p = 0;
		if ((bf[0] == '/' || bf[0] == '*') && exusr(&bf[0],&q,1))
			break;
	}
Y: close(ds);
 Z: p=enter(file,&q,"pnch");
  prf("%q%s %s",bf,mark(),p);
  post(2,&q,bf,xp);};

mesg(file)
 char *file;
 {int fbf[259];
  char bf[512];
  register int ds,t;
  struct jdat m;
  if (inhib) {
		unlink(file);
		return(0);
  }
  if (fopen(file,fbf)>=0)
   {
    while (getl(fbf,bf))
      if (exjobon(bf,&m)) {post0(&m,bf); continue;}
    close(*fbf);};
  unlink(file);};

enter(file,d,pfx)
 char *file,*pfx;
 struct jdat *d;
 {static char efile[60];
  int sw,sqn,slm,t,siz,fdv,ddv,sb[18];
  char *p,dir[41],bf[36];
  char pref[7], bufo[QLEN+1], buf1[QLEN+1];

  if (d->uid<0) d->uid=getuid(d->unm);
  if (stat(file,sb)<0) return (file);
  siz=(077600&(sb[4]>>1))+(0177&(sb[5]>>9));
  if (*pfx=='p') usage("out",d->uid,siz+1);
  fdv=sb[0];
	chmod(file,0454);
	copy(pfx,pref);
	if (pref[0] != 'p') {
			prf("%q%s/job",dir,hdir);
			goto putdef;
	}
	copy(d->rje,dir);
	if (dir[0]) {
		p = dir;
		while(*++p);
		while(*--p == '/')
			*p = '\0';
	}
	if  (!dir[0] ||  dir[0] == '/') {
		if (dir[0] == '/' && stat(dir,sb) >= 0 && (sw = sb[2] & 01)
			&& (sb[2] & 040000) == 0)
				goto xeqf;
		if (!SFILES) {
			unlink(file);
			return(file);
		}
	}
	if (*d->unm != 0 && (p=getdir(d->unm)) != 0) {
			prf("%q%s/%s",efile,p,dir);
			copy(efile,dir);
	}
	else {
		if (SFILES)
			prf("%q%s/job",dir,hdir);
		else {
			unlink(file);
			return(file);
		}
	}
	if (stat(dir,sb) < 0) {
		copy(dir,efile);
		p = efile + len(efile) - 1;
		while(*--p != '/');
		*p = '\0';
		if (stat(efile,sb) < 0) {
			unlink(file);
			return(file);
		}
		else
			sw = 0;
	}
	else
		sw = sb[2] & 040001;
xeqf:	if (sw == 0) {
		ddv=sb[0];
		if (ddv!=fdv && fsfree(ddv,sb)>=0)
			if (sb[0]<(siz+1200) || sb[1]<50) {
				prf("%q%s/job",dir,hdir);
				ddv = fdv;
				goto putdef;
			}
		copy(dir,efile);
		goto mvfile;
	}
	else {
		if ((sw & 040000) == 0) {
			prf("%q%sq",pref,who);
			copy(dir,bufo);
			copy(QUEDIR,dir);
			stat(dir,sb);
		}
		else
			sw = 0;
		ddv=sb[0];
		if (ddv!=fdv && fsfree(ddv,sb)>=0)
			if (sb[0]<(siz+1200) || sb[1]<50) {
				prf("%q%s/job",dir,hdir);
				ddv = fdv;
			}
	putdef:	for (sqn=0; sqn<1000;sqn++) {
			prf("%q%s/%s%d",efile,dir,pref,sqn);
			if (stat(efile,sb) < 0) break;
		}
		if (sqn >= 1000) {
			unlink(file);
			return(file);
		}
	mvfile:	if (ddv==fdv) {
			unlink(efile);
			link(file,efile);
			unlink(file);
		}
		else
			if (fcopy(file,efile) < 0) {
				unlink(file);
				return(-1);
			}
		if (sw != 0) {
			copy(&bufo,&buf1);
	prf("%q%s:%s%d:%s:%s:%s:%s:",bufo,buf1,pref,sqn,d->jnm,d->pnm,d->jnb,d->unm);
			p = bufo + len(bufo);
			while (p != &bufo[QLEN])
				*p++ = ' ';
			bufo[QLEN-1] = '\n';
			if ((sw = open(QUELOG,2)) < 0) {
				sw = creat(QUELOG,0644);
				close(sw);
				sw = open(QUELOG,2);
			}
			for(t=0;t<5;t++) {
				bufo[QLEN] = buf1[QLEN]  = '\0';
				seek(sw,0,2);
				write(sw,bufo,QLEN);
				seek(sw,-QLEN,1);
				read(sw,buf1,QLEN);
				if (eq(bufo,buf1))
					break;
			}
			close(sw);
		}
		return(efile);
	}
}

nfy(l,d,s)
 struct jdat *d;
 char *s;
 {int lv,ds;
  char *p;
  if ((lv=d->lvl)==0) lv=54;
  if (l>=(lv%10) && (p=getty(d->unm,d->tty))!=0)
   {alarm(set14=10);
    ds=open(p,1);
    alarm(set14=0);
    if (ds>=0)
     {type(ds,s); close(ds); return;};};
  if (l>=(lv/10)) mail(d,s);};

mail(d,s)
 struct jdat *d;
 char *s;
 {int ds,tv[2];
  char *p,bf[200];
  p=d->dir;
  if (*p==0) p=getdir(d->unm);
  if (*p==0) p=".";
  prf("%q%s/.mail",bf,p);
  if ((ds=open(bf,1))>=0)
   {time(tv);
    p=prf("%qFrom rje %s%s\n\n",bf,ctime(tv),s);
    seek(ds,0,2);
    write(ds,bf,p-bf);
    close(ds);};};

fcopy(f0,f1)
 char *f0,*f1;
 {char f0x[40];
  int d0,d1,a0,a1,fk;
  if ((d1=creat(f1,0454))<0) return (-1);
  if ((fk=tfork())==(-1)) {unlink(f1); return (-1);};
  if (fk==0) {close(d1); return (0);};
  chmod(f0,0454);
  d0=open(f0,0);
  close(0);
  a0=d0+'0'; a1=d1+'0';
  prf("%q%s/%s",f0x,hdir,f0);
  prf("%q%scopy",strx,who);
  execl(strx,strx,&a0,&a1,f0x,f1,0);
  prf("%qCouldn't execute %scopy - see %s.\n",strx,who,f0x);
  write(d1,strx,len(strx));
  exit(~0);};

tfork()
 {int t,fk;
  fk=(-1);
  for (t=0;t<10;t++)
   {if ((fk=fork())!=(-1)) break; sleep(1);};
  return (fk);};

getl(fbf,line)
 int *fbf;
 char *line;
 {register int t;
  register char *p,*p0;
  do t=getc(fbf); while (t=='\n' || t==014);
  if (t<0) return (0);
  p=line; p0=p+145;
  while (*p++=t)
   {if (p>p0) p--;
    if ((t=getc(fbf))>=' ') continue;
    if (t=='\t') t=' ';
    if (t<0 || t=='\n') t=0;};
  return (~0);};

char *putl(s,p)
 char *s,*p;
 {return (copy("\n",copy(s,p)));};

getuid(usr)
 char *usr;
 {int rv;
  rv=(-1);
  if (pwget(usr,-1)) rv=pwid;
  return (rv);};

getnm(uid)
 {static char usr[10];
  char *rp;
  rp="";
  if (pwget("",uid)) copy(pwnm,rp=usr);
  return (rp);};

getty(s,ttyx)
 char *s,*ttyx;
 {static char tty[20];
  char usr[10],bf[512],*rp;
  register char *p,*p0,*rs;
  int t,ds,let;
  if (*s==0) return (0);
  let=(*ttyx);
  p=usr; p0=p+8; rs=s;
  while ((*p++=(*rs++)) && p<=p0);
  for (p--;p<p0;*p++=' ');
  *p0=0; rp=0;
  if ((ds=open("/etc/utmp",0))>=0)
   {while ((t=read(ds,bf,512))>0)
     {p0=(p=bf)+t;
      for (;p<p0;p=+16)
       {t=0377&(*(p+8));
        *(p+8)=0;
        if (eq(p,usr))
         {if (rp==0 || t==let)
           {copy(&t,copy("/dev/tty",tty)); rp=tty;};
          if (t==let) break;};};};
    close(ds);};
  return (rp);};

getdir(usr)
 char *usr;
 {static char gdir[50];
  char *rp;
  rp="";
  if (pwget(usr,-1)) copy(pwdr,rp=gdir);
  return (rp);};

post0(d,s)
 struct jdat *d; char *s;
 {char *bp,bf[512],line[150];
  int t,ds,fnd,fbf[259];
  struct jdat q;
  fnd=0;
  if (fopen(jobsout,fbf)>=0)
   {while (fnd==0 && getl(fbf,line))
     {exjbout(line,&q);
      if (eq(q.jnm,d->jnm) && eq(q.pnm,d->pnm)) fnd=1;};
    if (fnd==0) close(*fbf);};
  if (fnd==0) {zdat(&q); q.pnm=d->pnm;};
  if (fnd!=0) nfy(5,&q,s);
  else return;
  seek(*fbf,0,0); fbf[1]=0;
  unlink(jobsout);
  while (getl(fbf,line))
   {exjbout(line,&q);
    if (eq(q.jnm,d->jnm) && eq(q.pnm,d->pnm)) break;
    prf("%qJob not acknowledged: name=%s, pgmr=%s.",strx,q.jnm,q.pnm);
    nfy(5,&q,strx);};
  if ((ds=creat(jobsout,0645))>=0)
   {while (getl(fbf,line))
     {bp=putl(line,bf);
      write(ds,bf,bp-bf);};
    close(ds);};
  close(*fbf);};

post(c,d,s,xp)
 struct jdat *d; char *s;
{
     prf("%q%s %s.%s ready",strx,s,d->jnb,d->jnm);
    nfy(6,d,strx);};

exjbout(s,d)		/*jnm,pnm,xmt,unm,rje,lvl,uid,dir,tty*/
 char *s; struct jdat *d;
 {char *p,*sp[2]; int t;
  zdat(d);
  sp[0]=s; sp[1]=d->sbf;
  d->jnm=delim('|',sp);
  d->pnm=delim('|',sp);
  d->xmt=delim('|',sp);
  d->unm=delim('|',sp);
  d->rje=delim('|',sp);
  p=delim('|',sp);
  for (t=0;*p;p++) t=(t*10)+(*p-'0');
  d->lvl=t;
  p=delim('|',sp);
  if (*p==0) t=(-1); else
   {for (t=0;*p;p++) t=(10*t)+(*p-'0');};
  d->uid=(t>=0? t:-1);
  d->dir=delim('|',sp);
  d->tty=delim('|',sp);};

delim(c,sp)
 char *sp[];
 {register int t;
  register char *s,*p;
  s=sp[0]; p=sp[1];
  while ((t=(*s++)) && t!=c) *p++=t;
  *p++=0; if (t==0) s--;
  t=sp[1]; sp[0]=s; sp[1]=p;
  return (t);};

sk(p,s)
 char *p,*s;
 {register char *rp,*rs,*rr;
  for (rp=p;*rp;rp++)
   {rs=s; rr=rp;
    while (*rs && *rs==(*rr++)) rs++;
    if (*rs==0) return (rr);};
  return (0);};

locase(s)
 char *s;
 {register char *p;
  register int c;
  for (p=s;*p;p++)
   {if ((c=(*p))>='A' && c<='Z') *p=c+('a'-'A');};};

exjobon(s,d)		/*jnb,jnm,pnm*/
 char *s; struct jdat *d;
 {register char *sp,*p,*q;
  int t,c;
  zdat(d);
  if ((p=exjnumb(s,d))==0) goto Z;
  sp=d->sbf; while (*sp++);
  if (s[0]!='$')
   {if ((p=sk(p," $HASP100 "))==0) goto Z;}
  else
   {if ((p=sk(p," ON RM"))==0) goto Z;
    if ((p=sk(p,".RD1"))==0) goto Z;
    if ((p=sk(p," -- "))==0) goto Z;};
  while (*p==' ') p++;
  d->jnm=sp;
  while (*p && *p!=' ') *sp++=(*p++);
  *sp++=0; locase(d->jnm);
  if (s[0]!='$')
   {if ((p=sk(p," ON R"))==0) goto Z;
    if ((p=sk(p,".RD1 "))==0) goto Z;};
  if ((q=sk(p," PR "))!=0 && *q>='0' && *q<='9'
        && *++q>='0' && *q<='9' && *++q==0)
   {for (q=- 6;*q==' ';q--) *q=0;};
  while (*p==' ') p++;
  d->pnm=sp;
  while (*sp++=(*p++));
  locase(d->pnm);
  return (~0);
 Z: zdat(d); return (0);};

exjnumb(s,d)
 char *s; struct jdat *d;
 {register int t;
  register char *p,*q;
  char *p0;
  p=s;
  if ((p=sk(p," JOB "))==0) goto Z;
  while (*p==' ') p++;
  for (p0=p;(t=(*p0))>='0' && t<='9';p0++);
  if (t!=' ') goto Z;
  if ((t=p0-p)>4) goto Z;
  d->jnb=q=d->sbf;
  while (t<3) {*q++='0'; t++;};
  while (p<p0) *q++=(*p++);
  *q=0; return (p0);
 Z: zdat(d); return (0);};

exjobc(s,d,fflg)		/*jnb,jnm,pnm,unm,rje,lvl,err*/
 char *s; struct jdat *d; int fflg;
 {register char *sp,*p;
  register int c;
  int t,flag,ret; char *p0;
  if (fflg == 0) {
	zdat(d);
	sp = d->sbf;
  }
  else {
	zdatj(d);
	sp = d->esbf;
  }
	ret = 1;
  p=s;
  if (*p++!='/'||*p++!='/') goto Z;
  d->jnm=sp;
  while (*p && *p!=' ') *sp++=(*p++);
  *sp++=0; locase(d->jnm);
  if (p<(s+3) || p>(s+10)) goto Z;
  while (*p==' ') p++;
  p0=p+4;
  if ((p=sk(p,"JOB "))!=p0) goto Z;
  while (*p==' ') p++;
  if (*p!='(') goto Z;
  if ((p=sk(p,")"))==0) goto Z;
  if (*p++!=',') goto W;
  d->pnm=sp; p0=p;
  for (flag=0;;)
   {if ((c=(*p++))=='\'')
     {if ((c=(*p++))!='\'') flag=~flag;};
    if (c==0) break;
    if (flag==0)
     {if (c==' ' || c==',') break;
      if (c=='=') {p=p0; sp=d->pnm; break;};};
    *sp++=c;};
  if (sp>(d->pnm+20)) {p=p0; sp=d->pnm;};
  *sp++=0; locase(d->pnm);
 W: if (fflg != 0) goto X;
 if ((p=sk(p-1," USR="))==0) goto X;
  if (*p=='(') {p++; flag=1;} else flag=0;
  d->unm=sp;
  while ((c=(*p++)) && c!=' ' && c!=',' && c!=')') *sp++=c;
  if (flag!=0 && c!=',' && c!=')') sp=d->unm;
  if (flag==0 && c==')') sp=d->unm;
  *sp++=0; locase(d->unm);
  if (getuid(d->unm)<0) d->unm="";
  if (flag==0 || c!=',') {d->unm = ""; goto X;}
  d->rje=sp;
  while ((c=(*p++)) && c!=',' && c!=')') *sp++=c;
  if (c==0) sp=d->rje;
  *sp++=0; locase(d->rje);
	ret++;
  if (c!=',') goto X;
  for (t=0;c=(*p);p++)
   {if (c<'0' || c>'9') break;
    t=(t*10)+(c-'0');};
  if (c!=')' && c!=',') goto X;
  if (t>=0 && t<100) d->lvl=t;
  if (c!=',') goto X;
  c=(*p++);
  if (c==')') goto X;
  d->err=1;
  if (c=='y' || c=='Y') d->err=0;
  if (c>='0' && c<='9') d->err=c-'0';
 X: p=s+(len(s)-4);
  while(*p<'0' || *p>'9')
   {if (*p==0) goto Y; p++;};
  for (p0=p;(t=(*p0))>='0' && t<='9';p0++);
  if (t!=0) goto Y;
  if ((t=p0-p)>4 || t<=0) goto Y;
  d->jnb=sp;
  while (t<3) {*sp++='0'; t++;};
  while (p<p0) *sp++=(*p++);
  *sp++ = 0;
 Y: d->esbf = sp; return (ret);
 Z: if (fflg == 0) zdat(d);
    else zdatj(d);
    return(0);
}

exterm(s,d)		/*jnb,jnm,pnm*/
 char *s; struct jdat *d;
 {register char *p,*sp;
  register int t;
  char *p0;
  int jes2;
  zdat(d);
  p=s; sp=d->sbf;
  jes2=(sk(p,"JOBNAME=")? 0:1);
  if (jes2)
   {if ((p=sk(p," JOB "))==0) goto Z;}
  else
   {if ((p=sk(p,"JOBNAME="))==0) goto Z;
    d->jnm=sp;
    while (*p && *p!=' ') *sp++=(*p++);
    *sp++=0; locase(d->jnm);
    if ((p=sk(p,"JOB #="))==0) goto Z;};
  while (*p==' ') p++;
  for (p0=p;(t=(*p0))>='0' && t<='9'; p0++);
  if (t!=' ') goto Z;
  if ((t=p0-p)>4) goto Z;
  d->jnb=sp;
  while (t<3) {*sp++='0'; t++;};
  while (p<p0) *sp++=(*p++);
  *sp++=0;
  if (jes2)
   {while (*p==' ') p++;
    d->jnm=sp;
    while (*p && *p!=' ') *sp++=(*p++);
    *sp++=0; locase(d->jnm);};
  while (*p==' ') p++;
  d->pnm=sp;
  while (*p && *p!=' ') *sp++=(*p++);
  *sp=0; locase(d->pnm);
  return (~0);
 Z: zdat(d); return (0);};

pwget(s,n)
 char *s;
 {int rv,id,t,fbf[259];
  char *sp[2],bf[150];
  register char *p;
  if (*s==0 && n<0) return (0);
  if (*s!=0 && eq(s,pwnm)) return (1);
  if (n>=0 && n==pwid) return (1);
  rv=0;
  if (fopen("/etc/passwd",fbf)>=0)
   {while (getl(fbf,bf))
     {sp[0]=sp[1]=bf;
      p=delim(':',sp);
      if (len(p)>8) continue;
      if (*s!=0 && !eq(s,p)) continue;
      copy(p,pwnm);
      delim(':',sp);
      p=delim(':',sp);
      for (id=0;t=(*p++);) id=(10*id)+(t-'0');
      if (id<0 || id>255) continue;
      if (n>=0 && n!=id) continue;
      pwid=id;
      delim(':',sp);
      p=delim(':',sp);
      if (len(p)>40) continue;
      copy(p,pwdr);
      rv=1; break;};
    close(*fbf);};
  if (rv==0)
   {pwid=(-1); *pwnm=0; *pwdr=0;};
  return (rv);};

usage(s,id,n)
 char *s;
 {char *p;
  int k,bf[256];
  register int d,t,c;
  if (*usgdr==0) return;
  if (id<2 || id>255) id=huid;
  if (id<2 || id>255) return;
  id=<<1;
  for (t=0;t<3;t++)
   {if (t==0) p="cnt";
    if (t==1) p="max";
    if (t==2) p="sum";
    prf("%q%s/%s.%s.%s",strx,usgdr,who,s,p);
    p=strx;
    if ((d=open(p,2))>=0)
     {if (seek(d,id,0)<0 || read(d,&k,2)!=2) k=0;
      if (t==0) k++;
      if (t==1 && k<n) k=n;
      if (t==2) k=+n;
      if (seek(d,id,0)>=0) write(d,&k,2);
      close(d); continue;};
    if ((d=creat(p,0666))<0) continue;
    for (c=0;c<256;c++) bf[c]=0;
    time(bf);
    bf[id>>1]=(t==0? 1:n);
    write(d,bf,512);
    close(d); continue;};};

exusr(s,d,fflg2)
 char *s; struct jdat *d; int fflg2;
{register char *sp,*p;
 register int c;
 int flag,t;

    zdatu(d);
    p = s + 3;
    if (fflg2 > 0)
		sp = d->esbf;
    else
		sp = d->sbf;
    if ((p=sk(p-1," USR="))==0) goto Z;
  if (*p=='(') {p++; flag=1;} else flag=0;
  d->unm=sp;
  while ((c=(*p++)) && c!=' ' && c!=',' && c!=')') *sp++=c;
  if (flag!=0 && c!=',' && c!=')') sp=d->unm;
  if (flag==0 && c==')') sp=d->unm;
  *sp++=0; locase(d->unm);
  if (getuid(d->unm)<0) d->unm="";
  if (flag==0 || c!=',') goto Z;
  d->rje=sp;
  while ((c=(*p++)) && c!=',' && c!=')') *sp++=c;
  if (c==0) sp=d->rje;
  *sp++=0; locase(d->rje);
  if (c!=',') goto X;
  for (t=0;c=(*p);p++)
   {if (c<'0' || c>'9') break;
    t=(t*10)+(c-'0');};
  if (c!=')' && c!=',') goto X;
  if (t>=0 && t<100) d->lvl=t;
  if (c!=',') goto X;
  c=(*p++);
  if (c==')') goto X;
  d->err=1;
  if (c=='y' || c=='Y') d->err=0;
  if (c>='0' && c<='9') d->err=c-'0';
X: d->esbf = sp; return(~0);
Z: zdatu(d); return(0);
}
