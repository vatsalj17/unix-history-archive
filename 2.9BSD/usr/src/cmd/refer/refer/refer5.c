# include "refer.h"
# define SAME 0

#define NLABC	(NREF+1)		/* start from 1 */

static char bflab[NFLAB];
static char *labtab[NLABC];
static char *lbp bflab;
char labc[NLABC];
int  prevsig;
static char punct;
char lastpunct();

putsig (nf, flds, nref, nstline, endline)
char *flds[], *nstline, *endline;
{
	/* choose signal style */
	char t[100], t1[100], t2[100], format[10], *sd;
	int another 0;
	int addon;
	char *stline;
	static FILE *fhide 0;

	trimnl(nstline);
	trimnl(endline);
	if (nonblank(nstline+2) || nonblank(endline+2))
	{
		stline = nstline+2;
		endline += 2;
	}
	else
	{
#ifdef	MFLAG
		stline  = labels ? "\\*((." : "\\*([.";
		endline = labels ? "\\*(.)" : "\\*(.]";
#else
		stline  = "\\*([.";
		endline = "\\*(.]";
#endif
	}
	if (nref==0)		/* non-existent paper, just flush */
		goto flush;
	if (prevsig==0)
	{
		/* save period or comma from last line */
		punct=lastpunct();
		output(stline);
		prevsig=1;
	}
	if (labels)
	{
		if (nf==0) {	/* old */
			if (labc[nref] == 1)
				sprintf(t,"%s%c%d%c",labtab[nref],KEYLET,
				    nref,KEYLET);
			else
				sprintf(t, "%s%c", labtab[nref], labc[nref]);
		}
		else
		{
			*t=0;
			if (keywant)
				fpar(nf, flds, t, keywant, 1, 0);
			if (t[0]==0)
			{
#ifdef	MFLAG
			    if (mflag) {
				fpar(nf, flds, t, 'A', 1, 0);
				if (fpar(nf, flds, t2, 'A', 2, 0)) {
				    if (fpar(nf, flds, t1, 'A', 3, 0))
					    /* Author1 et al, date */
					sprintf(t1, "%s \\*([e",t);
				    else
					    /* Author1 and Author2, date */
					sprintf(t1,"%s \\*(&1 %s",t,t2);
				}
				else
				    /* Author, date */
				    strcpy(t1,t);
			    }
			    else
#endif
			        sprintf(format, nmlen>0 ? "%%.%ds%%s" : "%%s%%s", nmlen);
			    /* format is %s%s for default labels
				    or %.3s%s eg if wanted */
			    sd = fpar(nf, flds, t2, 'D', 1, 0);
			    if (dtlen>0)
			    {
				    char *sdb;
				    for(sdb=sd; *sd; sd++);
				    sd = sd-dtlen;
				    if (sd<sdb) sd=sdb;
			    }
#ifdef	MFLAG
			    if (mflag)
				sprintf(t,"%s, %s",t1,sd);
			    else
#endif
				sprintf(t,format,fpar(nf,flds,t1,'A',1,0), sd);
			}
			if (keywant)
			{
				addon=0;
				for(sd=t; *sd; sd++);
				if (*--sd == '-')
				{
					addon=1;
					*sd=0;
				}
			}
			if (!keywant || addon) {
				addon = keylet(t, nref);
				if (addon == 1) {
				    sprintf(t1,"%c%d%c",KEYLET,nref,KEYLET);
				    strcat(t,t1);
				} else
				    addch(t, addon);
			}
		}
	}
	else 
	{
		if (sort)
			sprintf(t, "%c%d%c", FLAG, nref, FLAG);
		else
			sprintf(t, "%d", nref);
	}
flush:
	another = prefix (".[", sd=lookat());
	if (another && nonblank(sd+2))
		fprintf(stderr, "File %s, line %d: punctuation ignored: %s",
			Ifile, Iline, sd);
	if (nref)
		output(t);
	if (bare==0)
	{
		if (another==0)
		{
			output(endline);
			if (punct) {
#ifdef	MFLAG
				sprintf(t1,labels? "%c\n" : "\\*(>%c\n", punct);
#else
				sprintf(t1, "\\*(>%c\n", punct);
#endif
				output(t1);
				punct = 0;
			}
			else
				output("\n");
			flout();
			prevsig=0;
			if (fo == fhide && fhide != (FILE *)NULL)
			{
				int ch;
				fclose(fhide); 
				fhide= fopen(hidenam, "r");
				fo= ftemp;
				while ((ch = getc(fhide)) != EOF)
					putc(ch, fo);
				fclose(fhide);
				unlink(hidenam);
			}
		}
		else
		{
#ifdef	MFLAG
			output(mflag ? "; " : (labels ? ", " : ",\\|"));
#else
			output(labels ? ", " : ",\\|");
#endif
			/* hide if need be */
			if (fo == ftemp)
			{
				sprintf(hidenam, "/tmp/rj%dc", getpid());
				fhide= fopen(hidenam, "w");
				if (fhide==NULL) err("Can't get scratch file %s", hidenam);
				fo = fhide;
			}
		}
	}
	if (nf>0) {
		fprintf(fo,".ds [F %s%c",t,sep);
		if (sort)
			fprintf(fo,".nr [F %c%d%c%c",FLAG,nref,FLAG,sep);
		else
			fprintf(fo,".nr [F %d%c",nref,sep);
	}
	if (bare>0)
		flout();
}

