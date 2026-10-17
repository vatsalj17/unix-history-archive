#include	<fio.h>

#define	WIDTH	32

scsiodump(p, n)
	register unsigned char *p;
{
	register i, nd, l;
	unsigned char buf[WIDTH];
	int didstar;
	unsigned char *s;

	for(nd = 0; n > 0; n -= l, nd += l){
		l = min(WIDTH, n);
		if(nd && (l == WIDTH) && (memcmp(buf, p, l) == 0)){
			p += WIDTH;
			if(didstar++ == 0)
				Fprint(1, "*\n");
			continue;
		}
		memcpy(buf, p, l);
		didstar = 0;
		Fprint(1, "%5.5d", nd);
		s = p;
		for(i = 0; i < l; i++){
			if((i%4) == 0) Fputc(1, ' ');
			Fprint(1, "%2.2ux", *p++);
		}
		Fputc(1, '\n');
		Fprint(1, "     ");
		for(i = 0; i < l; i++){
			if((i%4) == 0) Fputc(1, ' ');
			if((*s >= ' ') && (*s < 0177))
				Fprint(1, " %c", *s++);
			else switch(*s++)
			{
			case '\n':	Fprint(1, "\\n"); break;
			case '\t':	Fprint(1, "\\t"); break;
			default:	Fprint(1, ".."); break;
			}
		}
		Fputc(1, '\n');
	}
	Fprint(1, "%5.5d\n", nd);
}
