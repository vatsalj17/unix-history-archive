/*
 * Definitions and declarations used throughout the run-time system.
 * These are also used by the linker in constructing data for use by
 * the run-time system.
 */

#include <stdio.h>
#include "../h/config.h"
#include "../h/cpuconf.h"
#include "../h/memsize.h"

/*
 * Constants that are not likely to vary between implementations.
 */
#define BitOffMask (CIntSize-1)
#define CsetSize (256/CIntSize)	/* number of ints to hold 256 cset
				     bits.  Use (256/IntSize)+1 if
				     256 % IntSize != 0 */
#define MinListSlots	     8	/* number of elements in an expansion
				 *  list element block  */
#define MaxCvtLen	   257	/* largest string in conversions */
#define MaxReadStr	  2049	/* largest string to read() in one piece */
#define RandA	    1103515245	/* random seed multiplier */
#define RandC	     453816694	/* random seed additive constant */
#define RanScale 4.65661286e-10	/* random scale factor = 1/(2^31-1)) */
#define WordSize sizeof(word)	/* size in bytes of a word */

/*
 * Type codes (descriptors and blocks).
 */

/* >types */
#define T_Null		 0	/* null value */
#define T_Integer	 1	/* integer */
#define T_Longint	 2	/* long integer */
#define T_Real		 3	/* real number */
#define T_Cset		 4	/* cset */
#define T_File		 5	/* file */
#define T_Proc		 6	/* procedure */
#define T_List		 7	/* list header */
#define T_Table		 8	/* table header */
#define T_Record	 9	/* record */
#define T_Telem		10	/* table element */
#define T_Lelem		11	/* list element */
#define T_Tvsubs	12	/* substring trapped variable */
#define T_Tvkywd	13	/* keyword trapped variable */
#define T_Tvtbl		14	/* table element trapped variable */
#define T_Set		15	/* set header */
#define T_Selem		16	/* set element */
#define T_Refresh	17	/* refresh block */
#define T_Coexpr	18	/* co-expression */
/* <types */

#define MaxType		18	/* maximum type number */

/*
 * Descriptor types and flags.
 */

/* >dwords */
#define D_Null		(T_Null | F_Nqual)
#define D_Integer	(T_Integer | F_Nqual)
#define D_Longint	(T_Longint | F_Ptr | F_Nqual)
#define D_Real		(T_Real | F_Ptr | F_Nqual)
#define D_Cset		(T_Cset | F_Ptr | F_Nqual)
#define D_File		(T_File | F_Ptr | F_Nqual)
#define D_Proc		(T_Proc | F_Ptr | F_Nqual)
#define D_List		(T_List | F_Ptr | F_Nqual)
#define D_Table		(T_Table | F_Ptr | F_Nqual)
#define D_Set		(T_Set | F_Ptr | F_Nqual)
#define D_Selem		(T_Selem | F_Ptr | F_Nqual)
#define D_Record	(T_Record | F_Ptr | F_Nqual)
#define D_Telem		(T_Telem | F_Ptr | F_Nqual)
#define D_Lelem		(T_Lelem | F_Ptr | F_Nqual)
#define D_Tvsubs	(T_Tvsubs | D_Tvar)
#define D_Tvtbl		(T_Tvtbl | D_Tvar)
#define D_Tvkywd	(T_Tvkywd | D_Tvar)
#define D_Coexpr	(T_Coexpr | F_Ptr | F_Nqual)
#define D_Refresh	(T_Refresh | F_Ptr | F_Nqual)

#define D_Var		(F_Var | F_Nqual | F_Ptr)
#define D_Tvar		(D_Var | F_Tvar)
/* <dwords */

/*
 * Run-time data structures.
 */

union numeric {			/* long integers or real numbers */
   long integer;
   double real;
   };

/* >descrip */
struct descrip {		/* descriptor */
   word dword;			/* type field */
   union {
      word integr;		/* integer value */
      char *sptr;		/* pointer to character string */
      union block *bptr;	/* pointer to a block */
      struct descrip *dptr;	/* pointer to a descriptor */
      } vword;
   };
/* <descrip */

struct sdescrip {
   word length;			/* length of string */
   char *string;		/* pointer to string */
   };

/*
 * Expression frame marker
 */
