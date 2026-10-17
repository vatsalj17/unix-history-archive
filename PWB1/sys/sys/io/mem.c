#
/*
 *	Memory special file
 *	minor device 0 is physical memory
 *	minor device 1 is kernel memory
 *	minor device 2 is EOF/NULL
 */

#include "../hd/param.h"
#include "../hd/user.h"
#include "../hd/seg.h"

mmread(dev)
{
	register c, bn, on;
	int a, d;

	if(dev == 2)
		return;
	do {
		bn = lshift(u.u_offset, -6);
		on = u.u_offset[1] & 077;
		a = UISA->r[0];
		d = UISD->r[0];
		spl7();
		UISA->r[0] = bn;
		UISD->r[0] = 077406;
		if(dev == 1)
			UISA->r[0] = (ka6-6)->r[(bn>>7)&07] + (bn & 0177);
		if ((c = fuibyte(on)) < 0)
			u.u_error = ENXIO;
		UISA->r[0] = a;
		UISD->r[0] = d;
		spl0();
	} while(u.u_error==0 && passc(c)>=0);
}

mmwrite(dev)
{
	register c, bn, on;
	int a, d;

	if(dev == 2) {
		c = u.u_count;
		u.u_count = 0;
		u.u_base =+ c;
		dpadd(u.u_offset, c);
		return;
	}
	for(;;) {
		bn = lshift(u.u_offset, -6);
		on = u.u_offset[1] & 077;
		if ((c=cpass())<0 || u.u_error!=0)
			break;
		a = UISA->r[0];
		d = UISD->r[0];
		spl7();
		UISA->r[0] = bn;
		UISD->r[0] = 077406;
		if(dev == 1)
			UISA->r[0] = (ka6-6)->r[(bn>>7)&07] + (bn & 0177);
		if (suibyte(on, c) < 0)
			u.u_error = ENXIO;
		UISA->r[0] = a;
		UISD->r[0] = d;
		spl0();
	}
}
