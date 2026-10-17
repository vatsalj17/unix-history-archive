/*
 * File: fmisc.c
 *  Contents: collect, copy, display, image, seq, runstats, type
 */

#include "../h/rt.h"
#include "gc.h"
#ifdef RunStats
#ifndef VMS
#include <sys/types.h>
#include <sys/times.h>
#else VMS
#include <types.h>
struct tms {
    time_t    tms_utime;	/* user time */
    time_t    tms_stime;	/* system time */
    time_t    tms_cutime;	/* user time, children */
    time_t    tms_cstime;	/* system time, children */
};
#endif	VMS
#endif RunStats

#define Stralc(a,b) alcstr(a,(word)(b))
/*
 * collect() - explicit call to garbage collector.
 */

FncDcl(collect,0)
   {
   collect();
   Arg0 = nulldesc;
   Return;
   }


/*
 * copy(x) - make a copy of object x.
 */

FncDcl(copy,1)
   {
   register int i;
   struct descrip  *d1, *d2;
   union block *bp, *ep, **tp;
   extern struct b_table *alctable();
   extern struct b_telem *alctelem();
   extern struct b_set *alcset();
   extern struct b_selem *alcselem();
   extern struct b_record *alcrecd();

   if (Qual(Arg1))
      /*
       * x is a string, just copy its descriptor
       *  into Arg0.
       */
      Arg0 = Arg1;
   else {
      switch (Type(Arg1)) {
         case T_Null:
         case T_Integer:
         case T_Longint:
         case T_Real:
         case T_File:
         case T_Cset:
         case T_Proc:
         case T_Coexpr:
            /*
             * Copy the null value, integers, long integers, reals, files,
             *  csets, procedures, and co-expressions by copying the descriptor.
             *   Note that for integers, this results in the assignment
             *   of a value, for the other types, a pointer is directed to
             *   a data block.
             */
            Arg0 = Arg1;
            break;

         case T_List:
            /*
             * Pass the buck to cplist to copy a list.
             */
            cplist(&Arg1, &Arg0, (word)1, BlkLoc(Arg1)->list.size + 1);
            break;

         case T_Table:
            /*
             * Allocate space for table and elements and copy old table
             *  block into new.
             */
            blkreq((sizeof(struct b_table)) +
                  (sizeof(struct b_telem)) * BlkLoc(Arg1)->table.size);
            bp = (union block *) alctable(&nulldesc);
            bp->table = BlkLoc(Arg1)->table;
            /*
             * Work down the chain of table element blocks in each bucket
             *  and create identical chains in new table.
             */
            for (i = 0; i < TSlots; i++) {
               tp = &(BlkLoc(bp->table.buckets[i]));
               for (ep = *tp; ep != NULL; ep = *tp) {
                  *tp = (union block *) alctelem();
                  (*tp)->telem = ep->telem;
                  tp = &(BlkLoc((*tp)->telem.clink));
                  }
               }
            /*
             * Return the copied table.
             */
            Arg0.dword = D_Table;
            BlkLoc(Arg0) = bp;
            break;

         case T_Set:
            /*
             * Allocate space for set and elements and copy old set
             *  block into new.
             */
            blkreq((sizeof(struct b_set)) +
                  (sizeof(struct b_selem)) * BlkLoc(Arg1)->set.size);
            bp = (union block *) alcset(&nulldesc);
            bp->set = BlkLoc(Arg1)->set;
            /*
             * Work down the chain of set elements in each bucket
             *  and create identical chains in new set.
             */
            for (i = 0; i < SSlots; i++) {
               tp = &(BlkLoc(bp->set.sbucks[i]));
               for (ep = *tp; ep != NULL; ep = *tp) {
                  *tp = (union block *) alcselem(&nulldesc,(word)0);
                  (*tp)->selem = ep->selem;
                  tp = &(BlkLoc((*tp)->selem.clink));
                  }
               }
            /*
             * Return the copied set.
             */
            Arg0.dword = D_Set;
            BlkLoc(Arg0) = bp;
            break;

         case T_Record:
            /*
             * Allocate space for the new record and copy the old
             *  one into it.
             */
            blkreq(BlkLoc(Arg1)->record.blksize);
            i = BlkLoc(BlkLoc(Arg1)->record.recdesc)->proc.nfields;
            bp = (union block *)alcrecd(i,&BlkLoc(Arg1)->record.recdesc);
            bp->record = BlkLoc(Arg1)->record;
            d1 = bp->record.fields;
            d2 = BlkLoc(Arg1)->record.fields;
            while (i--)
               *d1++ = *d2++;
            /*
             * Return the copied record
             */
            Arg0.dword = D_Record;
            BlkLoc(Arg0) = bp;
            break;

         default:
            syserr("copy: illegal datatype.");
         }
      }
   Return;
   }


