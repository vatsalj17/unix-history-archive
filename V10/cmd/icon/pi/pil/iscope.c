/*
 *	ISCOPE
 *
 *	Inspect Icon internals.
 *
 *	Ralph E. Griswold
 *
 *	Last modified  6/8/86
 *
 */

#include "../h/rt.h"

/*
 * Vword(x) - return second word of descriptor as integer
 */

FncDcl(Vword,1)
   {
   Arg0.dword = D_Integer;
   Arg0.vword.integr = Arg1.vword.integr;
   Return;
   }


/*
 * Dword(x) - return first word of descriptor as integer.
 */

FncDcl(Dword,1)
   {
   Arg0.dword = D_Integer;
   Arg0.vword.integr = Arg1.dword;
   Return;
   }

/*
 * Descr(x,y) - construct descriptor from integer values of x and y
 */

FncDcl(Descr,2)
   {
   defshort(&Arg1, 0);
   defshort(&Arg2, 0);
   Arg0.dword = Arg1.vword.integr;
   Arg0.vword.integr = Arg2.vword.integr;
   Return;
   }

/*
 * Indir(x) - return integer to where x points.
 */

FncDcl(Indir,1)
   {
   Arg0.dword = D_Integer;
   Arg0.vword.integr = *((int *) Arg1.vword.integr);
   Return;
   }

/*
 * Symbol(x) - get address of Icon symbol.
 */

FncDcl(Symbol,1)
   {
   extern globals, eglobals, gnames;
   char sbuf[MaxCvtLen];
   if (cvstr(&Arg1, sbuf) == NULL)
      runerr(103, &Arg1);
   qtos(&Arg1, sbuf);
   ((Arg0).dword) = D_Integer;
   if (strcmp(sbuf, "globals") == 0)
      IntVal(Arg0) = (int) &globals;
   else if (strcmp(sbuf, "eglobals") == 0)
      IntVal(Arg0) = (int) &eglobals;
   else if (strcmp(sbuf, "gnames") == 0)
      IntVal(Arg0) = (int) &gnames;
   else if (strcmp(sbuf, "strbase") == 0)
      IntVal(Arg0) = (int) strbase;
   else if (strcmp(sbuf, "strfree") == 0)
      IntVal(Arg0) = (int) strfree;
   else if (strcmp(sbuf, "blkbase") == 0)
      IntVal(Arg0) = (int) blkbase;
   else if (strcmp(sbuf, "blkfree") == 0)
      IntVal(Arg0) = (int) blkfree;
   else runerr(205, &Arg1);
   Return;
   }
/*
 * Ivar(x) - get value of interpreter state variable
 */

FncDcl(Ivar,1)
   {
   char sbuf[MaxCvtLen];
   if (cvstr(&Arg1, sbuf) == NULL)
      runerr(103, &Arg1);
   qtos(&Arg1, sbuf);
   ((Arg0).dword) = D_Integer;
   if (strcmp(sbuf, "sp") == 0)
      IntVal(Arg0) = (int)sp;
   else if (strcmp(sbuf, "efp") == 0)
      IntVal(Arg0) = (int)efp;
   else if (strcmp(sbuf, "gfp") == 0)
      IntVal(Arg0) = (int)gfp;
   else if (strcmp(sbuf, "argp") == 0)
      IntVal(Arg0) = (int)argp;
   else if (strcmp(sbuf, "pfp") == 0)
      IntVal(Arg0) = (int)pfp;
   else if (strcmp(sbuf, "ilevel") == 0)
      IntVal(Arg0) = (int)ilevel;
   else runerr(205, &Arg1);
   Return;
   }
