/*
 * File: fstr.c
 *  Contents: center, left, map, repl, reverse, right, trim
 */

#include "../h/rt.h"

/*
 * center(s1,n,s2) - pad s1 on left and right with s2 to length n.
 */

FncDcl(center,3)
   {
   register char *s, *st;
   word cnt, slen, hcnt;
   char *sbuf, *s3;
   char sbuf1[MaxCvtLen], sbuf2[MaxCvtLen];
   extern char *alcstr();

   /*
    * s1 must be a string.  n must be a non-negative integer and defaults
    *  to 1.  s2 must be a string and defaults to a blank.
    */
   if (cvstr(&Arg1, sbuf1) == NULL)
      runerr(103, &Arg1);
   defshort(&Arg2, 1);
   if ((cnt = IntVal(Arg2)) < 0)
      runerr(205, &Arg2);
   defstr(&Arg3, sbuf2, &blank);

   strreq(cnt);

   if (StrLen(Arg3) == 0) {
      /*
       * The padding string is null, make it a blank.
       */
      slen = 1;
      s3 = " ";
      }
   else {
      slen = StrLen(Arg3);
      s3 = StrLoc(Arg3);
      }

   /*
    * Get n bytes of string space for the new string.  Start at the right
    *  of the new string and copy s2 into it from right to left as
    *  many times as will fit in the right half of the new string.
    */
   sbuf = alcstr(NULL, cnt);
   hcnt = cnt / 2;
   s = sbuf + cnt;
   while (s > sbuf + hcnt) {
      st = s3 + slen;
      while (st > s3 && s > sbuf + hcnt)
         *--s = *--st;
      }

   /*
    * Start at the left end of the new string and copy s1 into it from
    *  left to right as many time as will fit in the left half of the
    *  new string.
    */
   s = sbuf;
   while (s < sbuf + hcnt) {
      st = s3;
      while (st < s3 + slen && s < sbuf + hcnt)
         *s++ = *st++;
      }

   slen = StrLen(Arg1);
   if (cnt < slen) {
      /*  
       * s1 is larger than the field to center it in.  The source for the
       *  copy starts at the appropriate point in s1 and the destination
       *  starts at the left end of of the new string.
       */
      s = sbuf;
      st = StrLoc(Arg1) + slen/2 - hcnt + (~cnt&slen&1);
      }
   else {
      /*
       * s1 is smaller than the field to center it in.  The source for the
       *  copy starts at the left end of s1 and the destination starts at
       *  the appropriate point in the new string.
       */
      s = sbuf + hcnt - slen/2 - (~cnt&slen&1);
      st = StrLoc(Arg1);
      }
   /*
    * Perform the copy, moving min(*s1,n) bytes from st to s.
    */
   if (slen > cnt)
      slen = cnt;
   while (slen-- > 0)
      *s++ = *st++;

   /*
    * Return the new string.
    */
   StrLen(Arg0) = cnt;
   StrLoc(Arg0) = sbuf;
   Return;
   }


/*
 * left(s1,n,s2) - pad s1 on right with s2 to length n.
 */

FncDcl(left,3)
   {
   register char *s, *st;
   word cnt, slen;
   char *sbuf, *s3, sbuf1[MaxCvtLen], sbuf2[MaxCvtLen];
   extern char *alcstr();

   /*
    * s1 must be a string.  n must be a non-negative integer and defaults
    *  to 1.  s2 must be a string and defaults to a blank.
    */
   if (cvstr(&Arg1, sbuf1) == NULL)
      runerr(103, &Arg1);
   defshort(&Arg2, 1);
   if ((cnt = IntVal(Arg2)) < 0)
      runerr(205, &Arg2);
   defstr(&Arg3, sbuf2, &blank);

   strreq(cnt);
   if (StrLen(Arg3) == 0) {
      /*
       * The padding string is null, make it a blank.
       */
      slen = 1;
      s3 = " ";
      }
   else {
      slen = StrLen(Arg3);
      s3 = StrLoc(Arg3);
      }

   /*
    * Get n bytes of string space.  Start at the right end of the new
    *  string and copy s2 into the new string as many times as it fits.
    *  Note that s2 is copied from right to left.
    */
   sbuf = alcstr(NULL, cnt);
   s = sbuf + cnt;
   while (s > sbuf) {
      st = s3 + slen;
      while (st > s3 && s > sbuf)
         *--s = *--st;
      }

   /*
    * Copy s1 into the new string, starting at the left end.  If *s1 > n,
    *  only copy n bytes.
    */
   s = sbuf;
   slen = StrLen(Arg1);
   st = StrLoc(Arg1);
   if (slen > cnt)
      slen = cnt;
   while (slen-- > 0)
      *s++ = *st++;

   /*
    * Return the new string.
    */
   StrLen(Arg0) = cnt;
   StrLoc(Arg0) = sbuf;
   Return;
   }

