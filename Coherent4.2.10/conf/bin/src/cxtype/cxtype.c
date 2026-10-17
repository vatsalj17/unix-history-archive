/*
 * cxtype.c - return type of ndp in use - mainly for shell scripts
 *
 * Usage:  /etc/conf/bin/cxtype
 *
 * Revised: Mon Aug  9 14:12:25 1993 CDT
 */

/*
 * ----------------------------------------------------------------------
 * Includes.
 */
#include <stdio.h>
#include <sys/compat.h>
#include <kernel/param.h>
#include <sys/cyrix.h>

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
	char		* message;

	/* Do a cohcall to get floating point hardware type. */
	result = cohcall(COH_GETCYRIX);

	if (result >= 0) {

		/* System call succeeded. */
		switch (result) {

			case CYRIX_OEM:
				message = "CxOEM";
				break;

			case CYRIX_NOT:
				message = "CxNOT";
				break;

			default:
			switch (result & 0xFF) {

				case CYRIX_SRX:
					message = "Cx486_SRx";
					break;

				case CYRIX_DRX:
					message = "Cx486_DRx";
					break;

				case CYRIX_SRX2:
					message = "Cx486_SRx2";
					break;

				case CYRIX_DRX2:
					message = "Cx486_DRx2";
					break;

				default:
					fprintf(stderr,
					  "%s: unexpected result %x from COH_GETCYRIX\n",
					  argv[0], result);
					exit(1);
					break;
			}
		}
	} else {
		perror(argv[0]);
		return 1;
	}

	puts(message);
	return 0;
}
