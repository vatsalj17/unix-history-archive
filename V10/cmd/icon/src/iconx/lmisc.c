/*
 * File: lmisc.c
 *  Contents: create, keywd, limit, llist
 */

#include "../h/rt.h"
#include "../h/keyword.h"
#include "../h/version.h"

#ifndef VMS
#ifndef MSDOS
#include <sys/types.h>
#include <sys/times.h>
#include SysTime
#else MSDOS

#ifdef LATTICE
#include <time.h>
#endif LATTICE
#ifdef MSoft
#include <sys/types.h>
#include <time.h>
#endif MSoft
#endif MSDOS

#else VMS
#include <types.h>
#include <time.h>
struct tms {
    time_t    tms_utime;	/* user time */
    time_t    tms_stime;	/* system time */
    time_t    tms_cutime;	/* user time, children */
    time_t    tms_cstime;	/* system time, children */
};
#endif VMS

/*
 * create - return an entry block for a co-expression.
 */

OpBlock(create, 1, "create", 0)

create(entryp, cargp)
word *entryp;
register struct descrip *cargp;
   {
   register struct b_coexpr *sblkp;
   register struct b_refresh *rblkp;
   register struct descrip *dp, *ndp, *dsp;
   register word *newsp;
   int na, nl, i;
   struct b_proc *cproc;
   extern struct b_coexpr *alcstk();
   extern struct b_refresh *alceblk();

   /*
    * Get a new co-expression stack and initialize.
    */
   sblkp = alcstk();
   sblkp->activator = nulldesc;
   sblkp->size = 0;
   sblkp->nextstk = stklist;
   stklist = sblkp;
   /*
    * Icon stack starts at word after co-expression stack block.  C stack
    *  starts at end of stack region on machines with down-growing C stacks
    *  and somewhere in the middle of the region.
    *
    * The C stack is aligned on a doubleword boundary.	For upgrowing
    *  stacks, the C stack starts in the middle of the stack portion
    *  of the static block.  For downgrowing stacks, the C stack starts
    *  at the end of the static block.
    */
   newsp = (word *)((char *)sblkp + sizeof(struct b_coexpr));
#ifdef UpStack
   sblkp->cstate[0] =
      ((word)((char *)sblkp + (stksize - sizeof(*sblkp))/2)
       &~(WordSize*2-1));
#else
   sblkp->cstate[0] =
	((word)((char *)sblkp + stksize - WordSize)&~(WordSize*2-1));
#endif UpStack
   sblkp->es_argp = (struct descrip *)newsp;
   /*
    * Calculate number of arguments and number of local variables.
    *  na is nargs + 1 to include Arg0.
    */
   na = pfp->pf_nargs + 1;
   cproc = (struct b_proc *)BlkLoc(argp[0]);
   nl = cproc->ndynam;

   /*
    * Get a refresh block for the new co-expression.
    */
   blkreq((word)sizeof(struct b_refresh) + (na + nl) * sizeof(struct descrip));
   rblkp = alceblk(entryp, na, nl);
   sblkp->freshblk.dword = D_Refresh;
   BlkLoc(sblkp->freshblk) = (union block *) rblkp;

   /*
    * Copy current procedure frame marker into refresh block.
    */
   rblkp->pfmkr = *pfp;
   rblkp->pfmkr.pf_pfp = 0;

   /*
    * Copy arguments into refresh block and onto new stack.
    */
   dp = &argp[0];
   ndp = &rblkp->elems[0];
   dsp = (struct descrip *)newsp;
   for (i = 1; i <= na; i++) {
      *dsp++ = *dp;
      *ndp++ = *dp++;
      }

   /*
    * Copy procedure frame to new stack and point dsp to word after frame.
    */
   *((struct pf_marker *)dsp) = *pfp;
   sblkp->es_pfp = (struct pf_marker *)dsp;
   sblkp->es_pfp->pf_pfp = 0;
   dsp = (struct descrip *)((word *)dsp + Vwsizeof(*pfp));
   sblkp->es_ipc = entryp;
   sblkp->es_gfp = 0;
   sblkp->es_efp = 0;
   sblkp->es_ilevel = 0;
   sblkp->tvalloc = NULL;

   /*
    * Copy locals to new stack and refresh block.
    */
   dp = &(pfp->pf_locals)[0];
   for (i = 1; i <= nl; i++) {
      *dsp++ = *dp;
      *ndp++ = *dp++;
      }
   /*
    * Push two null descriptors on the stack.
    */
   *dsp++ = nulldesc;
   *dsp++ = nulldesc;

   sblkp->es_sp = (word *)dsp - 1;

   /*
    * Establish line and file values and clear location for transmitted value.
    */
   sblkp->es_line = line;

   /*
    * Return the new co-expression.
    */
   Arg0.dword = D_Coexpr;
   BlkLoc(Arg0) = (union block *) sblkp;
   Return;
   }

