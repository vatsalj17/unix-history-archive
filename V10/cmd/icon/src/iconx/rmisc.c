/*
 * File: rmisc.c
 *  Contents: deref, hash, outimage, qtos, trace, tvkeys
 */

#include "../h/rt.h"
/*
 * deref - dereference a descriptor.
 */

/* >deref1 */
deref(dp)
struct descrip *dp;
   {
   register word i, j;
   register union block *bp;
   struct descrip v, tbl, tref;
   char sbuf[MaxCvtLen];
   extern char *alcstr();

   if (!Qual(*dp) && Var(*dp)) {
      /*
       * dp points to a variable and must be dereferenced.
       */
/* <deref1 */
/* >deref2 */
      if (!Tvar(*dp))
         /*
          * An ordinary variable is being dereferenced, just replace
          *  *dp with the the descriptor *dp is pointing to.
          */
         *dp = *VarLoc(*dp);
/* <deref2 */
/* >deref3 */
      else switch (Type(*dp)) {

            case T_Tvsubs:
               Inc(ev_n_tsderef);
               /*
                * A substring trapped variable is being dereferenced.
                *  Point bp to the trapped variable block and v to
                *  the string.
                */
               bp = TvarLoc(*dp);
               v = bp->tvsubs.ssvar;
               DeRef(v);
               if (!Qual(v))
                  runerr(103, &v);
               if (bp->tvsubs.sspos + bp->tvsubs.sslen - 1 > StrLen(v))
                  runerr(205, NULL);
               /*
                * Make a descriptor for the substring by getting the
                *  length and pointing into the string.
                */
               StrLen(*dp) = bp->tvsubs.sslen;
               StrLoc(*dp) = StrLoc(v) + bp->tvsubs.sspos - 1;
               break;
/* <deref3 */

/* >deref4 */
            case T_Tvtbl:
               Inc(ev_n_ttderef);
               if (BlkLoc(*dp)->tvtbl.title == T_Telem) {
                  /*
                   * The tvtbl has been converted to a telem and is in
                   *  the table.  Replace the descriptor pointed to by dp
                   *  with the value of the element.
                   */
                   *dp = BlkLoc(*dp)->telem.tval;
                   break;
                   }

               /*
                *  Point tbl to the table header block, tref to the
                *  subscripting value, and bp to the appropriate element
                *  chain.  Point dp to a descriptor for the default
                *  value in case the value referenced by the subscript
                *  is not in the table.
                */
               tbl = BlkLoc(*dp)->tvtbl.clink;
               tref = BlkLoc(*dp)->tvtbl.tref;
               i = BlkLoc(*dp)->tvtbl.hashnum;
               *dp = BlkLoc(tbl)->table.defvalue;
               bp = BlkLoc(BlkLoc(tbl)->table.buckets[SlotNum(i,TSlots)]);

               /*
                * Traverse the element chain looking for the subscript value.
                *  If found, replace the descriptor pointed to by dp with
                *  the value of the element.
                */
               while (bp != NULL && bp->telem.hashnum <= i) {
                  if ((bp->telem.hashnum == i) &&
                     (equiv(&bp->telem.tref, &tref))) {
                        *dp = bp->telem.tval;
                        break;
                        }
                  bp = BlkLoc(bp->telem.clink);
                  }
               break;
/* <deref4 */

/* >deref5 */
            case T_Tvkywd:
               bp = TvarLoc(*dp);
               *dp = bp->tvkywd.kyval;
               break;
/* <deref5 */

            default:
               syserr("deref: illegal trapped variable");
            }
      }
#ifdef Debug
   if (!Qual(*d) && Var(*d))
      syserr("deref: didn't get dereferenced");
#endif Debug
   return 1;
   }
/* <deref */


/*
 * hash - compute hash value of arbitrary object for table and set accessing.
 */

