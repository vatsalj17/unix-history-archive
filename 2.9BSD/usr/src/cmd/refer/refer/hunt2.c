# include "refer.h"
static int *coord 0;
int hh[NQUERY]; 
extern int *hfreq, hfrflg, hcomp(), hexch();
extern int nhash;

doquery(nitem)
{
	long k;
	int nf 0;
	register nterm 0, i, g, j;
	long lp;
	long getl(); 
	unsigned getw(); 

	if (coord==0)
		coord = zalloc(lmaster, sizeof(lmaster));
	if (coord==0)
		err("No space for coord list",0);
	for(i=0; i<nitem; i++)
		hh[i] = hash(qitem[i])%nhash;
	/* if possible, sort query into decreasing frequency of hashes */
	if (hfrflg)
		shell (nitem, hcomp, hexch);
	lp = hpt [hh[0]];
	fseek(fb,lp,0);
	for(i=0; i<lmaster; i++)
	{
		coord[i]=1;
		if (iflong) {
			if ((master.b[i] = getl(fb)) == -1L) break;
		} else {
			if ((master.a[i] = getw(fb)) == -1) break;
		}
	}
	if (i==lmaster)
		fprintf(stderr,"master list too short\n");
	nf= i;
	for(nterm=1; nterm<nitem; nterm++)
	{
		lp = hpt[hh[nterm]];
		fseek(fb, lp, 0);
		g=j=0;
		while (1)
		{
			if (iflong)
				k = getl(fb);
			else
				k = getw(fb);
			if (k== -1) break;
			while (j<nf && (iflong?master.b[j]:master.a[j])<k)
			{
				if (coord[j] <= nterm)
					j++;
				else
				{
					if (iflong)
						master.b[g] = master.b[j];
					else
						master.a[g] = master.a[j];
					coord[g++] = coord[j++];
					continue;
				}
			}
			if (j>=nf) break;
			if ((iflong? master.b[j]: master.a[j]) == k)
			{
				if (iflong)
					master.b[g]=k;
				else
					master.a[g]=k;
				coord[g++] = coord[j++]+1;
			}
		}
		nf = g;
	}
	return(nf);
}
long
getl(fb)
FILE *fb;
{
	int x[2];
	long *lp;
	x[0] = getw(fb);
	x[1] = getw(fb);
	lp= x;
	return(*lp);
}
hcomp( n1, n2)
{
	return (hfreq[hh[n1]]<=hfreq[hh[n2]]);
}
hexch( n1, n2 )
{
	int t;
	t = hh[n1];
	hh[n1] = hh[n2];
	hh[n2] = t;
}
