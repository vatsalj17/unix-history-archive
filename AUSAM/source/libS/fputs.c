#include	"stdio.h"

fputs(s, iop)
register char *s;
register FILE *iop;
{
	register c;

	while (c = *s++)
		putc(c, iop);
}