static char *day[] = {
   "Sunday", "Monday", "Tuesday", "Wednesday",
   "Thursday", "Friday", "Saturday"
   };

static char *month[] = {
   "January", "February", "March", "April", "May", "June",
   "July", "August", "September", "October", "November", "December"
   };

/*
 * keywd - process keyword.
 */

LibDcl(keywd,0,"&keywd")
   {
   register int hour;
   register word i;
   register char *merid;
   char sbuf[MaxCvtLen];
   struct tm *tbuf, *localtime();
#ifndef MSDOS
   struct tms tp;
#endif MSDOS
   long time();
   long clock, runtim;
   char *alcstr();

   /*
    * This is just plug and chug code.	For whatever keyword is desired,
    *  the appropriate value is dug out of the system and made into
    *  a suitable Icon value.
    *
    * A few special cases are worth noting:
    *  &pos, &random, &trace - built-in trapped variables are returned
    */
   switch (IntVal(Arg0)) {
      case K_ASCII:
	 Arg0.dword = D_Cset;
	 BlkLoc(Arg0) = (union block *) &k_ascii;
	 break;
      case K_CLOCK:
	 strreq((word)8);
	 time(&clock);
	 tbuf = localtime(&clock);
	 sprintf(sbuf,"%02d:%02d:%02d",tbuf->tm_hour,tbuf->tm_min,tbuf->tm_sec);
	 StrLen(Arg0) = 8;
	 StrLoc(Arg0) = alcstr(sbuf,(word)8);
	 break;
      case K_CSET:
	 Arg0.dword = D_Cset;
	 BlkLoc(Arg0) = (union block *) &k_cset;
	 break;
      case K_DATE:
	 strreq((word)10);
	 time(&clock);
	 tbuf = localtime(&clock);
	 sprintf(sbuf, "%04d/%02d/%02d",
		      (tbuf->tm_year)+1900,tbuf->tm_mon+1,tbuf->tm_mday);
	 StrLen(Arg0) = 10;
	 StrLoc(Arg0) = alcstr(sbuf,(word)10);
	 break;
      case K_DATELINE:
	 time(&clock);
	 tbuf = localtime(&clock);
	 if ((hour = tbuf->tm_hour) >= 12) {
	    merid = "pm";
	    if (hour > 12)
	       hour -= 12;
	    }
	 else {
	    merid = "am";
	    if (hour < 1)
	       hour += 12;
	    }
	 sprintf(sbuf, "%s, %s %d, %d  %d:%02d %s",
		 day[tbuf->tm_wday], month[tbuf->tm_mon], tbuf->tm_mday,
		 1900 + tbuf->tm_year, hour, tbuf->tm_min, merid);
	 strreq(i = strlen(sbuf));
	 StrLen(Arg0) = i;
	 StrLoc(Arg0) = alcstr(sbuf, i);
	 break;
      case K_ERROUT:
	 Arg0.dword = D_File;
	 BlkLoc(Arg0) = (union block *) &k_errout;
	 break;
      case K_HOST:
	 iconhost(sbuf);
	 strreq(i = strlen(sbuf));
	 StrLen(Arg0) = i;
	 StrLoc(Arg0) = alcstr(sbuf, i);
	 break;
      case K_INPUT:
	 Arg0.dword = D_File;
	 BlkLoc(Arg0) = (union block *) &k_input;
	 break;
      case K_LCASE:
	 Arg0.dword = D_Cset;
	 BlkLoc(Arg0) = (union block *) &k_lcase;
	 break;
      case K_LEVEL:
	 Arg0.dword = D_Integer;
	 IntVal(Arg0) = k_level;
	 break;
      case K_MAIN:
	 Arg0 = k_main;
	 break;
      case K_OUTPUT:
	 Arg0.dword = D_File;
	 BlkLoc(Arg0) = (union block *) &k_output;
	 break;
      case K_POS:
	 Arg0.dword = D_Tvkywd;
	 BlkLoc(Arg0) = (union block *) &tvky_pos;
	 break;
      case K_RANDOM:
	 Arg0.dword = D_Tvkywd;
	 BlkLoc(Arg0) = (union block *) &tvky_ran;
	 break;
      case K_SOURCE:
	 Arg0 = BlkLoc(current)->coexpr.activator;
	 break;
      case K_SUBJECT:
	 Arg0.dword = D_Tvkywd;
	 BlkLoc(Arg0) = (union block *) &tvky_sub;
	 break;
      case K_TIME:
#ifndef MSDOS
	 times(&tp);
	 runtim =
	   1000 * ((tp.tms_utime - starttime) / (double)Hz);
#else MSDOS
	 runtim = time() - starttime;
#endif MSDOS
	 Mkint(runtim, &Arg0);
	 break;
      case K_TRACE:
	 Arg0.dword = D_Tvkywd;
	 BlkLoc(Arg0) = (union block *) &tvky_trc;
	 break;
      case K_UCASE:
	 Arg0.dword = D_Cset;
	 BlkLoc(Arg0) = (union block *) &k_ucase;
	 break;
      case K_VERSION:
	 strreq(i = strlen(VERSION));
	 StrLen(Arg0) = i;
	 StrLoc(Arg0) = VERSION;
	 break;
      default:
	 syserr("keyword: unknown keyword type.");
      }
   Return;
   }


