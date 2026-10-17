/*
 * bports.c
 *
 * Report com1-4 and lpt1-3 as configured by the BIOS.
 *
 * Revised: Mon Aug 16 07:50:21 1993 CDT
 */

/*
 * ----------------------------------------------------------------------
 * Includes.
 */
#include <sys/compat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

/*
 * ----------------------------------------------------------------------
 * Definitions.
 *	Constants.
 *	Macros with argument lists.
 *	Typedefs.
 *	Enums.
 */
#define MEM_FILE	"/dev/mem"
#define SBP		sizeof(bp)
#define PORTADDR	0x400
#define CMD		"bports"

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

/* How the BIOS keeps port addresses. */
struct bports {
	short com[4];
	short lpt[3];
} bp;

/*
 * ----------------------------------------------------------------------
 * Code.
 */
/************************************************************************
 * usage
 *
 ***********************************************************************/
#if USE_PROTO
void usage(void)
#else
void
usage()
#endif
{
	fprintf(stderr, "Usage:  %s [-lc]\n", CMD);
	exit(1);
}

/************************************************************************
 * main
 *
 ***********************************************************************/
#if USE_PROTO
int main(int argc, char * argv[])
#else
int
main(argc, argv)
int argc;
char * argv[];
#endif
{
	int fd, i, res, argn;
	int op_default = 1;
	int op_com = 0;
	int op_lpt = 0;
	char * cp;

	/* Parse command line options. */
	for (argn = 1; argn < argc; argn++) {
		if (argv[argn][0] == '-') {

			for(cp = argv[argn] + 1; *cp; cp++) {
				switch(*cp) {
				case 'c':
					op_com = 1;
					op_default = 0;
					break;
				case 'l':
					op_lpt = 1;
					op_default = 0;
					break;
				default:
					usage();
				}
			}
		} else
			usage();
	}

	/* Try to open physical memory device. */
	if ((fd = open(MEM_FILE, O_RDONLY)) < 0) {
		fprintf(stderr, "%s: can't open %s\n", CMD, MEM_FILE);
		exit(1);
	}

	/* Try to seek to BIOS RAM area in physical memory. */
	if (lseek(fd, PORTADDR, 0) != PORTADDR) {
		fprintf(stderr, "%s: can't lseek to offset %d in %s\n",
		  CMD, PORTADDR, MEM_FILE);
		exit(1);
	}

	/* Try to read all com and lpt port addresses from physical memory. */
	if ((res = read(fd, &bp, SBP)) != SBP) {
		fprintf(stderr, "%s: could only read %d bytes from %s, "
		  "%d wanted.\n",
		  CMD, res, MEM_FILE, SBP);
		perror("read");
		exit(1);
	}

	/* If default display mode selected, show all ports verbosely. */
	if (op_default) {
		for (i = 0; i < 4; i++)
			printf("com%d = 0x%04x\n", i+1, bp.com[i]);
		for (i = 0; i < 3; i++)
			printf("lpt%d = 0x%04x\n", i+1, bp.lpt[i]);
	}

	/* If com display selected, print com port base addresses tersely. */
	if (op_com) {
		for (i = 0; i < 4; i++)
			if (bp.com[i])
				printf("0x%04x  ", bp.com[i]);
			else
				printf("NONE  ");
		putchar('\n');
	}

	/* If lpt display selected, print com port base addresses tersely. */
	if (op_lpt) {
		for (i = 0; i < 3; i++)
			if (bp.lpt[i])
				printf("0x%04x  ", bp.lpt[i]);
			else
				printf("NONE  ");
		putchar('\n');
	}

	close(fd);
	exit(0);
}
