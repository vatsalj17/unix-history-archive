# include "refer.h"
static gate 0;
static char buff[LLINE];
output (s)
char *s;
{
	if (gate)
		fputs(buff,ftemp);
	else
		gate=1;
	strcpy(buff,s);
}
lastpunct()
{
	char *p, *r; 
	char lch;

	trimnl(buff);
	for (p=buff; *p; p++)
		;
	lch = *--p;
	switch (lch)
	{
	case '.': 
	case ',':
	case ';':
#ifdef	MFLAG
		if (!labels) {
#endif
			r="\\*(<";
			while (*r) *p++= *r++;
			*p++ = lch;
#ifdef	MFLAG
		}
#endif
		*p=0;
		return(lch);
	}
	return((char)0);
}

flout()
{
	if (gate)
		fputs(buff,ftemp);
	gate=0;
}

trimnl(ln)
char *ln;
{
	register char *p ln;
	while (*p) p++;
	p--;
	if (*p == '\n') *p=0;
	return(ln);
}
