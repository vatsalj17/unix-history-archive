#include	"../mac/mac.h"
#include	"mactab.h"
#include	"mactab.x"


/*
 *   Format descriptors.
 */

pfmt()
{
	register struct fd *q;
	register int i;

	if (!head.h_bu_len)  {
		error("basic addr width in bits required before dc", 0);
		exit(1);
		}

	q = &format[0];
	for (i=0; i<NFMT; i++)  {
		if (!getlin())
			break;

		q->f_class = getnum();
		while (*p == ' ' || *p == '\t')
			p++;
		q->f_len = fmtscan(q);
		q++;
		}

	head.h_formats = nfmt = i;
	return;
}

fmtscan(q)
register struct fd *q;
{
	register int n;
	register int i;
	register cc;
	register int width;
	register int w;


	width = 0;
	n = 0;

	while (cc = *p++)  {

		if (cc == '\n')  {
			q->f_desc[n] = 0;
			break;
			}

		if (cc >= 'a' && cc <= 'm' ||
		    cc == 'o' || cc == '!'    )  {
			p++;			/* skip : */
			w = getnum();
			q->f_desc[n] = cc;
			q->f_width[n] = w;
			n++;
			width =+ w;
			continue;
			}

		if (cc == 'p')  {
			cc = *p++;
			p++;				/* bump past : */
			if (cc < 'a' || cc > 'm')  {
				error("pc relative arg only", 0);
				return(0);
				}
			w = getnum();
			q->f_desc[n] = cc | PMODE;
			q->f_width[n] = w;
			n++;
			width =+ w;
			continue;
			}

		if (cc == 'r')  {
			w = getnum();
			q->f_value[n] = w;
			if (w <= 0)  {
				error("bad reloc. width", 0);
				return(0);
				}
			cc = *p++;
			p++;			/* skip :*/
			if (cc < 'a' || cc > 'm')  {
				error("bad reloc. arg %c", cc);
				return(0);
				}
			w = getnum();
			q->f_desc[n] = cc | RMODE;
			q->f_width[n] = w;
			n++;
			width =+ w;
			continue;
			}

		if (cc == '#')  {		/* const */
			q->f_value[n] = i = getnum();
			p++;
			w = getnum();		/* width */
			if (i >> w)
				error("data overflow", 0);
			width =+ w;
			q->f_desc[n] = '#';
			q->f_width[n] = w;
			n++;
			continue;
			}

		if (cc == 'n')  {		/* next n bits */
			p++;
			w = getnum();
			width =+ w;
			q->f_desc[n] = cc;
			q->f_width[n] = w;
			n++;
			continue;
			}

		error("bad format character %c", cc);
		}

	if (width % head.h_bu_len)  {
		error("format descriptor uneven multiple",0);
		return(ERR);
		}

	return(width / head.h_bu_len);
}
