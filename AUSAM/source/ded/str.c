#include "ded.h"

int streq(p_s1,p_s2)
char *p_s1, *p_s2;
 { register char c;
    register char *s1, *s2;

    s1=p_s1; s2=p_s2;
    while ((c = *s1++) == *s2++)
      if (c==0) return(true);

    return(false);
 }

int streqtail(str,tail)
char *str, *tail;
 { while (*str != 0)
     { if (streq(str,tail)) return(true);
	str++;
     }
 }

int anytail(str, tails)
char *str, *tails[];
 { while (*tails != 0)
      if (streqtail(str, *tails)) return(true);
      else tails++;
    return(false);
 }

int alpha(c)
char c;
 { return( ('a'<=c && c<='z') || ('A'<=c && c<='Z') ); }

char lcase(c)
char c;
 { return('A'<=c && c<='Z' ? c+('a'-'A') : c); }

int digit(c)
char c;
 { return('0'<=c && c<='9'); }

int alphmer(c)
char c;
 { return(alpha(c) || digit(c)); }

/* a procedure to ensure that you always produce a filename with
 * .old (or .dlog or whatever)
 */
ncat2(s1,s2,s)
char *s1,*s2,*s;
 { char *p, *p1;
    char c, ns1[ENOUGH];
    int l;

    p = p1 = s1;
    while ((c = *p++) != 0)
      if (c=='/') p1 = p;

    if ((l = strlength(p1)+strlength(s2)) > 14)
     { cat2(s1,"",ns1);
	ns1[strlength(ns1)-(l-14)] = 0;
	cat2(ns1,s2,s);
     }
    else
      cat2(s1,s2,s);
 }

cat2(s1,s2,s)
char *s1,*s2,*s;
 {  char *p;

    p = s;
    while (*p++ = *s1++);
    p--;
    while (*p++ = *s2++);
 }

cat3(s1,s2,s3,s)
char *s1,*s2,*s3,*s;
 { cat2(s1,s2,s);
    cat2(s,s3,s);
 }

int strlength(p_str)
char *p_str;
 { register int count;
    register char *str;

    str = p_str;
    count=0;
    while (*str++ != 0) count++;

    return(count);
 }

char *dtos(num,str)
int num;
char *str;
 { if (num>9) str = dtos(num/10,str);
    *str++ = (num % 10) + '0';
    *str = 0;
    return(str);
 }
