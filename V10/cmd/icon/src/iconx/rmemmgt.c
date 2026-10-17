/*
 * File: rmemmgt.c
 *  Contents: allocation routines, block description arrays, dump routines,
 *  garbage collaction, sweep, malloc/free
 */

#include "../h/rt.h"
#include "gc.h"

#ifdef IconAlloc
/*
 *  If IconAlloc is defined the system allocation routines are not overloaded.
 *  The names are changed so that Icon's allocation routines are independently
 *  used.  This works as long as no other system calls cause the break value
 *  to change.
 */
#define malloc mem_alloc
#define free mem_free
#define realloc mem_realloc
#define calloc mem_calloc
#endif IconAlloc

#ifdef RunStats
#ifndef VMS
#include <sys/types.h>
#include <sys/times.h>
#else VMS
#include <types.h>
#include <types.h>
struct tms {
    time_t    tms_utime;        /* user time */
    time_t    tms_stime;        /* system time */
    time_t    tms_cutime;        /* user time, children */
    time_t    tms_cstime;        /* system time, children */
};
#endif VMS
#endif RunStats

/*
 * Note: function calls beginning with "MM" are just empty macros
 * unless MEMMON is defined.
 */

/*
 * allocate - returns pointer to nbytes of free storage in block region.
 */

static union block *allocate(nbytes)
word nbytes;
   {
   register uword fspace, *sloc;

   Inc(al_n_total);
   IncSum(al_bc_btotal,nbytes);
   /*
    * See if there is enough room in the block region.
    */
   fspace = maxblk - blkfree;
   if (fspace < nbytes)
      syserr("block allocation botch");

   /*
    * If monitoring, show the allocation.
    */
   MMAlc(nbytes);

   /*
    * Decrement the free space in the block region by the number of bytes allocated
    *  and return the address of the first byte of the allocated block.
    */
   sloc = (uword *) blkfree;
   blkneed -= nbytes;
   blkfree = blkfree + nbytes;
   return (union block *) (sloc);
   }

/*
 * alclint - allocate a long integer block in the block region.
 */

struct b_int *alclint(val)
long val;
   {
   register struct b_int *blk;
   extern union block *allocate();

   MMType(T_Longint);
   blk = (struct b_int *)allocate((word)sizeof(struct b_int));
   blk->title = T_Longint;
   blk->intval = val;
   return blk;
   }

/*
 * alcreal - allocate a real value in the block region.
 */

struct b_real *alcreal(val)
double val;
   {
   register struct b_real *blk;
   extern union block *allocate();

   Inc(al_n_real);
   MMType(T_Real);
   blk = (struct b_real *) allocate((word)sizeof(struct b_real));
   blk->title = T_Real;
#ifdef Double
/* access real values one word at a time */
   { int *rp, *rq;	
     rp = (word *) &(blk->realval);
     rq = (word *) &val;
     *rp++ = *rq++;
     *rp   = *rq;
   }
#else Double
   blk->realval = val;
#endif Double
   return blk;
   }

/*
 * alccset - allocate a cset in the block region.
 */

/* >alccset */
struct b_cset *alccset(size)
int size;
   {
   register struct b_cset *blk;
   register i;
   extern union block *allocate();

   Inc(al_n_cset);
   MMType(T_Cset);
   blk = (struct b_cset *) allocate((word)sizeof(struct b_cset));
   blk->title = T_Cset;
   blk->size = size;
   /*
    * Zero the bit array.
    */
   for (i = 0; i < CsetSize; i++)
     blk->bits[i] = 0;
   return blk;
   }
/* <alccset */


/*
 * alcfile - allocate a file block in the block region.
 */

struct b_file *alcfile(fd, status, name)
FILE *fd;
int status;
struct descrip *name;
   {
   register struct b_file *blk;
   extern union block *allocate();

   Inc(al_n_file);
   MMType(T_File);
   blk = (struct b_file *) allocate((word)sizeof(struct b_file));
   blk->title = T_File;
   blk->fd = fd;
   blk->status = status;
   blk->fname = *name;
   return blk;
   }

/*
 * alcrecd - allocate record with nflds fields in the block region.
 */

struct b_record *alcrecd(nflds, recptr)
int nflds;
struct descrip *recptr;
   {
   register struct b_record *blk;
   register i, size;
   extern union block *allocate();

   Inc(al_n_recd);
   MMType(T_Record);
   size = Vsizeof(struct b_record) + nflds*sizeof(struct descrip);
   blk = (struct b_record *) allocate((word)size);
   blk->title = T_Record;
   blk->blksize = size;
   blk->recdesc.dword = D_Proc;
   BlkLoc(blk->recdesc) = (union block *)recptr;
   /*
    * Assign &null to each field in the record.
    */
   for (i = 0; i < nflds; i++)
       blk->fields[i] = nulldesc;
   return blk;
   }

/*
 * alclist - allocate a list header block in the block region.
 */

struct b_list *alclist(size)
word size;
   {
   register struct b_list *blk;
   extern union block *allocate();

   Inc(al_n_list);
   MMType(T_List);
   blk = (struct b_list *) allocate((word)sizeof(struct b_list));
   blk->title = T_List;
   blk->size = size;
   blk->listhead = nulldesc;
   return blk;
   }

/*
 * alclstb - allocate a list element block in the block region.
 */