/* >hash */
word hash(dp)
struct descrip *dp;
   {
   word i;
   double r;
   register word j;
   register char *s;

   if (Qual(*dp)) {

      /*
       * Compute the hash value for the string by summing the value
       *  of all the characters (up to a maximum of 10) plus the length.
       */
      i = 0;
      s = StrLoc(*dp);
      j = StrLen(*dp);
      for (j = (j <= 10) ? j : 10 ; j > 0; j--)
         i += *s++ & 0377;
      i += StrLen(*dp) & 0377;
      }
   else {
      switch (Type(*dp)) {
         /*
          * The hash value for numeric types is the bitstring
          *  representation of the value.
          */

         case T_Integer:
            i = IntVal(*dp);
            break;

         case T_Longint:
            i = BlkLoc(*dp)->longint.intval;
            break;

         case T_Real:
            GetReal(dp,r);
            i = r;
            break;

         case T_Cset:
            /*
             * Compute the hash value for a cset by exclusive or-ing
             *  the words in the bit array.
             */
            i = 0;
            for (j = 0; j < CsetSize; j++)
               i ^= BlkLoc(*dp)->cset.bits[j];
            break;

         default:
            /*
             * For other types, use the type code as the hash
             *  value.
             */
            i = Type(*dp);
            break;
         }
      }

   return i;
   }
/* <hash */


#define StringLimit	16		/* limit on length of imaged string */
#define ListLimit	 6		/* limit on list items in image */

/*
 * outimage - print image of d on file f.  If restrict is non-zero,
 *  fields of records will not be imaged.
 */

