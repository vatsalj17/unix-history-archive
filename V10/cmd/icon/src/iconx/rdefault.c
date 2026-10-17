/*
 * File: rdefault.c
 *  Contents: defcset, deffile, defint, defshort, defstr
 */

#include "../h/rt.h"

/*
 * defcset(dp,cp,buf,def) - if dp is null, default to def;
 *  otherwise, convert to cset or die trying.
 */

defcset(dp, cp, buf, def)
struct descrip *dp;
int **cp;
int *buf, *def;
   {
   if (ChkNull(*dp)) {
      *cp = def;
      return 1;
      }
   if (cvcset(dp, cp, buf) == NULL)
      runerr(104, dp);
   return 0;
   }


/*
 * deffile - if dp is null, default to def; otherwise, make sure it's a file.
 */

deffile(dp, def)
struct descrip *dp, *def;
   {
   if (ChkNull(*dp)) {
      *dp = *def;
      return 1;
      }
   if (Qual(*dp) || (*dp).dword != D_File)
      runerr(105, dp);
   return 0;
   }


/*
 * defint - if dp is null, default to def; otherwise, convert to integer.
 *  Note that *lp gets the value.
 */

defint(dp, lp, def)
struct descrip *dp;
long *lp;
word def;
   {
   if (ChkNull(*dp)) {
      *lp = (long)def;
      return 1;
      }
   if (cvint(dp, lp) == NULL)
      runerr(101, dp);
   return 0;
   }


/*
 * defshort - if dp is null, default to def; otherwise, convert to short
 *  integer.  The result is an integer value in *dp.
 */

defshort(dp, def)
struct descrip *dp;
int def;
   {
   long l;

   if (ChkNull(*dp)) {
      dp->dword = D_Integer;
      IntVal(*dp) = (int)def;
      return 1;
      }
   switch (cvint(dp, &l)) {

      case T_Integer:
         Mkint(l, dp);
	 break;

      case T_Longint:
         runerr(205, dp);

      default:
	 runerr(101, dp);
      }
   return 0;
   }


/*
 * defstr - if dp is null, default to def; otherwise, convert to string.
 *  *dp gets a descriptor for the resulting string.  buf is used as
 *  a scratch buffer for the conversion (if necessary).
 */

defstr(dp, buf, def)
struct descrip *dp;
char *buf;
struct descrip *def;
   {
   if (ChkNull(*dp)) {
      *dp = *def;
      return 1;
      }
   if (cvstr(dp, buf) == NULL)
      runerr(103, dp);
   return 0;
   }