struct b_lelem *alclstb(nelem, first, nused)
word nelem, first, nused;
   {
   register struct b_lelem *blk;
   register word i, size;
   extern union block *allocate();

   Inc(al_n_lstb);
   MMType(T_Lelem);
#ifdef MaxListSize
   if (nelem >= MaxListSize)
      runerr(205, NULL);
#endif MaxListSize
   size = Vsizeof(struct b_lelem)+nelem*sizeof(struct descrip);
   blk = (struct b_lelem *) allocate(size);
   blk->title = T_Lelem;
   blk->blksize = size;
   blk->nelem = nelem;
   blk->first = first;
   blk->nused = nused;
   blk->listprev = nulldesc;
   blk->listnext = nulldesc;
   /*
    * Set all elements to &null.
    */
   for (i = 0; i < nelem; i++)
      blk->lslots[i] = nulldesc;
   return blk;
   }

/*
 * alctable - allocate a table header block in the block region.
 */

struct b_table *alctable(def)
struct descrip *def;
   {
   register int i;
   register struct b_table *blk;
   extern union block *allocate();

   Inc(al_n_table);
   MMType(T_Table);
   blk = (struct b_table *) allocate((word)sizeof(struct b_table));
   blk->title = T_Table;
   blk->size = 0;
   blk->defvalue = *def;
   /*
    * Zero out the buckets.
    */
   for (i = 0; i < TSlots; i++)
      blk->buckets[i] = nulldesc;
   return blk;
   }

/*
 *  alctelem - allocate a table element block in the block region.
 */

struct b_telem *alctelem()
   {
   register struct b_telem *blk;
   extern union block *allocate();

   Inc(al_n_telem);
   MMType(T_Telem);
   blk = (struct b_telem *) allocate((word)sizeof(struct b_telem));
   blk->title = T_Telem;
   blk->hashnum = 0;
   blk->clink = nulldesc;
   blk->tref = nulldesc;
   blk->tval = nulldesc;
   return blk;
   }

/*
 * alcset - allocate a set header block.
 */

struct b_set *alcset()
   {
     register int i;
     register struct b_set *blk;
     extern union block *allocate();

     MMType(T_Set);
     blk = (struct b_set *) allocate((word)sizeof(struct b_set));
     blk->title = T_Set;
     blk->size = 0;
     /*
      *  Zero out the buckets.
      */
     for (i = 0; i < SSlots; i++)
        blk->sbucks[i] = nulldesc;
     return blk;
     }

/* 
 *   alcselem - allocate a set element block.
 */

struct b_selem *alcselem(mbr,hn)
word hn;
struct descrip *mbr;

   { register struct b_selem *blk;
     extern union block *allocate();

     MMType(T_Selem);
     blk = (struct b_selem *) allocate((word)sizeof(struct b_selem));
     blk->title = T_Selem;
     blk->clink = nulldesc;
     blk->setmem = *mbr;
     blk->hashnum = hn;
     return blk;
     }

/*
 * alcsubs - allocate a substring trapped variable in the block region.
 */

struct b_tvsubs *alcsubs(len, pos, var)
word len, pos;
struct descrip *var;
   {
   register struct b_tvsubs *blk;
   extern union block *allocate();

   Inc(al_n_subs);
   MMType(T_Tvsubs);
   blk = (struct b_tvsubs *) allocate((word)sizeof(struct b_tvsubs));
   blk->title = T_Tvsubs;
   blk->sslen = len;
   blk->sspos = pos;
   blk->ssvar = *var;
   return blk;
   }

/*
 * alctvtbl - allocate a table element trapped variable block in the block region.
 */

struct b_tvtbl *alctvtbl(tbl, ref, hashnum)
register struct descrip *tbl, *ref;
word hashnum;
   {
   register struct b_tvtbl *blk;
   extern union block *allocate();

   Inc(al_n_tvtbl);
   MMType(T_Tvtbl);
   blk = (struct b_tvtbl *) allocate((word)sizeof(struct b_tvtbl));
   blk->title = T_Tvtbl;
   blk->hashnum = hashnum;
   blk->clink = *tbl;
   blk->tref = *ref;
   blk->tval = nulldesc;
   return blk;
   }

/*
 * alcstr - allocate a string in the string space.
 */

/* >alcstr */
char *alcstr(s, slen)
register char *s;
register word slen;
   {
   register char *d;
   char *ofree;

   Inc(al_n_str);
   IncSum(al_bc_stotal,slen);
   MMStr(slen);
   /*
    * See if there is enough room in the string space.
    */
   if (strfree + slen > strend)
      syserr("string allocation botch");
   strneed -= slen;

   /*
    * Copy the string into the string space, saving a pointer to its
    *  beginning.  Note that s may be null, in which case the space
    *  is still to be allocated but nothing is to be copied into it.
    */
   ofree = d = strfree;
   if (s != NULL) {
      while (slen-- > 0)
         *d++ = *s++;
      }
   else
      d += slen;
   strfree = d;
   return ofree;
   }
/* <alcstr */

/*
 * alcstk - allocate a co-expression stack block.
 */

struct b_coexpr *alcstk()
   {
   struct b_coexpr *ep;
   char *malloc();

   Inc(al_n_estk);
   ep = (struct b_coexpr *)malloc(stksize);
   ep->title = T_Coexpr;
   return ep;
   }

