/*
 * File: rconv.c
 *  Contents: ctype, cvcset, cvint, cvnum, cvpos, cvreal, cvstr, gcvt, mkint,
 *    mkreal, mksubs, strprc
 */

#include "../h/rt.h"
#include <math.h>

/*
 * Structure for mapping string names of procedures to block addresses.
 */
struct pstrnm {
   char *pstrep;
   struct b_proc *pblock;
   };

extern struct b_proc
#define FncDef(p) Cat(B,p),
#include "../h/fdef.h"
   Bnoproc;	/* Hack to avoid ,; in expansion */
#undef FncDef

extern struct b_proc
   Basgn,
   Bbang,
   Bcat,
   Bcompl,
   Bdiff,
   Bdiv,
   Beqv,
   Binter,
   Blconcat,
   Blexeq,
   Blexge,
   Blexgt,
   Blexle,
   Blexlt,
   Blexne,
   Bminus,
   Bmod,
   Bmult,
   Bneg,
   Bneqv,
   Bnonnull,
   Bnull,
   Bnumber,
   Bnumeq,
   Bnumge,
   Bnumgt,
   Bnumle,
   Bnumlt,
   Bnumne,
   Bplus,
   Bpower,
   Brandom,
   Brasgn,
   Brefresh,
   Brswap,
   Bsect,
   Bsize,
   Bsubsc,
   Bswap,
   Btabmat,
   Btoby,
   Bunions,
   Bvalue;

struct pstrnm pntab[] = {
#define FncDef(p) "p", Cat(&B,p),
#include "../h/fdef.h"
#undef FncDef
	":=",           &Basgn,
	"!",            &Bbang,
	"||",           &Bcat,
	"~",            &Bcompl,
	"--",           &Bdiff,
	"/",            &Bdiv,
	"===",          &Beqv,
	"**",           &Binter,
	"|||",          &Blconcat,
	"==",           &Blexeq,
	">>=",          &Blexge,
	">>",           &Blexgt,
	"<<=",          &Blexle,
	"<<",           &Blexlt,
	"~==",          &Blexne,
	"-",            &Bminus,
	"%",            &Bmod,
	"*",            &Bmult,
	"-",            &Bneg,
	"~===",         &Bneqv,
	"\\",           &Bnonnull,
	"/",            &Bnull,
	"+",            &Bnumber,
	"=",            &Bnumeq,
	">=",           &Bnumge,
	">",            &Bnumgt,
	"<=",           &Bnumle,
	"<",            &Bnumlt,
	"~=",           &Bnumne,
	"+",            &Bplus,
	"^",            &Bpower,
	"?",            &Brandom,
	"<-",           &Brasgn,
	"^",            &Brefresh,
	"<->",          &Brswap,
	":",            &Bsect,
	"*",            &Bsize,
	"[]",           &Bsubsc,
	":=:",          &Bswap,
	"=",            &Btabmat,
	"...",          &Btoby,
	"++",           &Bunions,
	".",            &Bvalue,
	0,		0
	};

/*
 * The array is used to establish a "type" for a character.  The
 *  codes in use are:
 *    C - control character
 *    H - hexadecimal digit
 *    L - lower case alphabetic
 *    N - decimal digit
 *    P - punctuation
 *    S - whitespace
 *    U - upper case alphabetic
 *
 * Note that logical or'ing is used to associate more than one attribute
 *  with a character.
 */

/*
 * Macros for determining character type.
 *
 * The table cytpe classifies each character
 *  in one of the categories defined below.
 */

#define _U	01		/* upper case */
#define _L	02		/* lower case */
#define _N	04		/* digit */
#define _S	010		/* space */
#define _P	020		/* punctuation */
#define _C	040		/* control */
#define _X	0100		/* hex digit (a-f) */