/* >eframe */
struct ef_marker {
   word *ef_failure;		/* failure ipc */
   struct ef_marker *ef_efp;	/* saved efp */
   struct gf_marker *ef_gfp;	/* saved gfp */
   word ef_ilevel;		/* saved ilevel */
   };
/* <eframe */

/*
 * Procedure frame marker
 */
struct pf_marker {
   word pf_nargs;		/* number of arguments */
   struct pf_marker *pf_pfp;	/* saved pfp */
   struct ef_marker *pf_efp;	/* saved efp */
   struct gf_marker *pf_gfp;	/* saved gfp */
   struct descrip *pf_argp;	/* saved argp */
   word *pf_ipc;			/* saved ipc */
   word pf_line;			/* saved line number */
   word pf_ilevel;		/* saved ilevel */
   struct descrip pf_locals[1];	/* vector of descriptors for locals */
   };

/*
 * Generator frame marker
 */
struct gf_marker {
   word gf_gentype;		/* type of generator */
   struct ef_marker *gf_efp;	/*  efp, */
   struct gf_marker *gf_gfp;	/*  gfp */
   word *gf_ipc;		/*  ipc, */
   word gf_line;		/*  line, */
   struct pf_marker *gf_pfp;	/*  pfp, */
   struct descrip *gf_argp;	/* saved argp, */
   };

/*
 * Generator frame marker dummy -- used only for sizing "small"
 *  generator frames where procedure infomation need not be saved.
 *  The first five members here *must* be identical to those for
 *  gf_marker.
 */
/* >gframe */
struct gf_smallmarker {
   word gf_gentype;		/* type of generator */
   struct ef_marker *gf_efp;	/* efp */
   struct gf_marker *gf_gfp;	/* gfp */
   word *gf_ipc;		/* ipc */
   word gf_line;		/* line */
   };
/* <gframe */


struct b_int {			/* long integer block */
   word title;			/*   T_Longint */
   long intval;			/*   value */
   };

struct b_real {			/* real block */
   word title;			/*   T_Real */
   double realval;		/*   value */
   };

/* >cset */
struct b_cset {			/* cset block */
   word title;			/*   T_Cset */
   word size;			/*   size of cset */
   int bits[CsetSize];		/*   array of bits, one per ascii character */
   };
/* <cset */

struct b_file {			/* file block */
   word title;			/*   T_File */
   FILE *fd;			/*   Unix file descriptor */
   int status;			/*   file status */
   struct descrip fname;	/*   file name (string qualifier) */
   };

struct b_proc {			/* procedure block */
   word title;			/*   T_Proc */
   word blksize;			/*   size of block */
   union {			/*  entry points for */
      int (*ccode)();		/*   C routines */
      char *icode;		/*   and icode */
      } entryp;
   word nparam;			/*   number of parameters */
   word ndynam;			/*   number of dynamic locals */
   word nstatic;			/*   number of static locals */
   word fstatic;			/*   index (in global table) of first static */
   char *filename;		/*   file name */
   struct descrip pname;	/*   procedure name (string qualifier) */
   struct descrip lnames[1];	/*   list of local names (qualifiers) */
   };

/*
 * b_iproc blocks are used to statically initialize information about
 *  functions.  They are identical to b_proc blocks except for
 *  the pname field which is a sdecrip (simple/string descriptor) instead
 *  of a descrip.  This is done because unions can't be initialized.
 */
	
struct b_iproc {		/* procedure block */
   word ip_title;		/*   T_Proc */
   word ip_blksize;		/*   size  of block */
   int (*ip_entryp)();		/*   entry point (code) */
   word ip_nparam;		/*   number of parameters */
   word ip_ndynam;		/*   number of dynamic locals */
   word ip_nstatic;		/*   number of static locals */
   word ip_fstatic;		/*   index (in global table) of first static */
   char *ip_filename;		/*   file name */
   struct sdescrip ip_pname;	/*   procedure name (string qualifier) */
   struct descrip ip_lnames[1];	/*   list of local names (qualifiers) */
   };

/*
 * Alternate uses for procedure block fields, applied to records.
 */
#define nfields	nparam		/* number of fields */
#define recnum	nstatic		/* record number */
#define recname	pname		/* record name */

/* >listhead */
struct b_list {			/* list header block */
   word title;			/*   T_List */
   word size;			/*   current list size */
   struct descrip listhead;	/*   pointer to first list element block */
   struct descrip listtail;	/*   pointer to last list element block */
   };
