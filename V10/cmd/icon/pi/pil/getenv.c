/*
 *	GETENV
 *
 *	Get values of environment variables.
 *
 *	Stephen B. Wampler
 *
 *	Last modified 5/2/86 by Ralph E. Griswold
 *
 */

#include "../h/rt.h"

/*
 * getenv(s) - return contents of environment variable s
 */

FncDcl(getenv,1)
   {
   register char *p;
   register int len;
   char sbuf[256];
   extern char *getenv();
   extern char *alcstr();


   if (!Qual(Arg1))			/* check legality of argument */
      runerr(103, &Arg1);
   if (StrLen(Arg1) <= 0 || StrLen(Arg1) >= MaxCvtLen)
      runerr(401, &Arg1);
   qtos(&Arg1, sbuf);			/* convert argument to C-style string */

   if ((p = getenv(sbuf)) != NULL) {	/* get environment variable */
      len = strlen(p);
      strreq(len);
      StrLen(Arg0) = len;
      StrLoc(Arg0) = alcstr(p, len);
      Return;
      }
   else					/* fail if variable not in environment */
      Fail;
   }