/*
 * alceblk - allocate a co-expression block.
 */

struct b_refresh *alceblk(entryx, na, nl)
word *entryx;
int na, nl;
   {
   int size;
   struct b_refresh *blk;
   extern union block *allocate();

   Inc(al_n_eblk);
   MMType(T_Refresh);
   size = Vsizeof(struct b_refresh) + (na + nl) * sizeof(struct descrip);
   blk = (struct b_refresh *) allocate((word)size);
   blk->title = T_Refresh;
   blk->blksize = size;
   blk->ep = entryx;
   blk->numlocals = nl;
   return blk;
   }


/*
 * Allocated block size table (sizes given in bytes).  A size of -1 is used
 *  for types that have no blocks; a size of 0 indicates that the
 *  second word of the block contains the size; a value greater than
 *  0 is used for types with constant sized blocks.
 */

int bsizes[] = {
    -1,				/* 0, not used */
    -1,				/* 1, not used */
     sizeof(struct b_int),	/* T_Longint (2), long integer type */
     sizeof(struct b_real),	/* T_Real (3), real number */
     sizeof(struct b_cset),	/* T_Cset (4), cset */
     sizeof(struct b_file),	/* T_File (5), file block */
     0,				/* T_Proc (6), procedure block */
     sizeof(struct b_list),	/* T_List (7), list header block */
     sizeof(struct b_table),	/* T_Table (8), table header block */
     0,				/* T_Record (9), record block */
     sizeof(struct b_telem),	/* T_Telem (10), table element block */
     0,				/* T_Lelem (11), list element block */
     sizeof(struct b_tvsubs),	/* T_Tvsubs (12), substring trapped variable */
    -1,				/* T_Tvkywd (13), keyword trapped variable */
     sizeof(struct b_tvtbl),	/* T_Tvtbl (14), table element trapped variable */
     sizeof(struct b_set),	/* T_Set  (15), set header block */
     sizeof(struct b_selem),	/* T_Selem  (16), set element block */
     0,				/* T_Refresh (17), expression block */
    -1,				/* T_Coexpr (18), expression stack header */
    };

/*
 * Table of offsets (in bytes) to first descriptor in blocks.  -1 is for
 *  types not allocated, 0 for blocks with no descriptors.
 */
int firstd[] = {
    -1,				/* 0, not used */
    -1,				/* 1, not used */
     0,				/* T_Longint (2), long integer type */
     0,				/* T_Real (3), real number */
     0,				/* T_Cset (4), cset */
     3*WordSize,		/* T_File (5), file block */
     8*WordSize,		/* T_Proc (6), procedure block */
     2*WordSize,		/* T_List (7), list header block */
     2*WordSize,		/* T_Table (8), table header block */
     2*WordSize,		/* T_Record (9), record block */
     2*WordSize,		/* T_Telem (10), table element block */
     5*WordSize,		/* T_Lelem (11), list element block */
     3*WordSize,		/* T_Tvsubs (12), substring trapped variable */
    -1,				/* T_Tvkywd (13), keyword trapped variable */
     2*WordSize,		/* T_Tvtbl (14), table element trapped variable */
     2*WordSize,		/* T_Set  (15), set header block */
     2*WordSize,		/* T_Selem  (16), set element block */
     (4+Vwsizeof(struct pf_marker))*WordSize,
				/* T_Refresh (17), expression block */
    -1,				/* T_Coexpr (18), expression stack header */
    };

/*
 * Table of block names used by debugging functions.
 */
char *blkname[] = {
   "illegal",				/* T_Null (0), not block */
   "illegal",		 		/* T_Integer (1), not block */
   "long integer",			/* T_Longint (2) */
   "real number",			/* T_Real (3) */
   "cset",				/* T_Cset (4) */
   "file",				/* T_File (5) */
   "procedure",				/* T_Proc (6) */
   "list",				/* T_List (7) */
   "table",				/* T_Table (8) */
   "record",				/* T_Record (9) */
   "table element",			/* T_Telem (10) */
   "list element",			/* T_Lelem (11) */
   "substring trapped variable",	/* T_Tvsubs (12) */
   "keyword trapped variable",		/* T_Tvkywd (13) */
   "table element trapped variable",	/* T_Tvtbl (14) */
   "set",				/* T_Set (15) */
   "set elememt",			/* T_Selem (16) */
   "refresh",				/* T_Refresh (17) */
   "co-expression",		 	/* T_Coexpr (18) */
   };


/*
 * descr - dump a descriptor.  Used only for debugging.
 */

descr(dp)
struct descrip *dp;
   {
   int i;

   fprintf(stderr,"%08lx: ",(long)dp);
   if (Qual(*dp))
      fprintf(stderr,"%15s","qualifier");
   else if (Var(*dp) && !Tvar(*dp))
      fprintf(stderr,"%15s","variable");
   else {
      i =  Type(*dp);
      switch (i) {
         case T_Null:
            fprintf(stderr,"%15s","null");
            break;
         case T_Integer:
            fprintf(stderr,"%15s","integer");
            break;
         default:
            fprintf(stderr,"%15s",blkname[i]);
         }
      }
   fprintf(stderr," %08lx %08lx\n",(long)dp->dword,(long)dp->vword.integr);
   }

/*
 * blkdump - dump the allocated block region.  Used only for debugging.
 */