outimage(f, d, restrict)
FILE *f;
struct descrip *d;
int restrict;
   {
   register word i, j;
   register char *s;
   register union block *bp, *vp;
   char *type;
   FILE *fd;
   struct descrip q;
   extern char *blkname[];
   double rresult;

outimg:

   if (Qual(*d)) {
      /*
       * *d is a string qualifier.  Print StringLimit characters of it
       *  using printimage and denote the presence of additional characters
       *  by terminating the string with "...".
       */
      i = StrLen(*d);
      s = StrLoc(*d);
      j = Min(i, StringLimit);
      putc('"', f);
      while (j-- > 0)
         printimage(f, *s++, '"');
      if (i > StringLimit)
         fprintf(f, "...");
      putc('"', f);
      return;
      }

   if (Var(*d) && !Tvar(*d)) {
      /*
       * *d is a variable.  Print "variable =", dereference it and loop
       *  back to the top to cause the value of the variable to be imaged.
       */
      fprintf(f, "variable = ");
      d = VarLoc(*d);
      goto outimg;
      }

   switch (Type(*d)) {

      case T_Null:
         if (restrict == 0)
            fprintf(f, "&null");
         return;

      case T_Integer:
         fprintf(f, "%d", (int)IntVal(*d));
         return;

      case T_Longint:
         fprintf(f, "%ld", BlkLoc(*d)->longint.intval);
         return;

      case T_Real:
         {
         char s[30];
         struct descrip junk;
         double rresult;

         GetReal(d,rresult);
         rtos(rresult, &junk, s);
         fprintf(f, "%s", s);
         return;
         }

      case T_Cset:
         /*
          * Check for distinguished csets by looking at the address of
          *  of the object to image.  If one is found, print its name.
          */
         if (BlkLoc(*d) == (union block *) &k_ascii) {
            fprintf(f, "&ascii");
            return;
            }
         else if (BlkLoc(*d) == (union block *) &k_cset) {
            fprintf(f, "&cset");
            return;
            }
         else if (BlkLoc(*d) == (union block *) &k_lcase) {
            fprintf(f, "&lcase");
            return;
            }
         else if (BlkLoc(*d) == (union block *) &k_ucase) {
            fprintf(f, "&ucase");
            return;
            }
         /*
          * Use printimage to print each character in the cset.  Follow
          *  with "..." if the cset contains more than StringLimit
          *  characters.
          */
         putc('\'', f);
         j = StringLimit;
         for (i = 0; i < 256; i++) {
            if (Testb(i, BlkLoc(*d)->cset.bits)) {
               if (j-- <= 0) {
                  fprintf(f, "...");
                  break;
                  }
               printimage(f, (int)i, '\'');
               }
            }
         putc('\'', f);
         return;

      case T_File:
         /*
          * Check for distinguished files by looking at the address of
          *  of the object to image.  If one is found, print its name.
          */
         if ((fd = BlkLoc(*d)->file.fd) == stdin)
            fprintf(f, "&input");
         else if (fd == stdout)
            fprintf(f, "&output");
         else if (fd == stderr)
            fprintf(f, "&output");
         else {
            /*
             * The file isn't a special one, just print "file(name)".
             */
            i = StrLen(BlkLoc(*d)->file.fname);
            s = StrLoc(BlkLoc(*d)->file.fname);
            fprintf(f, "file(");
            while (i-- > 0)
               printimage(f, *s++, '\0');
            putc(')', f);
            }
         return;

      case T_Proc:
         /*
          * Produce one of:
          *  "procedure name"
          *  "function name"
          *  "record constructor name"
          *
          * Note that the number of dynamic locals is used to determine
          *  what type of "procedure" is at hand.
          */
         i = StrLen(BlkLoc(*d)->proc.pname);
         s = StrLoc(BlkLoc(*d)->proc.pname);
         switch (BlkLoc(*d)->proc.ndynam) {
            default:  type = "procedure"; break;
            case -1:  type = "function"; break;
            case -2:  type = "record constructor"; break;
            }
         fprintf(f, "%s ", type);
         while (i-- > 0)
            printimage(f, *s++, '\0');
         return;

      case T_List:
         /*
          * listimage does the work for lists.
          */
         listimage(f, (struct b_list *)BlkLoc(*d), restrict);
         return;

      case T_Table:
         /*
          * Print "table(n)" where n is the size of the table.
          */
         fprintf(f, "table(%ld)", (long)BlkLoc(*d)->table.size);
         return;
      case T_Set:
	/*
         * print "set(n)" where n is the cardinality of the set
         */
	fprintf(f,"set(%ld)",(long)BlkLoc(*d)->set.size);
	return;

      case T_Record:
         /*
          * If restrict is non-zero, print "record(n)" where n is the
          *  number of fields in the record.  If restrict is zero, print
          *  the image of each field instead of the number of fields.
          */
         bp = BlkLoc(*d);
         i = StrLen(BlkLoc(bp->record.recdesc)->proc.recname);
         s = StrLoc(BlkLoc(bp->record.recdesc)->proc.recname);
         fprintf(f, "record ");
         while (i-- > 0)
            printimage(f, *s++, '\0');
         j = BlkLoc(bp->record.recdesc)->proc.nfields;
         if (j <= 0)
            fprintf(f, "()");
         else if (restrict > 0)
            fprintf(f, "(%ld)", (long)j);
         else {
            putc('(', f);
            i = 0;
            for (;;) {
               outimage(f, &bp->record.fields[i], restrict+1);
               if (++i >= j)
                  break;
               putc(',', f);
               }
            putc(')', f);
            }
         return;

      case T_Tvsubs:
         /*
          * Produce "v[i+:j] = value" where v is the image of the variable
          *  containing the substring, i is starting position of the substring
          *  j is the length, and value is the string v[i+:j].	If the length
          *  (j) is one, just produce "v[i] = value".
          */
         bp = BlkLoc(*d);
         d = VarLoc(bp->tvsubs.ssvar);
         if ((word)d == (word)&tvky_sub)
            fprintf(f, "&subject");
         else outimage(f, d, restrict);
         if (bp->tvsubs.sslen == 1)
            fprintf(f, "[%ld]", (long)bp->tvsubs.sspos);
         else
            fprintf(f, "[%ld+:%ld]", (long)bp->tvsubs.sspos, (long)bp->tvsubs.sslen);
         if ((word)d == (word)&tvky_sub) {
            fprintf(f, " = ");
            vp = BlkLoc(bp->tvsubs.ssvar);
            StrLen(q) = bp->tvsubs.sslen;
            StrLoc(q) = StrLoc(vp->tvkywd.kyval) + bp->tvsubs.sspos-1;
            d = &q;
            goto outimg;
            }
         else if (Qual(*d)) {
            StrLen(q) = bp->tvsubs.sslen;
            StrLoc(q) = StrLoc(*VarLoc(bp->tvsubs.ssvar)) + bp->tvsubs.sspos-1;
            fprintf(f, " = ");
            d = &q;
            goto outimg;
            }
         return;

      case T_Tvtbl:
         bp = BlkLoc(*d);
         /*
          * It is possible that descriptor d which thinks it is pointing
          *  at a TVTBL may actually be pointing at a TELEM which had
          *  been converted from a trapped variable. Check for this first
          *  and if it is a TELEM produce the outimage of its value.
          */
         if (bp->tvtbl.title == T_Telem) {
            outimage(f, &bp->tvtbl.tval, restrict);
            return;
            }
         /*
          * It really was a TVTBL - Produce "t[s]" where t is the image of
          *  the table containing the element and s is the image of the
          *  subscript.
          */
         else {
            outimage(f, &bp->tvtbl.clink, restrict);
            putc('[', f);
            outimage(f, &bp->tvtbl.tref, restrict);
            putc(']', f);
            return;
            }

      case T_Tvkywd:
         bp = BlkLoc(*d);
         i = StrLen(bp->tvkywd.kyname);
         s = StrLoc(bp->tvkywd.kyname);
         while (i-- > 0)
            putc(*s++, f);
         fprintf(f, " = ");
         outimage(f, &bp->tvkywd.kyval, restrict);
         return;


      case T_Coexpr:
         fprintf(f, "co-expression");
         return;

      default:
         if (Type(*d) <= MaxType)
            fprintf(f, "%s", blkname[Type(*d)]);
         else
            syserr("outimage: unknown type");
      }
   }

