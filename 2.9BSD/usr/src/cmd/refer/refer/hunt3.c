# include "refer.h"
getq(v)
char *v[];
{
	static char buff[QLEN];
	register char *p, *q;
	int c, n 0, las 0;
	p = buff;
	q = sinput;
	while (c = *q++)
	{
		if (c== '\n')
			break;
		if (isalpha(c) || isdigit(c))
		{
			if (las==0)
			{
				if (n == NQUERY) {
				   fprintf(stderr,
				      "too many specifications:\n%s\n",sinput);
				   break;
				}
				v[n++] = p;
				las=1;
			}
			if (las++ <= 6)
				*p++ = c;
		}
		else
		{
			if (las>0)
				*p++ = 0;
			las=0;
		}
	}
	*p=0;
	return(n);
}