#define isalpha(c)	(ctype[(c)&0377]&(_U|_L))
#define isupper(c)	(ctype[(c)&0377]&_U)
#define islower(c)	(ctype[(c)&0377]&_L)
#define isdigit(c)	(ctype[(c)&0377]&_N)
#define isxdigit(c)	(ctype[(c)&0377]&(_N|_X))
#define isspace(c)	(ctype[(c)&0377]&_S)
#define ispunct(c)	(ctype[(c)&0377]&_P)
#define isalnum(c)	(ctype[(c)&0377]&(_U|_L|_N))
#define isprint(c)	(ctype[(c)&0377]&(_P|_U|_L|_N))
#define iscntrl(c)	(ctype[(c)&0377]&_C)
#define isascii(c)	((unsigned)(c)<=0177)
#define toupper(c)	(((c)&0377)-'a'+'A')
#define tolower(c)	(((c)&0377)-'A'+'a')
#define toascii(c)	(((c)&0177)
#define tonum(c)	(isdigit(c)?(c)-'0':10+(((c)|(040))-'a'))

char ctype[] = {
	_C,	_C,	_C,	_C,	_C,	_C,	_C,	_C,
	_C,	_S,	_S,	_S,	_S,	_S,	_C,	_C,
	_C,	_C,	_C,	_C,	_C,	_C,	_C,	_C,
	_C,	_C,	_C,	_C,	_C,	_C,	_C,	_C,
	_S,	_P,	_P,	_P,	_P,	_P,	_P,	_P,
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P,
	_N,	_N,	_N,	_N,	_N,	_N,	_N,	_N,
	_N,	_N,	_P,	_P,	_P,	_P,	_P,	_P,
	_P,	_U|_X,	_U|_X,	_U|_X,	_U|_X,	_U|_X,	_U|_X,	_U,
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U,
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U,
	_U,	_U,	_U,	_P,	_P,	_P,	_P,	_P,
	_P,	_L|_X,	_L|_X,	_L|_X,	_L|_X,	_L|_X,	_L|_X,	_L,
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L,
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L,
	_L,	_L,	_L,	_P,	_P,	_P,	_P,	_C,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0
   };
/*
 * cvcset(d, cs, csbuf) - convert d to a cset and
 *  make cs point to it, using csbuf as a buffer if necessary.
 */

cvcset(d, cs, csbuf)
register struct descrip *d;
int **cs, *csbuf;
   {
   register char *s;
   register word l;
   char sbuf[MaxCvtLen];

   Inc(cv_n_cset);


   if (!Qual(*d) && (d)->dword == D_Cset) {
      Inc(cv_n_rcset);
      *cs = BlkLoc(*d)->cset.bits;
      return T_Cset;
      }

   if (cvstr(d, sbuf) == NULL)
      return NULL;

   for (l = 0; l < CsetSize; l++)
      csbuf[l] = 0;

   s = StrLoc(*d);
   l = StrLen(*d);
   while (l--) {
      Setb(*s, csbuf);
      s++;
      }
   *cs = csbuf;
   return 1;
   }


/*
 * cvint - convert the value represented by dp into an integer and write
 *  the value into the location referenced by i.  cvint returns Integer or
 *  NULL depending on the outcome of the conversion.
 */

cvint(dp, i)
register struct descrip *dp;
long *i;
   {
   union numeric result;
   int minsh, maxsh;

   minsh = MinShort;
   maxsh = MaxShort;

#ifdef RunStats
   Inc(cv_n_int);
   if (!Qual(*dp) && (dp)->dword == D_Integer)
      cv_n_rint++;
#endif RunStats
   /*
    * Use cvnum to attempt the conversion into "result".
    */
   switch (cvnum(dp, &result)) {

      case T_Integer:
         *i = result.integer;
         return T_Integer;

      case T_Longint:
         *i = result.integer;
         return T_Longint;

      case T_Real:
         /*
          * The value converted into a real number.  If it's not in the
          *  range of an integer, return a 0, otherwise convert the
          *  real value into an integer.  As before, distinguish between
          *  integers and long integers if necessary.
          */
         if (result.real > MaxLong || result.real < MinLong)
            return NULL;
         *i = (long)result.real;
#if IntSize == 16
         if (*i < (long)minsh || *i > (long)maxsh)
            return T_Longint;
#endif IntSize == 16
         return T_Integer;

      default:
         return NULL;
      }
   }


/*
 * cvnum - convert the value represented by d into a numeric quantity and
 *  place the value into *result.  Value returned is Integer, D_Real, or
 *  NULL.
 */

