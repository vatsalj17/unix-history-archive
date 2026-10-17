/*
 * Initialization and error routines.
 */

#include "../h/rt.h"
#include "../h/version.h"
#include "gc.h"
#include "../h/header.h"
#include <signal.h>
#include <ctype.h>
#ifndef VMS
#ifndef MSDOS
#include <sys/types.h>
#include <sys/times.h>
#else MSDOS
#ifdef LATTICE
#include <fcntl.h>
#include <time.h>
#endif LATTICE
#ifdef MSoft
#include <sys/types.h>
#include <fcntl.h>
#include SysTime
#endif MSoft
#endif MSDOS
#else VMS
#include <types.h>
struct tms {
   time_t    tms_utime;		/* user time */
   time_t    tms_stime;		/* system time */
   time_t    tms_cutime;		/* user time, children */
   time_t    tms_cstime;		/* system time, children */
   };
#endif VMS

/*
 * A number of important variables follow.
 */

#ifndef MaxHeader
#define MaxHeader MaxHdr
#endif MaxHeader

extern putpos();			/* assignment function for &pos */
extern putran();			/* assignment function for &random */
extern putsub();			/* assignment function for &subject */
extern puttrc();			/* assignment function for &trace */

word *stack;				/* interpreter stack */
int line = 0;				/* source program line number */
int k_level = 0;			/* &level */
struct descrip k_main;			/* &main */
char *code;				/* interpreter code buffer */
word *records;				/* pointer to record procedure blocks */
word *ftab;				/* pointer to record/field table */
struct descrip *globals, *eglobals;	/* pointer to global variables */
struct descrip *gnames, *egnames;	/* pointer to global variable names */
struct descrip *statics, *estatics;	/* pointer to static variables */
char *ident;				/* pointer to identifier table */

int numbufs = NumBuf;			/* number of i/o buffers */
char (*bufs)[BUFSIZ];			/* pointer to buffers */
FILE **bufused;				/* pointer to buffer use markers */

word tallybin[16];			/* counters for tallying */
int tallyopt = 0;			/* want tally results output? */

int mstksize = MStackSize;		/* initial size of main stack */
int stksize = StackSize;		/* co-expression stack size */
struct b_coexpr *stklist;		/* base of co-expression block list */

word statsize = MaxStatSize;		/* size of static region */
word statincr = MaxStatSize/4;		/* increment for static region */
char *statbase;				/* start of static space */
char *statend;				/* end of static space */
char *statfree;				/* static space free pointer */

word ssize = MaxStrSpace;		/* initial string space size (bytes) */
char *strbase;				/* start of string space */
char *strend;				/* end of string space */
char *strfree;				/* string space free pointer */
char *currend;				/* current end of memory region */

word abrsize = MaxAbrSize;		/* initial size of allocated block
					   region (bytes) */
char *blkbase;				/* start of block region */
char *maxblk;				/* end of allocated blocks */
char *blkfree;				/* block region free pointer */

uword statneed;				/* stated need for static space */
uword strneed;				/* stated need for string space */
uword blkneed;				/* stated need for block space */

struct descrip **quallist;		/* string qualifier list */
struct descrip **qualfree;		/* qualifier list free pointer */
struct descrip **equallist;		/* end of qualifier list */

int dodump;				/* if non-zero, core dump on error */
int noerrbuf;				/* if non-zero, do not buffer stderr */

struct descrip current;			/* current expression stack pointer */
struct descrip maps2;			/* second cached argument of map */
struct descrip maps3;			/* third cached argument of map */

int ntended = 0;			/* number of active tended descrips */
long starttime;				/* start time of job in milliseconds */

/*
 * Next there are several structures for built-in values.  Parts of some
 * of these structures are initialized later.
 */

/*
 * Built-in csets
 */

/*
 * &ascii; 128 bits on, second 128 bits off.
 */
struct b_cset  k_ascii = {
   T_Cset,
   128,
   cset_display(~0, ~0, ~0, ~0, ~0, ~0, ~0, ~0,
                 0,  0,  0,  0,  0,  0,  0,  0)
   };

/*
 * &cset; all 256 bits on.
 */
