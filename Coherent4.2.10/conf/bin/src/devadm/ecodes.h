#ifndef	ECODES_H
#define	ECODES_H

/*
 * Table of error codes for return from 'devadm' and their meanings.
 */

enum {
	NO_ERROR,
	FORMAT_ERROR,		/* error in input data format */
	MISSING_FILE,		/* required input file missing */
	CANNOT_UPDATE,		/* cannot write output properly */
	LEX_ERROR,		/* problem lexing input */
	NO_MEMORY,		/* unable to malloc () storage */
	INTERNAL_ERROR		/* internal assertion failure */
};

#endif	/* ! defined (ECODES_H) */
