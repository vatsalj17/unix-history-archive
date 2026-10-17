/*
 * ndptype.c - return type of ndp in use - mainly for shell scripts
 *
 * Usage:  /etc/conf/bin/ndptype
 *
 * Revised: Mon Aug  9 14:12:25 1993 CDT
 */

/*
 * ----------------------------------------------------------------------
 * Includes.
 */
#include <stdio.h>
#include <sys/compat.h>
#include <sys/sysi86.h>

/*
 * ----------------------------------------------------------------------
 * Definitions.
 *	Constants.
 *	Macros with argument lists.
 *	Typedefs.
 *	Enums.
 */

/*
 * ----------------------------------------------------------------------
 * Functions.
 *	Import Functions.
 *	Export Functions.
 *	Local Functions.
 */

/*
 * ----------------------------------------------------------------------
 * Global Data.
 *	Import Variables.
 *	Export Variables.
 *	Local Variables.
 */

/*
 * ----------------------------------------------------------------------
 * Code.
 */

/************************************************************************
 * main
 *
 ***********************************************************************/
#if USE_PROTO
int main (int NOTUSED(argc), char * argv[])
#else
int
main (argc, argv)
int argc;
char * argv[];
#endif
{
	int		result;
	int		data;
	char		* message;

	/* Do a sysi86 to get floating point hardware type. */
	result = sysi86(SI86FPHW, &data);

	if (result >= 0) {

		/* System call succeeded. */
		switch (data) {
			case FP_NO:
				message = "FP_NO";
				break;
			case FP_SW:
				message = "FP_SW";
				break;
			case FP_287:
				message = "FP_287";
				break;
			case FP_387:
				message = "FP_387";
				break;
			default:
				fprintf(stderr,
				  "%s: unexpected result from SI86FPHW\n",
				  argv[0]);
				return 1;
		}
	} else {
		perror(argv[0]);
		return 1;
	}

	puts(message);
	return 0;
}