blkdump()
   {
   register char *blk;
   register word type, size, fdesc;
   register struct descrip *ndesc;

   fprintf(stderr,"\nDump of allocated block region.  base:%08lx free:%08lx max:%08lx\n",
      (long)blkbase,(long)blkfree,(long)maxblk);
   fprintf(stderr,"  loc     type              size  contents\n");

   for (blk = blkbase; blk < blkfree; blk += BlkSize(blk)) {
      type = BlkType(blk);
      size = BlkSize(blk);
      fprintf(stderr," %08lx   %15s   %4ld\n",(long)blk,blkname[type],(long)size);
      if ((fdesc = firstd[type]) > 0)
         for (ndesc = (struct descrip *) (blk + fdesc);
               ndesc < (struct descrip *) (blk + size);ndesc++)  {
            fprintf(stderr,"                                 ");
            descr(ndesc);
            }
      fprintf(stderr,"\n");
      }
   fprintf(stderr,"end of block region.\n");
   }


/*
 * blkreq - insure that at least bytes of space are left in the block region.
 *  The amount of space needed is transmitted to the collector via
 *  the global variable blkneed.
 */

blkreq(bytes)
uword bytes;
   {
   blkneed = bytes;
   if (bytes > maxblk - blkfree) {
      Inc(gc_n_blk);
      collect();
      }
   }

/*
 * strreq - insure that at least n of space are left in the string
 *  space.  The amount of space needed is transmitted to the collector
 *  via the global variable strneed.
 */

/* >strreq */
strreq(n)
uword n;
   {
   strneed = n;			/* save in case of collection */
   if (n > strend - strfree) {
      Inc(gc_n_string);
      collect();
      }
   }
/* <strreq */

/*
 * cofree - collect co-expression blocks.  This is done after
 *  the marking phase of garbage collection and the stacks that are
 *  reachable have pointers to data blocks, rather than T_Coexpr,
 *  in their type field.
 */

/* >cofree */
cofree()
   {
   register struct b_coexpr **ep, *xep;

   /*
    * Reset the type for &main.
    */
   BlkLoc(k_main)->coexpr.title = T_Coexpr;

   /*
    * The co-expression blocks are linked together through their
    *  nextstk fields, with stklist pointing to the head of the list.
    *  The list is traversed and each stack that was not marked
    *  is freed.
    */
   ep = &stklist;
   while (*ep != NULL) {
      if (BlkType(*ep) == T_Coexpr) {
         xep = *ep;
         *ep = (*ep)->nextstk;
         free((char *)xep);
         }
      else {
         BlkType(*ep) = T_Coexpr;
         ep = &(*ep)->nextstk;
         }
      }
   }
/* <cofree */

/*
 * collect - do a garbage collection.
 *  the static region is needed.
 */