/* >cvnum */
cvnum(dp,result)
register struct descrip *dp;
union numeric *result;
   {
   static char sbuf[MaxCvtLen];

   Inc(cv_n_num);
   if (Qual(*dp)) {
      qtos(dp,  sbuf);
      return ston(sbuf, result);
      }

   switch (Type(*dp)) {

      case T_Integer:
         Inc(cv_n_rnum);
         result->integer = (long)IntVal(*dp);
         return T_Integer;

      case T_Longint:
         Inc(cv_n_rnum);
         result->integer = BlkLoc(*dp)->longint.intval;
         return T_Longint;

      case T_Real:
         Inc(cv_n_rnum);
         GetReal(dp,result->real);
         return T_Real;

      default:
         /*
          * Try to convert the value to a string and
          *  then try to convert the string to an integer.
          */
         if (cvstr(dp, sbuf) == NULL)
            return NULL;
         return ston(StrLoc(*dp), result);
      }
   }
/* <cvnum */

#define BIG 72057594037927936.	/* numbers larger than 2^56 lose precision */

/*
 * ston - convert a string to a numeric quantity if possible.
 */
static ston(s, result)
register char *s;
union numeric *result;
   {
   register int c;
   int realflag = 0;	/* indicates a real number */
   char msign = '+';    /* sign of mantissa */
   char esign = '+';    /* sign of exponent */
   double mantissa = 0; /* scaled mantissa with no fractional part */
   int scale = 0;	/* number of decimal places to shift mantissa */
   int digits = 0;	/* total number of digits seen */
   int sdigits = 0;	/* number of significant digits seen */
   int exponent = 0;	/* exponent part of real number */
   double fiveto;	/* holds 5^scale */
   double power;	/* holds successive squares of 5 to compute fiveto */
   int minsh, maxsh;
#ifndef VMS
   extern int errno;
#else VMS
#include <errno.h>
#endif

   minsh = MinShort;
   maxsh = MaxShort;

   c = *s++;

   /*
    * Skip leading white space.
    */
   while (isspace(c))
      c = *s++;

   /*
    * Check for sign.
    */
   if (c == '+' || c == '-') {
      msign = c;
      c = *s++;
      }

   /*
    * Get integer part of mantissa.
    */
   while (isdigit(c)) {
      digits++;
      if (mantissa < BIG) {
         mantissa = mantissa * 10 + (c - '0');
         if (mantissa > 0.0)
            sdigits++;
         }
      else
         scale++;
      c = *s++;
      }

   /*
    * Check for based integer.
    */
   if (c == 'r' || c == 'R')
      return radix(msign, (int)mantissa, s, result);

   /*
    * Get fractional part of mantissa.
    */
   if (c == '.') {
      realflag++;
      c = *s++;
      while (isdigit(c)) {
         digits++;
         if (mantissa < BIG) {
            mantissa = mantissa * 10 + (c - '0');
            scale--;
            if (mantissa > 0.0)
               sdigits++;
            }
         c = *s++;
         }
      }

   /*
    * Check that at least one digit has been seen so far.
    */
   if (digits == 0)
      return NULL;

   /*
    * Get exponent part.
    */
   if (c == 'e' || c == 'E') {
      realflag++;
      c = *s++;
      if (c == '+' || c == '-') {
         esign = c;
         c = *s++;
         }
      if (!isdigit(c))
         return NULL;
      while (isdigit(c)) {
         exponent = exponent * 10 + (c - '0');
         c = *s++;
         }
      scale += (esign == '+')? exponent : -exponent;
      }

   /*
    * Skip trailing white space.
    */
   while (isspace(c))
      c = *s++;

   /*
    * Check that entire string has been consumed.
    */
   if (c != '\0')
      return NULL;

   /*
    * Test for integer.
    */
   if (!realflag && mantissa >= MinLong && mantissa <= MaxLong) {
      result->integer = (msign == '+')? mantissa : -mantissa;
#if IntSize == 16
      if (result->integer < (long)minsh || result->integer > (long)maxsh)
         return T_Longint;
#endif IntSize == 16
         return T_Integer;
      }

   /*
    * Rough tests for overflow and underflow.
    */
   if (sdigits + scale > LogHuge)
      return NULL;

   if (sdigits + scale < -LogHuge) {
      result->real = 0.0;
      return T_Real;
      }

   /*
    * Put the number together by multiplying the mantissa by 5^scale and
    *  then using ldexp() to multiply by 2^scale.
    */

#ifdef PDP11
   /*
    * Load floating point status register on PDP-11.
    */
   ldfps(0200);
#endif PDP11
   exponent = (scale > 0)? scale : -scale;
   fiveto = 1.0;
   power = 5.0;
   for (;;) {
      if (exponent & 01)
         fiveto *= power;
      exponent >>= 1;
      if (exponent == 0)
         break;
      power *= power;
      }
   if (scale > 0)
      mantissa *= fiveto;
   else
      mantissa /= fiveto;

   errno = 0;
   mantissa = ldexp(mantissa, scale);
#ifdef PDP11
   /*
    * Load floating point status register on PDP-11
    */
   ldfps(03200);
#endif PDP11
   if (errno > 0 && mantissa > 0)
      /*
       * ldexp caused overflow.
       */
      return NULL;

   result->real = (msign == '+')? mantissa : -mantissa;
   return T_Real;
   }