/*
 * printimage - print character c on file f using escape conventions
 *  if c is unprintable, '\', or equal to q.
 */

static printimage(f, c, q)
FILE *f;
int c, q;
   {
   if (c >= ' ' && c < '\177') {
      /*
       * c is printable, but special case ", ', and \.
       */
      switch (c) {
         case '"':
            if (c != q) goto def;
            fprintf(f, "\\\"");
            return;
         case '\'':
            if (c != q) goto def;
            fprintf(f, "\\'");
            return;
         case '\\':
            fprintf(f, "\\\\");
            return;
         default:
         def:
            putc(c, f);
            return;
         }
      }

   /*
    * c is some sort of unprintable character.	If it one of the common
    *  ones, produce a special representation for it, otherwise, produce
    *  its octal value.
    */
   switch (c) {
      case '\b':                        /* backspace */
         fprintf(f, "\\b");
         return;
      case '\177':                        /* delete */
         fprintf(f, "\\d");
         return;
      case '\33':                        /* escape */
         fprintf(f, "\\e");
         return;
      case '\f':                        /* form feed */
         fprintf(f, "\\f");
         return;
      case '\n':                        /* new line */
         fprintf(f, "\\n");
         return;
      case '\r':                        /* return */
         fprintf(f, "\\r");
         return;
      case '\t':                        /* horizontal tab */
         fprintf(f, "\\t");
         return;
      case '\13':                        /* vertical tab */
         fprintf(f, "\\v");
         return;
      default:				      /* octal constant */
         fprintf(f, "\\%03o", c&0377);
         return;
      }
   }

/*
 * listimage - print an image of a list.
 */

static listimage(f, lp, restrict)
FILE *f;
struct b_list *lp;
int restrict;
   {
   register word i, j;
   register struct b_lelem *bp;
   word size, count;

   bp = (struct b_lelem *) BlkLoc(lp->listhead);
   size = lp->size;

   if (restrict > 0 && size > 0) {
      /*
       * Just give indication of size if the list isn't empty.
       */
      fprintf(f, "list(%ld)", (long)size);
      return;
      }

   /*
    * Print [e1,...,en] on f.  If more than ListLimit elements are in the
    *  list, produce the first ListLimit/2 elements, an ellipsis, and the
    *  last ListLimit elements.
    */
   putc('[', f);
   count = 1;
   i = 0;
   if (size > 0) {
      for (;;) {
         if (++i > bp->nused) {
            i = 1;
            bp = (struct b_lelem *) BlkLoc(bp->listnext);
            }
         if (count <= ListLimit/2 || count > size - ListLimit/2) {
            j = bp->first + i - 1;
            if (j >= bp->nelem)
               j -= bp->nelem;
            outimage(f, &bp->lslots[j], restrict+1);
            if (count >= size)
               break;
            putc(',', f);
            }
         else if (count == ListLimit/2 + 1)
            fprintf(f, "...,");
         count++;
         }
      }
   putc(']', f);
   }


/*
 * qtos - convert a qualified string named by *d to a C-style string in
 *  in str.  At most MaxCvtLen characters are copied into str.
 */

qtos(d, str)
struct descrip *d;
char *str;
   {
   register word cnt, slen;
   register char *c;

   c = StrLoc(*d);
   slen = StrLen(*d);
   for (cnt = Min(slen, MaxCvtLen - 1); cnt > 0; cnt--)
      *str++ = *c++;
   *str = '\0';
   }


/*
 * ctrace - procedure *bp is being called with nargs arguments, the first
 *  of which is at arg; produce a trace message.
 */
