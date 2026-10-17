/* rjecopy */


/* This information is proprietary and is the property of Bell
   Telephone Laboratories, Incorporated.  Its reproduction or
   disclosure to others, either orally or in writing, is pro-
   hibited without written permission of Bell Laboratories. */


rjecopy() {return ("~|^`rjecopy.c 1.1 1/23/76");};

char bf[512];

main(ac,av)
 char *av[];
 {char *f0,*f1,*p;
  register int t,d0,d1;
  d0=(*av[1]-'0');
  d1=(*av[2]-'0');
  f0=av[3];
  f1=av[4];
  while ((t=read(d0,bf,512))>0)
   {if (write(d1,bf,t)!=t) break;};
  if (t==0)
   {unlink(f0); exit(0);};
  chmod(f1,0645);
  if ((t=creat(f1,0454))>=0) seek(d1,0,0);
  chmod(f1,0454);
  p=cat(bf,"\n\n** Read/write error during copy - original is ",f0," **\n\n");
  write(d1,bf,p-bf);
  exit(~0);};

cat(s)
 char *s;
 {char **ax;
  register char *p,*q,**a;
  ax=(a=(&s))+nargs();
  p=(*a++);
  while (a<ax)
   {q=(*a++);
    while (*p++=(*q++));
    p--;};
  *p=0;
  return (p);};

/*end*/