/* <listhead */

/* >listelem */
struct b_lelem {		/* list element block */
   word title;			/*   T_Lelem */
   word blksize;			/*   size of block */
   word nelem;			/*   total number of elements */
   word first;			/*   index of first element */
   word nused;			/*   number of used elements */
   struct descrip listprev;	/*   pointer to previous list element block */
   struct descrip listnext;	/*   pointer to next list element block */
   struct descrip lslots[1];	/*   array of elements */
   };
/* <listelem */

struct b_table {		/* table header block */
   word title;			/*   T_Table */
   word size;			/*   current table size */
   struct descrip defvalue;	/*   default table element value */
   struct descrip buckets[TSlots]; /* hash buckets */
   };

struct b_telem {		/* table element block */
   word title;			/*   T_Telem */
   word hashnum;			/*   for ordering chain */
   struct descrip clink;	/*   hash chain link */
   struct descrip tref;		/*   entry value */
   struct descrip tval;		/*   assigned value */
   };

struct b_set {			/* set header block */
   word title;			/*   T_Set */
   word size;			/*   size of the set */
   struct descrip sbucks[SSlots];  /* hash buckets */
   };

struct b_selem {		/* set element block */
   word title;			/*   T_Selem */
   word hashnum;			/*   hash number */
   struct descrip clink;	/*   hash chain link */
   struct descrip setmem;	/*   the element */
   };

/* >record */
struct b_record {		/* record block */
   word title;			/*   T_Record */
   word blksize;			/*   size of block */
   struct descrip recdesc;	/*   descriptor for record constructor */
   struct descrip fields[1];	/*   fields */
   };
/* <record */

/* >tvkywd */
struct b_tvkywd {		/* keyword trapped variable block */
   word title;			/*   T_Tvkywd */
   int (*putval) ();		/*  assignment function for keyword */
   struct descrip kyval;	/*  keyword value */
   struct descrip kyname;	/*  keyword name */
   };
/* <tvkywd */

struct b_tvsubs {		/* substring trapped variable block */
   word title;			/*   T_Tvsubs */
   word sslen;			/*   length of substring */
   word sspos;			/*   position of substring */
   struct descrip ssvar;	/*   variable that substring is from */
   };

struct b_tvtbl {		/* table element trapped variable block */
   word title;			/*   T_Tvtbl */
   word hashnum;			/*   hash number */
   struct descrip clink;	/*   pointer to table header block */
   struct descrip tref;		/*   entry value */
   struct descrip tval;		/*   reserved for assigned value */
   };

struct b_coexpr {		/* co-expression stack block */
   word title;			/*   T_Coexpr */
   word size;			/*   number of results produced */
   struct b_coexpr *nextstk;	/*   pointer to next allocated stack */
   struct pf_marker *es_pfp;	/*   current pfp, */
   struct ef_marker *es_efp;	/*    efp, */
   struct gf_marker *es_gfp;	/*    gfp, */
   struct descrip *es_argp;	/*    argp, */
   word *es_ipc;			/*    ipc, */
   word es_line;			/*   saved line number */
   word es_ilevel;		/*    interpreter level. */
   word *es_sp;			/*    sp, */
   struct descrip *tvalloc;	/*   where to place transmitted value */
   struct descrip activator;	/*   most recent activator */
   struct descrip freshblk;	/*   refresh block pointer */
   word cstate[15];		/*   C state information */
   };

struct b_refresh {		/* co-expression block */
   word title;			/*   T_Refresh */
   word blksize;			/*   size of block */
   word *ep;			/*   entry point */
   word numlocals;		/*   number of locals */
   struct pf_marker pfmkr;	/*   marker for enclosing procedure */
   struct descrip elems[1];	/*   arguments and locals, including Arg0 */
   };

/* >union */
union block {			/* general block */
   struct b_int longint;
   struct b_real realblk;
   struct b_cset cset;
   struct b_file file;
   struct b_proc proc;
   struct b_list list;
   struct b_lelem lelem;
   struct b_table table;
   struct b_telem telem;
   struct b_set set;
   struct b_selem selem;
   struct b_record record;
   struct b_tvkywd tvkywd;
   struct b_tvsubs tvsubs;
   struct b_tvtbl tvtbl;
   struct b_coexpr coexpr;
   struct b_refresh refresh;
   };
