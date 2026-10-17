/*
 * Routines for symbol table manipulation.
 */

#include "ilink.h"

int dynoff;			/* stack offset counter for locals */
int argoff;			/* stack offset counter for arguments */
int static1;			/* first static in procedure */
int statics = 0;		/* static variable counter */

int nlocal;			/* number of locals in local table */
int nconst;			/* number of constants in constant table */
int nfields = 0;		/* number of fields in field table */

/*
 * instalid - copy the string s to the start of the string free space
 *  and call putident with the length of the string.
 */
char *instalid(s)
char *s;
   {
   register int l;
   register char *p1, *p2;
   extern char *putident();

   p1 = strfree;
   p2 = s;
   l = 1;
   while (*p1++ = *p2++)
      l++;
   return putident(l);
   }

/*
 * putident - install the identifier named by the string starting at strfree
 *  and extending for len bytes.  The installation entails making an
 *  entry in the identifier hash table and then making an identifier
 *  table entry for it with alcident.  A side effect of installation
 *  is the incrementing of strfree by the length of the string, thus
 *  "saving" it.
 *
 * Nothing is changed if the identifier has already been installed.
 */
char *putident(len)
int len;
   {
   register int hash;
   register char *s;
   register struct ientry *ip;
   int l;
   extern struct ientry *alcident();

   /*
    * Compute hash value by adding bytes and masking result with imask.
    *  (Recall that imask is ihsize-1.)
    */
   s = strfree;
   hash = 0;
   l = len;
   while (l--)
      hash += *s++;
   l = len;
   s = strfree;
   hash &= imask;
   /*
    * If the identifier hasn't been installed, install it.
    */
   if ((ip = ihash[hash]) != NULL) {	 /* collision */
      for (;;) { /* work down i_blink chain until id is found or the
                     end of the chain is reached */
         if (l == ip->i_length && lexeq(l, s, ip->i_name))
            return (ip->i_name); /* id is already installed, return it */
         if (ip->i_blink == NULL) { /* end of chain */
            ip->i_blink = alcident(NULL, s, l);
            strfree += l;
            return s;
            }
         ip = ip->i_blink;
         }
      }
   /*
    * Hashed to an empty slot.
    */
   ihash[hash] = alcident(NULL, s, l);
   strfree += l;
   return s;
   }

/*
 * lexeq - compare two strings of given length.  Returns non-zero if
 *  equal, zero if not equal.
 */
lexeq(l, s1, s2)
register int l;
register char *s1, *s2;
   {
   while (l--)
      if (*s1++ != *s2++)
         return 0;
   return 1;
   }

/*
 * alcident - get the next free identifier table entry, and fill it in with
 *  the specified values.
 */
struct ientry *alcident(blink, nam, len)
struct ientry *blink;
char *nam;
int len;
   {
   register struct ientry *ip;

   if (ifree >= &itable[isize])
      syserr("out of identifier table space");
   ip = ifree++;
   ip->i_blink = blink;
   ip->i_name = nam;
   ip->i_length = len;
   return ip;
   }

/*
 * locinit -  clear local symbol table.
 */
locinit()
   {
   dynoff = 0;
   argoff = 0;
   nlocal = -1;
   nconst = -1;
   static1 = statics;
   }

/*
 * putloc - make a local symbol table entry.
 */
struct lentry *putloc(n, id, flags, imperror, procname)
int n;
char *id;
register int flags;
int imperror;
char *procname;
   {
   register struct lentry *lp;
   register union {
      struct gentry *gp;
      int bn;
      } p;
   extern struct gentry *glocate(), *putglob();

   if (n >= lsize)
      syserr("out of local symbol table space.");
   if (n > nlocal)
      nlocal = n;
   lp = &ltable[n];
   lp->l_name = id;
   lp->l_flag = flags;
   if (flags == 0) {				/* undeclared */
      if ((p.gp = glocate(id)) != NULL) {	/* check global */
         lp->l_flag = F_Global;
         lp->l_val.global = p.gp;
         }
      else if ((p.bn = blocate(id)) != 0) {	/* check builtin */
         lp->l_flag = F_Builtin;
         lp->l_val.global = putglob(id, F_Builtin | F_Proc, -1, p.bn);
         }
      else {					/* implicit local */
         if (imperror)
            warn(id, "undeclared identifier, procedure ", procname);
         lp->l_flag = F_Dynamic;
         lp->l_val.offset = ++dynoff;
         }
      }
   else if (flags & F_Global) {			/* global variable */
      if ((p.gp = glocate(id)) == NULL)
         syserr("putloc: global not in global table");
      lp->l_val.global = p.gp;
      }
   else if (flags & F_Argument)			/* procedure argument */
      lp->l_val.offset = ++argoff;
   else if (flags & F_Dynamic)			/* local dynamic */
      lp->l_val.offset = ++dynoff;
   else if (flags & F_Static)			/* local static */
      lp->l_val.staticid = ++statics;
   else
      syserr("putloc: unknown flags");
   return lp;
   }

/*
 * putglob - make a global symbol table entry.
 */
