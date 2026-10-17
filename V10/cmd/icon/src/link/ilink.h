/*
 * External declarations for the linker.
 */

#include "../h/rt.h"

#ifdef ATT3B
#include <sys/types.h>
#include <sys/stat.h>
#endif ATT3B

/*
 * Miscellaneous external declarations.
 */

extern FILE *infile;		/* current input file */
extern FILE *outfile;		/* interpreter output file */
extern FILE *dbgfile;		/* debug file */
extern char inname[];		/* input file name */
extern char outname[];		/* output file name */
extern char *pname;		/* this program name (from command line) */
extern int line;		/* source program line number (from ucode) */
extern char *file;		/* source program file name (from ucode) */
extern int statics;		/* total number of statics */
extern int dynoff;		/* stack offset counter for locals */
extern int argoff;		/* stack offset counter for arguments */
extern int static1;		/* first static in procedure */
extern int nlocal; 		/* number of locals in local table */
extern int nconst;		/* number of constants in constant table */
extern int nrecords;		/* number of records in program */
extern int trace;		/* initial setting of &trace */
extern int Dflag;		/* debug flag */
extern char ixhdr[];		/* header line for direct execution */
extern char *iconx;		/* location of iconx to put in #! line */
extern int hdrloc;		/* location to place hdr block at */
extern struct lfile *lfiles;	/* list of files to link */
extern struct lfile *getlfile();

/*
 * Structures for symbol table entries.
 */

struct lentry {			/* local table entry */
   char *l_name;		/*   name of variable */
   int l_flag;			/*   variable flags */
   union {			/*   value field */
      int staticid;		/*     unique id for static variables */
      word offset;		/*     stack offset for args and locals */
      struct gentry *global;	/*     global table entry */
      } l_val;
   };

struct gentry {			/* global table entry */
   struct gentry *g_blink;	/*   link for bucket chain */
   char *g_name;		/*   name of variable */
   int g_flag;			/*   variable flags */
   int g_nargs; 		/*   number of args or fields */
   int g_procid;		/*   procedure or record id */
   word g_pc;			/*   position in icode of object */
   };

struct centry {			/* constant table entry */
   int c_flag;			/*   type of literal flag */
   union {			/*   value field */
      long ival;		/*     integer */
      double rval;		/*     real */
      char *sval;		/*     string */
      } c_val;
   int c_length;		/*   length of literal string */
   word c_pc;			/*   position in icode of object */
   };

struct ientry {			/* identifier table entry */
   struct ientry *i_blink;	/*   link for bucket chain */
   char *i_name;		/*   pointer to string */
   int i_length;		/*   length of string */
   };

struct fentry {			/* field table header entry */
   struct fentry *f_blink;	/*   link for bucket chain */
   char *f_name;		/*   name of field */
   int f_fid;			/*   field id */
   struct rentry *f_rlist;	/*   head of list of records */
   };

struct rentry {			/* field table record list entry */
   struct rentry *r_link;	/*   link for list of records */
   int r_recid;			/*   record id */
   int r_fnum;			/*   offset of field within record */
   };

/*
 * Structure for linked list of file names to link.
 */
struct lfile {
   struct lfile *lf_link;	/* next file in list */
   char *lf_name;		/* name of file */
   };

/*
 * Flag values in symbol tables.
 */

#define F_Global	    01	/* variable declared global externally */
#define F_Proc		    05	/* procedure (includes GLOBAL) */
#define F_Record	   011	/* record (includes GLOBAL) */
#define F_Dynamic	   020	/* variable declared local dynamic */
#define F_Static	   040	/* variable declared local static */
#define F_Builtin	  0101	/* identifier refers to built-in procedure */
#define F_ImpError  	  0400	/* procedure has default error */
#define F_Argument 	 01000	/* variable is a formal parameter */
#define F_IntLit   	 02000	/* literal is an integer */
#define F_RealLit  	 04000	/* literal is a real */
#define F_StrLit  	010000	/* literal is a string */
#define F_CsetLit 	020000	/* literal is a cset */
#define F_LongLit	040000	/* literal is a long integer */

/*
 * Symbol table region pointers.
 */

extern struct gentry **ghash;	/* hash area for global table */
extern struct ientry **ihash;	/* hash area for identifier table */
extern struct fentry **fhash;	/* hash area for field table */

extern struct lentry *ltable;	/* local table */
extern struct gentry *gtable;	/* global table */
extern struct centry *ctable;	/* constant table */
extern struct ientry *itable;	/* identifier table */
extern struct fentry *ftable;	/* field table headers */
extern struct rentry *rtable;	/* field table record lists */
extern char *strings;		/* string space */
extern word *labels;		/* label table */
extern char *code;		/* generated code space */

extern struct gentry *gfree;	/* free pointer for global table */
extern struct ientry *ifree;	/* free pointer for identifier table */
extern struct fentry *ffree; 	/* free pointer for field table headers */
extern struct rentry *rfree; 	/* free pointer for field table record lists */
extern char *strfree;		/* free pointer for string space */
extern char *codep;		/* free pointer for code space */

extern word lsize;		/* size of local table */
extern word gsize;		/* size of global table */
extern word csize;		/* size of constant table */
extern word isize;		/* size of identifier table */
extern word fsize;		/* size of field table headers */
extern word rsize;		/* size of field table record lists */
extern word ssize;		/* size of string space */
extern word ihsize;		/* size of identifier table hash area */
extern word ghsize;		/* size of global table hash area */
extern word fhsize;		/* size of field table hash area */
extern word maxlabels;		/* maximum number of labels per procedure */
extern word maxcode;		/* maximum amount of code per procedure */

extern int gmask;		/* mask for global table hash */
extern int imask;		/* mask for identifier table hash */
extern int fmask;		/* mask for field table hash */

/*
 * Symbol table parameters.
 */

#define Lsize		 100	/* default size of local table */
#define GSize		 200	/* default size of global table */
#define CSize		 100	/* default size of constant table */
#define ISize		 500	/* default size of identifier table */
#define FSize		 100	/* default size of field table headers */
#define RSize		 100	/* default size of field table record lists */
#define StrSize		5000	/* default size of string space */
#define GhSize		  64	/* default size of global table hash area */
#define IhSize		 128	/* default size of identifier table hash area */
#define FhSize		  32	/* default size of field table hash area */
#define MaxLabels	 500	/* default maximum number of labels/proc */

/*
 * Hash computation macros.
 */

#define ghasher(x)	(((word)x)&gmask)	/* for global table */
#define fhasher(x)	(((word)x)&fmask)	/* for field table */

/*
 * Machine-dependent constants.
 */


#define RkBlkSize 9*WordSize	/* size of record constructor block */