/*
 * radix - convert string s in radix r into an integer in *result.  sign
 *  will be either '+' or '-'.
 */
static radix(sign, r, s, result)
char sign;
register int r;
register char *s;
union numeric *result;
   {
   register int c;
   long num;
   int minsh, maxsh;

   minsh = MinShort;
   maxsh = MaxShort;

   if (r < 2 || r > 36)
      return NULL;

   c = *s++;
   num = 0L;
   while (isalnum(c)) {
      c = tonum(c);
      if (c >= r)
         return NULL;
      num = num * r + c;
      c = *s++;
      }

   while (isspace(c))
      c = *s++;

   if (c != '\0')
      return NULL;

   result->integer = (sign == '+')? num : -num;
   if (result->integer < (long)minsh || result->integer > (long)maxsh)
      return T_Longint;
   else
      return T_Integer;
   }


/*
 * cvpos - convert position to strictly positive position
 *  given length.
 */

word cvpos(pos, len)
long pos;
register word len;
   {
   register word p;

   /*
    * Return 0 if the position isn't in the range of an int. (?)
    */
   if ((long)(p = pos) != pos)
      return(0);
   /*
    * Return 0 if the position is off either end.
    */
   if (p < -len || p > len + 1)
      return(0);
   /*
    * If the position is greater than zero, just return it.  Otherwise,
    *  convert the zero/negative position.
    */
   if (pos > 0)
      return p;
   return (len + p + 1);
   }


/*
 * cvreal - convert to real and put the result into *r.
 */

cvreal(d, r)
register struct descrip *d;
double *r;
   {
   union numeric result;

#ifdef RunStats
   Inc(cv_n_real);
   if (!Qual(*d) && (d)->dword == D_Real)
      cv_n_rreal++;
#endif RunStats
   /*
    * Use cvnum to classify the value.	Cast integers into reals and
    *  fail if the value is non-numeric.
    */
   switch (cvnum(d, &result)) {

      case T_Integer:
      case T_Longint:
         *r = result.integer;
         return T_Real;

      case T_Real:
         *r = result.real;
         return T_Real;

      default:
         return NULL;
      }
   }


/*
 * cvstr(d,s) - convert d (in place) into a string, using s as buffer
 *  if necessary.  cvstr returns 0 if the conversion fails, 1 if d
 *  wasn't a string but was converted into one, and 2 if d was already
 *  a string.  When a string conversion takes place, sbuf gets the
 *  resulting string.
 */

/* >cvstr */
cvstr(dp, sbuf)
register struct descrip *dp;
char *sbuf;
   {
   double rres;

   Inc(cv_n_str);
   if (Qual(*dp)) {
      Inc(cv_n_rstr);
      return NoCvt;			/* It is already a string */
      }

   switch (Type(*dp)) {
      /*
       * For types that can be converted into strings, call the
       *  appropriate conversion routine and return its result.
       *  Note that the conversion routines change the descriptor
       *  pointed to by dp.
       */
      case T_Integer:
         return itos((long)IntVal(*dp), dp, sbuf);

      case T_Longint:
         return itos(BlkLoc(*dp)->longint.intval, dp, sbuf);

      case T_Real:
         GetReal(dp,rres);
         return rtos(rres, dp, sbuf);

      case T_Cset:
         return cstos(BlkLoc(*dp)->cset.bits, dp, sbuf);

      default:
         /*
          * The value cannot be converted to a string.
          */
         return NULL;
      }
   }