/*
 * display(i,f) - display local variables of i most recent
 * procedure activations, plus global variables.
 * Output to file f (default &errout).
 */

FncDcl(display,2)
   {
   struct pf_marker *fp;
   register struct descrip *dp;
   register struct descrip *np;
   register int n;
   long l;
   int count;
   FILE *f;
   struct b_proc *bp;
   extern struct descrip *globals, *eglobals;
   extern struct descrip *gnames;
   extern struct descrip *statics;

   /*
    * i defaults to &level; f defaults to &errout.
    */
   defint(&Arg1, &l, (word)k_level);
   deffile(&Arg2, &errout);
   /*
    * Produce error if file can't be written on.
    */
   f = BlkLoc(Arg2)->file.fd;
   if ((BlkLoc(Arg2)->file.status & Fs_Write) == 0)
      runerr(213, &Arg2);

   /*
    * Produce error if i is negative; constrain i to be >= &level.
    */
   if (l < 0)
      runerr(205, &Arg1);
   else if (l > k_level)
      count = k_level;
   else
      count = l;

   fp = pfp;		/* start fp at most recent procedure frame */
   dp = argp;
   while (count--) {		/* go back through 'count' frames */

      bp = (struct b_proc *) BlkLoc(*dp); /* get address of procedure block */

      /*
       * Print procedure name.
       */
      putstr(f, StrLoc(bp->pname), StrLen(bp->pname));
      fprintf(f, " local identifiers:\n");

      /*
       * Print arguments.
       */
      np = bp->lnames;
      for (n = bp->nparam; n > 0; n--) {
         fprintf(f, "   ");
         putstr(f, StrLoc(*np), StrLen(*np));
         fprintf(f, " = ");
         outimage(f, ++dp, 0);
         putc('\n', f);
         np++;
         }

      /*
       * Print local dynamics.
       */
      dp = &fp->pf_locals[0];
      for (n = bp->ndynam; n > 0; n--) {
         fprintf(f, "   ");
         putstr(f, StrLoc(*np), StrLen(*np));
         fprintf(f, " = ");
         outimage(f, dp++, 0);
         putc('\n', f);
         np++;
         }

      /*
       * Print local statics.
       */
      dp = &statics[bp->fstatic];
      for (n = bp->nstatic; n > 0; n--) {
         fprintf(f, "   ");
         putstr(f, StrLoc(*np), StrLen(*np));
         fprintf(f, " = ");
         outimage(f, dp++, 0);
         putc('\n', f);
         np++;
         }

      dp = fp->pf_argp;
      fp = fp->pf_pfp;
      }

   /*
    * Print globals.
    */
   fprintf(f, "global identifiers:\n");
   dp = globals;
   np = gnames;
   while (dp < eglobals) {
      fprintf(f, "   ");
      putstr(f, StrLoc(*np), StrLen(*np));
      fprintf(f, " = ");
      outimage(f, dp++, 0);
      putc('\n', f);
      np++;
      }
   fflush(f);
   Arg0 = nulldesc;     	/* Return null value. */
   Return;
   }


/*
 * image(x) - return string image of object x.  Nothing fancy here,
 *  just plug and chug on a case-wise basis.
 */

