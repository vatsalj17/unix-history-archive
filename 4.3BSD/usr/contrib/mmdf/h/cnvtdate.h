#
/*  parameters for cnvdate () */

#undef	TIMJUL  1       /* unique id string */
#undef	TIMID   1
#define	TIMCOM	1	/* compact date (yymmddhhmm) */
#define TIMSECS 2       /* full time, down to the second */
#define TIMREG  3       /* rfc 733 standard format */
#define TIMSHRT 4       /* rfc 733 format, without day of week */


extern char *cnvtdate ();
