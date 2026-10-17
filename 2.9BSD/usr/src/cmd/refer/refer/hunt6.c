# include "stdio.h"
#include "refer.h"
extern long indexdate;
extern char usedir[];

baddrop(nf, nitem)
{
	/* checks list of drops for real bad drops */
	int i, g, j, na, len;
	long lp;
	char res[TAGLEN], *ar[50], output[TXTLEN];
	char *s; 

	for (i=g=0; i<nf; i++)
	{
		lp = iflong ? master.b[i] : master.a[i];
		fseek (fc, lp, 0);
		fgets( res, TAGLEN, fc);
		for(s=res; *s; s++)
			if (*s == ';' || *s == '\n')
			{
				*s=0; 
				break;
			}
		len = findline (res, output, TXTLEN, indexdate);
		if (len==0)
			continue;
		na=0;
		ar[na++] = "fgrep";
		ar[na++] = "-r";
		ar[na++] = "-n";
		ar[na++] = nitem;
		ar[na++] = "-i";
		ar[na++] = output;
		ar[na++] = len;
		for(j=0; j<nitem; j++)
			ar[na++] = qitem[j];
		if (fgrep(na, ar)==0)
		{
			if (iflong)
				master.b[g++] = master.b[i];
			else
				master.a[g++] = master.a[i];
			if (g>1) {
			    if (g==2) {
				fprintf(stderr,
				    "%s: Multiple matches for '%s'",
				    Ifile,sinput);
				fprintf(stderr,"in %s\n",Index);
				title(text);
			    }
			    title(output);
			} else {
			    if (res[0]=='/' || usedir[0]==0)
				strcpy(tagout, res);
			    else
				sprintf(tagout, "%s/%s", usedir, res);
			    strcpy (text, output);
			}
		}
	}
	return(g);
}
