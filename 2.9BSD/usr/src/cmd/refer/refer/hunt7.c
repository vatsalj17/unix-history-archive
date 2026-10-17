# include "stdio.h"
# define SAME 0
#define FGCT	10		/* number of files for out-of-date index */
#define FGSIZE	150		/* total chars in filenames from OOD index */
int keepold 1; /* keep old things for fgrep search */
char fgspace[FGSIZE];
char *fgp fgspace;
char *fgnames[FGCT];
char **fgnamp fgnames;

findline(in, out, outlen, indexdate)
long indexdate;
char *in, *out;
{
	static char name[100] "";
	register char *p;
	char **ftp;
	extern long gdate();
	static FILE *fa NULL;
	long lp, llen;
	int len;
	register k;
	char savc;

	if (in[0]==0)
		return(0);
	for(p=in; *p && *p != ':' && *p != ';'; p++)
		;
	if (savc = *p) *p++=0;
	else p=in;
	k = sscanf(p, "%ld,%ld", &lp, &llen);
	if (k<2)
	{
		lp = 0;
		llen=outlen;
	}
	if (strcmp (name, in) != 0)
	{
		if (fa != NULL)
			fa = freopen(in, "r", fa);
		else
			fa = fopen(in, "r");
		if (fa == NULL)
			return(0);
		/* err("Can't open %s", in); */
		strcpy(name, in);
		if (gdate(fa) > indexdate && indexdate != 0)
		{
			if (keepold)
			{
				for(ftp=fgnames; ftp<fgnamp; ftp++)
					if (strcmp(*ftp, name)==SAME)
						return(0);
				if ((fgnamp==fgnames+FGCT-1) ||
				   (fgp+strlen(name)+1 >= &fgspace[FGSIZE])) {
					fprintf(stderr,
						"Too many out-of-date files\n");
					return(0);
				}
				strcpy (*fgnamp++ = fgp, name);
				return(0);
			}
			fprintf(stderr, "Warning: index predates file '%s'\n", name);
		}
	}
	if (savc)
		*(p-1) = savc;
	if (fa != NULL)
	{
		fseek (fa, lp, 0);
		if (llen >= outlen) {
			fprintf(stderr,"Reference too long, truncated:\n");
			len = outlen-1;
		} else
			len = llen;
		len = fread (out, 1, len, fa);
		out[len] = 0;
		if (len != llen) {
			out[len-1] = '\n';
			fprintf(stderr,"%s\n",out);
		}
		return(len);
	} else {
		return(0);
	}
}