FncDcl(image,1)
   {
   register word len, outlen, rnlen;
   register char *s;
   register union block *bp;
   char *type;
   extern char *alcstr();
   word prescan();
   char sbuf[MaxCvtLen];
   FILE *fd;

   if (Qual(Arg1)) {
      /*
       * Get some string space.  The magic 2 is for the double quote at each
       *  end of the resulting string.
       */
      strreq(prescan(&Arg1) + 2);
      len = StrLen(Arg1);
      s = StrLoc(Arg1);
      outlen = 2;

      /*
       * Form the image by putting a " in the string space, calling
       *  doimage with each character in the string, and then putting
       *  a " at then end.  Note that doimage directly writes into the
       *  string space.  (Hence the indentation.)  This techinique is used
       *  several times in this routine.
       */
      StrLoc(Arg0) = Stralc("\"", 1);
      while (len-- > 0)
         outlen += doimage(*s++, '"');
      Stralc("\"", 1);
      StrLen(Arg0) = outlen;
      Return;
      }

   switch (Type(Arg1)) {

      case T_Null:
         StrLoc(Arg0) = "&null";
         StrLen(Arg0) = 5;
         Return;

      case T_Integer:
      case T_Longint:
      case T_Real:
         /*
          * Form a string representing the number and allocate it.
          */
         cvstr(&Arg1, sbuf);
         len = StrLen(Arg1);
         strreq(len);
         StrLoc(Arg0) = alcstr(StrLoc(Arg1), len);
         StrLen(Arg0) = len;
         Return;

      case T_Cset:

         /*
          * Check for distinguished csets by looking at the address of
          *  of the object to image.  If one is found, make a string
          *  naming it and return.
          */
         if (BlkLoc(Arg1) == ((union block *) &k_ascii)) {
            StrLoc(Arg0) = "&ascii";
            StrLen(Arg0) = 6;
            Return;
            }
         else if (BlkLoc(Arg1) == ((union block *) &k_cset)) {
            StrLoc(Arg0) = "&cset";
            StrLen(Arg0) = 5;
            Return;
            }
         else if (BlkLoc(Arg1) == ((union block *) &k_lcase)) {
            StrLoc(Arg0) = "&lcase";
            StrLen(Arg0) = 6;
            Return;
            }
         else if (BlkLoc(Arg1) == ((union block *) &k_ucase)) {
            StrLoc(Arg0) = "&ucase";
            StrLen(Arg0) = 6;
            Return;
            }
         /*
          * Convert the cset to a string and proceed as is done for
          *  string images but use a ' rather than " to bound the
          *  result string.
          */
         cvstr(&Arg1, sbuf);
         strreq(prescan(&Arg1) + 2);
         len = StrLen(Arg1);
         s = StrLoc(Arg1);
         outlen = 2;
         StrLoc(Arg0) = Stralc("'", 1);
                        while (len-- > 0)
                            outlen += doimage(*s++, '\'');
                        Stralc("'", 1);
         StrLen(Arg0) = outlen;
         Return;

      case T_File:
         /*
          * Check for distinguished files by looking at the address of
          *  of the object to image.  If one is found, make a string
          *  naming it and return.
          */
         if ((fd = BlkLoc(Arg1)->file.fd) == stdin) {
            StrLen(Arg0) = 6;
            StrLoc(Arg0) = "&input";
            }
         else if (fd == stdout) {
            StrLen(Arg0) = 7;
            StrLoc(Arg0) = "&output";
            }
         else if (fd == stderr) {
            StrLen(Arg0) = 7;
            StrLoc(Arg0) = "&errout";
            }
         else {
            /*
             * The file is not a standard one, form a string of the form
             *  file(nm) where nm is the argument originally given to
             *  open.
             */
            strreq(prescan(&BlkLoc(Arg1)->file.fname)+6);
            len = StrLen(BlkLoc(Arg1)->file.fname);
            s = StrLoc(BlkLoc(Arg1)->file.fname);
            outlen = 6;
            StrLoc(Arg0) = Stralc("file(", 5);
                           while (len-- > 0)
                              outlen += doimage(*s++, '\0');
                           Stralc(")", 1);
            StrLen(Arg0) = outlen;
            }
         Return;

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
         len = StrLen(BlkLoc(Arg1)->proc.pname);
         s = StrLoc(BlkLoc(Arg1)->proc.pname);
         switch (BlkLoc(Arg1)->proc.ndynam) {
            default:  type = "procedure "; break;
            case -1:  type = "function "; break;
            case -2:  type = "record constructor "; break;
            }
         outlen = strlen(type);
         strreq(len + outlen);
         StrLoc(Arg0) = alcstr(type, outlen);
         alcstr(s, len);
         StrLen(Arg0) = len + outlen;
         Return;

      case T_List:
         /*
          * Produce:
          *  "list(n)"
          * where n is the current size of the list.
          */
         bp = BlkLoc(Arg1);
         sprintf(sbuf, "list(%ld)", (long)bp->list.size);
         len = strlen(sbuf);
         strreq(len);
         StrLoc(Arg0) = alcstr(sbuf, len);
         StrLen(Arg0) = len;
         Return;

      case T_Lelem:
         StrLen(Arg0) = 18;
         StrLoc(Arg0) = "list element block";
         Return;

      case T_Table:
         /*
          * Produce:
          *  "table(n)"
          * where n is the size of the table.
          */
         bp = BlkLoc(Arg1);
         sprintf(sbuf, "table(%ld)", (long)bp->table.size);
         len = strlen(sbuf);
         strreq(len);
         StrLoc(Arg0) = alcstr(sbuf, len);
         StrLen(Arg0) = len;
         Return;

      case T_Telem:
         StrLen(Arg0) = 19;
         StrLoc(Arg0) = "table element block";
         Return;

      case T_Set:
         /*
          * Produce "set(n)" where n is size of the set.
          */
         bp = BlkLoc(Arg1);
         sprintf(sbuf, "set(%ld)", (long)bp->set.size);
         len = strlen(sbuf);
         strreq(len);
         StrLoc(Arg0) = alcstr(sbuf,len);
         StrLen(Arg0) = len;
         Return;

      case T_Selem:
         StrLen(Arg0) = 17;
         StrLoc(Arg0) = "set element block";
         Return;

      case T_Record:
         /*
          * Produce:
          *  "record name(n)"
          * where n is the number of fields.
          */
         bp = BlkLoc(Arg1);
         rnlen = StrLen(BlkLoc(bp->record.recdesc)->proc.recname);
         strreq(15 + rnlen);	/* 15 = *"record " + *"(nnnnnn)" */
         bp = BlkLoc(Arg1);
         sprintf(sbuf, "(%ld)", (long)BlkLoc(bp->record.recdesc)->proc.nfields);
         len = strlen(sbuf);
         StrLoc(Arg0) = Stralc("record ", 7);
            alcstr(StrLoc(BlkLoc(bp->record.recdesc)->proc.recname),rnlen);
            alcstr(sbuf, len);
         StrLen(Arg0) = 7 + len + rnlen;
         Return;

      case T_Coexpr:
         /*
          * Produce:
          *  "co-expression(n)"
          * where n is the number of results that have been produced.
          */
         strreq(22);
         sprintf(sbuf, "(%ld)", (long)BlkLoc(Arg1)->coexpr.size);
         len = strlen(sbuf);
         StrLoc(Arg0) = Stralc("co-expression", 13);
         alcstr(sbuf, len);
         StrLen(Arg0) = 13 + len;
         Return;

      default:
         syserr("image: unknown type.");
      }
   Return;
   }

