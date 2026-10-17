#include "refer.h"
# include "stdio.h"
# define unopen(fil) {if (fil!=NULL) {fclose(fil); fil=NULL;}}
extern char refdir[];
int lmaster LLIST;
int *hfreq, hfrflg;
union ptr master NULL;
int iflong;
long *hpt 0;
int nhash 0;
extern char *fgnames[], **fgnamp;
extern char fgspace[], *fgp;
int typeindex 0;
char usedir[100];
long indexdate 0, gdate();
FILE *fa, *fb, *fc;
char *qitem[NQUERY];

huntmain(index)
char *index;
{
	char indexname[100];
	char grepquery[QLEN];
	static char oldname[15] ;
	static int maxhash 0;
	int nitem, frtbl;

	savedir();
	strcpy (indexname, todir(index));
	fgnamp = fgnames;
	fgp = fgspace;
	if (typeindex == 0 || strcmp (oldname, indexname) !=0)
	{
		strcpy (oldname, indexname);
		unopen(fa); 
		unopen(fb); 
		unopen(fc);

		if (ckexist(indexname, ".ib"))
		{
			fa = iopen(indexname, ".ia");
			fb = iopen(indexname, ".ib");
			fc = iopen(indexname, ".ic");
			typeindex =1;
			indexdate = gdate(fb);
			fread (&nhash, sizeof(nhash), 1, fa);
			fread (&iflong, sizeof(iflong), 1, fa);
			if (nhash > maxhash)
			{
				if (hpt)
					free (hpt, maxhash, sizeof(*hpt));
				hpt=0;
				if (hfreq)
					free(hfreq, maxhash, sizeof(*hfreq));
				hfreq=0;
				maxhash=nhash;
			}
			if (hpt==0)
				hpt = zalloc(nhash, sizeof(*hpt));
			if (hpt == NULL)
				err ("No space for hash list (%d)", nhash);
			fread( hpt, sizeof(*hpt), nhash, fa);
			if (hfreq==0)
				hfreq=zalloc(nhash, sizeof(*hfreq));
			if (hfreq==NULL)
				err ("No space for hash frequencies (%d)", nhash);
			frtbl = fread(hfreq, sizeof(*hfreq), nhash, fa);
			hfrflg = (frtbl == nhash);
			if (master.a == NULL)
				if (iflong)
					master.b = zalloc(lmaster, sizeof(long));
				else
					master.a = zalloc(lmaster, sizeof(int));
			if (master.a == NULL)
				err ("no space for answer list",0);
		}
		else
			if (makefgrep(indexname))
				typeindex=2;
			else
			{
				err ("No files %s\n",indexname);
				exit(1);
			}
	}

	tagout[0]=0;
	if (typeindex==2)
	{
		grepcall(sinput, indexname);
		restodir();
		return;
	}
	nitem = getq(qitem);
	nfound = doquery(nitem);
	nfound = baddrop(nfound, nitem);
	if (fgnamp>fgnames)
	{
		char **fgp;
		int k;
		grepquery[0]=0;
		for(k=0; k<nitem; k++)
		{
			strcat(grepquery, " ");
			strcat(grepquery, qitem[k]);
		}
		for(fgp=fgnames; fgp<fgnamp; fgp++)
		{
			makefgrep(*fgp);
			grepcall(grepquery, *fgp);
		}
	}
	restodir();
}

todir(t)
char *t;
{
	char *s;
	s=t;
	while (*s) s++;
	while (s>=t && *s != '/') s--;
	if (s<t) {
		if (usedir[0])
			typeindex = 0;
		usedir[0] = 0;
		return(t);
	}
	*s++ = 0;
	t = (*t ? t : "/");
	chdir (t);
	if (strcmp(usedir,t))
		typeindex = 0;
	strcpy (usedir,t);
	*(s-1) = '/';
	return(s);
}
# include "sys/types.h"
# include "sys/stat.h"
long gdate(f)
FILE *f;
{
	struct stat sb;
	fstat (f->_file, &sb);
	return  (sb . st_mtime);
}
