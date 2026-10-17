/*
 * Definitions and declarations used for storage management.
 */

#define F_Mark		0100000 	/* high-order bit for gc marking */
#define Sqlinc		    128 	/* increment for qualifier pointer space */

/*
 * External definitions.
 */

extern char *currend;			/* current end of memory region */
extern uword blkneed;			/* stated need for block space */
extern uword strneed;			/* stated need for string space */
extern uword statneed;
extern struct descrip *globals; 	/* start of global variables */
extern struct descrip *eglobals;	/* end of global variables */
extern struct descrip *gnames;		/* start of global variable names */
extern struct descrip *egnames; 	/* end of global variable names */
extern struct descrip *statics; 	/* start of static variables */
extern struct descrip *estatics;	/* end of static variables */
extern struct descrip tended[]; 	/* tended descriptors, used by doasgn */
extern struct descrip etended[];	/* end of tended descriptors */
extern struct descrip **quallist; 	/* beginning of qualifier list */
extern struct descrip **qualfree; 	/* qualifier list free pointer */
extern struct descrip **equallist;	/* end of qualifier list */

/*
 * Get type of block pointed at by x.
 */
#define BlkType(x)   (*(word *)x)

/*
 * BlkSize(x) takes the block pointed to by x and if the size of
 *  the block as indicated by bsizes[] is non-zero it returns the
 *  indicated size, otherwise it returns the second word in the
 *  block which should be the size.
 */
#define BlkSize(x) (bsizes[*(word *)x & ~F_Mark] ? \
		     bsizes[*(word *)x & ~F_Mark] : *((word *)x + 1))

/*
 * If memory monitoring is not enabled, redefine function calls
 * to do nothing.
 */
#ifndef MemMon
#define MMAlc(n)
#define MMBGC()
#define MMEGC()
#define MMInit()
#define MMMark(b,t)
#define MMStr(n)
#define MMSMark(a,n)
#define MMType(t)
#endif MemMon