/* <union */

/*
 * External declarations.
 */

extern char (*bufs)[BUFSIZ];	/* i/o buffers */
extern FILE **bufused;		/* i/o buffer use markers */

extern int stksize;		/* size of co-expression stacks in words */
extern struct b_coexpr *stklist;/* base of co-expression stack list */

extern char *statbase;		/* start of static space */
extern char *statend;		/* end of static space */
extern char *statfree;		/* static space free list header */
extern word statsize;		/* size of static space */
extern word statincr;		/* size of increment for static space */

extern word ssize;		/* size of string space (bytes) */
extern char *strbase;		/* start of string space */
extern char *strend;		/* end of string space */
extern char *strfree;		/* string space free pointer */

extern word abrsize;		/* size of allocated block region (words) */
extern char *blkbase;		/* base of allocated block region */
extern char *maxblk;		/* maximum address in allocated block region */
extern char *blkfree;		/* first free location in allocated block region */

extern int bsizes[];		/* sizes of blocks */
extern int firstd[];		/* offset (words) of first descrip. */
extern char *blkname[];		/* print names for block types. */

extern int numbufs;		/* number of buffers */

extern struct b_tvkywd tvky_pos;	/* trapped variable block for &pos */
extern struct b_tvkywd tvky_ran;	/* trapped variable block for &random */
#if IntSize == 16
extern struct b_int long_ran;		/* long integer block for &random */
#endif IntSize == 16
extern struct b_tvkywd tvky_sub;	/* trapped variable block for &subject */
extern struct b_tvkywd tvky_trc;	/* trapped variable block for &trace */

#define k_pos tvky_pos.kyval.vword.integr /* value of &pos */
#if IntSize == 16
#define k_random long_ran.intval	/* value of &random */
#else IntSize == 16
#define k_random tvky_ran.kyval.vword.integr /* value of &random */
#endif IntSize == 16
#define k_subject tvky_sub.kyval		/* value of &subject */
#define k_trace tvky_trc.kyval.vword.integr	/* value of &trace */

extern struct b_cset k_ascii;	/* value of &ascii */
extern struct b_cset k_cset;	/* value of &cset */
extern struct b_file k_errout;	/* value of &errout */
extern struct b_file k_input;	/* value of &input */
extern struct b_cset k_lcase;	/* value of &lcase */
extern int k_level;		/* value of &level */
extern struct descrip k_main;	/* value of &main */
extern struct b_file k_output;	/* value of &output */
extern struct b_cset k_ucase;	/* value of &ucase */
extern long starttime;		/* start time in milliseconds */
extern struct descrip nulldesc;	/* universal &null */
extern struct descrip zerodesc;	/* universal 0 */
extern struct descrip onedesc;	/* universal 1 */
extern struct descrip emptystr;	/* universal null string */
extern struct descrip blank;	/* universal blank */
extern struct descrip letr;	/* universal letter 'r' */
extern struct descrip maps2;	/* save space for 2nd arg to map() */
extern struct descrip maps3;	/* save space for 3rd arg to map() */
extern struct descrip current;	/* current co-expression block */
extern struct descrip input;	/* universal input file */
extern struct descrip errout;	/* universal error file */
extern struct descrip lcase;	/* universal lower case string */
extern struct descrip ucase;	/* universal upper case string */

extern int line;		/* current source program line number */
extern int ntended;		/* number of active tended descriptors */

/*
 * Descriptor flags.
 */
#define TypeMask	63	/* type mask */
#define OffsetMask	(~(D_Tvar)) /* offset mask for variables */

/*
 * File status flags in status field of file blocks.
 */

#define Fs_Read		 01	/* read access */
#define Fs_Write	 02	/* write access */
#define Fs_Create	 04	/* file created on open */
#define Fs_Append	010	/* append mode */
#define Fs_Pipe		020	/* reading/writing on a pipe */

/*
 * Macros for testing descriptors. The arguments must be of type struct descrip.
 */