struct b_cset  k_cset = {
   T_Cset,
   256,
   cset_display(~0, ~0, ~0, ~0, ~0, ~0, ~0, ~0,
                ~0, ~0, ~0, ~0, ~0, ~0, ~0, ~0)
   };



/*
 * Cset for &lcase; bits corresponding to lowercase letters are on.
 */
struct b_cset  k_lcase = {
   T_Cset,
   26,
   cset_display( 0,  0,  0,  0,  0,  0, ~01, 03777,
                 0,  0,  0,  0,  0,  0,  0,  0)
   };

/*
 * &ucase; bits corresponding to uppercase characters are on.
 */
struct b_cset  k_ucase = {
   T_Cset,
   26,
    cset_display(0,  0,  0,  0, ~01, 03777, 0, 0,
                 0,  0,  0,  0,  0,  0,  0,  0)
   };

/*
 * Built-in files.
 */

struct b_file  k_errout;		/* &errout */
struct b_file  k_input;			/* &input */
struct b_file  k_output;		/* &outout */

/*
 * Keyword trapped variables.
 */

/*
 * &pos.
 */
struct b_tvkywd tvky_pos = {
   T_Tvkywd,
   /* putpos, */
   /* D_Integer, */
   /* 1, */
   /* 4, */
   /* "&pos" */
   };

/*
 * &random.
 */
struct b_tvkywd tvky_ran = {
   T_Tvkywd,
   /* putran, */
   /* D_Integer, */
   /* 0, */
   /* 7, */
   /* "&random" */
   };

/*
 * &subject.
 */
struct b_tvkywd tvky_sub = {
   T_Tvkywd,
   /* putsub, */
   /* 0, */
   /* 0, */
   /* 8, */
   /* "&subject" */
   };

/*
 * &trace.
 */
struct b_tvkywd tvky_trc = {
   T_Tvkywd,
   /* puttrc, */
   /* D_Integer, */
   /* 0, */
   /* 6, */
   /* "&trace" */
   };

/*
 * Co-expression block header for &main.
 */

static struct b_coexpr *mainhead;

#if IntSize == 16
/*
 * Long integer block for &random.
 */
struct b_int long_ran = {
   T_Longint,
   0L
   };
#endif IntSize == 16

/*
 * Various constant descriptors.
 */

struct descrip blank = {1, /*" "*/};
struct descrip emptystr = {0, /*""*/};
struct descrip errout = {D_File, /*&k_errout*/};
struct descrip input = {D_File, /*&k_input*/};
struct descrip lcase = {26, /*lowercase*/};
struct descrip letr = {1, /*"r"*/};
struct descrip nulldesc = {D_Null, /*0*/};
struct descrip onedesc = {D_Integer, /*1*/};
struct descrip ucase = {26, /*uppercase*/};
struct descrip zerodesc = {D_Integer, /*0*/};

#ifdef RunStats
#endif RunStats

/*
 * init - initialize memory and prepare for Icon execution.
 */