collect()
   {
   register word extra;
   register char *newend;
   register struct descrip *dp;
   char *strptr;
   struct b_coexpr *cp;
#ifndef MSDOS
   extern char *brk();
#endif MSDOS
   extern char *sbrk();
#ifdef RunStats
   struct tms tmbuf;

   times(&tmbuf);
   gc_t_start = tmbuf.tms_utime + tmbuf.tms_stime;
   Inc(gc_n_total);
#endif RunStats
   MMBGC();

   /*
    * Sync the values (used by sweep) in the coexpr block for &current
    *  with the current values.
    */
   cp = (struct b_coexpr *)BlkLoc(current);
   cp->es_pfp = pfp;
   cp->es_gfp = gfp;
   cp->es_efp = efp;
   cp->es_sp = sp;

   /*
    * Reset the string qualifier free list pointer.
    */
   qualfree = quallist;

   /*
    * Mark the stacks for &main and the current co-expression.
    */
   markblock(&k_main);
   markblock(&current);
   /*
    * Mark &subject and the cached s2 and s3 strings for map.
    */
   postqual(&k_subject);
   if (Qual(maps2))			/*  caution:  the cached arguments of */
      postqual(&maps2);			/*  map may not be strings. */
   else if (Pointer(maps2))
      markblock(&maps2);
   if (Qual(maps3))
      postqual(&maps3);
   else if (Pointer(maps3))
      markblock(&maps3);
   /*
    * Mark the tended descriptors and the global and static variables.
    */
   for (dp = &tended[1]; dp <= &tended[ntended]; dp++)
/* >locate */
      if (Qual(*dp))
         postqual(dp);
      else if (Pointer(*dp))
         markblock(dp);
/* <locate */
   for (dp = globals; dp < eglobals; dp++)
      if (Qual(*dp))
         postqual(dp);
      else if (Pointer(*dp))
         markblock(dp);
   for (dp = statics; dp < estatics; dp++)
      if (Qual(*dp))
         postqual(dp);
      else if (Pointer(*dp))
         markblock(dp);

   /*
    * Collect available co-expression stacks.
    */
   cofree();
   if (statneed) {
      /*
       * The static region needs to be expanded. Make sure the end has not
       * been changed by someone else.
       */
      if (currend != sbrk(0))
         runerr(304, NULL);
      extra = statneed;
      newend = (char *) quallist + extra;
      /*
       * This next calculation determines if there is space for the requested
       *  static region expansion.  The checks involving quallist and newend
       *  appear to only be required on machines where the above addition
       *  of extra might overflow.
       */

      if (newend < (char *)quallist || newend > (char *)0x7fffffff ||
         (newend > (char *)equallist && ((word) brk(newend) == (word)-1)))
            runerr(303, NULL);
      statend += statneed;
      statneed = 0;
      currend = sbrk(0);
      }
   else
      /*
       * Expansion of the static memory region is not required.
       */
      extra = 0;

   /*
    * Collect the string space, indicating that it must be moved back
    *  extra bytes.
    */
   scollect(extra);
   /*
    * strptr is post-gc value for strings.  Move back pointers for strend
    *  and quallist according to value of extra.
    */
   strptr = strbase + extra;
   strend += extra;
   quallist = (struct descrip **)((char *)quallist + extra);
   if (quallist > equallist)
      equallist = quallist;

   /*
    * Calculate a value for extra space.  The value is (the larger of
    *  (twice the string space needed) or (the number of words currently
    *  in the string space)) plus the unallocated string space.
    */
   extra = (Max(2*strneed, (strend-(char *)statend)/4) -
            (strend - extra - strfree) + (GranSize-1)) & ~(GranSize-1);

   while (extra > 0) {
      /*
       * Try to get extra more bytes of storage.  If it can't be gotten,
       *  decrease the value by GranSize and try again.  If it's gotten,
       *  move back strend and quallist.  First make sure that someone
       *  has not moved the end of the region.
       */
      if (currend != sbrk(0))
         runerr(304, NULL);
      newend = (char *)quallist + extra;
      if (newend >= (char *)quallist &&
          (newend <= (char *)equallist || ((int) brk(newend) != -1))) {
         strend += extra;
         quallist = (struct descrip **) newend;
         currend = sbrk(0);
         break;
         }
      extra -= GranSize;
      }

   /*
    * Adjust the pointers in the block region.  Note that blkbase is the old base
    *  of the block region and strend will be the post-gc base of the block region.
    */
   adjust(blkbase,strend);
   /*
    * Compact the block region.
    */
   compact(blkbase);
   /*
    * Calculate a value for extra space.  The value is (the larger of
    *  (twice the block region space needed) or (the number of words currently
    *  in the block region space)) plus the unallocated block space.
    */
   extra = (Max(2*blkneed, (maxblk-blkbase)/4) +
            blkfree - maxblk + (GranSize-1)) & ~(GranSize-1);
   while (extra > 0) {
      /*
       * Try to get extra more bytes of storage.  If it can't be gotten,
       *  decrease the value by GranSize and try again.  If it's gotten,
       *  move back quallist.  First make sure the end has not been changed.
       */
      if (currend != sbrk(0))
         runerr(304, NULL);
      newend = (char *)quallist + extra;
      if (newend >= (char *)quallist &&
          (newend <= (char *)equallist || ((int) brk(newend) != -1))) {
         quallist = (struct descrip **) newend;
         currend = sbrk(0);
         break;
         }
      extra -= GranSize;
      }
   if (quallist > equallist)
      equallist = quallist;

   if (strend != blkbase) {
      /*
       * strend is not equal to blkbase and this indicates that the
       *  static region or string region was expanded and thus
       *  the block region must be moved.  There is an assumption here that the
       *  block region always moves up in memory, i.e., the static and
       *  string regions never shrink.  With this assumption in hand,
       *  the block region must be moved before the string space lest the string
       *  space overwrite block data.  The assumption is valid, but beware
       *  if shrinking regions are ever implemented.
       */
      mvc((uword)(blkfree - blkbase), blkbase, strend);
      blkfree += strend - blkbase;
      blkbase = strend;
      }
   if (strptr != strbase) {
      /*
       * strptr is not equal to strbase and this indicates that the
       *  co-expression space was expanded and thus the string space
       *  must be moved up in memory.
       */
      mvc((uword)(strfree - strbase), strbase, strptr);
      strfree += strptr - strbase;
      strbase = strptr;
      }
      
   /*
    * Expand the block region.
    */
   maxblk = (char *)quallist;
#ifdef RunStats
   times(&tmbuf);
   gc_t_last =
   	1000*(((tmbuf.tms_utime + tmbuf.tms_stime)-gc_t_start)/(double)Hz);
   IncSum(gc_t_total,gc_t_last);
#endif RunStats
   MMEGC();
   return;
   }
/*
 * markblock- mark each accessible block in the block region and build back-list of
 *  descriptors pointing to that block. (Phase I of garbage collection.)
 */

