# include "refer.h"
int newr[NREF+1];
chkdup ( tag )
char *tag;
{
	int i;

	for(i=1; i<=refnum; i++)
	{
		if (strcmp(reftable[i], tag)==0)
			return(i);
	}
	if (refnum>NREF) {
		err("Too many references, limit %d", NREF);
	}
	if (rtp + strlen(tag)> reftext + NRFTXT)
		err("reftext too big", 0);
	reftable[refnum+1] = rtp;
	strcpy (rtp, tag);
	while (*rtp++);
	return(0);
}
dumpold()
{
	FILE *fi;
	int c, g1 0, nr 1;
	char tb[20], *s;
	if (!endpush) return;
	fclose(fo);
	fo=NULL;
	if (sort)
	{
		char comm[100];
		sprintf(comm, "sort %s -o %s", tfile, tfile);
		system(comm);
	}
	fi = fopen(tfile, "r");
	if (fi==NULL) return;
	flout();
	fprintf(ftemp,".]<\n");
	while ( (c = getc(fi)) >0)
	{
		if (c=='\n')
		{
			nr++;
			g1 = 0;
		}
		if (c==sep)
			c = '\n';
		if (c== FLAG)
		{
			/* make old-new ref number table */
			s = tb;
			while ( (c=getc(fi)) != FLAG)
				*s++ = c;
			*s=0;
			if (g1++ == 0)
				newr[atoi(tb)] = nr;
			fprintf(ftemp,"%d", nr);
			continue;
		}
		putc(c, ftemp);
	}
	fclose(fi);
	unlink(tfile);
	fprintf(ftemp, ".]>\n");
}

recopy (fnam)
char *fnam;
{
	int c;
	char tb[10], *s;
	fclose(ftemp);

	ftemp = fopen(fnam, "r");
	if (ftemp==NULL)
		err("Can't reopen %s\n", fnam);
	while ( (c = getc(ftemp)) != EOF)
	{
		if (c == FLAG)
		{
			s = tb;
			while ( (c = getc(ftemp)) != FLAG)
				*s++ = c;
			*s=0;
			printf("%d", newr[atoi(tb)]);
			continue;
		}
#ifdef	MFLAG
		if ((c == KEYLET) && mflag)
		{
			s = tb;
			while ( (c = getc(ftemp)) != KEYLET)
				*s++ = c;
			*s=0;
			if (labc[atoi(tb)] == 'a')
				putchar('a');
			continue;
		}
#endif
		putchar(c);
	}
	fclose(ftemp);
	unlink(fnam);
}
