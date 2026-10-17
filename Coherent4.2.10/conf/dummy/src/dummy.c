int dummyInt = 0x98547632;
short dummyShort = 0x9876;
char dummyChar = 'Z';

/*
 * KER/conf/dummy.c
 *
 * dummy device driver
 *
 * Revised: Fri Jul 23 12:42:16 1993 CDT
 */

/*
 * -----------------------------------------------------------------
 * Includes.
 */

#include <sys/coherent.h>
#include <sys/uproc.h>
#include <sys/cmn_err.h>
#include <sys/errno.h>

/*
 * -----------------------------------------------------------------
 * Definitions.
 *	Constants.
 *	Macros with argument lists.
 *	Typedefs.
 *	Enums.
 */

/*
 * -----------------------------------------------------------------
 * Functions.
 *	Import Functions.
 *	Export Functions.
 *	Local Functions.
 */

/*
 * Configuration functions (local functions).
 */
static void dummyclose();
static void dummyioctl();
static void dummyload();
static void dummyopen();
static void dummyread();
static void dummyunload();
static void dummywrite();

/*
 * Support functions (local functions).
 */

/*
 * -----------------------------------------------------------------
 * Global Data.
 *	Import Variables.
 *	Export Variables.
 *	Local Variables.
 */

/*
 * Configuration table (export data).
 */
CON dummycon ={
	DFCHR,				/* Flags */
	0,				/* Major index */
	dummyopen,			/* Open */
	dummyclose,			/* Close */
	NULL,				/* Block */
	dummyread,			/* Read */
	dummywrite,			/* Write */
	dummyioctl,			/* Ioctl */
	NULL,				/* Powerfail */
	NULL,				/* Timeout */
	dummyload,			/* Load */
	dummyunload,			/* Unload */
	NULL				/* Poll */
};

/*
 * -----------------------------------------------------------------
 * Code.
 */

/*
 * dummyload()
 */
static void
dummyload()
{
	cmn_err(CE_NOTE, "Dummy: load");
}

/*
 * dummyunload()
 */
static void
dummyunload()
{
	cmn_err(CE_NOTE, "Dummy: unload");
}

/*
 * dummyopen()
 */
static void
dummyopen(dev, mode)
dev_t dev;
int mode;
{
	cmn_err(CE_NOTE, "Dummy: open");
}

/*
 * dummyclose()
 */
static void
dummyclose(dev, mode)
dev_t dev;
int mode;
{
	cmn_err(CE_NOTE, "Dummy: close");
}

/*
 * dummyread()
 */
static void
dummyread(dev, iop)
dev_t dev;
register IO * iop;
{
	cmn_err(CE_NOTE, "Dummy: read");
}

/*
 * dummywrite()
 */
static void
dummywrite(dev, iop)
dev_t dev;
register IO * iop;
{
	cmn_err(CE_NOTE, "Dummy: write");
}

/*
 * dummyioctl()
 */
static void
dummyioctl(dev, com, vec)
dev_t	dev;
int	com;
struct sgttyb *vec;
{
	cmn_err(CE_NOTE, "Dummy: ioctl");
}
