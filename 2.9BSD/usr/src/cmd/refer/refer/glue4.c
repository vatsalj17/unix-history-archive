# include "refer.h"
# include "stdio.h"
# include "ctype.h"

int	grepin, grepout;

grepcall (in, arg)
char *in, *arg;
{
	char line[TAGLEN], *s, argig[100], *cv[NQUERY+1];
	char *inp, inb[QLEN];
	extern char gfile[];
	FILE *qf, *gf;
	int c, oldc 0, alph 0, nv 0;
	strcpy (argig, arg); 
	strcat(argig, ".ig");
	strcpy (inp=inb, in);
	if (gfile[0]==0)
		sprintf(gfile, "/tmp/rj%dg", getpid());
	for(cv[nv++] = "fgrep"; (c = *inp) && (nv<=NQUERY); inp++)
	{
		if (c== ' ')
			c = *inp = 0;
		else if (isupper(c))
			*inp = tolower(c);
		alph = (c==0) ? 0 : alph+1;
		if (alph == 1)
			cv[nv++] = inp;
		if (alph > 6)
			*inp = 0;
		oldc=c;
	}
	if ((grepin=open (argig, 0)) < 0)
		err("Can't read fgrep index %s", argig);
	if ((grepout=creat(gfile, 0666)) < 0)
		err("Can't write fgrep output %s", gfile);
	fgrep(nv, cv);
	close(grepin);
	close (grepout); 

	gf = fopen(gfile, "r");
	if (gf==NULL)
		err("can't read fgrep output %s", gfile);
	while (fgets(line, TAGLEN, gf) == line)
	{
		for(s=line; *s && (*s != '\t'); s++);
		if (*s == '\t')
		{
			*s++ = '\n';
			*s++ = 0;
		}
		if (line[0]) {
			if (++nfound == 1) {
				strcpy(tagout, line);
				findline(tagout,text,TXTLEN,0L);
			} else {
			    if (nfound == 2) {
				fprintf(stderr,
				    "%s: Multiple matches for '%s'",
				    Ifile,sinput);
				fprintf(stderr,"in %s\n",Index);
				title(text);
			    }
			    findline(line,inb,QLEN,0L);
			    title(inb);
			}
		}
		while (*s) s++;
		if (s[-1]!= '\n')
			while (!feof(gf) && getc(gf)!= '\n') ;
	}
	fclose(gf);
	unlink (gfile);
	return(0);
}

clfgrep()
{
	if (gfile[0])
		unlink(gfile);
}