ctrace(bp, nargs, arg)
struct b_proc *bp;
int nargs;
struct descrip *arg;
   {
   register int n;

   if (k_trace > 0)
      k_trace--;
   showline(bp->filename, line);
   showlevel(k_level);
   putstr(stderr, StrLoc(bp->pname), StrLen(bp->pname));
   putc('(', stderr);
   while (nargs--) {
      outimage(stderr, arg++, 0);
      if (nargs)
         putc(',', stderr);
      }
   putc(')', stderr);
   putc('\n', stderr);
   fflush(stderr);
   }

/*
 * rtrace - procedure *bp is returning *rval; produce a trace message.
 */

rtrace(bp, rval)
register struct b_proc *bp;
struct descrip *rval;
   {
   register int n;

   if (k_trace > 0)
      k_trace--;
   showline(bp->filename, line);
   showlevel(k_level);
   putstr(stderr, StrLoc(bp->pname), StrLen(bp->pname));
   fprintf(stderr, " returned ");
   outimage(stderr, rval, 0);
   putc('\n', stderr);
   fflush(stderr);
   }

/*
 * ftrace - procedure *bp is failing; produce a trace message.
 */

ftrace(bp)
register struct b_proc *bp;
   {
   register int n;

   if (k_trace > 0)
      k_trace--;
   showline(bp->filename, line);
   showlevel(k_level);
   putstr(stderr, StrLoc(bp->pname), StrLen(bp->pname));
   fprintf(stderr, " failed");
   putc('\n', stderr);
   fflush(stderr);
   }

/*
 * strace - procedure *bp is suspending *rval; produce a trace message.
 */

strace(bp, rval)
register struct b_proc *bp;
struct descrip *rval;
   {
   register int n;

   if (k_trace > 0)
      k_trace--;
   showline(bp->filename, line);
   showlevel(k_level);
   putstr(stderr, StrLoc(bp->pname), StrLen(bp->pname));
   fprintf(stderr, " suspended ");
   outimage(stderr, rval, 0);
   putc('\n', stderr);
   fflush(stderr);
   }

/*
 * atrace - procedure *bp is being resumed; produce a trace message.
 */

atrace(bp)
register struct b_proc *bp;
   {
   register int n;

   if (k_trace > 0)
      k_trace--;
   showline(bp->filename, line);
   showlevel(k_level);
   putstr(stderr, StrLoc(bp->pname), StrLen(bp->pname));
   fprintf(stderr, " resumed");
   putc('\n', stderr);
   fflush(stderr);
   }

/*
 * showline - print file and line number information.
 */
static showline(f, l)
char *f;
int l;
   {
   if (l > 0)
      fprintf(stderr, "%.10s: %d\t", f, l);
   else
      fprintf(stderr, "\t\t");
   }

/*
 * showlevel - print "| " n times.
 */
static showlevel(n)
register int n;
   {
   while (n-- > 0) {
      putc('|', stderr);
      putc(' ', stderr);
      }
   }


/*
 * putpos - assign value to &pos
 */

putpos(d1)
struct descrip *d1;
   {
   register word l1;
   long l2;
   switch (cvint(d1, &l2)) {

      case T_Integer:
         break;

      case T_Longint:
         return NULL;

      default:	runerr(101, d1);
      }

   l1 = cvpos(l2, StrLen(k_subject));
   if (l1 == 0)
      return NULL;
   k_pos = l1;
   return 1;
   }


/*
 * putran - assign value to &random
 */

putran(d1)
struct descrip *d1;
   {
   long l1;
   switch (cvint(d1, &l1)) {

      case T_Integer:
      case T_Longint:
         break;

      default: runerr(101, d1);
      }

   k_random = l1;
   return 1;
   }


/*
 * putsub - assign value to &subject
 */

/* >putsub */
putsub(dp)
struct descrip *dp;
   {
   char sbuf[MaxCvtLen];
   extern char *alcstr();

   switch (cvstr(dp, sbuf)) {

      case NULL:
         runerr(103, dp);

      case Cvt:
         strreq(StrLen(*dp));
         StrLoc(*dp) = alcstr(StrLoc(*dp), StrLen(*dp));

      case NoCvt:
         k_subject = *dp;
         k_pos = 1;
      }

   return 1;
   }
/* <putsub */


/*
 * puttrc - assign value to &trace
 */

puttrc(d1)
struct descrip *d1;
   {
   long l1;
   switch (cvint(d1, &l1)) {

      case T_Integer:
         k_trace = (int)l1;
         break;

      case T_Longint:
         k_trace = -1;
         break;

      default:	runerr(101, d1);
      }

   return 1;
   }
