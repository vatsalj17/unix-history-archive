/*
 *  M M D F . H 
 *
 *  EE/CIS Computer Lab
 *  Department of Computer and Information Sciences
 *  Department of Electrical Engineering
 *  University of Delaware
 *
 *  REVISION HISTORY:
 *
 *  $Revision: 1.4 $
 *
 *  $Log:	mmdf.h,v $
 * Revision 1.4  86/01/02  16:23:51  galvin
 * Add some defines from "util.h" since we can not include
 * it directly.  What a pity.
 * 
 * Revision 1.3  85/12/18  13:24:03  galvin
 * Add declaration of sentprotect.
 * 
 * Revision 1.2  85/11/20  14:37:19  galvin
 * First set of needed MMDF global variables.  We can read the maildrop now.
 * 
 * Revision 1.1  85/11/18  13:18:42  galvin
 * Added comment header for revision history.
 * 
 *
 */

#ifndef FILE
#include <stdio.h>
#endif

/*
 * Global declarations needed to use some MMDF routines.
 */

extern char	*delim1,	/* message prefix delimiter */
		*delim2,	/* message suffix delimiter */
		*mldflfil,	/* default mailbox file */
		*mldfldir;	/* directory containing mailbox file */

extern FILE	*lk_fopen();
extern char	sentprotect;	/* default protection for mail files */

/*
 * We pick some of "util.h" to put here.  Would be nice to just include
 * it directly but it includes many files which are otherwise include
 * be Berkeley and the hack to undo it is not worth it.
 */

#define	isnull(chr)	((chr) == '\0')

#define YES     1
#define NO      0
#define OK      0
#define NOTOK   -1
