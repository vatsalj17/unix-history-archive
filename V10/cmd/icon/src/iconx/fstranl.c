/*
 * File: fstranl.c
 *  Contents: any, bal, find, many, match, move, pos, tab, upto
 */

#include "../h/rt.h"

/*
 * any(c,s,i,j) - test if first character of s[i:j] is in c.
 */

FncDcl(any,4)
   {
   register word i, j;
   long l1, l2;
   int *cs, csbuf[CsetSize];
   char sbuf[MaxCvtLen];

   /*
    * c must be a cset.  s defaults to &subject; i defaults to &pos if s
    *  defaulted, 1 otherwise.  j defaults to 0.
    */
   if (cvcset(&Arg1, &cs, csbuf) == NULL)
      runerr(104, &Arg1);
   if (defstr(&Arg2, sbuf, &k_subject))
      defint(&Arg3, &l1, k_pos);
   else
      defint(&Arg3, &l1, (word)1);
   defint(&Arg4, &l2, (word)0);

   /*
    * Convert i and j to positions in s. If i == j then the specified
    *  substring of s is empty and any fails. Otherwise make i the smaller of
    *  the two.  (j is of no further use.)
    */
   i = cvpos(l1, StrLen(Arg2));
   if (i == 0)
      Fail;
   j = cvpos(l2, StrLen(Arg2));
   if (j == 0)
      Fail;
   if (i == j)
      Fail;
   if (i > j)
      i = j;

   /*
    * If s[i] is not in the cset c, fail.
    */
   if (!Testb(StrLoc(Arg2)[i-1], cs))
      Fail;

   /*
    * Return pos(s[i+1]).
    */
   Arg0.dword = D_Integer;
   IntVal(Arg0) = i + 1;
   Return;
   }


/*
 * bal(c1,c2,c3,s,i,j) - find end of a balanced substring of s[i:j].
 *  Generates successive positions.
 */

