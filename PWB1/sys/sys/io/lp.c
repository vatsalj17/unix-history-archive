#
/*
 *  Line printer driver
 */

#include "../hd/param.h"
#include "../hd/user.h"
#include "../hd/tty.h"

extern lp_addr[], lp_cnt;

#define	LPPRI	10
#define	LPLOWAT	50
#define	LPHIWAT	100
#define	LPMAX	2

struct	{ int lpcsr, lpbuf; };

struct  {
	struct	clist l_outq;
	char	flag, ind;
	int	ccc, mcc, mlc;
	int	line, col;
} lp_dt[LPMAX];

#define	OPEN	010
#define	CAP	020
#define	NOCR	040
#define	ASLEEP	0100

#define	FORM	014

lpopen(dev, flag)
{
	register unit, *lp;

	unit = dev&07;
	if (unit >= lp_cnt || unit >= LPMAX ||
	 (lp = &lp_dt[unit])->flag || lp_addr[unit]->lpcsr <0 ) {
		u.u_error = EIO;
		return;
	}
	lp->flag = (dev&077)|OPEN;
	lp->ind = 8;
	lp->col = 132;
	lp->line = 60;
	lp_addr[unit]->lpcsr =| IENABLE;
	lpoutput(unit, FORM);
}

lpclose(dev)
{
	register unit;

	unit = dev&07;
	lpoutput(unit, FORM);
	lp_dt[unit].flag = 0;
}

lpwrite(dev)
{
	register c, *lp;

	while ((c=cpass())>=0)
		lpoutput(dev&07, c);
}

lpoutput(dev, c)
register dev, c;
{
	register *lp;

	lp = &lp_dt[dev];
	if(lp->flag&CAP) {
		if(c>='a' && c<='z')
			c =+ 'A'-'a'; else
		switch(c) {
		case '{':
			c = '(';
			goto esc;
		case '}':
			c = ')';
			goto esc;
		case '`':
			c = '\'';
			goto esc;
		case '|':
			c = '!';
			goto esc;
		case '~':
			c = '^';
		esc:
			lpoutput(c);
			lp->ccc--;
			c = '-';
		}
	}
	switch(c) {
	case '\t':
		lp->ccc = (lp->ccc+8) & ~7;
		return;
	case '\n':
		lp->mlc++;
		if(lp->mlc >= lp->line )
			c = FORM;
	case FORM:
		lp->mcc = 0;
		if (lp->mlc) {
			lpputc(dev, c);
			if(c == FORM)
				lp->mlc = 0;
		}
	case '\r':
		lp->ccc = 0;
		if(lp->ind)
			lp->ccc = lp->ind;
		return;
	case 010:
		if(lp->ccc > 0)
			lp->ccc--;
		return;
	case ' ':
		lp->ccc++;
		return;
	default:
		if(lp->ccc < lp->mcc) {
			if (lp->flag&NOCR) {
				lp->ccc++;
				return;
			}
			lpputc(dev, '\r');
			lp->mcc = 0;
		}
		if(lp->ccc < lp->col) {
			while(lp->ccc > lp->mcc) {
				lpputc(dev, ' ');
				lp->mcc++;
			}
			lpputc(dev, c);
			lp->mcc++;
		}
		lp->ccc++;
	}
}

lpputc(dev, c)
register dev, c;
{
	register *lp;

	lp = &lp_dt[dev];
	spl4();
	while (lp->l_outq.c_cc > LPHIWAT) {
		lp->flag =| ASLEEP;
		sleep(lp, LPPRI);
	}
	putc(c, &lp->l_outq);
	lpintr(dev);
	spl0();
}

lpintr(dev)
register dev;
{
	register *lp, c;

	lp = &lp_dt[dev];
	while (lp_addr[dev]->lpcsr&DONE && (c = getc(&lp->l_outq)) >= 0)
		lp_addr[dev]->lpbuf = c;
	if (lp->l_outq.c_cc <= LPLOWAT && lp->flag&ASLEEP) {
		lp->flag =& ~ASLEEP;
		wakeup(lp);
	}
}

lpsgtty(dev, v)
register *v;
{
	register *lp;

	lp = &lp_dt[dev&07];
	if (v) {
		v->lobyte = lp->flag;
		v->hibyte = lp->ind;
		v[1] = lp->line;
		v[2] = lp->col;
	} else {
		lp->flag = (u.u_arg[0].lobyte&077)|OPEN;
		lp->ind = u.u_arg[0].hibyte;
		lp->line = u.u_arg[1];
		lp->col = u.u_arg[2];
	}
}
