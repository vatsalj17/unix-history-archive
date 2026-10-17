/*
 * Memory sizing.
 */
#ifdef LargeMem
/*
 * Translator-specific defintions.
 */
#define TSize	15000		/* default size of parse tree space */
#define SSize	15000		/* default size of string space */
/*
 * Linker-specific.
 */
#define MaxCode		20000	/* default maximum amount of code/proc */
/*
 * Run-time system specific.
 */
#define MaxAbrSize		51200	/* size of the block region in bytes */
#define MaxStrSpace		51200	/* size of the string space in bytes */
#define MaxStatSize		20480	/* size of the static region in bytes*/
#define MStackSize	        10000	/* size of the main stack in words */
#define StackSize		 2000	/* words in co-expression stack */
#define NumBuf			   10	/* number of i/o buffers available */
#define SSlots			   37	/* number of set slots */
#define TSlots			   37	/* number of table slots */

#else LargeMem

/*
 * Translator-specific.
 */
#define TSize	 7500		/* default size of parse tree space */
#define SSize	 5000		/* default size of string space */
/*
 * Linker-specific.
 */
#define MaxCode	      4000	/* default maximum amount of code/proc */
/*
 * Run-time system specific.
 */
#define MaxAbrSize		10240	/* size of the block region in bytes */
#define MaxStrSpace		10240	/* size of the string space in bytes */
#define MaxStatSize		 4096	/* size of the static region in bytes*/
#define MStackSize	         2000	/* size of the main stack in words */
#define StackSize		 1000	/* words in co-expression stack */
#define NumBuf			    5	/* number of i/o buffers available */
#define SSlots			   13	/* number of set slots */
#define TSlots			   13	/* number of table slots */
#define MaxListSize		 4090	/* Defined if lists are limited in size
					    due to addressing limitations of a
					    particular architecture.  Specified
					    value is the largest list element
					    block that can be made. */
#endif LargeMem
