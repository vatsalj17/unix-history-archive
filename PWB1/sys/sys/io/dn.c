#
/*
 * DN-11 ACU interface
 */

#include "../hd/param.h"
#include "../hd/user.h"

struct {
	int	dn_reg[4];
};

extern	dn_addr[], dn_cnt;

#define	PWI	0100000
#define	ACR	040000
#define	DLO	010000
#define	DONE	0200
#define	IENABLE	0100
#define	DSS	040
#define	PND	020
#define	MENABLE	04
#define	DPR	02
#define	CRQ	01

#define	DNPRI	5

dnopen(dev)
register dev;
{
	register *dp;

	if (dev >= dn_cnt ||
	   (dp = dn_addr[dev>>2])->dn_reg[dev&03]&(PWI|DLO))
		u.u_error = ENXIO;
	else {
		*dp =| MENABLE;
		dp->dn_reg[dev&03] = IENABLE|MENABLE|CRQ;
	}
}

dnclose(dev)
{
	dn_addr[dev>>2]->dn_reg[dev&03] = MENABLE;
}

dnwrite(dev)
register dev;
{
	register c, *dp;
	extern lbolt;

	dp = &(dn_addr[dev>>2]->dn_reg[dev&03]);
	while ((*dp & (PWI|ACR|DSS)) == 0) {
		spl5();
		if ((*dp&DONE) == 0) {
			sleep(dn_addr[dev>>2], DNPRI);
		} else if (u.u_count == 0) {
			*dp = IENABLE|MENABLE|CRQ;
		} else if ((c=cpass()) == '-') {
			sleep(&lbolt, DNPRI);
			sleep(&lbolt, DNPRI);
		} else if (*dp&PND) {
			*dp = (c<<8)|IENABLE|MENABLE|DPR|CRQ;
		}
		spl0();
	}
	if (*dp&(PWI|ACR))
		u.u_error = EIO;
	*dp = MENABLE|CRQ;
}

dnintr(dev)
{
	wakeup(dn_addr[dev]);
}
