/*       Copyright (c) 1987 AT&T   */
/*       All Rights Reserved       */

/*       THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T   */
/*       The copyright notice above does not evidence any      */
/*       actual or intended publication of such source code.   */

#include <sccsid.h>
HVERSION(label, @(#)label.h	1.1.1.2	(5/12/87));


#define LABEL_HEIGHT	20	/* height of label area */

/* the following are icon position in the label area */

#define L_HOST_POSITION		1
#define L_MUX_POSITION		2
#define L_PRINT_POSITION	3
#define L_SCROLL_POSITION	4
#define L_CAP_POSITION		5
#define L_USER_POSITION		6	/* first space the user can use */

/* where the text appears in the label */

#define L_LEFT		0
#define L_CENTER	1
#define L_RIGHT		2

