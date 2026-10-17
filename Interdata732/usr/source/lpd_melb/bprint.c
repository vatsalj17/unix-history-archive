#include "chset.h"

bprint(s)
char *s;
{
	register w, line, c, realh;
	register char *p;
	register int col, h;

	do {
		for (h=0; h<CHEIGHT+CDEPTH; h++) {
			col = 0;
			for (p = s; c = *p; p++) {
				if (c == '\n') {
					p++;
					break;
				}
				if ((col += CWIDTH+CSPACE) > LWIDTH)
					break;
				if (c < ' ')
					c = ' ';
				realh = h;
				switch (c) {
				case 'g': case 'j': case 'p': case 'q':
				case 'y': case ',': case ';':
					realh = h - CDEPTH;
				}
				if (realh<0 || realh >= CHEIGHT)
					line = 0;
				else
					line = chset[c-' '][realh];

				for (w = 1<<(CWIDTH-1); w; w>>=1)
					putchar(w&line? FILL : SPACE);
				for (w = 0; w < CSPACE; w++)
					putchar(SPACE);
			}
			putchar('\n');
		}
	} while (*(s = p));
}
