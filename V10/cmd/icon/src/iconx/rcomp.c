/*
 * File: rcomp.c
 *  Contents: anycmp, equiv, lexcmp, numcmp
 */

#include "../h/rt.h"

/*
 * anycmp - compare any two objects.  The result of the comparison is
 *  an integer such that:
 *    d1 = d2 -> 0
 *    d1 > d2 -> >0  (1 if same type)
 *    d1 < d2 -> <0  (-1 if same type)
 */

anycmp(d1,d2)
struct descrip *d1, *d2;
   {
   register int o1, o2;
   register long lresult;
   double rres1, rres2, rresult;

   /*
    * Get a collating number for d1 and d2.
    */
   o1 = order(d1);
   o2 = order(d2);

   /*
    * If d1 and d2 aren't of the same type, return the difference of
    *  their collating numbers.
    */
   if (o1 != o2)
      return (o1 - o2);

   if (o1 == D_Null)
      /*
       * o1 0, (D_Null), return 0 because all null values are the same.
       */
      return 0;
   if (o1 == 3)
      /*
       * d1 and d2 are strings, use lexcmp to compare them.
       */
      return lexcmp(d1,d2);

   switch (Type(*d1)) {
      /*
       * For numbers, return -1, 0, 1, depending on whether d1 <, =, > d2.
       */
      case T_Integer:
	 lresult = IntVal(*d1) - IntVal(*d2);
	 if (lresult == 0)
	    return 0;
	 return ((lresult > 0) ? 1 : -1);

      case T_Longint:
	 lresult = BlkLoc(*d1)->longint.intval - BlkLoc(*d2)->longint.intval;
	 if (lresult == 0)
	    return 0;
	 return ((lresult > 0) ? 1 : -1);

      case T_Real:
         GetReal(d1,rres1);
         GetReal(d2,rres2);
         rresult = rres1 - rres2;
	 if (rresult == 0)
	    return 0;
	 return ((rresult > 0) ? 1 : -1);

      case T_Cset:
      case T_File:
      case T_Proc:
      case T_List:
      case T_Table:
      case T_Set:
      case T_Record:
      case T_Coexpr:
	 /*
	  * Csets, files, procedures, lists, tables, records, co-expressions
	  *  and sets have no specified collating sequence so any two of
	  *  the same type are considered to be equal.
	  */
	 return 0;

      default:
	 syserr("anycmp: unknown datatype.");
      }
   }

/*
 * order(x) - return collating number for object x.
 */

order(d)
struct descrip *d;
   {
   if (Qual(*d))
      return 3; 	     /* string */
   switch (Type(*d)) {
      case T_Null:
	 return 0;
      case T_Integer:
      case T_Longint:
	 return 1;
      case T_Real:
	 return 2;
      case T_Cset:
	 return 4;
      case T_Coexpr:
	 return 5;
      case T_File:
	 return 6;
      case T_Proc:
	 return 7;
      case T_List:
	 return 8;
      case T_Table:
	 return 9;
      case T_Set:
	 return 10;
      case T_Record:
	 return 11;
      default:
	 syserr("order: unknown datatype.");
      }
   }


/*
 * equiv - test equivalence of two objects.
 */

equiv(dp1, dp2)
struct descrip *dp1, *dp2;
   {
   register int result, i;
   register char *s1, *s2;
   double rres1, rres2;

   result = 0;

      /*
       * If the descriptors are identical, the objects are equivalent.
       */
   if (EqlDesc(*dp1,*dp2))
      result = 1;
   else if (Qual(*dp1) && Qual(*dp2)) {

      /*
       *  If both are strings of equal length, compare their characters.
       */

      if ((i = StrLen(*dp1)) == StrLen(*dp2)) {
	 s1 = StrLoc(*dp1);
	 s2 = StrLoc(*dp2);
	 result = 1;
	 while (i--)
	   if (*s1++ != *s2++) {
	      result = 0;
	      break;
	      }
	 }
      }
   else if (dp1->dword == dp2->dword)
      switch (Type(*dp1)) {
	 /*
	  * For integers and reals, just compare the values.
	  */
	 case T_Integer:
	    result = (IntVal(*dp1) == IntVal(*dp2));
	    break;

	 case T_Longint:
	    result =
	       (BlkLoc(*dp1)->longint.intval == BlkLoc(*dp2)->longint.intval);
	    break;

	 case T_Real:
            GetReal(dp1, rres1);
            GetReal(dp2, rres2);
            result = (rres1 == rres2);
	    break;

	 case T_Cset:
	    /*
	     * Compare the bit arrays of the csets.
	     */
	    result = 1;
	    for (i = 0; i < CsetSize; i++)
	       if (BlkLoc(*dp1)->cset.bits[i] != BlkLoc(*dp2)->cset.bits[i]) {
		  result = 0;
		  break;
		  }
	 }
   else
      /*
       * dp1 and dp2 are of different types, so they can't be
       *  equivalent.
       */
      result = 0;

   return result;
   }


/*
 * lexcmp - lexically compare two strings.
 */

lexcmp(d1, d2)
struct descrip *d1, *d2;
   {
   register char *s1, *s2;
   register int minlen;
   int l1, l2;

   /*
    * Get length and starting address of both strings.
    */
   l1 = StrLen(*d1);
   s1 = StrLoc(*d1);
   l2 = StrLen(*d2);
   s2 = StrLoc(*d2);

   /*
    * Set minlen to length of the shorter string.
    */
   minlen = (l1 <= l2) ? l1 : l2;

   /*
    * Compare as many bytes as are in the smaller string.  If an
    *  inequality is found, return the difference of the differing
    *  bytes.
    */
   while (minlen--)
      if (*s1++ != *s2++)
	 return (int)((*--s1 & 0377) - (*--s2 & 0377));

   /*
    * The strings compared equal for the length of the shorter.  Return
    *  the difference in their lengths.  (Thus, the strings must be of
    *  the same length to be equal.)
    */
   return (l1 - l2);
   }


/*
 * numcmp - compare two numbers.  Returns -1, 0, 1 for dp1 <, =, > dp2.
 *  dp3 is made into a descriptor for the return value.
 */

numcmp(dp1, dp2, dp3)
struct descrip *dp1, *dp2, *dp3;
   {
   register int result;
   union numeric n1, n2;
   int t1, t2;
   /*
    * Be sure that both dp1 and dp2 are numeric.
    */

   if ((t1 = cvnum(dp1, &n1)) == NULL)
      runerr(102, dp1);
   if ((t2 = cvnum(dp2, &n2)) == NULL)
      runerr(102, dp2);

   if (!(t1 == T_Real || t2 == T_Real)) {
   /*
    *  dp1 and dp2 are both integers, compare them and
    *  create an integer descriptor in dp3
    */

	  result = 0;
	  if (n1.integer < n2.integer) result = -1;
	  else if (n1.integer != n2.integer) result = 1;
      Mkint(n2.integer, dp3);
      }
   else {

   /*
    *  Either dp1 or dp2 is real. Convert the other to a real,
    *  compare them and create a real descriptor in dp3.
    */

      if (!(t1 == T_Real))
	 n1.real = n1.integer;
      if (!(t2 == T_Real))
	 n2.real = n2.integer;
	  result = 0;
	  if (n1.real < n2.real) result = -1;
	  else if (n1.real != n2.real) result = 1;
      mkreal(n2.real, dp3);
      }

   return result;	      /* return result in r0 */
   }

