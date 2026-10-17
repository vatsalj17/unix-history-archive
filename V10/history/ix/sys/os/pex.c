#include "sys/param.h"
#include "sys/user.h"
#include "sys/stream.h"
#include "sys/proc.h"
#include "sys/inode.h"
#include "sys/filio.h"
#include "sys/ttyio.h"
#include "sys/label.h"
#include "sys/pex.h"
#include "sys/log.h"

#define setpex(ip,k) (ip)->i_pexflag = ((ip)->i_pexflag&~PEX_STATE)|(k)
#define pexstate(ip) ((ip)->i_pexflag&PEX_STATE)

extern struct inode *otherend();

pexioc(ip, cmd, arg, timid)
register struct inode *ip;
caddr_t arg;
{
	register struct stdata *sp;
	register struct proc *p;
	register int impure = 0;
	register struct inode *xp;
	struct pexclude x;

	x.oldnear = ip->i_pex ? FIOPX : FIONPX;
	xp = otherend(ip);
	sp = ip->i_sptr;
	
	if(cmd==FIOPX||cmd==FIONPX) {
		if(ip->i_pexflag&ISLOGFILE) {
			u.u_error = EPERM;
			return;
		}
		/*
		 * user may not change pex state of non empty streams
		 */
		if(sp != 0 && timid != 0 
		&& (
			(cmd==FIOPX&&pexstate(ip)!=PEX1) 
		     || (cmd==FIONPX&&pexstate(ip)!=PEX0)
		)
		&& sqlen(sp)>0) {
			u.u_error = EBUSY;
			return;
		}
		ip->i_pexflag &= ~PEX_SLEEP;
		if(xp == 0) {
			if(cmd == FIOPX && ((sp==0)||(ip->i_pexflag&PEX_APX))) {
				ip->i_pex = u.u_procp;
				setpex(ip, PEX1);
			} else if(cmd == FIOPX) {
				impure = 1;
				u.u_error = EPERM;
			} else if(cmd == FIONPX) {
				ip->i_pex = 0;
				setpex(ip, PEX0);
			}
			ip->i_pexflag &= ~PEX_IMPURE;
		} else { /* pipe case harder */
			if(cmd == FIOPX) {
				if(pexstate(xp)==PEX2)
					u.u_error = ECONC;
				else if(pexstate(ip)==PEX2)
					;
				else {
					ip->i_pex = u.u_procp;
					setpex(ip, PEX1);
				}
			} else if(cmd == FIONPX) {
				if(pexstate(ip)==PEX1 && pexstate(xp)==PEX1)
					setpex(xp, PEX2);
				ip->i_pex = 0;
				setpex(ip, PEX0);
			}
			if(pexstate(ip)==PEX2||pexstate(ip)!=pexstate(xp)) {
				impure = 1;
				ip->i_pexflag |= PEX_SLEEP;
				ip->i_pexflag |= PEX_IMPURE;
				xp->i_pexflag |= PEX_IMPURE;
			} else {
				ip->i_pexflag &= ~PEX_IMPURE;
				xp->i_pexflag &= ~PEX_IMPURE;
			}
		}
	}

	switch(cmd) {
	case FIOPX:
	case FIONPX:
		if(sp!=0 && impure!=0) {
			stioctl(ip, TIOCXFLUSH, (char*)0);
			selwakeup((struct proc *)0, 1);
			wachetauf(ip);
			if((xp=otherend(ip)) != 0) {
				wachetauf(xp);
				xp = otherend(ip);
			}
		}
		if(xp != 0) {
			xp->i_pexflag &= ~PEX_SLEEP;
			wakeup((caddr_t)&xp->i_pex);	
			if(ip->i_pexflag&PEX_SLEEP)
				(void)tsleep((caddr_t)&ip->i_pex, PWAIT-1, 30);
		}
		break;
	case FIOAPX:
	case FIOANPX:
		if(xp!=0 || sp==0)
			u.u_error = ENOTTY;
		else if(jl_t(u.u_procp->p_lab) & T_EXTERN) {
			if(cmd==FIOAPX)
				ip->i_pexflag |= PEX_APX;
			else
				ip->i_pexflag &= ~PEX_APX;
		}
		else
			u.u_error = EPRIV;
		if(loggable(LX))
			seclog(LOG_EXTERN, 60, "C1", cmd);
		break;
	}
	x.newnear = ip->i_pex ? FIOPX : FIONPX;
	xp = otherend(ip);
	if(xp == 0) {
		x.farpid = -1;
		x.farcap = -1;
		x.faruid = -1;
	} else if((p=xp->i_pex) == 0) {
		x.farpid = 0;
		x.farcap = -1;
		x.faruid = -1;
	} else {
		x.farpid = p->p_pid;
		x.farcap = jl_t(p->p_lab);
		x.faruid = p->p_uid;
	}
	if(timid)
		u.u_r.r_val1 = (ip->i_pexflag&PEX_IMPURE) ? 1 : 0;
	if(arg!=0)
		copyout((caddr_t)&x, arg, sizeof(x));
}

wachetauf(ip)
struct inode *ip;
{
	struct stdata *sp = ip->i_sptr;
	if(ip->i_pexflag&PEX_IMPURE) wakeup((caddr_t)sp->wrq);
	if(ip->i_pexflag&PEX_IMPURE) wakeup((caddr_t)RD(sp->wrq));
}