/* <cvstr */

/*
 * itos - convert the integer num into a string using s as a buffer and
 *  making q a descriptor for the resulting string.
 */
static itos(num, q, s)
long num;
struct descrip *q;
char *s;
   {
   register char *p;
   long ival;

   p = s + MaxCvtLen - 1;
   ival = num;

   *p = '\0';
   if (num >= 0L)
      do {
         *--p = ival % 10L + '0';
         ival /= 10L;
         } while (ival != 0L);
   else {
      do {
         *--p = '0' - (ival % 10L);
         ival /= 10L;
         } while (ival != 0L);
      *--p = '-';
      }

   StrLen(*q) = s + MaxCvtLen - 1 - p;
   StrLoc(*q) = p;
   return Cvt;
   }

/*
 * rtos - convert the real number n into a string using s as a buffer and
 *  making q a descriptor for the resulting string.
 */
rtos(n, q, s)
double n;
struct descrip *q;
char *s;
   {
   char *gcvt();
   /*
    * gcvt does all the work.
    */
   gcvt(n, 8, s);
   StrLen(*q) = strlen(s);
   StrLoc(*q) = s;
   return Cvt;
   }

/*
 * cstos - convert the cset bit array pointed at by cs into a string using
 *  s as a buffer and making q a descriptor for the resulting string.
 */

/* >cstos */
static cstos(cs, q, s)
int *cs;
struct descrip *q;
char *s;
   {
   register char *p;
   register int i;

   p = s;
   for (i = 0; i < CsetSize * CIntSize; i++) {
      if (Testb(i, cs))
         *p++ = (char)i;
      }
   *p = '\0';

   StrLen(*q) = p - s;
   StrLoc(*q) = s;
   return Cvt;
   }
/* <cstos */


/*
 * gcvt - Convert number to a string in buf.  If possible, ndigit significant
 *  digits are produced, otherwise a form with an exponent is used.
 */
char   *ecvt();
char *
gcvt(number, ndigit, buf)
double number;
char *buf;
   {
   int sign, decpt;
   register char *p1, *p2;
   register i;


   p1 = ecvt(number, ndigit, &decpt, &sign);
   p2 = buf;
   if (sign)
      *p2++ = '-';
   for (i=ndigit-1; i>0 && p1[i]=='0'; i--)
      ndigit--;
   if (decpt >= 0 && decpt-ndigit > 4
      || decpt < 0 && decpt < -3) { /* use E-style */
         decpt--;
         *p2++ = *p1++;
         *p2++ = '.';
         for (i=1; i<ndigit; i++)
            *p2++ = *p1++;
         *p2++ = 'e';
         if (decpt<0) {
            decpt = -decpt;
            *p2++ = '-';
            }
         if (decpt/10 > 0)
            *p2++ = decpt/10 + '0';
         *p2++ = decpt%10 + '0';
      } else {
         if (decpt<=0) {
         /* if (*p1!='0') */
         *p2++ = '0';
         *p2++ = '.';
         while (decpt<0) {
            decpt++;
            *p2++ = '0';
            }
         }
         for (i=1; i<=ndigit; i++) {
            *p2++ = *p1++;
            if (i==decpt)
               *p2++ = '.';
            }
      if (ndigit<decpt) {
         while (ndigit++<decpt)
            *p2++ = '0';
         *p2++ = '.';
         }
   }
   if (p2[-1]=='.')
      *p2++ = '0';
   *p2 = '\0';
   return(buf);
   }


/*
 * mkint - make an integer descriptor for l in *d.  A long integer is used
 *  if the value is too large for a regular integer.
 */

mkint(l, d)
long l;
register struct descrip *d;
   {
   extern struct b_int *alclint();
   int minsh, maxsh;

   minsh = MinShort;
   maxsh = MaxShort;

   if (l < (long)minsh || l > (long)maxsh) {
      blkreq(sizeof(struct b_int));
      d->dword = D_Longint;
      BlkLoc(*d) = (union block *)alclint(l);
      }
   else {
      d->dword = D_Integer;
      IntVal(*d) = (word)l;
      }
   }


