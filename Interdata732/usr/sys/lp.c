#
/*
 *  Line printer driver
 */

#include "param.h"
#include "conf.h"
#include "user.h"
#include "tty.h"

/* Configuration */

#define LPPRI	75	/* line printer wakeup priority */
#define LPLWAT 70        /* line printer low water mark */
#define LPHWAT 150       /* line printer high water mark */

#define LPWIDTH	131	/* page width (not 132, as lp auto prints on 132nd char */
#define LPDEPTH	60	/* lines per page */

int	lpaddr	0x62;	/* printer address */

/* Printer status & commands */

#define ENABLE	0x40
#define DISARM	0xc0

#define NO_PAPER	0x40
#define INTLOCK		0x10
#define DU		0x01


struct {    /* info about the printer */
	struct clist outq;	/* output char queue */
	char flag;	/* current mode */
	char state;	/* internal status */
	int mcc;	/* actual column posn */
	int ccc;	/* logical column posn */
	int mlc;	/* curr line # */
	int mxc;	/* max line width */
	int mxl;	/* max page depth */
} lp;

#define	P_WASTE	ECHO	/* if on, printer wastes paper */
#define IND	XTABS	/* if on, lines are indented 8 spaces */

#define FORM 0x0c
#define VTAB 0x0b


/*
 *    open - line printer
 */

lpopen(dev, flag)
{
	if(lp.state&ISOPEN || (ss(lpaddr)&(NO_PAPER|INTLOCK|DU))) {
		u.u_error = EIO;
		return;
	}

	trace(01<<8,"lpopen",0);

	lp.state = ISOPEN;
	if (lp.mxc == 0) {
		lp.mxc = LPWIDTH;
		lp.mxl = LPDEPTH;
	}
	oc(lpaddr, ENABLE);
	lpcanon(FORM);
}

/*
 *   close - line printer
 */

lpclose(dev, flag)
{

	lpcanon(FORM);
	trace(01<<8,"lpclose",0);
	lp.state = 0;
}

/*
 *     user write to line printer
 */

lpwrite()
{
	register c;

	while ((c=cpass()) >= 0)
		lpcanon(c);
}

/*
 * lpcanon - character transformations
 */

lpcanon(ac)
{
	register char c;
	register char *p;

	c = ac;

	if (c == '_')
		c =| 0x80;   /*
			      *   Interdata line printer i/face
			      *   turns x'5f' into x'3c' .....
			      *
			      *   But it isn't smart enough to
			      *   regognize x'df', fortunately.
			      */
	if (lp.flag & RAW) {
		lpoutput(c);
		return;
	}

	if (lp.flag & LCASE) {		/* Uppercase-only printer */
		if (c>='a' && c<='z')
			c =+ 'A'-'a';
		else {
			p = "({)}!|^~'`";
			while (*p++)
				if (c == *p++) {
					lpcanon(p[-2]);
					lp.ccc--;
					c = '-';
					break;
				}
		}
	}

	switch(c&0x7f) {

	case '\t':
		lp.ccc = (lp.ccc + 8) & ~7;
		return;

	case FORM:
	case '\n':
		if(lp.mcc != 0 || lp.mlc != 0 || lp.flag & P_WASTE) {
			if(lp.mcc) {
				lpoutput('\r');  /* cause line to print */
			}
			lp.mlc++;
			if(lp.mlc > lp.mxl && lp.mxl)
				c = FORM;
			lpoutput(c);
			if (c == FORM)
				lp.mlc = 0;
			lp.mcc = 0;
		}   /* now fall through to '\r' code */

	case '\r':
		lp.ccc = 0;
		if (lp.flag & IND)
			lp.ccc = 8;
		return;

	case '\b':
		if (lp.ccc > 0)
			lp.ccc--;
		return;

	case 0x07:
		lpoutput(c); /* ring bell, no char advance */
		return;

	case ' ':
		lp.ccc++;
		return;

	case VTAB:
		if(lp.mcc)
			lpcanon('\n');
		while(lp.mlc & 7)
			lpcanon('\n');
		return;

	default:
		if ((c & 0x7f) < 0x20)	/* non printing - ignore */
			return;
		if(lp.ccc < lp.mcc) {
			lpoutput('\r');    /* cause overprint */
			lp.mcc = 0;
		}
		if (lp.ccc < lp.mxc) {
			while(lp.ccc > lp.mcc) {
				lpoutput(' ');
				lp.mcc++;
			}
			lpoutput(c);
			lp.mcc++;
		}
		lp.ccc++;
	}
}


/*
 *   start transmission to printer
 */

lpstart()
{
	register c;
	register s;

	trace(02<<8,"lpstart",lpaddr);
	trace(01<<8,"lpstat",ss(lpaddr));

	while( (s = ss(lpaddr)) == 0
		&&    (c = getc(&lp.outq)) >= 0  ) {
			trace(010<<8,"lpchar",c);
			wd(lpaddr, c);
		}
	trace(01<<8,"lpstat",s);
}


/*
 *   line printer interrupt
 */

lpint(dev, stat)
{
	trace(02<<8,"lpint",stat);

/***	if(stat & DU)
		printf("\nline printer offline\n");
	else if (stat & NO_PAPER)
		printf("\nline printer paper out\n");
	else {			***/   if(!(stat & (DU|NO_PAPER))) {
		lpstart();
		if(lp.outq.c_cc <= LPLWAT && lp.state & ASLEEP) {
			trace(04<<8,"lpwakeup",lp.outq.c_cc);
			lp.state =& ~ASLEEP;
			wakeup(&lp);
		}
	}
}

/*
 *  write a character to line printer
 */

lpoutput(c)
{
	trace(02<<8,"lpoutput",c);
	spl(4);
	while(lp.outq.c_cc >= LPHWAT) {
		lp.state =| ASLEEP;
		trace(04<<8,"lpsleep",lp.outq.c_cc);
		sleep(&lp, LPPRI);
	}

	putc(c, &lp.outq);
	lpstart();
	spl(0);
}

/*
 * allow several line printer attributes to be dynamically altered
 */

lpsgtty(dev, av)
int *av;
{
	register *v;

	if(v=av) {
		*v++ = 0;
		v->lobyte = lp.mxc;
		v->hibyte = lp.mxl;
		v[1] = lp.flag;
		return(1);
	}
	v = u.u_arg;
	lp.mxc = (++v)->lobyte;
	lp.mxl = v->hibyte;
	lp.flag = v[1];
	return(0);
}