/*
 * doimage(c,q) - allocate character c in string space, with escape
 *  conventions if c is unprintable, '\', or equal to q.
 *  Returns number of characters allocated.
 */

doimage(c, q)
int c, q;
   {
   static char *cbuf = "\\\0\0\0";
   extern char *alcstr();

   if (c >= ' ' && c < '\177') {
      /*
       * c is printable, but special case ", ', and \.
       */
      switch (c) {
         case '"':
            if (c != q) goto def;
            Stralc("\\\"", 2);
            return 2;
         case '\'':
            if (c != q) goto def;
            Stralc("\\'", 2);
            return 2;
         case '\\':
            Stralc("\\\\", 2);
            return 2;
         default:
         def:
            cbuf[0] = c;
            cbuf[1] = '\0';
            Stralc(cbuf,1);
            return 1;
         }
      }

   /*
    * c is some sort of unprintable character.  If it is one of the common
    *  ones, produce a special representation for it, otherwise, produce
    *  its octal value.
    */
   switch (c) {
      case '\b':			/*      backspace    */
         Stralc("\\b", 2);
         return 2;
      case '\177':			/*      delete       */
         Stralc("\\d", 2);
         return 2;
      case '\33':			/*      escape       */
         Stralc("\\e", 2);
         return 2;
      case '\f':			/*      form feed    */
         Stralc("\\f", 2);
         return 2;
      case '\n':			/*      new line     */
         Stralc("\\n", 2);
         return 2;
      case '\r':			/*      return       */
         Stralc("\\r", 2);
         return 2;
      case '\t':			/*      horizontal tab     */
         Stralc("\\t", 2);
         return 2;
      case '\13':			/*      vertical tab     */
         Stralc("\\v", 2);
         return 2;
      default:				/*      octal constant  */
         cbuf[0] = '\\';
         cbuf[1] = ((c&0300) >> 6) + '0';
         cbuf[2] = ((c&070) >> 3) + '0';
         cbuf[3] = (c&07) + '0';
         Stralc(cbuf, 4);
         return 4;
      }
   }

