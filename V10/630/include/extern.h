/*       Copyright (c) 1987 AT&T   */
/*       All Rights Reserved       */

/*       THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T   */
/*       The copyright notice above does not evidence any      */
/*       actual or intended publication of such source code.   */

#include <sccsid.h>
HVERSION(extern, @(#)extern.h	1.1.1.3	(5/12/87));

extern Point		add();
extern Word		*addr();
extern char		*alloc();
extern Bitmap		*balloc();
extern Texture16	*cursswitch();
extern Point		div();
extern Rectangle	inset();
extern Point		jstring();
extern Point		mul();
extern struct Proc	*point2window();
extern Rectangle	raddp();
extern Rectangle	rsubp();
extern Point		string();
extern Point		sub();
extern char		*gcalloc();
extern Rectangle	newrect();
extern Point		transform();
extern Rectangle	rtransform();
extern unsigned long	realtime();
extern Rectangle	canon();
extern char		*itox();
extern long		version();
extern short		Iceil();
extern short		Ifloor();
extern long		Lsqrt();
extern char		*fgets();

extern char		*fontname();
extern struct Font	*fontrequest();
extern char		*fontiname();
extern struct Font	*fontfont();
extern char		*itoc();
extern char		*memcpy();
extern char		*realgcalloc();
extern struct Layer	*whichlayer();
extern long		msgget();
extern struct message_list *rcv_message();
extern struct Proc	*whichproc();


extern Texture16 T_lightgrey;
extern Texture16 T_darkgrey;
extern Texture16 C_sweep;
extern Texture16 C_crosshair;
extern Texture16 C_confirm;

