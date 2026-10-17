#

/*
 * Memory special file
 * minor device 0 is physical memory
 * minor device 1 is kernel memory
 * minor device 2 is EOF/BITBUCKET
 */

/*** Rewritten for Interdata 7-8/32
 *** Code is made simpler (and less portable!) by the assumption
 *** that all physical memory is contiguous and directly addressable.
 ***/

#include "param.h"
#include "user.h"
#include "conf.h"
#include "seg.h"
#include "systm.h"

mmread(dev)
{
	char *addr;

	if(dev.d_minor == 2)
		return;
	do {
		addr = u.u_offset[1];
		if ((dev.d_minor == 1 && lraddr(&addr, kisa))
		    || addr >= memtop)
			break;
	} while(u.u_error==0 && passc(*addr) >= 0);
}

mmwrite(dev)
{
	char *addr;
	register c;

	if(dev.d_minor == 2) {
		c = u.u_count;
		u.u_count = 0;
		u.u_base =+ c;
		dpadd(u.u_offset, c);
		return;
	}
	for (;;) {
		addr = u.u_offset[1];
		if ((c=cpass())<0 || u.u_error!=0)
			break;
		if ((dev.d_minor == 1 && lraddr(&addr, kisa))
		    || addr >= memtop)
			break;
		*addr = c;
	}
}