/*
 * prescan(d) - return upper bound on length of expanded string.  Note
 *  that the only time that prescan is wrong is when the string contains
 *  one of the "special" unprintable characters, e.g. tab.
 */
word prescan(d)
struct descrip *d;
   {
   register word slen, len;
   register char *s, c;

   s = StrLoc(*d);
   len = 0;
   for (slen = StrLen(*d); slen > 0; slen--)
      if ((c = (*s++)) < ' ' || c >= 0177)
         len += 4;
      else if (c == '"' || c == '\\' || c == '\'')
         len += 2;
      else
         len++;

   return len;
   }


/*
 * seq(e1,e2)  - generate e1, e1+e2, e1+e2+e2, ... .
 */

FncDcl(seq,2)
   {
   long from, by;

   /*
    * Default e1 and e2 to 1.
    */
   defint(&Arg1, &from, (word)1);
   defint(&Arg2, &by, (word)1);
   
   /*
    * Produce error if e2 is 0, i.e., infinite sequence of e1's.
    */
   if (by == 0)
      runerr(211, &Arg2);

   /*
    * Suspend sequence, stopping when largest or smallest integer
    *  is reached.
    */
   while ((from <= MaxLong && by > 0) || (from >= MinLong && by < 0)) {
      Mkint(from, &Arg0);
      Suspend;
      from += by;
      }
   Fail;
   }


#ifdef RunStats
/*
 * runstats - return all sorts of junk (and junk of all sorts)
 */