FncDcl(bal,6)
   {
   register word i, j;
   register cnt, c;
   word t;
   long l1, l2;
   int *cs1, *cs2, *cs3;
   int csbuf1[CsetSize], csbuf2[CsetSize], csbuf3[CsetSize];
   char sbuf[MaxCvtLen];
   static int lpar[CsetSize] =	/* '(' */
      cset_display(0, 0, 0400, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
   static int rpar[CsetSize] =	/* ')' */
      cset_display(0, 0, 01000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

   /*
    *  c1 defaults to &cset; c2 defaults to '(' (lpar); c3 defaults to
    *   ')' (rpar); s to &subject; i to &pos if s defaulted, 1 otherwise;
    *   j defaults to 0.
    */
   defcset(&Arg1, &cs1, csbuf1, k_cset.bits);
   defcset(&Arg2, &cs2, csbuf2, lpar);
   defcset(&Arg3, &cs3, csbuf3, rpar);
   if (defstr(&Arg4, sbuf, &k_subject))
      defint(&Arg5, &l1, k_pos);
   else
      defint(&Arg5, &l1, (word)1);
   defint(&Arg6, &l2, (word)0);

   /*
    * Convert i and j to positions in s and order them.
    */
   i = cvpos(l1, StrLen(Arg4));
   if (i == 0)
      Fail;
   j = cvpos(l2, StrLen(Arg4));
   if (j == 0)
      Fail;
   if (i > j) {
      t = i;
      i = j;
      j = t;
      }

   /*
    * Loop through characters in s[i:j].  When a character in cs2 is
    *  found, increment cnt; when a chracter in cs3 is found, decrement
    *  cnt.  When cnt is 0 there have been an equal number of occurrences
    *  of characters in cs2 and cs3, i.e., the string to the left of
    *  i is balanced.  If the string is balanced and the current character
    *  (s[i]) is in c1, suspend i.  Note that if cnt drops below zero,
    *  bal fails.
    */
   cnt = 0;
   Arg0.dword = D_Integer;
   while (i < j) {
      c = StrLoc(Arg4)[i-1];
      if (cnt == 0 && Testb(c, cs1)) {
         IntVal(Arg0) = i;
         Suspend;
         }
      if (Testb(c, cs2))
         cnt++;
      else if (Testb(c, cs3))
         cnt--;
      if (cnt < 0)
         Fail;
      i++;
      }
   /*
    * Eventually fail.
    */
   Fail;
   }


/*
 * find(s1,s2,i,j) - find string s1 in s2[i:j] and return position in
 *  s2 of beginning of s1.
 * Generates successive positions.
 */

/* >find */
FncDcl(find,4)
   {
   register word l;
   register char *s1, *s2;
   word i, j, t;
   long l1, l2;
   char sbuf1[MaxCvtLen], sbuf2[MaxCvtLen];

   /*
    * Arg1 must be a string.  Arg2 defaults to &subject; Arg3 defaults
    *  to &pos if Arg2 is defaulted, or to 1 otherwise; Arg4 defaults
    *  to 0.

    */
   if (cvstr(&Arg1, sbuf1) == NULL)
      runerr(103, &Arg1);
   if (defstr(&Arg2, sbuf2, &k_subject))
      defint(&Arg3, &l1, k_pos);
   else
      defint(&Arg3, &l1, (word)1);
   defint(&Arg4, &l2, (word)0);

   /*
    * Convert i and j to absolute positions in s2 and order them
    * so that i <= j.
    */
   i = cvpos(l1, StrLen(Arg2));
   if (i == 0)
      Fail;
   j = cvpos(l2, StrLen(Arg2));
   if (j == 0)
      Fail;
   if (i > j) {
      t = i;
      i = j;
      j = t;
      }

   /*
    * Loop through s2[i:j] trying to find s1 at each point, stopping
    *  when the remaining portion s2[i:j] is too short to contain s1.
    */
   Arg0.dword = D_Integer;
   while (i <= j - StrLen(Arg1)) {
      s1 = StrLoc(Arg1);
      s2 = StrLoc(Arg2) + i - 1;
      l = StrLen(Arg1);

      /*
       * Compare strings on a byte-wise basis; if end is reached
       *  before inequality is found, suspend the position of the string.
       */
      do {
         if (l-- <= 0) {
            IntVal(Arg0) = i;
            Suspend;
            break;
            }
         } while (*s1++ == *s2++);
      i++;
      }

   Fail;
   }
/* <find */

/*
 * many(c,s,i,j) - find longest prefix of s[i:j] of characters in c.
 */

FncDcl(many,4)
   {
   register word i, j, t;
   int *cs, csbuf[CsetSize];
   long l1, l2;
   char sbuf[MaxCvtLen];

   /*
    * c must be a cset.  s defaults to &subject;  i defaults to &pos if s
    *  defaulted, 1 otherwise;  j defaults to 0.
    */
   if (cvcset(&Arg1, &cs, csbuf) == NULL)
      runerr(104, &Arg1);
   if (defstr(&Arg2, sbuf, &k_subject))
      defint(&Arg3, &l1, k_pos);
   else
      defint(&Arg3, &l1, (word)1);
   defint(&Arg4, &l2, (word)0);

   /*
    * Convert i and j to absolute positions and order them.  If i == j,
    *  then the specified substring of s is the empty string and many
    *  fails.
    */
   i = cvpos(l1, StrLen(Arg2));
   if (i == 0)
      Fail;
   j = cvpos(l2, StrLen(Arg2));
   if (j == 0)
      Fail;
   if (i == j)
      Fail;
   if (i > j) {
      t = i;
      i = j;
      j = t;
      }

   /*
    * Fail if first character of s[i:j] isn't in c.
    */
   if (!Testb(StrLoc(Arg2)[i-1], cs))
      Fail;

   /*
    * Move i along s[i:j] until a character that is not in c is found or
    *  the end of the string is reached.
    */
   i++;
   while (i < j && Testb(StrLoc(Arg2)[i-1], cs))
      i++;

   /*
    * Return the position of the first character not in c.
    */
   Arg0.dword = D_Integer;
   IntVal(Arg0) = i;
   Return;
   }


/*
 * match(s1,s2,i,j) - test if s1 is prefix of s2[i:j].
 */
FncDcl(match,4)
   {
   register word i;
   register char *s1, *s2;
   word j, t;
   long l1, l2;
   char sbuf1[MaxCvtLen], sbuf2[MaxCvtLen];

   /*
    * s1 must be a string.  s2 defaults to &subject;  i defaults to &pos
    *  if s defaulted, 1 otherwise; j defaults to 0.
    */
   if (cvstr(&Arg1, sbuf1) == NULL)
      runerr(103, &Arg1);
   if (defstr(&Arg2, sbuf2, &k_subject))
      defint(&Arg3, &l1, k_pos);
   else
      defint(&Arg3, &l1, (word)1);
   defint(&Arg4, &l2, (word)0);

   /*
    * Convert i and j to absolute positions and then make i the smaller
    *  of the two positions and make j the length of the substring.
    */
   i = cvpos(l1, StrLen(Arg2));
   if (i == 0)
      Fail;
   j = cvpos(l2, StrLen(Arg2));
   if (j == 0)
      Fail;
   if (i > j) {
      t = i;
      i = j;
      j = t - j;
      }
   else
      j = j - i;

   /*
    * Can't match unless s1 is as long as s2[i:j].
    */
   if (j < StrLen(Arg1))
      Fail;

   /*
    * Compare s1 with s2[i:j] for *s1 characters; fail if an inequality
    *  if found.
    */
   s1 = StrLoc(Arg1);
   s2 = StrLoc(Arg2) + i - 1;
   for (j = StrLen(Arg1); j > 0; j--)
      if (*s1++ != *s2++)
         Fail;

   /*
    * Return position of end of matched string in s2.
    */
   Arg0.dword = D_Integer;
   IntVal(Arg0) = i + StrLen(Arg1);
   Return;
   }


/*
 * move(i) - move &pos by i, return substring of &subject spanned.
 *  Reverses effects if resumed.
 */
FncDcl(move,1)
   {
   register word i, j;
   long l;
   word oldpos;

   /*
    * i must be a (non-long) integer.
    */
   switch (cvint(&Arg1, &l)) {

      case T_Integer:
         j = (word)l;
         break;

      case T_Longint:
         Fail;

      default:
         runerr(101, &Arg1);
      }

   /*
    * Save old &pos.  Local variable i holds &pos before the move.
    */
   oldpos = i = k_pos;

   /*
    * If attempted move is past either end of the string, fail.
    */
   if (i + j <= 0 || i + j > StrLen(k_subject) + 1)
      Fail;

   /*
    * Set new &pos.
    */
   k_pos += j;

   /*
    * Make sure j >= 0.
    */
   if (j < 0) {
      i += j;
      j = -j;
      }

   /*
    * Suspend substring of &subject that was moved over.
    */
   StrLen(Arg0) = j;
   StrLoc(Arg0) = StrLoc(k_subject) + i - 1;
   Suspend;

   /*
    * If move is resumed, restore the old position and fail.
    */
   k_pos = oldpos;
   if (k_pos > StrLen(k_subject) + 1)
      runerr(205, &tvky_pos.kyval);
   Fail;
   }


/*
 * pos(i) - test if &pos is at position i in &subject.
 */
FncDcl(pos,1)
   {
   register word i;
   long l;

   /*
    * i must be an integer.
    */
   if (cvint(&Arg1, &l) == NULL)
      runerr(101, &Arg1);

   /*
    * Fail if &pos isn't equivalent to i, return i otherwise.
    */
   if ((i = cvpos(l, StrLen(k_subject))) != k_pos)
      Fail;
   Arg0.dword = D_Integer;
   IntVal(Arg0) = i;
   Return;
   }


/*
 * tab(i) - set &pos to i, return substring of &subject spanned.
 *  Reverses effects if resumed..
 */

/* >tab */
FncDcl(tab,1)
   {
   register word i, j;
   word t, oldpos;
   long l1;

   /*
    * Arg1 must be an integer.
    */
   if (cvint(&Arg1, &l1) == NULL)
      runerr(101, &Arg1);
/* <tab */

   /*
    * Convert j to an absolute position.
    */
   j = cvpos(l1, StrLen(k_subject));
   if (j == 0)
      Fail;

   /*
    * Save old &pos.  Local variable i holds &pos before the tab.
    */
   oldpos = i = k_pos;

   /*
    * Set new &pos.
    */
   k_pos = j;

   /*
    *  Make j the length of the substring &subject[i:j]
    */
   if (i > j) {
      t = i;
      i = j;
      j = t - j;
      }
   else
      j = j - i;

   /*
    * Suspend the portion of &subject that was tabbed over.
    */
   StrLoc(Arg0) = StrLoc(k_subject) + i - 1;
   StrLen(Arg0) = j;
   Suspend;

   /*
    * If tab is resumed, restore the old position and fail.
    */
   k_pos = oldpos;
   if (k_pos > StrLen(k_subject) + 1)
      runerr(205, &tvky_pos.kyval);
   Fail;
   }


/*
 * upto(c,s,i,j) - find each occurrence in s[i:j] of a character in c.
 * Generates successive positions.
 */

FncDcl(upto,4)
   {
   register word i, j;
   word t;
   long l1, l2;
   int *cs, csbuf[CsetSize];
   char sbuf[MaxCvtLen];

   /*
    * c must be a cset.  s defaults to &subject; i defaults to &pos if
    *  s defaulted, 1 otherwise; j defaults to 0.
    */
   if (cvcset(&Arg1, &cs, csbuf) == NULL)
      runerr(104, &Arg1);
   if (defstr(&Arg2, sbuf, &k_subject))
      defint(&Arg3, &l1, k_pos);
   else
      defint(&Arg3, &l1, (word)1);
   defint(&Arg4, &l2, (word)0);

   /*
    * Convert i and j to positions in s and order them.
    */
   i = cvpos(l1, StrLen(Arg2));
   if (i == 0)
      Fail;
   j = cvpos(l2, StrLen(Arg2));
   if (j == 0)
      Fail;
   if (i > j) {
      t = i;
      i = j;
      j = t;
      }

   /*
    * Look through s[i:j] and suspend position of each occurrence of
    *  of a character in c.
    */
   while (i < j) {
      if (Testb(StrLoc(Arg2)[i-1], cs)) {
         Arg0.dword = D_Integer;
         IntVal(Arg0) = i;
         Suspend;
         }
      i++;
      }
   /*
    * Eventually fail.
    */
   Fail;
   }
