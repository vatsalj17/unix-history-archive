#include	"mac.h"
#include	"mac.x"


getsym()
{
	register int i;
	register int j;
	register char *s;
	register char cc;


	switch (cc = *p++)  {

		case ' ':
		case '\t':
			i=1;
			while (*p  == ' ' || *p == '\t')  {
				i++;
				p++;
				}
			sym = SPA;
			mem = i;
			return;

		case ',':
			sym = DEL;
			mem = ',';
			return;

		case '\n':
			sym = EOL;
			mem = NUL;
			return;

		case '\'':
			sym = CON;
			i = 0;
			while ((cc = getch()) != '\'')
				i = (i<<8) | cc;

			mem = i;
			return;

		case '"':
			s = clabel;
			i = 0;
			while ((cc = getch()) != '"')  {
				if (i++ > 30)
					break;
				*s++ = cc;
				}
			*s = '\0';
			sym = STR;
			mem = i;
			return;

		case ';':
			s = clabel;
			*s++ = cc;
			while (*p != '\n')  *s++ = *p++;
			sym = COM;
			mem = (s - clabel);
			return;

		case '\0':
			sym = EOF;
			mem = 0;
			return;
		}

	if ((j = any(cc, alptab)) >= 0)  {
		s = clabel;
		*s++ = cc;
		i = 1;
		while (any(*p, alptab) >= 0 || any(*p, dectab) >= 0)  {
			*s++ = *p++;
			i++;
			if (i > 8)
				break;
			}
		*s++ = '\0';

		/* check literal table */
		for (i=0; i<head.h_literals; i++)
			if (compar(literals+i*8, clabel))  {
				sym = LIT;
				mem = i;
				return;
				}

		sym = LBL;
		mem = j;
		return;
		}

	if ((j = any(cc, dectab)) >= 0)  {
		p--;
		sym = CON;
		mem = argnum();
		return;
		}

	if ((j = any(cc, oprtab)) >= 0)  {
		sym = OPR;
		mem = j;
		return;
		}


	sym = CHR;
	mem = cc;
	return;
}
