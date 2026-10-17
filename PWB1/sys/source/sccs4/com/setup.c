# include	"../hdr/defines.h"

SCCSID(@(#)setup	1.2);

setup(pkt,serial)
register struct packet *pkt;
int serial;
{
	register int n;
	register struct apply *rap;

	pkt->p_apply[serial].a_inline = 1;
	for (n = maxser(pkt); n; n--) {
		rap = &pkt->p_apply[n];
		if (rap->a_inline) {
			pkt->p_apply[pkt->p_idel[n].i_pred].a_inline = 1;
			if (pkt->p_idel[n].i_datetime > pkt->p_cutoff)
				condset(rap,NOAPPLY,CUTOFF);
			else
				condset(rap,APPLY,EMPTY);
		}
		else
			condset(rap,NOAPPLY,EMPTY);
		if (rap->a_code == APPLY)
			ixgsetup(pkt->p_apply,&(pkt->p_idel[n].i_ixg));
	}
}


ixgsetup(ap,ixgp)
struct apply *ap;
struct ixg *ixgp;
{
	int n;
	int code, reason;
	register int *ip;
	register struct ixg *cur, *prev;

	for (cur = ixgp; cur = (prev = cur)->i_next; ) {
		switch (cur->i_type) {

		case INCLUDE:
			code = APPLY;
			reason = INCL;
			break;
		case EXCLUDE:
			code = NOAPPLY;
			reason = EXCL;
			break;
		case IGNORE:
			code = EMPTY;
			reason = IGNR;
			break;
		}
		ip = cur->i_ser;
		for (n = cur->i_cnt; n; n--)
			condset(&ap[*ip++],code,reason);
	}
}


condset(ap,code,reason)
register struct apply *ap;
int code, reason;
{
	if (code == EMPTY)
		ap->a_reason =| reason;
	else if (ap->a_code == EMPTY) {
		ap->a_code = code;
		ap->a_reason =| reason;
	}
}
