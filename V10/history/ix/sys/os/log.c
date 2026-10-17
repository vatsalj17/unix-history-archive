/*
 * Kernel logging utilities
 */

#include "sys/param.h"
#include "sys/systm.h"
#include "sys/user.h"
#include "sys/inode.h"
#include "sys/proc.h"
#include "sys/conf.h"
#include "sys/label.h"
#include "sys/log.h"
#include "sys/file.h" /*for FWRITE */

long logmask[5];

/*
 * Turn logging on & off.
 */
syslog()
{
	unsigned v;
	register struct inode *ip;
	register struct a {
		int	command;
		int	which;
		int	val;
	} *uap;
	int mode = FREAD|FWRITE;
	register struct proc *pp;

	uap = (struct a *)u.u_ap;
	if(T_LOG & ~ jl_t(u.u_procp->p_lab))
		u.u_error = EPRIV;
	if(loggable(LL))
		seclog(LOG_LOG, 135, "C124", 
			uap->command, uap->which, uap->val);
	if(u.u_error)
		return;

	v = uap->val;
	switch(uap->command) {
	case LOGOFF:
		if(v >= lgcnt) {
			u.u_error = ENXIO;
			return;
		}
		if (lg[v] == NULL)
			return;
		break;
	case LOGON:
		if(v >= lgcnt) {
			u.u_error = ENXIO;
			return;
		}
		if (lg[v] != NULL) {
			u.u_error = EBUSY;
			return;
		}
		mode = FWRITE; /* to rule out logging onto directories...*/
	case LOGFGET:
	case LOGFSET:
		ip = geti(uap->which, mode, (struct file**)0);
		if(ip == NULL)
			return;
		break;
	default:
		ip = NULL;
		break;
	}
	switch(uap->command) {
	case LOGON:
		lg[v] = ip;
		ip->i_count++;
		ip->i_pexflag |= ISLOGFILE;
		break;
	case LOGOFF:
		if ((ip = lg[v]) != NULL) {
			plock(ip);
			ip->i_pexflag &= ~ISLOGFILE;
			lg[v] = NULL;
			for(v=0; v<lgcnt; v++)
				if(lg[v] == ip)
					ip->i_pexflag |= ISLOGFILE;
			iput(ip);
		}
		break;
	case LOGGET:
		u.u_r.r_val1 = logmask[uap->which];
		break;
	case LOGSET:
		logmask[uap->which] = uap->val;
		break;
	case LOGFGET:
		u.u_r.r_val1 = LOG_AC(ip);
		break;
	case LOGFSET:
		setpoison(ip, uap->val);
		break;
	case LOGPGET:
		if(pp = pfind(uap->which))
			u.u_r.r_val1 = pp->p_log;
		break;
	case LOGPSET:
		if(pp = pfind(uap->which))
			pp->p_log = uap->val;
		break;
	default:
		u.u_error = EINVAL;
		break;
	}
}


/*
 * Internal access to log file.
 * Each record is of form:
 *	short code, telling what kind of record it is
 *	short pid
 *	short bytelen of record, including these 3 shorts
 *	body of record 
 *
 * body is composed with formatted writes, printf-like.
 * format code letters (no literals, no %):
 *	s		arg is a string
 *	$		arg is a DIRSIZ string
 *	j		arg is a jlabel, print slab address
 *	J		same, but also print slab value
 *	1,2,3 or 4	arg is an n-byte number
 *	.		skip arg.
 *	I		arg is (struct inode *)
 *
 * some codes take no arguments
 *	r#, R#		1 or 2 retvals, following digit = precision.
 *	C		supply u.u_comm as a $ arg.
 *	e		if u.u_error != 0, put it as a single byte
 */

/*VARARGS3*/
seclog(code, mode, fmt, x1)
	register char *fmt;
{
	register char *ap, *aptop;
	int *adx;
	register int b, c, i;
	struct jlabel *j;
	struct slabel *sp;
	struct inode *ip;
	int k;
	char *s;

	if(lg[0] == NULL || (u.u_labcheck & LABAUDIT) || fmt == 0)
		return;
	u.u_labcheck |= LABAUDIT;
	plock(&alog);

	abuf.code = code;
	abuf.mode = mode;
	ap = abuf.body;
	aptop = &abuf.body[LOGLEN];
#define aputc(x) if(ap<aptop) *ap++ = (x)
	if(abuf.code == LOG_SYSCALL)
		adx = u.u_arg;
	else
		adx = &x1;
	if(u.u_error == ELAB) aputc('E');
	else if(u.u_error != 0) {
		aputc('e');
		aputc(u.u_error);
	}
	while (c = *fmt++) {
		switch(c) {
		case '.':
			adx++;
			break;
		case 'R':
		case 'r':
			k = c;
			c = *fmt++;
			b = u.u_r.r_val1;
			goto num;
		case '1':
		case '2':
		case '3':
		case '4':
			k = 0;
			b = *adx++;
		num:
			i = c - '0';
			aputc(c);
			while(i--) {
				aputc(b & 0xff);
				b >>= 8;
			}
			if(k == 'R') {
				b = u.u_r.r_val2;
				k = 0;
				goto num;
			}
			break;
		case 'J':
		case 'j':
			j = (struct jlabel *) *adx++;
			if(j != 0) {
				aputc(c);
				b = j->jl_priv;
				aputc(b);
				aputc(b>>8);
				sp = j->jl_slabp;
				b = sp-slabel;
				aputc(b);
				aputc(b>>8);
				if(c == 'J') {
					k = LABSIZ;
					s = (char *)sp->sl_bits;
					goto movstr;
				}
			}
			break;
		case 'C':
			s = u.u_comm;
			goto dollar;
		case '$':
			s = (char *)*adx++;
		dollar:
			for(k=0; k<DIRSIZ && s[k] != 0; k++)
				;
			aputc(c);
			aputc(k);
			goto movstr;
		case 's':
			s = (char *)*adx++;
			for(k=0; s[k]; k++)
				;
			aputc(c);
			aputc(k);
			aputc(k>>8);
		movstr:
			for(i=0; i<k; i++)
				aputc(*s++);
			break;
		case 'I':
			/*
			 * not quite right...
			 * (fstype + dev + long i_number = 7 bytes!)
			 */
			ip = (struct inode *)*adx++;
			if(ip != 0) {
				aputc(c);
				i = ip->i_dev;
				aputc(i);
				aputc(i>>8);
				i = ip->i_number;
				aputc(i);
				aputc(i>>8);
			}
			break;
		}
	}
	aflush(lg[0], ap - (char*)&abuf);
	prele(&alog);
}

logpoison(ip)
register struct inode *ip;
{
	if(ip != 0 && u.u_procp != 0) {
		u.u_procp->p_log |= logmask[LOG_AC(ip)];
		if(LOG_AC(ip) && loggable(LT))
			seclog(LOG_TRACE, 30, "I", ip);
	}
}
