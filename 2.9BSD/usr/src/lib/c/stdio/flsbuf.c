#include	<stdio.h>

char	*malloc();

_flsbuf(c, iop)
int c;
register FILE *iop;
{
	register char *base;
	register n, rn;
	char c1;
	extern char _sobuf[];

	if (iop->_flag & _IORW) {
		iop->_flag |= _IOWRT;
		iop->_flag &= ~_IOEOF;
	}

	if ((iop->_flag&_IOWRT)==0)
		return(EOF);
tryagain:
#ifdef UCB_LINEBUF
	if (iop->_flag&_IOLBF) {
		base = iop->_base;
		*iop->_ptr++ = c;
		if (iop->_ptr >= base+BUFSIZ || c == '\n') {
			n = write(fileno(iop), base, rn = iop->_ptr - base);
			iop->_ptr = base;
		} else
			rn = n = 0;
		iop->_cnt = 0;
	} else
#endif UCB_LINEBUF
	if (iop->_flag&_IONBF) {
		c1 = c;
		rn = 1;
		n = write(fileno(iop), &c1, rn);
		iop->_cnt = 0;
	} else {
		if ((base = iop->_base) == NULL) {
			if (iop == stdout) {
#ifndef UCB_LINEBUF
				if (isatty(fileno(stdout))) {
					iop->_flag |= _IONBF;
					goto tryagain;
				}
#else UCB_LINEBUF
				if (isatty(fileno(stdout)))
					iop->_flag |= _IOLBF;
#endif UCB_LINEBUF
				iop->_base = _sobuf;
				iop->_ptr = _sobuf;
				goto tryagain;
			}
			if ((iop->_base = base = malloc(BUFSIZ)) == NULL) {
				iop->_flag |= _IONBF;
				goto tryagain;
			}
			iop->_flag |= _IOMYBUF;
			rn = n = 0;
		} else if ((rn = n = iop->_ptr - base) > 0) {
			iop->_ptr = base;
			n = write(fileno(iop), base, n);
		}
		iop->_cnt = BUFSIZ-1;
		*base++ = c;
		iop->_ptr = base;
	}
	if (rn != n) {
		iop->_flag |= _IOERR;
		return(EOF);
	}
	return(c);
}

fflush(iop)
register FILE *iop;
{
	register char *base;
	register n;

	if ((iop->_flag & (_IONBF|_IOWRT)) == _IOWRT
	 && (base = iop->_base) != NULL && (n = iop->_ptr - base) > 0) {
		iop->_ptr = base;
#ifdef UCB_LINEBUF
		iop->_cnt = (iop->_flag&(_IOLBF|_IONBF)) ? 0 : BUFSIZ;
#else UCB_LINEBUF
		iop->_cnt = BUFSIZ;
#endif UCB_LINEBUF
		if (write(fileno(iop), base, n) != n) {
			iop->_flag |= _IOERR;
			return(EOF);
		}
	}
	return(0);
}

/*
 * Flush buffers on exit
 */

_cleanup()
{
	register FILE *iop;
	extern FILE *_lastbuf;

	for (iop = _iob; iop < _lastbuf; iop++)
		fclose(iop);
}

fclose(iop)
register FILE *iop;
{
	register r;

	r = EOF;
	if (iop->_flag&(_IOREAD|_IOWRT|_IORW) && (iop->_flag&_IOSTRG)==0) {
		r = fflush(iop);
		if (close(fileno(iop)) < 0)
			r = EOF;
		if (iop->_flag & _IOMYBUF)
			free(iop->_base);
#ifdef UCB_LINEBUF
		if (iop->_flag&(_IOMYBUF|_IONBF|_IOLBF))
#else UCB_LINEBUF
		if (iop->_flag & (_IOMYBUF|_IONBF))
#endif UCB_LINEBUF
			iop->_base = NULL;
	}
#ifdef UCB_LINEBUF
	iop->_flag &=
	   ~(_IOREAD|_IOWRT|_IOLBF|_IONBF|_IOMYBUF|_IOERR|_IOEOF|_IOSTRG|_IORW);
#else UCB_LINEBUF
	iop->_flag &=
		~(_IOREAD|_IOWRT|_IONBF|_IOMYBUF|_IOERR|_IOEOF|_IOSTRG|_IORW);
#endif UCB_LINEBUF
	iop->_cnt = 0;
	return(r);
}