/* >markblock */
markblock(dp)
struct descrip *dp;
   {
   register struct descrip *dp1;
   register char *endblock, *block;
   static word type, fdesc, off;

   /*
    * Get the block to which dp points.
    */

   block = (char *) BlkLoc(*dp);
   if (block >= blkbase && block < blkfree) {	/* check range */
      if (Var(*dp) && !Tvar(*dp)) {

         /*
          * The descriptor is a variable; point block to the head of the
          *  block containing the descriptor to which dp points.
          */
         off = Offset(*dp);
         if (off == 0)
            return;
         else
            block = (char *) ((word *) block - off);
            }

         type = BlkType(block);
         if ((uword)type <= MaxType)  {

            /*
             * The type is valid, which indicates that this block has not
             *  been marked.  Point endblock to the byte past the end
             *  of the block.
             */
            endblock = block + BlkSize(block);
            MMMark(block,type);
            }

         /*
          * Add dp to the back-chain for the block and point the
          *  block (via the type field) to dp.
          */
         BlkLoc(*dp) = (union block *) type;
         BlkType(block) = (word)dp;
         if (((unsigned)type <=  MaxType) && ((fdesc = firstd[type]) > 0))

            /*
             * The block has not been marked, and it does contain
             *  descriptors. Mark each descriptor.
             */
            for (dp1 = (struct descrip *) (block + fdesc);
               (char *) dp1 < endblock; dp1++) {
               if (Qual(*dp1))
                  postqual(dp1);
               else if (Pointer(*dp1))
                  markblock(dp1);
            }
         }
      else if (dp->dword == D_Coexpr &&
         (unsigned)BlkType(block) <= MaxType) {

         /*
          * dp points to a co-expression block that has not been
          *  marked.  Point the block to dp.  Sweep the interpreter
          *  stack in the block and mark the block for the
          *  activating co-expression and the refresh block.
          */
         BlkType(block) = (word)dp;
         sweep((struct b_coexpr *)block);
         markblock(&((struct b_coexpr *)block)->activator);
         markblock(&((struct b_coexpr *)block)->freshblk);
         }
   }
/* <markblock */

/*
 * adjust - adjust pointers into the block region, beginning with block oblk and
 *  basing the "new" block region at nblk.  (Phase II of garbage collection.)
 */

/* >adjust */
adjust(source,dest)
char *source, *dest;
   {
   register struct descrip *nxtptr, *tptr;

   /*
    * Loop through to the end of allocated block region, moving source
    *  to each block in turn and using the size of a block to find the
    *  next block.
    */
   while (source < blkfree) {
      if ((uword) (nxtptr = (struct descrip *)BlkType(source)) > MaxType) {

         /*
          * The type field of source is a back-pointer.  Traverse the
          *  chain of back pointers, changing each block location from
          *  source to dest.
          */
         while ((uword)nxtptr > MaxType) {
            tptr = nxtptr;
            nxtptr = (struct descrip *)BlkLoc(*nxtptr);
            if (Var(*tptr) && !Tvar(*tptr))
               BlkLoc(*tptr) = (union block *)((word *)dest + Offset(*tptr));
            else
               BlkLoc(*tptr) = (union block *)dest;
            }
         BlkType(source) = (uword)nxtptr | F_Mark;
         dest += BlkSize(source);
         }
      source += BlkSize(source);
      }
   }
/* <adjust */

/*
 * compact - compact good blocks in the block region. (Phase III of garbage collection.)
 */

/* >compact */
compact(source)
char *source;
   {
   register char *dest;
   register word size;

   /*
    * Start dest at source.
    */
   dest = source;

   /*
    * Loop through to end of allocated block space moving source to
    *  each block in turn, using the size of a block to find the next
    *  block.  If a block has been marked, it is copied to the
    *  location pointed to by dest and dest is pointed past the end
    *  of the block, which is the location to place the next saved
    *  block.  Marks are removed from the saved blocks.
    */
   while (source < blkfree) {
      size = BlkSize(source);
      if (BlkType(source) & F_Mark) {
         BlkType(source) &= ~F_Mark;
         if (source != dest)
            mvc((uword)size,source,dest);
         dest += size;
         }
      source += size;
      }

   /*
    * dest is the location of the next free block.  Now that compaction
    *  is complete, point blkfree to that location.
    */
   blkfree = dest;
   }
/* <compact */

/*
 * postqual - mark a string qualifier.  Strings outside the string space
 *  are ignored.
 */

/* >postqual */
postqual(dp)
struct descrip *dp;
   {
#ifndef MSDOS
   extern char *brk();
#endif MSDOS
   extern char *sbrk();

   if (StrLoc(*dp) >= strbase && StrLoc(*dp) < strend) {
      /*
       * The string is in the string space.  Add it to the string qualifier
       *  list.  But before adding it, expand the string qualifier list
       *  if necessary.
       */
      if (qualfree >= equallist) {
         equallist += Sqlinc;
         if (currend != sbrk(0))	/* make sure region has not changed */
            runerr(304, NULL);
         if ((int) brk(equallist) == -1)	/* make sure region can be expanded */
            runerr(303, NULL);
         currend = sbrk(0);
         }
      *qualfree++ = dp;
      }
   }
/* <postqual */

/*
 * scollect - collect the string space.  quallist is a list of pointers to
 *  descriptors for all the reachable strings in the string space.  For
 *  ease of description, it is referred to as if it were composed of
 *  descriptors rather than pointers to them.
 */

/* >scollect */
scollect(extra)
word extra;
   {
   register char *source, *dest;
   register struct descrip **qptr;
   char *cend;
   extern int qlcmp();

   if (qualfree <= quallist) {
      /*
       * There are no accessible strings.  Thus, there are none to
       *  collect and the whole string space is free.
       */
      strfree = strbase;
      return;
      }
   /*
    * Sort the pointers on quallist in ascending order of string locations.
    */
   qsort(quallist, qualfree-quallist, sizeof(struct descrip *), qlcmp);
   /*
    * The string qualifiers are now ordered by starting location.
    */
   dest = strbase;
   source = cend = StrLoc(**quallist);

   /*
    * Loop through qualifiers for accessible strings.
    */
   for (qptr = quallist; qptr < qualfree; qptr++) {
      if (StrLoc(**qptr) > cend) {

         /*
          * qptr points to a qualifier for a string in the next clump;
          *  the last clump is moved and source and cend are set for
          *  the next clump.
          */
         MMSMark(source,cend - source);
         while (source < cend)
            *dest++ = *source++;
         source = cend = StrLoc(**qptr);
         }
      if (StrLoc(**qptr)+StrLen(**qptr) > cend)
         /*
          * qptr is a qualifier for a string in this clump; extend the clump.
          */
         cend = StrLoc(**qptr) + StrLen(**qptr);
      /*
       * Relocate the string qualifier.
       */
      StrLoc(**qptr) += dest - source + extra;
      }

   /*
    * Move the last clump.
    */
   MMSMark(source,cend - source);
   while (source < cend)
      *dest++ = *source++;
   strfree = dest;
   }
