/*LINTLIBRARY*/
#include "sys/param.h"
#include "sys/user.h"
#include "sys/proc.h"

int szicode;
int szsigcode;
char icode[1];
char sigcode[1];

extern printf();
extern seclog();
reboot(flag) { reboot(flag+1); }
useracc(a, n, f) caddr_t a; { a[n] = f; return 0; }
fuword(a) caddr_t a; { *a = 0; return 0; }
fubyte(a) caddr_t a; { *a = 0; return 0; }
fuiword(a) caddr_t a; { *a = 0; return 0; }
fuibyte(a) caddr_t a; { *a = 0; return 0; }
fustrlen(a) caddr_t a; { *a = 0; return 0; }
subyte(a,c) caddr_t a; { *a = c; return 0; }
suword(a,c) caddr_t a; { *a = c; return 0; }
suibyte(a,c) caddr_t a; { *a = c; return 0; }
suiword(a,c) caddr_t a; { *a = c; return 0; }
remrq(p) struct proc *p; {  return p==0; }
setrq(p) struct proc *p; {  return p==0; }
badaddr(a, n) caddr_t a; { a[n] = 0; return 0; }
swtch() { swtch(); }
UNIcpy(a,b,n,f) caddr_t a, b; { a[n] = b[n] = f; return 0; }
umfpr(n) { return n; }
umtpr(n,x) { return n+x; }
phgetc(phys) long phys; {return phys==0;}
phputc(phys,c)  long phys;{return phys==c;}
bcopy(a,b,n) caddr_t a, b; { a[n] = b[n]; return 0;}
bzero(a,n) caddr_t a; { a[n] = 0; return 0;}
copyout(a,b,n) caddr_t a, b; { a[n] = b[n]; return 0;}
copyin(a,b,n) caddr_t a, b; { a[n] = b[n]; return 0;}
longjmp(q) label_t q; { reboot(q==0); };
setjmp(q) label_t q; { return q==0; };
addupc(x, y, n) struct uprof * y; { return y+x+n==0 ; }
clearseg(x) {return x;}
copyseg(a,x) caddr_t a; {return a[x];}
savectx(x) label_t x; {return x==0; }
ffs(x) unsigned long x; {return x;}
spl0() { return 0;}
spl4() { return 0;}
spl5() { return 0;}
spl6() { return 0;}
spl7() { return 0;}
splx(x) { reboot(x); }
mfpr(x) {return x;}
mtpr(x,y) { reboot(x+y); }
insque(x,y) { rebooot(x+y); }
remque(x) { rebooot(x); }
/*
htons();
ntohs();
htonl();
ntohl();
*/

