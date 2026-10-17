/*
 * Configurable information for the patch driver.
 */
#define __KERNEL__	1

#include <sys/patch.h>
#include <sys/compat.h>
#include <sys/con.h>
#include <kernel/param.h>

/*
 * These devices need deferred startup during installation.
 */
extern CON	atcon;		/* IDE hard disk. */
extern CON	sscon;		/* Seagate/Future Domain hard disk. */
extern CON	scsicon;	/* Hai SCSI driver. */

#define PATCHABLE_VAR(var)	{ STRING(var), (char *)(&(var)), sizeof(var) }
#define PATCHABLE_CON(var)	{ STRING(var), &(var) }

extern unsigned long _bar;
extern int ronflag;
extern unsigned long _entry;
extern int kb_lang;

extern unsigned int NSDRIVE;
extern unsigned int SS_INT;
extern unsigned int SS_BASE;

extern int haiss_base;
extern int haiss_intr;
extern int haiss_type;

extern unsigned short hai154x_base;
extern unsigned short hai154x_intr;
extern unsigned short hai154x_dma;
extern unsigned short hai154x_haid;

extern int HAI_DISK;
extern int ATSREG;
extern short at_drive_ct;
extern int fl_dsk_ch_prob;
extern int HAI_HAINDEX;

struct patchVarInternal	patchVarTable [] = {
	PATCHABLE_VAR(_bar),
	PATCHABLE_VAR(ronflag),
	PATCHABLE_VAR(_entry),
	PATCHABLE_VAR(kb_lang),
	PATCHABLE_VAR(NSDRIVE),
	PATCHABLE_VAR(SS_INT),
	PATCHABLE_VAR(SS_BASE),
	PATCHABLE_VAR(haiss_type),
	PATCHABLE_VAR(haiss_intr),
	PATCHABLE_VAR(haiss_base),
	PATCHABLE_VAR(hai154x_base),
	PATCHABLE_VAR(hai154x_intr),
	PATCHABLE_VAR(hai154x_haid),
	PATCHABLE_VAR(hai154x_dma),
	PATCHABLE_VAR(HAI_DISK),
	PATCHABLE_VAR(HAI_HAINDEX),
	PATCHABLE_VAR(ATSREG),
	PATCHABLE_VAR(at_drive_ct),
	PATCHABLE_VAR(fl_dsk_ch_prob)
};

int	patchVarCount = sizeof(patchVarTable)/sizeof(patchVarTable[0]);

struct patchConInternal	patchConTable [] = {
	PATCHABLE_CON(atcon),
	PATCHABLE_CON(sscon),
	PATCHABLE_CON(scsicon)
};

int	patchConCount = sizeof(patchConTable)/sizeof(patchConTable[0]);