/* <scollect */

/*
 * qlcmp - compare the location fields of two string qualifiers for qsort.
 */

/* >qlcmp */
qlcmp(q1,q2)
struct descrip **q1, **q2;
   {
   return (int)(StrLoc(**q1) - StrLoc(**q2));
   }
/* <qlcmp */

/*
 * mvc - move n bytes from src to dst.
 */

mvc(n, s, d)
uword n;
register char *s, *d;
   {
   register int words;
   register int *srcw, *dstw;
   int bytes;

   words = n / sizeof(int);
   bytes = n % sizeof(int);

   srcw = (int *)s;
   dstw = (int *)d;

   if (d < s) {
      /*
       * The move is from higher memory to lower memory.  (It so happens
       *  that leftover bytes are not moved.)
       */
      while (--words >= 0)
         *(dstw)++ = *(srcw)++;
      while (--bytes >= 0)
         *d++ = *s++;
      }
   else if (d > s) {
      /*
       * The move is from lower memory to higher memory.
       */
      s += n;
      d += n;
      while (--bytes >= 0)
         *--d = *--s;
      srcw = (int *)s;
      dstw = (int *)d;
      while (--words >= 0)
         *--dstw = *--srcw;
      }
   }

/*
 * sweep - sweep the stack, marking all descriptors there.  Method
 *  is to start at a known point, specifically, the frame that the
 *  fp points to, and then trace back along the stack looking for
 *  descriptors and local variables, marking them when they are found.
 *  The sp starts at the first frame, and then is moved down through
 *  the stack.  Procedure, generator, and expression frames are
 *  recognized when the sp is a certain distance from the fp, gfp,
 *  and efp respectively.
 *
 * Sweeping problems can be manifested in a variety of ways due to
 *  the "if it can't be identified it's a descriptor" methodology.
 */
sweep(ce)
struct b_coexpr *ce;
   {
   register word *s_sp;
   register struct pf_marker *fp;
   register struct gf_marker *s_gfp;
   register struct ef_marker *s_efp;
   word nargs, type, gsize;

   fp = ce->es_pfp;
   s_gfp = ce->es_gfp;
   if (s_gfp != 0) {
      type = s_gfp->gf_gentype;
      if (type == G_Psusp)
         gsize = Wsizeof(*s_gfp);
      else
         gsize = Wsizeof(struct gf_smallmarker);
      }
   s_efp = ce->es_efp;
   s_sp =  ce->es_sp;
   nargs = 0;				/* Nargs counter is 0 initially. */

   while ((fp != 0 || nargs)) {		/* Keep going until current fp is
					    0 and no arguments are left. */
      if (s_sp == (word *)fp + Vwsizeof(*pfp) - 1) {/*The sp has reached the upper
					    boundary of a procedure frame,
					    process the frame. */
         s_efp = fp->pf_efp;		/* Get saved efp out of frame */
         s_gfp = fp->pf_gfp;		/* Get save gfp */
         if (s_gfp != 0) {
            type = s_gfp->gf_gentype;
            if (type == G_Psusp)
               gsize = Wsizeof(*s_gfp);
            else
               gsize = Wsizeof(struct gf_smallmarker);
            }
         s_sp = (word *)fp - 1;		/* First argument descriptor is
					    first word above proc frame */
         nargs = fp->pf_nargs;
         fp = fp->pf_pfp;
         }
      else if (s_sp == (word *)s_gfp + gsize - 1) {
					/* The sp has reached the lower end
					    of a generator frame, process
					    the frame.*/
         if (type == G_Psusp)
            fp = s_gfp->gf_pfp;
         s_sp = (word *)s_gfp - 1;
         s_efp = s_gfp->gf_efp;
         s_gfp = s_gfp->gf_gfp;
            if (s_gfp != 0) {
            type = s_gfp->gf_gentype;
            if (type == G_Psusp)
               gsize = Wsizeof(*s_gfp);
            else
               gsize = Wsizeof(struct gf_smallmarker);
            }
         nargs = 1;
         }
      else if (s_sp == (word *)s_efp + Wsizeof(*s_efp) - 1) {
					    /* The sp has reached the upper
      						end of an expression frame,
						process the frame. */
         s_gfp = s_efp->ef_gfp;		/* Restore gfp, */
         if (s_gfp != 0) {
            type = s_gfp->gf_gentype;
            if (type == G_Psusp)
               gsize = Wsizeof(*s_gfp);
            else
               gsize = Wsizeof(struct gf_smallmarker);
            }
         s_efp = s_efp->ef_efp;		/*  and efp from frame. */
         s_sp -= Wsizeof(*s_efp);		/* Move down past expression frame
					    marker. */
         }
      else {				/* Assume the sp is pointing at a
					    descriptor. */
         if (Qual(*((struct descrip *)(&s_sp[-1]))))
            postqual(&s_sp[-1]);
         else if (Pointer(*((struct descrip *)(&s_sp[-1]))))
            markblock(&s_sp[-1]);
         s_sp -= 2;			/* Move past descriptor. */
         if (nargs)			/* Decrement argument count if in an*/
            nargs--;			/*  argument list. */
         }
      }
   }

