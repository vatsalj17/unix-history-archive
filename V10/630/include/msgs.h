/*       Copyright (c) 1987 AT&T   */
/*       All Rights Reserved       */

/*       THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T   */
/*       The copyright notice above does not evidence any      */
/*       actual or intended publication of such source code.   */

#include <sccsid.h>
HVERSION(msgs, @(#)msgs.h	1.1.1.2	(5/12/87));

/*
 * Character-driven state machine information for DMD to Unix communication.
 */

#define	C_SENDCHAR	1	/* Send character to layer process */
#define	C_NEW		2	/* Create new layer process group */
#define	C_UNBLK		3	/* Unblock layer process */
#define	C_DELETE	4	/* Delete layer process group */
#define	C_EXIT		5	/* Exit */
#define	C_BRAINDEATH	6	/* Send terminate signal to proc. group */
#define	C_SENDNCHARS	7	/* Send several characters to layer proc. */
#define	C_RESHAPE	8	/* Layer has been reshaped */

/*
 * SENDCHAR goes through canon, SENDNCHARS does not.
 * Usual format is: [command][data]
*/

/* 630 additionals */

#define W_JWIN		16	/* Window with label has been reshaped */

#define S_NONMUX	32	/* Clear the IO stream for nonmux environment */
#define S_MUX		33	/* Initiallize IO stream for mux environment */
#define S_NMLOAD	34	/* Set up stream for nonmux download */
#define S_OLYLOAD	35	/* Set up stream for overlay download */
#define S_HANGUP	36	/* Connectivity is lost, try to recover */

#define K_STRTBRK	64	/* Start Break */
#define K_STOPBRK	65	/* Stop Break */
#define K_DISCON	66	/* Disconnect */
#define K_STOPDISC	67	/* Stop Disconnect */

