# include "refer.h"
extern int prevsig;

doref(firline)
char *firline;
{
	char buff[QLEN], dbuff[TXTLEN];
	char line[LLINE];
	char *p, **sr, *flds[NFLD], *r;
	int nf, nr, alph, query 0, chp, digs;
	int len, skip;

	/* get query */
	buff[0] = dbuff[0] = 0;
	len = skip = 0;
	while (input(line))
	{
		if (prefix(".]", line))
			break;
		if (skip)
			continue;
		if (control(line[0])) {
			query=1;
			len = 0;
		}
		if (query) {
			if ((len += strlen(line)) > TXTLEN) {
			    fprintf(stderr,
				"in-line citation too long:\n%s",dbuff);
			    skip++;
			} else {
			    strcat (dbuff, line);
			}
		} else {
			if ((len += strlen(line)) > QLEN) {
			    fprintf(stderr,"citation too long:\n%s",buff);
			    skip++;
			} else {
			    strcat (buff, line);
			}
		}
	}
	if (strcmp (buff, "$LIST$\n")==0)
	{
		dumpold();
		return;
	}
	for( p=buff; *p; p++)
		if (isupper(*p)) *p |= 040;
	alph = digs =0;
	for(p=buff; *p; p++)
	{
		if (isalpha(chp = *p)) alph++;
		else
			if (isdigit(*p)) digs++;
			else
			{
				*p=0;
				if ( (alph+digs<3) || common(p-alph))
				{
					r = p-alph;
					while (r < p)
						*r++ = ' ';
				}
				if ( alph==0 && digs >0)
				{
					r = p-digs;
					if (digs != 4 || (atoi(r)/100 != 19))
					{
						while (r<p)
							*r++ = ' ';
					}
				}
				*p=' ';
				alph = digs = 0;
			}
	}
	text[0]=0;
	nfound = 0;
	sinput = buff;
	if (buff[0]) {	/* do not search if no query */
		for( sr= data; sr < search; sr++)
		{
			Index = *sr;
			huntmain (Index);
			if (nfound > 0) break;
		}
		if (nfound == 0) {
			fprintf (stderr, "%s: No such paper '%s'\n",
				Ifile,buff);
			if (prevsig)
				/* terminate signal */
				putsig (0, 0, 0, firline, line);
			return;
		}
		if (endpush)
			if (nr = chkdup(tagout))
			{
				if (bare<2)
					putsig (0, 0, nr, firline, line);
				return;
			}
	}
	nf = tabs(flds, text, NFLD);
	nf += tabs(flds+nf, dbuff,NFLD-nf);
	refnum++;
	if (sort) putkey (nf, flds, refnum, keystr);
	if (bare<2)
		putsig (nf, flds, refnum, firline, line);
	else
		flout();
	putref (nf, flds);
}
title( buff )
char *buff;
{
	register char *q, *t;
	int nl;

	nl = 1;
	for( q=buff; *q; q++)
	{
		if (nl && (q[0] == '.' || q[0] == '%') && q[1] == 'T')
		{
			q += 3;
			for (t=q; *t && *t != '\n'; t++);
			*t = 0;
			fprintf(stderr, "%.70s\n", q);
			q=0; 
			break;
		}
		nl = *q == '\n';
	}
	if (q)
	    fprintf(stderr, "<No title>\n");
}

control(c)
{
	if (c=='.') return(1);
	if (c=='%') return(1);
	return(0);
}
