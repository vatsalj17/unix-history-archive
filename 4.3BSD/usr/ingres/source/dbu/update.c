# include	<stdio.h>
# include	<ingres.h>
# include	<resp.h>
# include	<aux.h>
# include	<symbol.h>
# include	<access.h>
# include	<batch.h>
# include	<opsys.h>
# include	<catalog.h>
# include	<btree.h>
# include	<version.h>
# include	<sccs.h>
# include	<errors.h>

SCCSID(@(#)update.c	8.4	2/8/85)

/*
**	Update reads a batch file written by the
**	access method routines (openbatch, addbatch, closebatch)
**	and performs the updates stored in the file.
**
**	It assumes that it is running in the database. It
**	is driven by the data in the Batchhd struct (see ../batch.h).
**	If the relation has a secondary index then update calls
**	secupdate. As a last step the batch file is removed.
**
**	The global flag Batch_recovery is tested in case
**	of an error. It should be FALSE if update is being
**	run as the dbu deferred update processor. It should
**	be TRUE if it is being used as part of the recovery
**	procedure.
*/

update()
{
	register int	i, mode;
	DESC		rel, d;
	long		oldtid, tupcnt;
	char		oldtup[MAXTUP], newtup[MAXTUP], tuple[MAXTUP];
	char		*batchname(), *trim_relname();
	char		*tp;
	long		bad_lid[MAXLID], lid, old_lid[MAXLID], new_lid;
	char		bad[MAXLID][10], *locv();
	char		delbtree[MAXNAME + 4], replbtree[MAXNAME + 4];
	char		tup_buf[3 * LIDSIZE], lid_buf[LIDSIZE];
	char		btree[MAXNAME + 4], out[MAXNAME + 4];
	char		filname[MAXNAME+4];
	TID		tidpos;
	struct stat	sbuf;
	long		num;
	int		first, end, batchcnt, j, k;
	FILE		*fp;
	long		temp;
	long		del_cnt;
	extern int	Btree_fd;
	extern DESC	Btreesec;

#	ifdef xZTR1
	if (tTf(48, -1))
		printf("Update on %s\n", batchname());
#	endif
	/* set up to read batchhd */
	Batch_cnt = BATCHSIZE;	/* force a read on next getbatch */
	Batch_dirty = FALSE;
	if ((Batch_fp = open(batchname(), O_RDWR)) < 0)
		syserr("prim:can't open %s", batchname());
	getbatch(&Batchhd, sizeof Batchhd);

	tupcnt = Batchhd.num_updts;
#	ifdef xZTR1
	if (tTf(48, 0))
		printf("rel=%s tups=%ld\n", Batchhd.rel_name, tupcnt);
#	endif
	Resp.resp_tups = 0;
	if (!tupcnt)
	{
		rmbatch();
		return (1);
	}

	/* update the primary relation */
	if (i = openr(&rel, OR_WRITE, Batchhd.rel_name))
		syserr("prim:can't openr %s %d", Batchhd.rel_name, i);
	if (rel.reldum.reldim > 0)
	{
		bmove(rel.relbtree, &Btreesec, sizeof(Btreesec));
		Btree_fd = rel.btree_fd;
	}

	mode = Batchhd.mode_up;

	if (rel.reldum.reldim > 0)
	/* create files necessary for updating btrees in specified order */
	{
		concat(REPL_IN, Fileset, replbtree);
		if ((Repl_infp = fopen(replbtree, "w")) == NULL)
			syserr("can't open %s", replbtree);
		concat(DEL_IN, Fileset, delbtree);
		if ((Del_infp = fopen(delbtree, "w")) == NULL)
			syserr("can't open %s", delbtree);
	}
	Del_cnt = 0;
	for (i = 0; i < MAXLID; ++i)
	{
		Prev_lid[i] = 0;
		Repl_cnt[i] = 0;
	}

	if (rel.reldum.reldim > 0)
	{
		if (tupcnt <= 1 || (mode != mdREPL && mode != mdAPP))
			fclose(Repl_infp);
		else
		/* do replace's in ascending lid-value order */
		{
			d.reloff[1] = 0;
			d.reloff[2] = Batchhd.tido_size + Batchhd.tupo_size + Batchhd.tupn_size - LIDSIZE;
			for (i = 1; i <= rel.reldum.reldim; ++i)
			{
				d.reloff[i+2] = d.reloff[i+1] + LIDSIZE;
				d.relfrmt[i+1] = INT;
				d.relfrml[i+1] = LIDSIZE;
				d.relgiven[i+1] = i;
			}
			d.relfrmt[i+1] = CHAR;
			d.relfrml[i+1] = Batchhd.tidn_size;
			d.relgiven[0] = 0;
			d.relgiven[1] = i;
			d.relgiven[i+1] = i + 1;
			d.relfrmt[1] = CHAR;
			d.relfrml[1] = d.reloff[2];
			d.reldum.relspec = M_ORDER;
			d.reldum.relatts = 2 + rel.reldum.reldim;
			d.reldum.relwid = d.reloff[2] + LIDSIZE + Batchhd.tidn_size;
			/* extract information about tuples from batch file */
			if (stat(batchname(), &sbuf) < 0)
				syserr("bad file for stat %s", batchname());
			num = sbuf.st_size / (BATCHSIZE + IDSIZE);
			if (num >= 1)
				first = BATCHSIZE - Batch_cnt;
			else
				first = sbuf.st_size - sizeof Batchhd - IDSIZE;
			if ((i = fwrite(&Batchbuf.bbuf[Batch_cnt], 1, first, Repl_infp)) != first)
				syserr("can't write replace file");
			for (i = 2; i <= num; ++i)
			{
				Batch_cnt = BATCHSIZE;
				readbatch();
				if (fwrite(Batchbuf.bbuf, 1, BATCHSIZE, Repl_infp) != BATCHSIZE)
					syserr("can't write to replace file");
			}
			Batch_cnt = BATCHSIZE;
			readbatch();
			end = ((sbuf.st_size - BATCHSIZE - IDSIZE) % (BATCHSIZE + IDSIZE)) - IDSIZE;
			if (end > 0)
				if (fwrite(Batchbuf.bbuf, 1, end, Repl_infp) != end)
					syserr("can't write to replace file 2");
			fclose(Repl_infp);
			sortfile(replbtree, &d, FALSE);
			if ((Repl_outfp = fopen(ztack(REPL_OUT, Fileset), "r")) == NULL)
				syserr("can't open replace file in update for reading\n");
			concat("_SYStemp", Fileset, filname);
			/* rewrite in batch file in sorted order */
			if ((fp = fopen(filname, "w")) == NULL)
				syserr("can't open %s", filname);
			if ((k = fread(Batchbuf.bbuf, 1, first, Repl_outfp)) != first)
				syserr("read error0 from replace file %d", k);
			if (fwrite(Batchbuf.file_id, 1, IDSIZE, fp) != IDSIZE)
				syserr("write error in batch file");
			if (fwrite(&Batchhd, 1, sizeof Batchhd, fp) != sizeof Batchhd)
				syserr("write error in batch file");
			if (fwrite(Batchbuf.bbuf, 1, first, fp) != first)
				syserr("write error in batch file");
			for (i = 2; i <= num; ++i)
			{
				if ((k = fread(Batchbuf.bbuf, 1, BATCHSIZE, Repl_outfp)) != BATCHSIZE)
					syserr("read error1 in replace file %d",  k);
				if (fwrite(&Batchbuf, 1, BATCHSIZE + IDSIZE, fp) != BATCHSIZE + IDSIZE)
					syserr("write error into temp repl file");
			}
			if (end > 0)
			{
				if ((k = fread(Batchbuf.bbuf, 1, end, Repl_outfp)) != end)
					syserr("read error2 from replace file %d", k);
				if (fwrite(&Batchbuf, 1, end + IDSIZE, fp) != end + IDSIZE)
					syserr("write error into temp repl file");
			}
			fclose(fp);
			fclose(Repl_outfp);
			unlink(ztack(REPL_OUT, Fileset));
			rmbatch();
			if (link(filname, batchname()) == -1)
				syserr("can't link %s", batchname());
			unlink(filname);
			Batch_cnt = BATCHSIZE;
			Batch_dirty = FALSE;
			if ((Batch_fp = open(batchname(), O_RDWR)) < 0)
				syserr("can't open new batch file");
			getbatch(&Batchhd, sizeof Batchhd);
		}
		unlink(replbtree);
	}

	while (tupcnt--)
	{
		getbatch(&oldtid, Batchhd.tido_size);	/* read old tid */
		getbatch(oldtup, Batchhd.tupo_size);	/* and portions of old tuple */
		if (!rel.reldum.reldim)
			getbatch(newtup, Batchhd.tupn_size);	/* and the newtup */
		else
		{
			if (Batchhd.tupn_size > 0)
			{
				getbatch(newtup, Batchhd.tupn_size - rel.reldum.reldim * LIDSIZE);
				batchcnt = Batch_cnt;
				tp = newtup + Batchhd.tupn_size - rel.reldum.reldim * LIDSIZE;
				getbatch(tp, rel.reldum.reldim * LIDSIZE);
			}
		}

		switch (mode)
		{

		  case mdDEL:
			if ((i = delete(&rel, &oldtid)) < 0)
				syserr("prim:bad del %d %s", i, Batchhd.rel_name);
			break;

		  case mdREPL:
			if (i = replace(&rel, &oldtid, newtup, TRUE))
			{
				/* if newtuple is a duplicate, then ok */
				if (i == 1)
				{
					if (rel.reldum.reldim)
						++Resp.resp_tups;
					break;
				}
				if (i == 3)
				{
					bmove(newtup + rel.reldum.relwid - LIDSIZE, &new_lid, LIDSIZE);
					bmove(tp, bad_lid, LIDSIZE * rel.reldum.reldim);
					for(j = 0; j < rel.reldum.reldim; ++j)
						strcpy(bad[j], locv(bad_lid[j]));
					switch (rel.reldum.reldim)
					{
					case 1:
						nferror(BADLID1, trim_relname(rel.reldum.relid), bad[0], 0);
						break;
					case 2:
						nferror(BADLID2, trim_relname(rel.reldum.relid), bad[0], bad[1], 0);
						break;
					case 3:
						nferror(BADLID3, trim_relname(rel.reldum.relid), bad[0], bad[1], bad[2], 0);
						break;
					}					
					Batch_cnt = batchcnt + LIDSIZE * (rel.reldum.reldim - 1);
					lid = -1;
					putbatch(&lid, LIDSIZE);
					break;
				}
				/* if this is recovery and oldtup not there, try to insert newtup */
				if (Batch_recovery && i == 2)
					goto upinsert;
				syserr("prim:Non-functional replace on %s (%d)", i, Batchhd.rel_name);
			}
			Resp.resp_tups++;
			break;

		  case mdAPP:
		  upinsert:
			if ((i = insert(&rel, &oldtid, newtup, TRUE)) < 0)
				syserr("prim:bad insert %d %s", i, Batchhd.rel_name);
 			if (i == 2)
			{
				tp = newtup + rel.reldum.relwid - rel.reldum.reldim * LIDSIZE;
				bmove(tp, bad_lid, LIDSIZE * rel.reldum.reldim);
				for (j = 0; j < rel.reldum.reldim; ++j)
					strcpy(bad[j], locv(bad_lid[j]));
				switch (rel.reldum.reldim)
				{
				case 1:
					nferror(BADLID1, trim_relname(rel.reldum.relid), bad[0], 0);
					break;
				case 2:
					nferror(BADLID2, trim_relname(rel.reldum.relid), bad[0], bad[1], 0);
					break;
				case 3:
					nferror(BADLID3, trim_relname(rel.reldum.relid), bad[0], bad[1], bad[2], 0);
					break;
				}					
				oldtid = -1;
			}
			else if (rel.reldum.reldim > 0)
			{
				if (batchcnt + rel.reldum.reldim * LIDSIZE > BATCHSIZE)
				{
					if ((j = lseek(Batch_fp, (long) -(Batch_cnt + BATCHSIZE + 2 * IDSIZE), 1)) < 0)
						syserr("Lseek error in update");
					readbatch();
				}
				Batch_cnt = batchcnt;
				tp  =  newtup + rel.reldum.relwid - LIDSIZE * rel.reldum.reldim;
				putbatch(tp, rel.reldum.reldim * LIDSIZE);
			}
			break;

		  default:
			syserr("prim:impossible mode %d", mode);
		}
		putbatch(&oldtid, Batchhd.tidn_size);	/* write new tid if necessary */
	}
	if (rel.reldum.reldim > 0)
	/* do deletions in decending lid-value order */
	{
		fclose(Del_infp);
		if (Del_cnt != 0)
		{
			if (Del_cnt > 1)
			{
				d.reloff[0] = -LIDSIZE;
				d.relgiven[0] = 0;
				for (i = 1; i <= rel.reldum.reldim; ++i)
				{
					d.reloff[i] = d.reloff[i-1] + LIDSIZE;
					d.relfrmt[i] = INT;
					d.relfrml[i] = LIDSIZE;
					d.relgiven[i] = -i;
				}
				d.reldum.relspec = -M_ORDER;
				d.reldum.relatts = rel.reldum.reldim;
				d.reldum.relwid = LIDSIZE * rel.reldum.reldim;
				sortfile(delbtree, &d, TRUE);
			}
			btreename(rel.reldum.relid, btree);
			del_cnt = Del_cnt;
			if (del_cnt == 1)
				concat(DEL_IN, Fileset, out);
			else
				concat(DEL_OUT, Fileset, out);
			if ((Del_outfp = fopen(out, "r")) == NULL)
				syserr("can't open delete file in update for reading\n");
			while (del_cnt--)
			{
				if (fread(old_lid, 1, LIDSIZE * rel.reldum.reldim, Del_outfp) != LIDSIZE * rel.reldum.reldim)
					syserr("tup_buf read error");

				if (delete_btree(old_lid, rel.reldum.reldim) < 0)
				{
					printf("DELETE ERROR: %s: bad lid(s)\n", trim_relname(rel.reldum.relid));
					for (i = 0; i < rel.reldum.reldim; ++i)
						printf("\tlid%d=%ld\n", i + 1, old_lid[i]);
					syserr("DELETE ERROR");
				}
			}
			fclose(Del_outfp);
			unlink(out);
		}
		unlink(delbtree);
	}

	/* fix the tupchanged count if delete or append */
	if (mode != mdREPL)
		Resp.resp_tups = rel.reladds >= 0 ? rel.reladds : -rel.reladds;
	/* close the relation but secupdate will still use the decriptor */
	temp = rel.reladds;
	if (i = closer(&rel))
		syserr("prim:close err %d %s", i, Batchhd.rel_name);
	rel.reladds = temp;
	batchflush();

	/* if this relation is indexed, update the indexes */
	if (rel.reldum.relindxd > 0)
		secupdate(&rel);
	if (rel.reldum.reldim > 0)
		btreeupdate(&rel);
	rmbatch();

#	ifdef xZTR1
	if (tTf(48, 2))
		printf("%ld tups changed\n", Resp.resp_tups);
#	endif
	return (0);
}
