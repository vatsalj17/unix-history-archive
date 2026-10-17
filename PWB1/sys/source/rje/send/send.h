/* send.h */


/* This information is proprietary and is the property of Bell
   Telephone Laboratories, Incorporated.  Its reproduction or
   disclosure to others, either orally or in writing, is pro-
   hibited without written permission of Bell Laboratories. */


IDMOD() {return (IDSTR (1.15)");};

#define F 037&

#define LNL 301
#define LNX 302
#define NTB 22
#define NDR 12
#define DBR (1<<7)

#define BCD 1
#define ASC 2
#define SIG 4

#define ENG 1
#define ETB 2
#define EDT 4
#define ELL 8
#define EEX 16

#define SLS 0141
#define STR 0134
#define DOL 0133

#define TFIL  0
#define TCOM  1
#define TEOF  2
#define TINP  3
#define TTTY  4
#define TPIN  5
#define TPTY  6
#define TSFL  7
#define TRFL  8
#define TPKY  9
#define TDHX 10
#define TDKY 11
#define TEXC 12
#define TDOL 13
#define TFMT 14
#define TMSG 15
#define TVFL 16
#define TCHD 17

#define MTY 060044
#define CTY 020000

#define NSIG 15

#define ESPIPE 29

#define NHST 5

#define NLIN 6
#define NLIX 300

struct iobf
 {int fd,bc,bb;
  char *bp,bf[512];};

struct format
 {int ffd,ffe,ffm,ffs,fft;
  char ftab[NTB];};

struct keywd
 {struct keywd *nx,*lk;
  int nk,nr;
  char *kp,*rp;};

struct context
 {struct context *ocx;
  char flg[32];
  struct format *dsp;
  struct keywd *kw0,*kw1;
  char *nam;
  int lno,lvl,typ;
  int sfd;
  char stm[24];
  int dfl,dsv[NDR];};

struct trap
 {struct trap *str;
  int ssp,sr5,spc;};

struct hsts
 {int hcnt;
  char *hnam;};

int abort,error;

int didf;

int code,maxcol;

int qat,cnt,cue;

int tin,oed,tty;

char *qer,*tmpf,*home;

char ss[LNX];

struct iobf obf,dbf,qbf,tbf;

struct keywd *ikw,*kwx[128];

int nhst;

struct hsts host[NHST];

char trt[128],rtr[256];

/*end*/
