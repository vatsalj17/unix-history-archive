/* $Header: /ker/coh.386/RCS/sys4.c,v 2.3 93/08/19 03:26:55 nigel Exp Locker: nigel $ */
/*
 * Non-Sytem V (compatibility) system calls introduced by the 386 port.
 *
 * $Log:	sys4.c,v $
 * Revision 2.3  93/08/19  03:26:55  nigel
 * Nigel's r83 (Stylistic cleanup)
 * 
 * Revision 2.2  93/07/26  14:29:12  nigel
 * Nigel's R80
 * 
 * Revision 1.2  92/01/06  12:00:58  hal
 * Compile with cc.mwc.
 */

#include <sgtty.h> 

int ustty(); 
int ugtty(); 

/*
 *
 */ 

int ustty(fd, sp)
int fd;
struct sgttyb * sp;
{
	return uioctl(fd, TIOCSETP, sp);
}

int ugtty(fd, sp)
int fd;
struct sgttyb * sp;
{
	return uioctl(fd, TIOCGETP, sp);
}
