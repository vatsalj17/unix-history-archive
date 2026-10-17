#include	"../mac/mac.h"
#include	"mactab.h"
#include	"mactab.x"


/*
 *   Misc. routines.
 */


getlin()
{
	register int nchar;
	register cc;

	nchar = 0;
	nline++;
	p = buf;

	while ((cc = getchar()) > 0)  {
		if (nchar > 255)  {
			error("buffer overflow", 0);
			exit(1);
			}

		if (cc == '%' && !nchar)  {
			while ((cc = getchar()) != '\n');
			break;			/* simulate eof */
			}

		buf[nchar++] = cc;
		if (cc == '\n')
			return(nchar);
		}

	buf[0] = '\0';
	return(0);
}

getsym()
{
	register char *s;
	register cc;
	register int i;

	while ((cc = *p++) == ' ' || cc == '\t');

	if (any(cc, alptab) >= 0)  {
		s = clabel;
		*s++ = cc;
		while (any(*p, alptab) >= 0 || any(*p, dectab) >= 0)
			*s++ = *p++;
		*s = '\0';
		if (compar("expr", clabel))  {
			sym = EXP;
			mem = NUL;
			return;
			}

		for (mem=0; mem<nlit; mem++)
			if (compar(literals[mem], clabel))  {
				sym = LIT;
				return;
				}

		sym = LBL;
		mem = NUL;
		return;
		}


	if ((i = any(cc, oprtab)) >= 0)  {
		sym = OPR;
		mem = i;
		return;
		}


	if (cc >= '0' && cc <= '9')  {
		p--;
		sym = CON;
		mem = getnum();
		return;
		}

	if (cc == '\n')  {
		sym = EOL;
		mem = NUL;
		for (i=0; i<4; i++)
			mem4[i] = 0;
		return;
		}

	if (cc == '{')  {
		/* funny EOL symbol - with SELC options */
		s = &mem4[0];
		for (i=0; i<4; i++)
			*s++ = getnum();
		sym = EOL;
		while (*p != '\n')
			p++;
		return;
		}

	if (cc == ',')  {
		sym = DEL;
		mem = cc;
		return;
		}

	if (cc == '"')  {
		i = 0;
		while (*p != '"' && *p != '\n')
			clabel[i++] = *p++;
		clabel[i] = '\0';
		if (*p == '"')
			p++;
		sym = STR;
		mem = NUL;
		return;
		}

	sym = CHR;
	mem = cc;
	return;
}

compar(r, s)
register char *r;
register char *s;
{
	register int n;

	n = 1;
	while (*r == *s)  {
		if (*r == '\0')
			return(1);
		if (n > 8)
			return(0);
		r++;
		s++;
		n++;
		}
	return(0);
}

getnum()
{
	register int i,j;
	register cc;

	while ((cc = *p++) == ' ' || cc == '\t');

	i = 0;
	if (cc == '0')  {

		if (*p == 'x')  {
			p++;
			while ((j = any(*p, hextab)) >= 0)  {
				i = (i<<4) | j;
				p++;
				}
			return(i);
			}

		if (*p == 'b')  {
			p++;
			while (*p == '0' || *p == '1')
				i = (i<<1) | (*p++ - '0');
			return(i);
			}

		while ((j = any(*p, octtab)) >= 0)  {
			i = (i<<3) | j;
			p++;
			}
		return(i);
		}

	i = cc - '0';
	while ((j = any(*p, dectab)) >= 0)  {
		i = i * 10 + j;
		p++;
		}
	return(i);
}

any(cc, str)
register char cc;
char	 str[ ];
{
	register i = 0;

	while (str[i])  {
		if (cc == str[i])
			return(i);
		i++;
		}

	return(ERR);
}

error(s, v)
register char *s;
register int v;
{
	printf("%3d: ", nline);
	printf(s, v);
	putchar('\n');
	nerr++;

	return;
}

copy(r, s)
register char *r;
register char *s;
{
	while ((*s++ = *r++) != '\0');
	return;
}


/*
 *   Comparison for searching.
 *
 *	return  1:  *r > *r,
 *		0:  *r = *s,
 *	       -1:  *r < *s.
 */
cmp(r, s)
register char *r;
register char *s;
{
	register int i;

	/*
	 *   Fast compare.
	 */
	if (*r < *s)
		return(-1);
	if (*s < *r)
		return(1);
	i = 0;
	while (i < 8)  {
		r++;
		s++;

		if (*r < *s)
			return(-1);
		if (*s < *r)
			return(1);
		if (!*r)
			return(0);		/* equal on nul */
		i++;
		}
	return(0);
}
