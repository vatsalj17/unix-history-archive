/*
 *  Versatec matrix printer/plotter dma interface driver
 */

#include "../hd/param.h"
#include "../hd/user.h"
#include "../hd/buf.h"
#include "../hd/peri.h"

#define	VPPRI	10

struct {
	int	plbcr, pbae, prbcr, pbaddr;
	int	plcsr, plbuf, prcsr, prbuf;
};

extern vp_addr[], vp_cnt;

/* status bits */
#define	ERROR	0100000
#define	READY	0200
#define	IENABLE	0100
#define	FFCOM	020
#define	RESET	02
#define	SPP	01

/*states */
#define	PRINT	0100
#define	PLOT	0200
#define	PPLOT	0400

vpopen(dev)
register dev;
{
	register *vp, *vpaddr;

	if (dev>=vp_cnt || (vp = &vp_vp[dev])->vp_buf) {
		u.u_error = ENXIO;
		return;
	}
	vpaddr = &(vp_addr[dev]->plcsr);
	*vpaddr = IENABLE|RESET;
	vp->vp_state = PRINT;
	if (vpready(dev)<0) {
		*vpaddr = 0;
		u.u_error = EIO;
		return;
	}
	*vpaddr = IENABLE|FFCOM;
	vp->vp_buf = getblk(NODEV);
	vp->vp_offset = vp->vp_count = 0;
}

vpclose(dev)
{
	register *vp, *vpaddr;

	vp = &vp_vp[dev];
	vpaddr = &(vp_addr[dev]->plcsr);
	vpready(dev);
	*vpaddr = 0;
	brelse(vp->vp_buf);
	vp->vp_buf = 0;
}

vpwrite(dev)
register dev;
{
	register *vp, *vpaddr;

	vp = &vp_vp[dev];
	vpaddr = vp_addr[dev];
	if (vp->vp_count) {
		u.u_error = EIO;
		return;
	}
	while (u.u_count && u.u_error==0) {
		vp->vp_count = min(256,u.u_count);
		iomove(vp->vp_buf, vp->vp_offset, vp->vp_count, B_WRITE);
		if (vpready(dev)<0) {
			u.u_error = EIO;
			vpaddr->plcsr = IENABLE|RESET;
			break;
		}
		vpaddr->pbaddr = vp->vp_buf->b_addr + vp->vp_offset;
		if (vp->vp_state&PLOT)
			vpaddr->plbcr = vp->vp_count; else
			vpaddr->prbcr = vp->vp_count;
		vp->vp_offset = 256 - vp->vp_offset;
		if (vp->vp_state&PPLOT)
			vp->vp_state = PLOT;
	}
	vp->vp_count = 0;
}

vpready(dev)
{
	register *vp, *vpaddr;

	vp = &vp_vp[dev];
	vpaddr = (vp->vp_state&PLOT)?&(vp_addr[dev]->plcsr):
		&(vp_addr[dev]->prcsr);
	spl4();
	while ((*vpaddr&(READY|ERROR))==0)
		sleep(vp, VPPRI);
	spl0();
	return(*vpaddr);
}

vpsgtty(dev, av)
int *av;
{
	register *vp, *vpaddr;
	register bit;

	vp = &vp_vp[dev];
	vpaddr = &(vp_addr[dev]->plcsr);
	if (av) {
		*av = vp->vp_state;
		return;
	}
	vpready(dev);
	vp->vp_state = u.u_arg[0];
	if (vp->vp_state&PPLOT)
		*vpaddr = IENABLE|SPP; else
		*vpaddr = IENABLE;
	for (bit=2; bit<IENABLE; bit=<<1)
		if (vp->vp_state&bit) {
			vpready(dev);
			*vpaddr =| bit;
			vp->vp_state =& ~bit;
		}
}

vpintr(dev)
{
	wakeup(&vp_vp[dev]);
}
