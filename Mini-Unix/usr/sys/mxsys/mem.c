#
/*
 */

/*
 *	Memory special file
 *	minor device 0 is physical memory
 *	minor device 1 is kernel memory
 *	minor device 2 is EOF/RATHOLE
 */

#include "param.h"
#include "user.h"
#include "conf.h"

mmread(dev)
{
	register c;

	if(dev.d_minor == 2)
		return;
	do {
		c = fuibyte(u.u_offset[1]);
	} while(u.u_error==0 && passc(c)>=0);
}

mmwrite(dev)
{
	register c;

	if(dev.d_minor == 2) {
		c = u.u_count;
		u.u_count = 0;
		u.u_base =+ c;
		dpadd(u.u_offset, c);
		return;
	}
	for(;;) {
		if ((c=cpass())<0 || u.u_error!=0)
			break;
		suibyte(u.u_offset[1]-1, c);
	}
}
