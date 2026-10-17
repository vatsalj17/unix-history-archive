# include	<stdio.h>
# include	<ingres.h>
# include	<batch.h>
# include	<opsys.h>
# include	<version.h>
# include	<pv.h>
# include	<symbol.h>
# include	<sccs.h>

SCCSID(@(#)sort.c	8.1	12/31/84)

sortfile(infile, d, del)
char *infile;
register DESC *d;
int del;
{
	char		out[MAXNAME + 4];
	register int	j, i;
	char		buf[50];
	char		*temp;
	int		len;

	if (del)
	{
		concat(DEL_OUT, Fileset, out);
		if ((Del_outfp = fopen(out, "w")) == NULL)
			syserr("can't open %s", out);
		fclose(Del_outfp);
	}
	else
	{
		concat(REPL_OUT, Fileset, out);
		if ((Repl_outfp = fopen(out, "w")) == NULL)
			syserr("can't open %s", out);
		fclose(Repl_outfp);
	}

	flush_rel(d, TRUE);
	resetacc(NULL);

	len = length(Fileset) + 1;
	/*
	temp = (char *) need(Qbuf, len);
	*/
	temp = (char *) calloc(1,len);
	bmove(Fileset, temp, len);

	initp();
	setp(PV_STR, temp);
	setp(PV_STR, infile);
	setp(PV_STR, out);

	setp(PV_STR, d->reldum.relid);
	setp(PV_STR, d->reldum.relowner);
	setp(PV_INT, d->reldum.relspec);
	setp(PV_INT, d->reldum.relindxd);
	setp(PV_INT, d->reldum.relstat2);
	setp(PV_INT, d->reldum.relstat);
	setp(PV_INT, d->reldum.relsave);
	setp(PV_INT, d->reldum.reltups);
	setp(PV_INT, d->reldum.relatts);
	setp(PV_INT, d->reldum.relwid);
	setp(PV_INT, d->reldum.relprim);
	setp(PV_INT, d->reldum.relfree);
	setp(PV_INT, d->reldum.relstamp);
	/* whether or not relation is ordered is irrelevant */
	setp(PV_INT, 0);

	setp(PV_STR, d->relvname);
	setp(PV_INT, d->relfp);
	setp(PV_INT, d->relopn);
	setp(PV_INT, d->reladds);
	setp(PV_INT, d->reltid.ltid);
	for (i = 0; i <= d->reldum.relatts; ++i)
	{
		setp(PV_INT, d->reloff[i]);
		setp(PV_INT, d->relfrmt[i]);
		setp(PV_INT, d->relfrml[i]);
		setp(PV_INT, d->relxtra[i]);
		setp(PV_INT, d->relgiven[i]);
	}

	call(mdKSORT, NULL);

	flush_rel(d, TRUE);
	resetacc(NULL);

	if (del)
		fclose(Del_outfp);
	else
		fclose(Repl_outfp);
	cfree(temp);
}