FncDcl(runstats,0)
   {
   extern char *alcstr();
   char fmt[500],*p,*q;
   int i;
   struct tms tp;
   long time(), clock, runtim;
   times(&tp);

#ifndef MSDOS
   runtim = 1000 * ((tp.tms_utime - starttime) / (double)Hz);
#else MSDOS
   runtim = time() - starttime;
#endif MSDOS

#define NValues1  47
   for ((i = 1,p=fmt); i <= NValues1; i++) {
   	q = "%s\t%d\n";
         while (*p++ = *q++);
         --p;
         }

   strreq(3000);	/* just a guess */
   sprintf(strfree,fmt,
         "Lines executed/ex",ex_n_lines,
         "Opcodes executed/ex",ex_n_opcodes,
         "Total time/ex",runtim,
         "Invocations/ex",ex_n_invoke,
         "Icon procedure invocations/ex",ex_n_ipinvoke,
         "Built-in procedure invocations/ex",ex_n_bpinvoke,
         "Argument list adjustments/ex",ex_n_argadjust,
         "Operator invocations/ex",ex_n_opinvoke,
         "goal-directed invocations/ex",ex_n_mdge,
         "String invocations/ex",ex_n_stinvoke,
         "Keyword references/ex",ex_n_keywd,
         "Local variable references/ex",ex_n_locref,
         "Global variable references/ex",ex_n_globref,
         "Static variable references/ex",ex_n_statref,
         
         "Expression suspensions/gde",gde_n_esusp,
         "Esusp bytes copied/gde",gde_bc_esusp,
         "Icon procedure suspensions/gde",gde_n_psusp,
         "Psusp bytes copied/gde",gde_bc_psusp,
         "Operator & built-in suspensions/gde",gde_n_susp,
         "Susp bytes copied/gde",gde_bc_susp,
         
         "Expression failures/gde",gde_n_efail,
         "Icon procedure failures/gde",gde_n_pfail,
         "Operator & built-in failures/gde",gde_n_fail,
         "Evaluation resumptions/gde",gde_n_resume,
         "Expression returns/gde",gde_n_eret,
         "Icon procedure returns/gde",gde_n_pret,
         
   	"Block Region Size/gc",maxblk-blkbase,
         "Block Region Usage/gc",blkfree-blkbase,
         "String Size/gc",strend-strbase,
         "String Usage/gc",strfree-strbase,

         "Garbage collections/gc",gc_n_total,
         "String triggered collections/gc",gc_n_string,
         "Block region triggered collections/gc",gc_n_blk,
         "CE triggered collections/gc",gc_n_coexpr,
         
         "Total garbage collection time/gc",gc_t_total,
         "Last garbage collection time/gc",gc_t_last,

         "Dereferences/ev",ev_n_deref,
         "No-op dereferences/ev",ev_n_redunderef,
         "Tv substring dereferences/ev",ev_n_tsderef,
         "Tv table dereferences/ev",ev_n_ttderef,
         "&pos dereferences/ev",ev_n_tpderef,

         "Cvint operations/cv",cv_n_int,
         "No-op cvint operations/cv",cv_n_rint,
         "Cvreal operations/cv",cv_n_real,
         "No-op cvreal operations/cv",cv_n_rreal,
         "Cvnum operations/cv",cv_n_num,
         "No-op cvnum operations/cv",cv_n_rnum,
         "Cvstr operations/cv",cv_n_str,
         "No-op cvstr operations/cv",cv_n_rstr,
         "Cvcset operations/cv",cv_n_cset,
         "No-op cvcset operations/cv",cv_n_rcset,
         0,0,0,0);

#define NValues2  15
   for ((i = 1,p=fmt); i <= NValues2; i++) {
   	q = "%s\t%d\n";
         while (*p++ = *q++);
         --p;
         }

   sprintf(strfree+strlen(strfree),fmt,
         "Block region allocations/al",al_n_total,
         "Total block space allocated/al",al_bc_btotal,
         "String allocations/al",al_n_str,
         "Total string space allocated/al",al_bc_stotal,
         "Trapped substring allocations/al",al_n_subs,
         "Cset allocations/al",al_n_cset,
         "Real number allocations/al",al_n_real,
         "List allocations/al",al_n_list,
         "List block allocations/al",al_n_lstb,
         "Table allocations/al",al_n_table,
         "Table element allocations/al",al_n_telem,
         "Table element tvars/al",al_n_tvtbl,
         "File block allocations/al",al_n_file,
         "Co-expression block allocations/al",al_n_eblk,
         "Co-expression stack allocations/al",al_n_estk,

         0,0,0,0 /* who can count? */
         );
   StrLoc(Arg0) = alcstr(strfree,(word)strlen(strfree));
   StrLen(Arg0) = alcstr(StrLoc(Arg0));
   Return;
   }
#else RunStats
char junk;	/* prevent empty object module */
#endif RunStats


/*
 * type(x) - return type of x as a string.
 */

/* >type1 */
FncDcl(type,1)
   {

   if (Qual(Arg1)) {
      StrLen(Arg0) = 6;
      StrLoc(Arg0) = "string";
      }

   else {
      switch (Type(Arg1)) {

         case T_Null:
            StrLen(Arg0) = 4;
            StrLoc(Arg0) = "null";
            break;

         case T_Integer:
         case T_Longint:
            StrLen(Arg0) = 7;
            StrLoc(Arg0) = "integer";
            break;

         case T_Real:
            StrLen(Arg0) = 4;
            StrLoc(Arg0) = "real";
            break;
/* <type1 */

         case T_Cset:
            StrLen(Arg0) = 4;
            StrLoc(Arg0) = "cset";
            break;

         case T_File:
            StrLen(Arg0) = 4;
            StrLoc(Arg0) = "file";
            break;

         case T_Proc:
            StrLen(Arg0) = 9;
            StrLoc(Arg0) = "procedure";
            break;

         case T_List:
            StrLen(Arg0) = 4;
            StrLoc(Arg0) = "list";
            break;

         case T_Table:
            StrLen(Arg0) = 5;
            StrLoc(Arg0) = "table";
            break;

         case T_Set:
            StrLen(Arg0) = 3;
            StrLoc(Arg0) = "set";
            break;

         case T_Record:
            Arg0 = BlkLoc(BlkLoc(Arg1)->record.recdesc)->proc.recname;
            break;

         case T_Coexpr:
            StrLen(Arg0) = 13;
            StrLoc(Arg0) = "co-expression";
            break;


         default:
            syserr("type: unknown type.");
/* >type2 */
         }
      }
   Return;
   }
/* <type2 */
