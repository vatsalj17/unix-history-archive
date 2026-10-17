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