#define ChkNull(d)	((d).dword==D_Null)	/* check for &null */
#define EqlDesc(d1,d2)	((d1).dword == (d2).dword && BlkLoc(d1) == BlkLoc(d2))
#define Qual(d)		(!((d).dword & F_Nqual))	/* check for qualifier */
#define StrLen(q)	((q).dword)		/* length of string */
#define StrLoc(q)	((q).vword.sptr)	/* address of string */
#define Type(d)		((d).dword & TypeMask)	/* get type code */
#define Pointer(d)	((d).dword & F_Ptr)	/* check for pointer */
#define IntVal(d)	((d).vword.integr)	/* get short integer value */
#ifdef Double
#define GetReal(dp,res)	{ \
			 word *rp, *rq; \
			 rp = (word *) &(res); \
			 rq = (word *) &(BlkLoc(*dp)->realblk.realval); \
			 *rp++ = *rq++; \
			 *rp = *rq; \
			 }
#else Double
#define GetReal(dp,res)	res = BlkLoc(*dp)->realblk.realval
#endif Double
#if IntSize == 16
#define Mkint(i,dp)	mkint(i,dp)
#else IntSize == 16
#define Mkint(i,dp)	{ \
		 	 (dp)->dword = D_Integer; \
			 IntVal(*dp) = (word) (i); \
			 }
#endif IntSize == 16
#define BlkLoc(d)	((d).vword.bptr)	/* get pointer to block */
#define Var(d)		((d).dword & F_Var)	/* check for variable */
#define Tvar(d)		((d).dword & F_Tvar)	/* check for trapped var */
#define Offset(d)	((d).dword & OffsetMask)	/* get offset field */
#define VarLoc(d)	((d).vword.dptr)	/* get pointer to descriptor */
#define TvarLoc(d)	((d).vword.bptr)	/* get pointer to trapped variable */
#ifdef MixedSizes
#define SlotNum(i,j)	((word)(i &~(1 << LongSize-1) % j)
#else MixedSizes
#define SlotNum(i,j)	((i&~(1<<(IntSize-1))) % j)
#endif MixedSizes

/*
 * Macros to define procedure blocks and formal parameter lists.
 */
#define FncBlock(f,nargs)\
	int Cat(X,f)();\
	struct b_iproc Cat(B,f) = {\
	T_Proc,\
	Vsizeof(struct b_proc),\
	Cat(X,f),\
	nargs,\
	-1,\
	0, 0, 0,\
	{sizeof("f")-1,"f"}};

#define OpBlock(f,nargs,sname,realargs)\
	int f();\
	struct b_iproc Cat(B,f) = {\
	T_Proc,\
	Vsizeof(struct b_proc),\
	f,\
	nargs,\
	-1,\
	realargs,\
	0, 0,\
	{sizeof(sname)-1,sname}};

/*
 * Minimum of x and y.
 */
#define Min(x,y)	((x)<(y)?(x):(y)) 

/*
 * Maximum of x and y.
 */
#define Max(x,y)	((x)>(y)?(x):(y))

/*
 * Derefence d.
 */
#ifndef RunStats
#if PtrSize == 32
#define DeRef(d)	if(Var(d))deref(&d)
#else PtrSize == 32
#define DeRef(d)	if(!Qual(d)&&Var(d))deref(&d)
#endif PtrSize == 32 
#else RunStats
#define DeRef(d)	{\
			ev_n_deref++; \
			if(!Qual(d)&&Var(d)) \
				deref(&d); \
			else \
				ev_n_redunderef++; \
			}
#endif RunStats

/*
 *  Vsizeof is for use with variable-sized (i.e., indefinite)
 *  structures containing an array declared of size 1
 *  to avoid compiler warnings associated with 0-sized arrays.
 */

#define Vsizeof(s)	(sizeof(s) - sizeof(struct descrip))

/*
 * Procedure and Operator Declarations
 */ 
#define FncDcl(nm,n)\
	FncBlock(nm,n) Cat(X,nm)(cargp) register struct descrip *cargp;
#define OpDcl(nm,n,pn)\
	OpBlock(nm,n,pn,0) nm(cargp) register struct descrip *cargp;
#define OpDclV(nm,n,pn)\
	OpBlock(nm,n+1,pn,-n) nm(cargp) register struct descrip *cargp;
#define FncDclV(nm)\
	FncBlock(nm,-1) Cat(X,nm)(nargs,cargp) register struct descrip *cargp;
#define LibDcl(nm,n,pn)\
	OpBlock(nm,n,pn,0) nm(nargs,cargp) register struct descrip *cargp;

/*
 * Macros to access Icon arguments from C-language library procedures.
 *  Library procedures must have exactly one argument, named nargs.
 */

