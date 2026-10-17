/*       Copyright (c) 1987 AT&T   */
/*       All Rights Reserved       */

/*       THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T   */
/*       The copyright notice above does not evidence any      */
/*       actual or intended publication of such source code.   */

#include <sccsid.h>
HVERSION(mpx, @(#)mpx.h	1.1.1.11	(5/12/87));

#ifndef	MPX_H
#define	MPX_H	MPX_H

#ifndef	LAYER_H
#include <layer.h>
#endif

#ifndef	PROC_H
#include <dmdproc.h>
#endif



/***** START of globals ******/
extern long _addrSys;
typedef int (*ptr_fint)();
#define Sys		((ptr_fint *)_addrSys)
#define I_Ref(t,i)	(*(t *)(Sys[i]))
#define P_Array(t,i)	((t *)(Sys[i]))
#define D_Ref(t,i)	(*(t *)&(Sys[i]))
#define DP_Ref(t,i)	((t *)&(Sys[i]))
#define DD_Ref(t,o,i)	(((t *)&Sys[i])[o])


/* PTR ENTRIES */

#define	B_cap		I_Ref(struct Bitmap, 2)
#define	B_checkmark	I_Ref(struct Bitmap, 3)
#define	B_down		I_Ref(struct Bitmap, 4)
#define	B_h1		I_Ref(struct Bitmap, 5)
#define	B_h2		I_Ref(struct Bitmap, 6)
#define	B_insert	I_Ref(struct Bitmap, 7)
#define	B_local		I_Ref(struct Bitmap, 8)
#define	B_mux		I_Ref(struct Bitmap, 9)
#define	B_print		I_Ref(struct Bitmap, 10)
#define	B_rtarrow	I_Ref(struct Bitmap, 11)
#define	B_scroll	I_Ref(struct Bitmap, 12)
#define	B_up		I_Ref(struct Bitmap, 15)
#define	C_arrows	I_Ref(struct Texture16, 17)
#define	C_insert	I_Ref(struct Texture16, 20)
#define	C_cup		I_Ref(struct Texture16, 36)
#define	C_deadmouse	I_Ref(struct Texture16, 37)
#define	C_skull		I_Ref(struct Texture16, 38)
#define	C_target	I_Ref(struct Texture16, 39)
#define	T_background	I_Ref(struct Texture16, 72)
#define	T_black		I_Ref(struct Texture16, 73)
#define	T_checks	I_Ref(struct Texture16, 74)
#define	T_grey		I_Ref(struct Texture16, 75)
#define	T_grey2		I_Ref(struct Texture16, 76)
#define	T_white		I_Ref(struct Texture16, 77)
#define	botbits		P_Array(int, 176)
#define	mediumfont	I_Ref(struct Font, 239)
#define	physical	I_Ref(struct Bitmap, 254)
#define	largefont	I_Ref(struct Font, 412)
#define	smallfont	I_Ref(struct Font, 687)
#define syscache        P_Array(struct Cache, 465)
#define	topbits		P_Array(int, 731)
#define	ver_int		I_Ref(long, 750)
#define	ver_str		P_Array(char, 751)
#define useritems	I_Ref(struct Titem1, 796)

/* BSS ENTRIES */

#define Msg_queue	D_Ref(struct msqid_ds *, -35)
#define P		D_Ref(struct Proc *, -38)
#define Brammax         D_Ref(char *, -73)
#define Brammin         D_Ref(char *, -74)
#define Rammax          D_Ref(char *, -76)
#define Rammin          D_Ref(char *, -123)
#define Rommax          D_Ref(char *, -124)
#define Rommin          D_Ref(char *, -125)
#define Wbufsize        D_Ref(int, -94)
#define alloclevel	D_Ref(long *, -47)
#define alloclimit	D_Ref(long *, -48)
#define debugger	D_Ref(struct Proc *, -119)
#define gclevel		D_Ref(long *, -54)
#define kbdid		D_Ref(int, -101)
#define kbdproc		D_Ref(struct Proc *, -57)
#define memendp		D_Ref(long *, -65)
#define memstartp	D_Ref(long *, -66)
#define prog_env_id     D_Ref(unsigned long, -139)
#define RealMouse	I_Ref(struct Mouse, 479)
/**** END of globals ******/



#define Jdisplayp	(P->uJdisplayp)
#define	Drect		(P->uDrect)
#define mouse		(P->umouse)
#define PtCurrent 	(P->curpt)
#define	display		(*Jdisplayp)

#define button(i)	(clipbttn() & (8 >> i))
#define button1()	(clipbttn() & 4)
#define button2()	(clipbttn() & 2)
#define button3()	(clipbttn() & 1)
#define button12()	(clipbttn() & 6)
#define button13()	(clipbttn() & 5)
#define button23()	(clipbttn() & 3)
#define button123()	(clipbttn() & 7)

/* list of functions returning structures already declared */
Word *		addr();
Bitmap *	balloc();
Rectangle	canon();
Texture16 *	cursswitch();
Texture16 *	Cursswitch();
Point		fPt();
Rectangle	fRpt();
Rectangle	fRect();
Rectangle	inset();
Point		jstring();
Point		sPtCurrent();
Rectangle	newrect();
struct Proc *	peel();
Point		add();
Point		sub();
Point		mul();
Point		div();
Rectangle	raddp();
Rectangle	rsubp();
Point		string();
Point		transform();
Rectangle	rtransform();

/* globals in libraries */
extern Texture16 T_lightgrey;
extern Texture16 T_darkgrey;

/* the following are needed for attach */
#define LOG_HOST1	0
#define LOG_HOST2	1

/* undocumented (but needed) variables, pandora.h */

#ifdef lastkbdid
#undef lastkbdid
/* last time keyboard id was received */
#define lastkbdid       D_Ref(long, -61)
#endif

#endif	/* MPX_H */