init(name)
char *name;
   {
   register int i;
   word cbread;
   int f;
   char c, *p;
   struct descrip *dp;
   struct header hdr;
#ifndef MSDOS
   struct tms tp;
   extern char *brk(), *sbrk(), *index();
#else MSDOS
#ifdef LPTR
   long longread();
#endif LPTR
#endif MSDOS
   extern fpetrap(), segvtrap();

   /*
    * Catch floating point traps and memory faults.
    */
#ifndef MSoft
   signal(SIGFPE, fpetrap);
#endif MSoft
#ifndef MSDOS
#ifdef PYRAMID
{
   struct sigvec a;

   a.sv_handler = fpetrap;
   a.sv_mask = 0;
   a.sv_onstack = 0;
   sigvec(SIGFPE, &a, 0);
   sigsetmask(1 << SIGFPE);
}
#else PYRAMID
   signal(SIGFPE, fpetrap);
#endif PYRAMID
#endif MSDOS

   /*
    * Initializations that cannot be performed statically (at least for
    * some compilers).
    */

   k_errout.title = T_File;
   k_errout.fd = stderr;
   k_errout.status = Fs_Write;
   k_errout.fname.dword = 7;
   StrLoc(k_errout.fname) = "&errout";

   k_input.title = T_File;
   k_input.fd = stdin;
   k_input.status = Fs_Read;
   k_input.fname.dword = 6;
   StrLoc(k_input.fname) = "&input";

   k_output.title = T_File;
   k_output.fd = stdout;
   k_output.status = Fs_Write;
   k_output.fname.dword = 7;
   StrLoc(k_output.fname) = "&output";

   tvky_pos.putval = putpos;
   ((tvky_pos.kyval).dword) = D_Integer;
   IntVal(tvky_pos.kyval) = 1;
   StrLen(tvky_pos.kyname) = 4;
   StrLoc(tvky_pos.kyname) = "&pos";

   tvky_ran.putval = putran;
#if IntSize == 16
   ((tvky_ran.kyval).dword) = D_Longint;
#else IntSize == 16
   ((tvky_ran.kyval).dword) = D_Integer;
#endif IntSize == 16
   StrLen(tvky_ran.kyname) = 7;
   StrLoc(tvky_ran.kyname) = "&random";

   tvky_sub.putval = putsub;
   StrLen(tvky_sub.kyval) = 0;
   StrLen(tvky_sub.kyname) = 8;
   StrLoc(tvky_sub.kyname) = "&subject";

   tvky_trc.putval = puttrc;
   ((tvky_trc.kyval).dword) = D_Integer;
   StrLen(tvky_trc.kyname) = 6;
   StrLoc(tvky_trc.kyname) = "&trace";
#if IntSize == 16
   BlkLoc(tvky_ran.kyval) = (union block *) &long_ran;
#else IntSize == 16
   IntVal(tvky_ran.kyval) = 0;
#endif IntSize == 16
   IntVal(tvky_trc.kyval) = 0;
   StrLoc(tvky_sub.kyval) = "";


   StrLoc(k_subject) = "";
   IntVal(nulldesc) = 0;
   maps2 = nulldesc;
   maps3 = nulldesc;
   IntVal(zerodesc) = 0;
   IntVal(onedesc) = 1;
   StrLoc(emptystr) = "";
   StrLoc(blank) = " ";
   StrLoc(letr) = "r";
   BlkLoc(input) = (union block *) &k_input;
   BlkLoc(errout) = (union block *) &k_errout;
   StrLoc(lcase) = "abcdefghijklmnopqrstuvwxyz";
   StrLoc(ucase) = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
   
   /*
    * Open the icode file and read the header.
    */
   i = strlen(name);
#ifndef MSDOS
   f = open(name, 0);
#else MSDOS
#ifdef LATTICE
   f = open(name,O_RDONLY | O_RAW);
#endif LATTICE
#ifdef MSoft
   f = open(name,O_RDONLY | O_BINARY);
#endif MSoft
#endif MSDOS
   if (f < 0)
      error("can't open interpreter file");
#ifndef NoHeader
   lseek(f, (long)MaxHeader, 0);
#endif NoHeader
   if (read(f, (char *)&hdr, sizeof(hdr)) != sizeof(hdr))
      error("can't read interpreter file header");
    
   /*
    * Establish pointers to data regions.
    */
   code = (char *)sbrk((word)0);
   k_trace = hdr.trace;
   records = (word *) (word)(code + hdr.records);
   ftab = (word *) (word)(code + hdr.ftab);
   globals = (struct descrip *) (code + hdr.globals);
   gnames = eglobals = (struct descrip *) (code + hdr.gnames);
   statics = egnames = (struct descrip *) (code + hdr.statics);
   estatics = (struct descrip *) (code + hdr.ident);
   ident = (char *) estatics;

   /*
    * Examine the environment and make appropriate settings.
    */
   envlook();

   /*
    * Convert stack sizes from words to bytes.
    */
   stksize *= WordSize;
   mstksize *= WordSize;
 
   /*
    * Set up allocated memory.  The regions are:
    *
    *	Static memory region
    *	Allocated string region
    *	Allocate block region
    *	String qualifier list
    */
   /*
    * Align bufs on a word boundary
    */
   bufs = (char **)((word)(code + hdr.hsize + 3) & ~03);
   bufused = (FILE **) (bufs + numbufs);
   statfree = statbase = (char *)(((word)(bufused + numbufs) + 63)  & ~077);
   statend = statbase + mstksize + statsize;
   strfree = strbase = (char *)((word)(statend + 63) & ~077);
   blkfree = blkbase = strend = (char *)((word)(strbase + ssize + 63) & ~077);
   quallist = qualfree = equallist =
    (struct descrip **)(maxblk = (char *)((word)(blkbase + abrsize + 63) & ~077));

   /*
    * Try to move the break back to the end of memory to allocate (the
    *  end of the string qualifier list) and die if the space isn't
    *  available.
    */
   if ((int)brk(equallist) == -1)
      error("insufficient memory");
   currend = sbrk(0);			/* keep track of end of memory */

   /*
    * Allocate stack and initialize &main.
    */
   stack = (word *)malloc(mstksize);
   mainhead = (struct b_coexpr *)stack;
   mainhead->title = T_Coexpr;
   mainhead->activator.dword = D_Coexpr;
   BlkLoc(mainhead->activator) = (union block *)mainhead;
   mainhead->size = 0;
   mainhead->freshblk = nulldesc;	/* &main has no refresh block. */
					/*  This really is a bug. */

   /*
    * Point &main at the stack for the main procedure and set current,
    *  the pointer to the current co-expression to &main.
    */
   k_main.dword = D_Coexpr;
   BlkLoc(k_main) = (union block *) mainhead;
   current = k_main;
   
   /*
    * Read the interpretable code and data into memory.
    */
#ifndef MSDOS
   if ((cbread = read(f, code, hdr.hsize)) != hdr.hsize) {
#else MSDOS
#ifdef SPTR
   if ((cbread = read(f, code, hdr.hsize)) != hdr.hsize) {
#else /* Handle the case where hdr.hsize is long */
   if ((cbread = longread(f, code, hdr.hsize)) != hdr.hsize) {
#endif SPTR
#endif MSDOS
      fprintf(stderr,"Tried to read %ld bytes of code, and got %ld\n",
	(long)hdr.hsize,(long)cbread);
      error("can't read interpreter code");
      }
   close(f);

/*
 * Make sure the version number of the icode matches the interpreter version.
 */

   if (strcmp((char *)hdr.config,IVersion)) {
      fprintf(stderr,"icode version mismatch\n");
      fprintf(stderr,"\ticode version: %s\n",(char *)hdr.config);
      fprintf(stderr,"\texpected version: %s\n",IVersion);
      fflush(stderr);
      if (dodump)
         abort();
      c_exit(ErrorExit);
      }

   /*
    * Resolve references from icode to runtime system.
    */
   resolve();

   /*
    * Mark all buffers as available.
    */
   c = (char) NULL;
   for (i = 0; i < numbufs; i++)
      bufused[i] = (FILE *) c;

   /*
    * Buffer stdin if a buffer is available.
    */
#ifndef VMS
   if (numbufs >= 1) {
      setbuf(stdin, bufs[0]);
      bufused[0] = stdin;
      }
   else
      setbuf(stdin, NULL);

   /*
    * Buffer stdout if a buffer is available.
    */
   if (numbufs >= 2) {
      setbuf(stdout, bufs[1]);
      bufused[1] = stdout;
      }
   else
      setbuf(stdout, NULL);
   
   /*
    * Buffer stderr if a buffer is available.
    */
   if (numbufs >= 3 && !noerrbuf) {
      setbuf(stderr, bufs[2]);
      bufused[2] = stderr;
      }
   else
      setbuf(stderr, NULL);
#endif VMS

   /*
    * Initialize memory monitoring if enabled.
    */
   MMInit();

   /*
    * Get startup time.
    */
#ifndef MSDOS
   times(&tp);
   starttime = tp.tms_utime;
#else MSDOS
   time(&starttime);
#endif MSDOS
   }

/*
 * Check for environment variables that Icon uses and set system
 *  values as is appropriate.
 */
envlook()
   {
   register char *p;
   extern char *getenv();

   if ((p = getenv("TRACE")) != NULL && *p != '\0')
      k_trace = atoi(p);
   if ((p = getenv("NBUFS")) != NULL && *p != '\0')
      numbufs = atoi(p);
   if ((p = getenv("COEXPSIZE")) != NULL && *p != '\0')
      stksize = atoi(p);
   if ((p = getenv("STRSIZE")) != NULL && *p != '\0')
      ssize = atoi(p);
   if ((p = getenv("HEAPSIZE")) != NULL && *p != '\0')
      abrsize = atoi(p);
   if ((p = getenv("STATSIZE")) != NULL && *p != '\0')
      statsize = atoi(p);
   if ((p = getenv("STATINCR")) != NULL && *p != '\0')
      statincr = atoi(p);
   if ((p = getenv("MSTKSIZE")) != NULL && *p != '\0')
      mstksize = atoi(p);
   if ((p = getenv("ICONCORE")) != NULL && *p != '\0') {
#ifndef MSoft
      signal(SIGFPE, SIG_DFL);
#endif MSoft
#ifndef MSDOS
      signal(SIGSEGV, SIG_DFL);
#endif MSDOS
      dodump++;
      }
   if ((p = getenv("NOERRBUF")) != NULL)
      noerrbuf++;
   }

/*
 * Produce run-time error 204 on floating point traps.
 */
#ifdef PYRAMID
fpetrap(code, subcode, sp)
int code, subcode, sp;
   {
   runerr(subcode == FPE_wordOVF_EXC ? 203 : 204, NULL);
   }
#else PYRAMID
fpetrap()
   {
   runerr(204, NULL);
   }
#endif PYRAMID

/*
 * Produce run-time error 302 on segmentation faults.
 */
segvtrap()
   {
   runerr(302, NULL);
   }

/*
 * error - print error message s; used only in startup code.
 */
error(s)
char *s;
   {
   fprintf(stderr, "error in startup code\n%s\n", s);
   fflush(stderr);
   if (dodump)
      abort();
   c_exit(ErrorExit);
   }

/*
 * syserr - print s as a system error.
 */
syserr(s)
char *s;
   {
   struct b_proc *bp;

   bp = (struct b_proc *)BlkLoc(argp[0]);
   if (line > 0)
      fprintf(stderr, "System error at line %ld in %s\n%s\n", (long)line,
         bp->filename, s);
   else
      fprintf(stderr, "System error in startup code\n%s\n", s);
   fflush(stderr);
   if (dodump)
      abort();
   c_exit(ErrorExit);
   }

/*
 * errtab maps run-time error numbers into messages.
 */
struct errtab {
   int errno;
   char *errmsg;
   } errtab[] = {
   101, "integer expected",
   102, "numeric expected",
   103, "string expected",
   104, "cset expected",
   105, "file expected",
   106, "procedure or integer expected",
   107, "record expected",
   108, "list expected",
   109, "string or file expected",
   110, "string or list expected",
   111, "variable expected",
   112, "invalid type to size operation",
   113, "invalid type to random operation",
   114, "invalid type to subscript operation",
   115, "list or table expected",
   116, "invalid type to element generator",
   117, "missing main procedure",
   118, "co-expression expected",
   119, "set expected",

   201, "division by zero",
   202, "remaindering by zero",
   203, "integer overflow",
   204, "real overflow, underflow, or division by zero",
   205, "value out of range",
   206, "negative first operand to real exponentiation",
   207, "invalid field name",
   208, "second and third arguments to map of unequal length",
   209, "invalid second argument to open",
   210, "argument to system function too long",
   211, "by clause equal to zero",
   212, "attempt to read file not open for reading",
   213, "attempt to write file not open for writing",
   214, "recursive co-expression activation",

   301, "interpreter stack overflow",
   302, "C stack overflow",
   303, "unable to expand memory region",
   304, "memory region size changed",

   0,   0
   };

/*
 * runerr - print message corresponding to error n and if v is nonnull,
 *  print it as the offending value.
 */
#ifdef PCIX
/*
 * For PC/IX, runerr is an assembly language routine that jumps into this
 * xruner procedure past the call to csv which occurs at the beginning of
 * all C procedures. This is necessary to defeat the stack data collision
 * testing which is done in csv in pc/ix and which would cause a loop,
 * since one of the possible reasons for calling runerr in the first place
 * might be stack/data collision.
 */
xruner(n, v)
#else PCIX
runerr(n, v)
#endif PCIX
register int n;
struct descrip *v;
   {
   register struct errtab *p;
   struct b_proc *bp;

   if (line > 0) {
      bp = (struct b_proc *)BlkLoc(argp[0]);
      fprintf(stderr, "Run-time error %d at line %ld in %s\n", n,
         (long)line, bp->filename);
      }
   else
      fprintf(stderr, "Run-time error %d in startup code\n", n);
   for (p = errtab; p->errno > 0; p++)
      if (p->errno == n) {
         fprintf(stderr, "%s\n", p->errmsg);
         break;
         }
   if (v != NULL) {
      fprintf(stderr, "offending value: ");
      outimage(stderr, v, 0);
      putc('\n', stderr);
      }
   fflush(stderr);
   if (dodump)
      abort();
   c_exit(ErrorExit);
   }

/*
 * resolve - perform various fixups on the data read from the interpretable
 *  file.
 */
resolve()
   {
   register word i;
   register struct b_proc *pp;
   register struct descrip *dp;
   extern mkrec();
   extern struct b_proc *functab[];

   /*
    * Scan the global variable list for procedures and fill in appropriate
    *  addresses.
    */
   for (dp = globals; dp < eglobals; dp++) {
      if ((*dp).dword != D_Proc)
         continue;
      /*
       * The second word of the descriptor for procedure variables tells
       *  where the procedure is.  Negative values are used for built-in
       *  procedures and positive values are used for Icon procedures.
       */
      i = IntVal(*dp);
      if (i < 0) {
         /*
          * *dp names a built-in function, negate i and use it as an index
          *  into functab to get the location of the procedure block.
          */
         BlkLoc(*dp) = (union block *) functab[-i-1];
         }
      else {
         /*
          * *dp names an Icon procedure or a record.  i is an offset to
          *  location of the procedure block in the code section.  Point
          *  pp at the block and replace BlkLoc(*dp).
          */
         pp = (struct b_proc *) (code + i);
         BlkLoc(*dp) = (union block *) pp;
         /*
          * Relocate the address of the name of the procedure.
          */
         StrLoc(pp->pname) += (word)ident;
         if (pp->ndynam == -2)
            /*
             * This procedure is a record constructor.  Make its entry point
             *  be the entry point of mkrec().
             */
            pp->entryp.ccode = mkrec;
         else {
            /*
             * This is an Icon procedure.  Relocate the entry point and
             *  the names of the parameters, locals, and static variables.
             */
            pp->entryp.icode = code + (word)pp->entryp.icode;
            if (pp->ndynam >= 0)
               pp->filename += (word)ident;
            for (i = 0; i < pp->nparam+pp->ndynam+pp->nstatic; i++)
               StrLoc(pp->lnames[i]) += (word)ident;
            }
         }
      }
   /*
    * Relocate the names of the global variables.
    */
   for (dp = gnames; dp < egnames; dp++)
      StrLoc(*dp) += (word)ident;
   }


/*
 * c_exit(i) - flush all buffers and exit with status i.
 */
c_exit(i)
int i;
{
	int j;

#ifdef MemMon
	MMTerm();
#endif MemMon
	if (tallyopt) {
		fprintf(stderr,"tallies: ");
		for (j=0; j<16; j++)
			fprintf(stderr," %ld", (long)tallybin[j]);
		fprintf(stderr,"\n");
	}
	exit(i);
}

err()
{
   syserr("call to 'err'\n");
}

#ifdef MSDOS
#ifdef LPTR
/* Write a long string in 32k chunks */
long longread(file,s,len)
int file;
char *s;
long int len;
{
   long int loopnum;
   long int leftover;
   long int tally;
   unsigned i;
   char *p;

   tally = 0;
   leftover = len % 32768;
   for(p = s, loopnum = len/32768;loopnum;loopnum--) {
       i = read(file,p,32768);
       tally += i;
       if (i != 32768) return tally;
       p += 32768;
   }
   if(leftover) tally += read(file,p,leftover);
   return tally;
}
#endif LPTR
#endif MSDOS
