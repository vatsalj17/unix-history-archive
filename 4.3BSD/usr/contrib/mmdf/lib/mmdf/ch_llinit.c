/*
 *	ch_llinit()
 *
 *	Initialize per channel logging.  Called after mmdf_init and
 *	determination of current channel by each channel program
 *	to activate channel specific logging.
 */
#include "util.h"
#include "mmdf.h"
#include "ch.h"

extern	Llog	*logptr;
extern	char	*logdfldir;
extern	char	*dupfpath();

ch_llinit (curchan)
Chan *curchan;
{
	logptr -> ll_file = curchan -> ch_logfile;
	if (logdfldir != (char *) 0)
		logptr -> ll_file = dupfpath (logptr -> ll_file, logdfldir);
	if (logptr -> ll_level < curchan -> ch_loglevel)
		logptr -> ll_level = curchan -> ch_loglevel;
	
	ll_close (logptr);	/* Will cause new values to take effect */
}