nonblank(s)
char *s;
{
	for (; *s; s++)
		if (*s != ' ' && *s != '\n')
			return(1);
	return(0);
}

fpar (nf, flds, out, c, seq, prepend)
char *flds[], *out;
{
	char *p, *s;
	int i, fnd 0;
	for(i=0; i<nf; i++)
		if (flds[i][1]==c && ++fnd >= seq)
		{
		     /* for titles use first word otherwise last */
			if (c=='T' || c == 'J')
			{
				p=flds[i]+3;
				if (prefix("A ", p)) p +=2;
				if (prefix("An ", p)) p +=3;
				if (prefix("The ", p)) p+= 4;
				mycpy2(out, p, 20);
				return(out);
			}
			for(s=p= flds[i]+2; *p; p++);
			while (p>s && *p != ' ') p--;
			/* special wart for authors */
			if (c=='A' && (p[-1] == ',' || p[1] =='('))
			{
				p--;
				while (p>s && *p != ' ') p--;
				mycpy (out, p+1);
			}
			else
				strcpy (out, p+1);
			if (c=='A' && prepend)
				initadd(out, flds[i]+2, p);
			return(out);
		}
	return(0);
}
putkey(nf, flds, nref, keystr)
char *flds[], *keystr;
{
	char t1[50], *sf;
	int ctype, i, count;
	fprintf(fo, ".\\\"");
	while (ctype= *keystr++)
	{
		count = atoi(keystr);
		if (*keystr=='+') count=999;
		if (count<=0) count=1;
		for(i=1; i<=count; i++)
		{
			sf= fpar(nf, flds, t1, ctype, i, 1);
			if (sf==0)
				break;
			sf = artskp(sf);
			fprintf(fo, "%s%c", sf, '-');
		}
	}
	fprintf(fo, "%c%d%c%c", FLAG, nref, FLAG, sep);
}
keylet(t, nref)
char *t;
{
	register i;
	register x 'a'-1;
	register first = 0;

	if (nref>NREF)
		err("Too many references for labels",0);
	for(i=1; i<nref;i++)
	{
		if (strcmp(labtab[i], t) == 0) {
			x = labc[i];
#ifdef	MFLAG
			if (x==1)
				x = 'a';
#endif
			if (first == 0)
				first = i;
		}
	}
	if (first == 0) {
		if (lbp+strlen(t) >= bflab+NFLAB)
			err("Label buffer overflow",0);
		strcpy(labtab[nref]=lbp, t);
		while (*lbp++) ;
#ifdef	MFLAG
		return(labc[nref] = mflag ? 1 : 'a');
#else
		return(labc[nref] = 'a');
#endif
	} else {
		labtab[nref] = labtab[first];
#ifdef	MFLAG
		labc[first] = 'a';
#endif
	}
	return(labc[nref]=x+1);
}
mycpy(s,t)
char *s, *t;
{
	while (*t && *t != ',' && *t != ' ')
		*s++ = *t++;
	*s=0;
}
mycpy2 (s, t, n)
char *s, *t;
{
	int c;
	while (n-- && (c= *t++)>0)
	{
		if (c==' ')c= '-';
		*s++ = c;
	}
	*s=0;
}
initadd(to, from, stop)
char *to, *from, *stop;
{
	int c, nalph 1;
	while (*to) to++;
	while (from<stop)
	{
		c = *from++;
		if (!isalpha(c))
		{
			if (nalph)
				*to++ = '.';
			nalph=0;
			continue;
		}
		if (nalph++ ==0)
			*to++ = c;
	}
	*to=0;
}

static char *articles[] {
	"the ", "an ", "a ", 0};
artskp(s)
char *s;
{
	/* skips over initial "a ", "an ", or "the " in s */
	char **p, *r1, *r2;
	for(p=articles; *p; p++)
	{
		r2 = s;
		for (r1= *p; ((*r1 ^ *r2) & ~040 ) == 0; r1++)
			r2++;
		if (*r1==0 && *r2 != 0)
			return(r2);
	}
	return(s);
}