/*
 * limit - explicit limitation initialization.
 */

LibDcl(limit,0,"limit")
   {
   long l;

   /*
    * The limit is both passed and returned in Arg0.  The limit must
    *  be an integer.  If the limit is 0, the expression being evaluated
    *  fails.  If the limit is < 0, it is an error.  Note that the
    *  result produced by limit is ultimately picked up by the lsusp
    *  function.
    */
   DeRef(Arg0);
   switch (cvint(&Arg0, &l)) {

      case T_Integer:
         Mkint(l, &Arg0);
         break;

      case T_Longint:
         runerr(205, &Arg0);

      default:
         runerr(101, &Arg0);
      }

   if (l < 0)
      runerr(205, &Arg0);
   if (l == 0)
      Fail;
   Return;
   }


/*
 * [ ... ] - create an explicitly specified list.
 */

LibDcl(llist,-1,"[...]")
   {
   register word i;
   register struct b_list *hp;
   register struct b_lelem *bp;
   extern struct b_list *alclist();
   extern struct b_lelem *alclstb();
   word nelem;

   /*
    * Round the number of elements in the list (as indicated by nargs)
    *  up to MinListSlots and ensure space for the list.
    */
   nelem = nargs;
   if (nelem < MinListSlots)
      nelem = MinListSlots;
   blkreq((word)sizeof(struct b_list) + sizeof(struct b_lelem) +
	 nelem * sizeof(struct descrip));

   /*
    * Allocate the list and a list block.
    */
   hp = alclist((word)nargs);
   bp = alclstb(nelem, (word)0, (word)nargs);

   /*
    * Make the list block just allocated into the first and last blocks
    *  for the list.
    */
   hp->listhead.dword = hp->listtail.dword = D_Lelem;
   BlkLoc(hp->listhead) = BlkLoc(hp->listtail) = (union block *) bp;
   /*
    * Dereference each argument in turn and assign it to a list element.
    */
   for (i = 1; i <= nargs; i++) {
      DeRef(Arg(i));
      bp->lslots[i-1] = Arg(i);
      }
   /*
    * Point Arg0 at the new list and return it.
    */
   ArgType(0) = D_List;
   Arg(0).vword.bptr = (union block *)hp;
   Return;
   }
