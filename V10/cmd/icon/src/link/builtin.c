/*
 * Routines for determining if a name is a built-in function.
 */

#include "ilink.h"

/*
 * btable is an array of the names of the Icon builtin functions.
 */
char *btable[] = {
#define FncDef(p) "p",
#include "../h/fdef.h"
#undef FncDef
   };

#define NBUILTIN (sizeof(btable)/sizeof(char *)) 

/*
 * blocate - binary search for a builtin function.
 * If found, returns pointer to entry.
 */

blocate(s)
register char *s;
   {
   register int test, cmp;
   int low, high;

   low = 0;
   high = NBUILTIN;
   do {
      test = (low + high) / 2;
      if ((cmp = strcmp(btable[test], s)) < 0)
         low = test + 1;
      else if (cmp > 0)
         high = test;
      else
         return (test + 1);
      } while (low < high);
   return 0;
   }