/*
 * mkreal(r, d) - make a real number descriptor and associated block
 *  for r and place it in *d.
 */

mkreal(r, d)
double r;
register struct descrip *d;
   {
   extern struct b_real *alcreal();

   blkreq((uword)sizeof(struct b_real));
   d->dword = D_Real;
   BlkLoc(*d) = (union block *) alcreal(r);
   }


/*
 * mksubs - form a substring.  var is a descriptor for the string from
 *  which the substring is to be formed.  var may be a variable.  val
 *  is a dereferenced version of val.  The descriptor for the resulting
 *  substring is placed in *result.  The substring starts at position
 *  i and extends for j characters.
 */

mksubs(var, val, i, j, result)
register struct descrip *var, *val, *result;
word i, j;
   {
   extern struct b_tvsubs *alcsubs();

   if (Qual(*var) || !Var(*var)) {
      /*
       * var isn't a variable, just form a descriptor that points into
       *  the string named by val.
       */
      StrLen(*result) = j;
      StrLoc(*result) = StrLoc(*val) + i - 1;
      return;
      }

   if ((var)->dword == D_Tvsubs) {
      /*
       * If var is a substring trapped variable,
       *  adjust the position and make var the substrung string.
       */
         i += BlkLoc(*var)->tvsubs.sspos - 1;
         var = &BlkLoc(*var)->tvsubs.ssvar;
         }

   /*
    * Make a substring trapped variable by passing the buck to alcsubs.
    */
   result->dword = D_Tvsubs;
   BlkLoc(*result) = (union block *) alcsubs(j, i, var);
   return;
   }


/*
 * strprc - Convert the qualified string named by *d into a procedure
 *  descriptor if possible.  n is the number of arguments that the desired
 *  procedure has.  n is only used when the name of the procedure is
 *  non-alphabetic (hence, an operator).
 *  A return value of 1 indicates successful conversion.
 *  0 indicates that the string could not be converted.
 */
strprc(d,n)
struct descrip *d;
word n;
   {
      extern struct descrip *gnames, *globals, *eglobals;
      struct descrip *np, *gp;
      struct pstrnm *p;
      char *s;
      word ns, l;

      /*
       * Look in global name list first.
       */
      np = gnames; gp = globals;
      while (gp < eglobals) {
         if (!lexcmp(np++,d))
            if (BlkLoc(*gp)->proc.title == T_Proc) {
               StrLen(*d) = D_Proc; /* really type field */
               BlkLoc(*d) = BlkLoc(*gp);
               return 1;
               }
         gp++;
         }
      /*
       * The name is not a global, see if it is a builtin or an operator.
       */
      s = StrLoc(*d);
      l = StrLen(*d);
      for (p = pntab; p->pstrep; p++)
         /*
          * Compare the desired name with each standard procedure/operator
          *  name.
          */
         if (!slcmp(s,l,p->pstrep)) {
            if (isalpha(*s)) {
               /*
		* The names are the same and s starts with an alphabetic,
		*  so it's the one being looked for; return it.
		*/
               StrLen(*d) = D_Proc;
               BlkLoc(*d) = (union block *) p->pblock;
               return 1;
               }
            if ((ns = p->pblock->nstatic) < 0)
               ns = -ns;
            else
               ns = p->pblock->nparam;
            if (n == ns) {
               StrLen(*d) = D_Proc; /* really type field */
               BlkLoc(*d) = (union block *) p->pblock;
               return 1;
               }
            }
      return 0;
   }

/*
 * slcmp - lexically compare l1 bytes of s1 with null-terminated s2.
 */

slcmp(s1, l1, s2)
word l1;
char *s1,*s2;
   {
   register word minlen;
   word l2;

   l2 = strlen(s2);

   minlen = (l1 <= l2) ? l1 : l2;

   while (minlen--)
      if (*s1++ != *s2++)
         return (int)((*--s1 & 0377) - (*--s2 & 0377));

   return (int)(l1 - l2);
   }
