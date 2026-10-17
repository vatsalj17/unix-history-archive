/*       Copyright (c) 1987 AT&T   */
/*       All Rights Reserved       */

/*       THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T   */
/*       The copyright notice above does not evidence any      */
/*       actual or intended publication of such source code.   */

#include <sccsid.h>
HVERSION(global, @(#)global.h	1.1.1.2	(5/12/87));

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
#define	topbits		P_Array(int, 731)
#define	ver_int		I_Ref(long, 750)
#define	ver_str		P_Array(char, 751)

/* BSS ENTRIES */

#define Msg_queue	D_Ref(struct msqid_ds *, -35)
#define P		D_Ref(struct Proc *, -38)
#define Brammax         D_Ref(char *, -73)
#define Brammin         D_Ref(char *, -74)
#define Rammax          D_Ref(char *, -76)
#define Rammin          D_Ref(char *, -123)
#define Rommax          D_Ref(char *, -124)
#define Rommin          D_Ref(char *, -125)
#define alloclevel	D_Ref(long *, -47)
#define alloclimit	D_Ref(long *, -48)
#define debugger	D_Ref(struct Proc *, -119)
#define gclevel		D_Ref(long *, -54)
#define kbdid		D_Ref(int, -101)
#define kbdproc		D_Ref(struct Proc *, -57)
#define memendp		D_Ref(long *, -65)
#define memstartp	D_Ref(long *, -66)