/*
 * map(s1,s2,s3) - map s1, using s2 and s3.
 */

/* >map */
FncDcl(map,3)
   {
   register int i;
   register word slen;
   register char *s1, *s2, *s3;
   char sbuf1[MaxCvtLen], sbuf2[MaxCvtLen], sbuf3[MaxCvtLen];
   static char maptab[256];
   extern char *alcstr();

   /*
    * s1 must be a string; s2 and s3 default to &ucase and &lcase,
    *  respectively.
    */
   if (cvstr(&Arg1, sbuf1) == NULL)
      runerr(103, &Arg1);
   if (ChkNull(Arg2))
      Arg2 = ucase;
   if (ChkNull(Arg3))
      Arg3 = lcase;

   /*
    * If s2 and s3 are the same as for the last call of map,
    *  the current values in maptab can be used. Otherwise, the
    *  mapping information must information must be recomputed.
    */
   if (!EqlDesc(maps2,Arg2) || !EqlDesc(maps3,Arg3)) {
      maps2 = Arg2;
      maps3 = Arg3;

      /*
       * Convert s2 and s3 to strings.  They must be of the
       *  same length.
       */
      if (cvstr(&Arg2, sbuf2) == NULL)
         runerr(103, &Arg2);
      if (cvstr(&Arg3, sbuf3) == NULL)
         runerr(103, &Arg3);
      if (StrLen(Arg2) != StrLen(Arg3))
         runerr(208, 0);

      /*
       * The array maptab is used to perform the mapping.  First,
       *  maptab[i] is initialized with i for i from 0 to 255.
       *  Then, for each character in s2, the position in maptab
       *  corresponding the value of the character is assigned
       *  the value of the character in s3 that is in the same 
       *  position as the character from s2.
       */
      s2 = StrLoc(Arg2);
      s3 = StrLoc(Arg3);
      for (i = 0; i <= 255; i++)
         maptab[i] = i;
      for (slen = 0; slen < StrLen(Arg2); slen++)
         maptab[s2[slen]&0377] = s3[slen];
      }

   if (StrLen(Arg1) == 0) {
      Arg0 = emptystr;
      Return;
      }

   /*
    * The result is a string the size of s1; ensure that much space.
    */
   slen = StrLen(Arg1);
   strreq(slen);
   s1 = StrLoc(Arg1);

   /*
    * Create the result string, but specify no value for it.
    */
   StrLen(Arg0) = slen;
   StrLoc(Arg0) = alcstr(NULL, slen);
   s2 = StrLoc(Arg0);
   /*
    * Run through the string using values in maptab to do the
    *  mapping.
    */
   while (slen-- > 0)
      *s2++ = maptab[(*s1++)&0377];
   Return;
   }
/* <map */

/*
 * repl(s,n) - concatenate n copies of string s.
 */

/* >repl */
FncDcl(repl,2)
   {
   register char *sloc;
   register int cnt;
   long len;
   char sbuf[MaxCvtLen];
   extern char *alcstr();

   /*
    * Make sure that Arg1 is a string.
    */
   if (cvstr(&Arg1, sbuf) == NULL)
      runerr(103, &Arg1);

   /*
    * Make sure that Arg2 is an integer.
    */
   switch (cvint(&Arg2, &len)) {

      case T_Integer:
         if ((cnt = (int)len) >= 0)                     /* make sure count is not negative */
            break;
         runerr(205, &Arg2);

      case T_Longint:
         runerr(205, &Arg2);

      default:
         runerr(101, &Arg2);
      }

   /*
    * Make sure the resulting string will not be too long.
    */
   if ((len * StrLen(Arg1)) > MaxStrLen)
      runerr(205, 0);

   /*
    * Return an empty string if Arg2 is 0.
    */
   if (cnt == 0)
      Arg0 = emptystr;

   else {
      /*
       * Ensure enough space for the replicated string and allocate
       *  a copy of s.  Then allocate and copy s n - 1 times.
       */
      strreq(cnt * StrLen(Arg1));
      sloc = alcstr(StrLoc(Arg1), StrLen(Arg1));
      cnt--;
      while (cnt--)
         alcstr(StrLoc(Arg1), StrLen(Arg1));

      /*
       * Make Arg0 a descriptor for the replicated string.
       */
      StrLen(Arg0) = (int)len * StrLen(Arg1);
      StrLoc(Arg0) = sloc;
      }
   Return;
   }