/*
 * n-th argument.
 */
#define Arg(n)	 	(cargp[n])
/*
 * Type field of n-th argument.
 */
#define ArgType(n)	(cargp[n].dword)
/*
 * Value field of n-th argument.
 */
#define ArgVal(n)	(cargp[n].vword.integr)

/*
 * Argument accessing macros.  Used only in top-level routines.
 */
#define Arg0	(cargp[0])
#define Arg1	(cargp[1])
#define Arg2	(cargp[2])
#define Arg3	(cargp[3])
#define Arg4	(cargp[4])
#define Arg5	(cargp[5])
#define Arg6	(cargp[6])

/*
 * Definitions for interpreter actions.
 */
#define A_Failure	1		/* routine failed */
#define A_Suspension	2		/* routine suspended */
#define A_Return	3		/* routine returned */
#define A_Pret_uw	4		/* interp unwind for Op_Pret */
#define A_Unmark_uw	5		/* interp unwind for Op_Unmark */
#define A_Resumption	6		/* resume generator */
#define A_Pfail_uw	7		/* interp unwind for Op_Pfail */
#define A_Lsusp_uw	8		/* interp unwind for Op_Lsusp */
#define A_Eret_uw	9		/* interp unwind for Op_Eret */
#define A_Coact		10		/* co-expression activated */
#define A_Coret		11		/* co-expression returned */
#define A_Cofail	12		/* co-expression failed */


/*
 * Code expansions for exits from C code for top-level routines.
 */
#define Fail		return A_Failure
#define Return		return A_Return
/* >suspend */
#define Suspend  { \
   int rc; \
   if ((rc = interp(G_Csusp,cargp)) != A_Resumption) \
      return rc; \
   }
/* <suspend */


/*
 * Macros for pushing stuff on the stack:
 *
 *  PushDesc(d) -- Push descriptor d
 *  PushNull    -- Push the null descriptor
 *  PushVal(v)  -- Push the integer value v as a single word
 */
#define PushDesc(d)	{sp++; *sp = ((d).dword); sp++;*sp =((d).vword.integr);}
#define PushNull	{sp++; *sp = D_Null; sp++; *sp = 0;}
#define PushVal(v)	{sp++; *sp = (word)(v);}

/*
 * Codes returned by invoke() to indicate action.
 */
#define I_Builtin	201	/* A built-in routine is to be invoked */
#define I_Goal_Fail	202	/* goal-directed evaluation failed */
#define I_Continue	203	/* Continue execution in the interp loop */
#define I_Vararg	204	/* A function with a variable number of args */

/*
 * Codes returned by type conversion routines.
 */
#define Cvt		1
#define NoCvt		2

/*
 * Offset in word of cset bit.
 */
#define CsetOff(b)	((b) & BitOffMask) 
/*
 * Address of word of cset bit.
 */
#define CsetPtr(b,c)	((c) + (((b)&0377) >> LogIntSize)) 
/*
 * Set bit b in cset c.
 */
#define Setb(b,c)	(*CsetPtr(b,c) |= (01 << CsetOff(b))) 
/*
 * Test bit b in cset c.
 */
#define Testb(b,c)	((*CsetPtr(b,c) >> CsetOff(b)) & 01) 

/*
 * Handy sizeof macros:
 *
 *  Wsizeof(x)  -- Size of x in words.
 *  Vwsizeof(x) -- Size of x in words, minus the size of a descriptor.  Used
 *		    when structures have a potentially null list of descriptors
 *		    at their end.
 */
#define Wsizeof(x)	((sizeof(x) + sizeof(word) - 1) / sizeof(word))
#define Vwsizeof(x)	((sizeof(x) - sizeof(struct descrip) +sizeof(word) - 1) / sizeof(word))

/*
 * Generator types.
 */
#define G_Csusp		1
#define G_Esusp		2
#define G_Psusp		3

/*
 * Externs
 */


extern word *sp;
extern word *stack;
extern struct pf_marker *pfp;
extern struct ef_marker *efp;
extern struct gf_marker *gfp;
extern word *ipc;
extern struct descrip *argp;
extern int ilevel;

#ifdef RunStats
#include "../h/rstats.h"
#else RunStats
#define Inc(x)
#define IncSum(s,x)
#endif RunStats