struct gentry *putglob(id, flags, nargs, procid)
char *id;
int flags;
int nargs;
int procid;
   {
   register struct gentry *p;
   extern struct gentry *glocate(), *alcglob();

   if ((p = glocate(id)) == NULL) {	/* add to head of hash chain */
      p = ghash[ghasher(id)];
      ghash[ghasher(id)] = alcglob(p, id, flags, nargs, procid);
      return ghash[ghasher(id)];
      }
   p->g_flag |= flags;
   p->g_nargs = nargs;
   p->g_procid = procid;
   return p;
   }

/*
 * putconst - make a constant symbol table entry.
 */
struct centry *putconst(n, flags, len, pc, val)
int n;
int flags, len;
word pc;
union {
   long  ival;
   double rval;
   char *sval;
   } val;
   {
   register struct centry *p;
#if IntSize == 16
   int minsh, maxsh;

   minsh = MinShort;
   maxsh = MaxShort;
#endif IntSize == 16
   if (n >= csize)
      syserr("out of constant table space");
   if (nconst < n)
      nconst = n;
   p = &ctable[n];
   p->c_flag = flags;
   p->c_pc = pc;
   if (flags & F_IntLit) {
      p->c_val.ival = val.ival;
#if IntSize == 16
      if (val.ival < (long)minsh | val.ival > (long)maxsh)
         p->c_flag |= F_LongLit;
#endif IntSize == 16
      }
   else if (flags & F_StrLit) {
      p->c_val.sval = val.sval;
      p->c_length = len;
      }
   else if (flags & F_CsetLit) {
      p->c_val.sval = val.sval;
      p->c_length = len;
      }
   else	if (flags & F_RealLit)
#ifdef Double
/* access real values one word at a time */
    {  int *rp, *rq;	
       rp = (int *) &(p->c_val.rval);
       rq = (int *) &(val.rval);
       *rp++ = *rq++;
       *rp   = *rq;
    }
#else Double
      p->c_val.rval = val.rval;
#endif Double
   else
      fprintf(stderr, "putconst: bad flags: %06o %011lo\n", flags, val.ival);
   return p;
   }

/*
 * putfield - make a record/field table entry.
 */
putfield(fname, rnum, fnum)
char *fname;
int rnum, fnum;
   {
   register struct fentry *fp;
   register struct rentry *rp, *rp2;
   word hash;
   extern struct fentry *flocate(), *alcfhead();
   extern struct rentry *alcfrec();

   fp = flocate(fname);
   if (fp == NULL) {		/* create a field entry */
      nfields++;
      hash = fhasher(fname);
      fp = fhash[hash];
      fhash[hash] = alcfhead(fp, fname, nfields, alcfrec(NULL, rnum, fnum));
      return;
      }
   rp = fp->f_rlist;		/* found field entry, look for */
   if (rp->r_recid > rnum) {	/*   spot in record list */
      fp->f_rlist = alcfrec(rp, rnum, fnum);
      return;
      }
   while (rp->r_recid < rnum) {	/* keep record list ascending */
      if (rp->r_link == NULL) {
         rp->r_link = alcfrec(NULL, rnum, fnum);
         return;
         }
      rp2 = rp;
      rp = rp->r_link;
      }
   rp2->r_link = alcfrec(rp, rnum, fnum);
   }

/*
 * glocate - lookup identifier in global symbol table, return NULL
 *  if not present.
 */
struct gentry *glocate(id)
char *id;
   {
   register struct gentry *p;

   p = ghash[ghasher(id)];
   while (p != NULL && p->g_name != id)
      p = p->g_blink;
   return p;
   }

/*
 * flocate - lookup identifier in field table.
 */
struct fentry *flocate(id)
char *id;
   {
   register struct fentry *p;

   p = fhash[fhasher(id)];
   while (p != NULL && p->f_name != id)
      p = p->f_blink;
   return p;
   }

/*
 * alcglob - create a new global symbol table entry.
 */
struct gentry *alcglob(blink, name, flag, nargs, procid)
struct gentry *blink;
char *name;
int flag;
int nargs;
int procid;
   {
   register struct gentry *gp;

   if (gfree >= &gtable[gsize])
      syserr("out of global symbol table space");
   gp = gfree++;
   gp->g_blink = blink;
   gp->g_name = name;
   gp->g_flag = flag;
   gp->g_nargs = nargs;
   gp->g_procid = procid;
   return gp;
   }

/*
 * alcfhead - allocate a field table header.
 */
struct fentry *alcfhead(blink, name, fid, rlist)
struct fentry *blink;
char *name;
int fid;
struct rentry *rlist;
   {
   register struct fentry *fp;

   if (ffree >= &ftable[fsize])
      syserr("out of field table space");
   fp = ffree++;
   fp->f_blink = blink;
   fp->f_name = name;
   fp->f_fid = fid;
   fp->f_rlist = rlist;
   return fp;
   }

/*
 * alcfrec - allocate a field table record list element.
 */
struct rentry *alcfrec(link, rnum, fnum)
struct rentry *link;
int rnum, fnum;
   {
   register struct rentry *rp;

   if (rfree >= &rtable[rsize])
      syserr("out of field table space for record lists");
   rp = rfree++;
   rp->r_link = link;
   rp->r_recid = rnum;
   rp->r_fnum = fnum;
   return rp;
   }