typedef int ALIGN;		/* pick most stringent type for alignment */

union bhead {			/* header of free block */
   struct {
      union bhead *ptr;		/* pointer to next free block */
      uword bsize;		/* free block size */
      } s;
      ALIGN x;			/* force block alignment */
   };

typedef union bhead HEADER;
#define NALLOC 1024		/* units to request at one time */


static HEADER base;		/* start with empty list */
static HEADER *allocp = NULL;	/* last allocated block */

char *malloc(nbytes)
unsigned nbytes;
   {
   HEADER *moremem();
   register HEADER *p, *q;
   register word nunits;
   int attempts;

   nunits = 1 + (nbytes + sizeof(HEADER) - 1) / sizeof(HEADER);
   if ((q = allocp) == NULL) {	/* no free list yet */
      base.s.ptr = allocp = q = &base;
      base.s.bsize = 0;
      }

   for (attempts = 2; attempts--; q = allocp) {
      for (p = q->s.ptr;; q = p, p = p->s.ptr) {
         if (p->s.bsize >= nunits) {	/* block is big enough */
            if (p->s.bsize == nunits)	/* exactly right */
               q->s.ptr = p->s.ptr;
            else {			/* allocate tail end */
               p->s.bsize -= nunits;
               p += p->s.bsize;
               p->s.bsize = nunits;
               }
            allocp = q;
            return (char *)(p + 1);
            }
         if (p == allocp) {	/* wrap around */
            moremem(nunits);	/* garbage collect and expand if needed */
            break;
            }
         }
      }
      syserr("cannot allocate requested storage");
   }

/*
 * realloc() allocates a new block of memory of a different size
 * that contains the contents of the current block or as much as
 * will fit.
 */

char *realloc(curmem,newsiz)
register char *curmem;		/* the current memory pointer */
register int newsiz;	/* the size of the new allocation */
   {
   register char *newmem, *p;	/* the new memory pointer */
   register int cursiz;	/* the size of the current allocation */
   register int n;

   HEADER *head;		/* the pointer to the current header */

   if ((newmem = malloc(newsiz)) != NULL) {
				/* get the current allocation size */
      head = (HEADER *) (curmem-1);
      cursiz = head->s.bsize;
      p = newmem;
      n = (cursiz < newsiz ? cursiz : newsiz);
      while (--n >= 0)
         *newmem++ = *curmem++;

				/* free the current block */
      free(curmem);
      return(p);
      }
   syserr("malloc failed in realloc");
   }

/*
 * calloc() allocates memory using malloc and zeroes it.
 */

char *calloc(ecnt,esiz)
register int ecnt, esiz;
   {
   register char *mem, *p;		/* the memory pointer */
   register int amount;			/* the amount of memory needed */

   amount = ecnt * esiz;

   if ((mem = malloc(amount)) != NULL) {
      p = mem;
      while (--amount >= 0)
       	  *mem++ = 0;
      return p;
      }
   syserr("malloc failure in calloc");
   }

static HEADER *moremem(nunits)
uword nunits;
   {
   register char *cp;
   register HEADER *up;
   register word rnu;
   word n;

   rnu = NALLOC * ((nunits + NALLOC - 1) / NALLOC);
   n = rnu * sizeof(HEADER);
   if (statfree + n > statend) {
      statneed = ((n / statincr) + 1) * statincr;
      collect();
      }
   if (statfree < statend) {	/* that is, if there is any room left */
      up = (HEADER *) statfree;
      up->s.bsize = (statend - statfree) / sizeof(HEADER);
      statfree = statend;
      free((char *) (up + 1));	/* add block to free memory */
      }
   }

free(ap)		/* return block pointed to by ap to free list */
char *ap;
   {
   register HEADER *p, *q;

   p = (HEADER *)ap - 1;	/* point to header */
   if (p->s.bsize * sizeof(HEADER) >= statneed)
     statneed = 0;
   for (q = allocp; !(p > q && p < q->s.ptr); q = q->s.ptr)
      if (q >= q->s.ptr && (p > q || p < q->s.ptr))
         break;			/* at one end or the other */
   if (p + p->s.bsize == q->s.ptr) {	/* join to upper */
      p->s.bsize += q->s.ptr->s.bsize;
      if (p->s.bsize * sizeof(HEADER) >= statneed)
	statneed = 0;
      p->s.ptr = q->s.ptr->s.ptr;
      }
   else
      p->s.ptr = q->s.ptr;
   if (q + q->s.bsize == p) {	/* join to lower */
      q->s.bsize += p->s.bsize;
      if (q->s.bsize * sizeof(HEADER) >= statneed)
	statneed = 0;
      q->s.ptr = p->s.ptr;
      }
   else
      q->s.ptr = p;
   allocp = q;
   }