/* <repl */


/*
 * reverse(s) - reverse string s.
 */

/* >reverse */
FncDcl(reverse,1)
   {
   register char c, *floc, *lloc;
   register word slen;
   char sbuf[MaxCvtLen];
   extern char *alcstr();

   /*
    * Make sure that Arg1 is a string.
    */
   if (cvstr(&Arg1, sbuf) == NULL)
      runerr(103, &Arg1);
/* <reverse */

   /*
    * Ensure that there is enough room and allocate a copy of s.
    */
   slen = StrLen(Arg1);
   strreq(slen);
   StrLen(Arg0) = slen;
   StrLoc(Arg0) = alcstr(StrLoc(Arg1), slen);

   /*
    * Point floc at the start of s and lloc at the end of s.  Work floc
    *  and sloc along s in opposite directions, swapping the characters
    *  at floc and lloc.
    */
   floc = StrLoc(Arg0);
   lloc = floc + --slen;
   while (floc < lloc) {
      c = *floc;
      *floc++ = *lloc;
      *lloc-- = c;
      }
   Return;
   }


/*
 * right(s1,n,s2) - pad s1 on left with s2 to length n.
 */

FncDcl(right,3)
   {
   register char *s, *st;
   word cnt, slen;
   char *sbuf, *s3, sbuf1[MaxCvtLen], sbuf2[MaxCvtLen];
   extern char *alcstr();

   /*
    * s1 must be a string.  n must be a non-negative integer and defaults
    *  to 1.  s2 must be a string and defaults to a blank.
    */
   if (cvstr(&Arg1, sbuf1) == NULL)
      runerr(103, &Arg1);
   defshort(&Arg2, 1);
   if ((cnt = IntVal(Arg2)) < 0)
      runerr(205, &Arg2);
   defstr(&Arg3, sbuf2, &blank);

   strreq(cnt);

   if (StrLen(Arg3) == 0) {
      /*
       * The padding string is null, make it a blank.
       */
      slen = 1;
      s3 = " ";
      }
   else {
      slen = StrLen(Arg3);
      s3 = StrLoc(Arg3);
      }

   /*
    * Get n bytes of string space.  Start at the left end of the new
    *  string and copy s2 into the new string as many times as it fits.
    */
   sbuf = alcstr(NULL, cnt);
   s = sbuf;
   while (s < sbuf + cnt) {
      st = s3;
      while (st < s3 + slen && s < sbuf + cnt)
         *s++ = *st++;
      }

   /*
    * Copy s1 into the new string, starting at the right end and copying
    *  s2 from right to left.  If *s1 > n, only copy n bytes.
    */
   s = sbuf + cnt;
   slen = StrLen(Arg1);
   st = StrLoc(Arg1) + slen;
   if (slen > cnt)
      slen = cnt;
   while (slen-- > 0)
      *--s = *--st;

   /*
    * Return the new string.
    */
   StrLen(Arg0) = cnt;
   StrLoc(Arg0) = sbuf;
   Return;
   }


/*
 * trim(s,c) - trim trailing characters in c from s.
 */

FncDcl(trim,2)
   {
   char *sloc;
   char sbuf[MaxCvtLen];
   int *cs, csbuf[CsetSize];
   extern char *alcstr();
   static int spcset[CsetSize] = /* just a blank */
      cset_display(0, 0, 01, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

   /*
    * s must be a string; c defaults to a cset containing a blank.
    */
   switch (cvstr(&Arg1, sbuf)) {

      case NULL:
         runerr(103, &Arg1);

      case Cvt:
         strreq(StrLen(Arg1));
         StrLoc(Arg1) = alcstr(StrLoc(Arg1), StrLen(Arg1));

     }

   defcset(&Arg2, &cs, csbuf, spcset);

   /*
    * Start at the end of s and then back up until a character that is
    *  not in c is found.  The actual trimming is done by having a descriptor
    *  that points at the string of s, but has a reduced length.
    */
   Arg0 = Arg1;
   sloc = StrLoc(Arg1) + StrLen(Arg1) - 1;
   while (sloc >= StrLoc(Arg1) && Testb(*sloc, cs)) {
      sloc--;
      StrLen(Arg0)--;
      }
   Return;
   }
