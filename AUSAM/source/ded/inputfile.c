#include "ded.h"
#include "char.h"
#include "file.h"

int tbout false;        /* set by -t flag, t+ command */

int filewritten false;  /* used to give exit status */
savefile(err, dlogging)
int (*err)();
int dlogging;
 { char newname[ENOUGH], oldname[ENOUGH];
    char *filen;
    int diag1, diag2;
    struct inode filestat;
    int f_out, f_mode;
    char f_buf[512];
    int oldf, filef;
    int bcount;

    /* if file doesn't exist, just write it */
    if (stat(filename, &filestat) == -1)
     { f_mode = 0666; diag1 = false; goto writeit; }
    else
     { f_mode = (filestat.flags) & 07777;
	if (filestat.nlinks <= 1) /* unlinked - try fast strategy */
	 { cat3(pathname,pidname,".nded",newname);

	    /* try to create a new version */
	    if ((f_out = creat(newname, f_mode)) == -1) goto strat_b;

	    writefile(f_out);

	    ncat2(filename,".old",oldname);
	    unlink(oldname); /* doesn't matter if this fails */

	    if (link(filename, oldname) == -1)
	     { unlink(newname); goto strat_b; }

	    if (unlink(filename) == -1)
	      { unlink(newname); unlink(oldname); goto strat_b; }

	    if (link(newname, filename) == -1)
	     { unlink(newname); unlink(oldname); goto strat_b; }

	    unlink(newname);
	    chmod(oldname, f_mode | 0600);
	 }
	else
	 { /* file has links - overwrite it */
    strat_b: /* come here when the simple strategy won't work */

	    /* first make a copy of the existing file */
	    ncat2(filename, ".old", oldname);
	    if ((oldf = creat(oldname, f_mode | 0600)) == -1)
	     { ncat2(tailname, ".old", oldname);
		if ((oldf = creat(oldname, f_mode | 0600)) == -1)
		 { cat3("/tmp/", pidname, ".old", oldname);
		    if ((oldf = creat(oldname, f_mode | 0600)) == -1)
		      editerror("cannot write .old file");
		 }
		diag1 = true;
	     }
	    else diag1 = false;

	    if ((filef = open(filename, 0)) == -1)
	      editerror("cannot re-open input file");

	    while ((bcount = read(filef, f_buf, 512)) > 0)
	      if (write(oldf, f_buf, bcount)!=bcount)
		editerror("write failed on .old file - no space?");

	    close(oldf); close(filef);

	    /* now write the file */
   writeit: filen = filename;
	    if ((f_out = creat(filen, f_mode)) == -1)
	     { filen = newname;
		cat2(pidname, "dedout", newname);
		if ((f_out = creat(filen, f_mode)) == -1)
		 { cat3("/tmp/", pidname, "dedout", newname);
		    if ((f_out = creat(filen, f_mode)) == -1)
		      editerror("cannot write output file");
		 }
		diag2 = true;
	     }
	    else diag2 = false;

	    writefile(f_out);

	    if (diag1 & diag2)
	      (*err)("old file is %s, new file is %s", oldname, filen);
	    else
	    if (diag1)
	      (*err)("old file is %s", oldname);
	    else
	    if (diag2)
	      (*err)("edited file is %s", filen);
	 }
     }

    if (dlogging && tty_input==2)
     { dlogflush(); close(dlogger);
	if ((dlogger = creat_dlog(dlogname)) == -1)
	  editerror("cannot re-create .dlog file");
     }

    filewritten = true;
    return(true);
 }

char *tabs(n, wbuf)
int n;
char *wbuf;
 { register int rn;
    register char *rw;

    rn = n; rw = wbuf;

    while (rn>=8)
     { *rw++ = c_TAB; rn =- 8; }

    while (rn>0)
     { *rw++ = c_SPACE; rn--; }

    return(rw);
 }

writefile(fout)
int fout;
 { int my_maxl;
    char lbuf[ENOUGH], wbuf[ENOUGH+1];
    char fbuf[512];
    int i;
    register char c, *lp, *fp;
    char *fplim;

    /* find true file length */
    my_maxl = maxl;
    do getline(my_maxl, lbuf); while (*lbuf==0 && --my_maxl>=0);

    fp = fbuf; fplim = &fbuf[511];

    for (i = 0; i<=my_maxl; i++)
     { getline(i,lbuf); tranline(lbuf, wbuf);
	lp = wbuf;
	do
	 { if (fp>fplim)
	     { if (write(fout, fbuf, 512)!=512)
		  editerror("write failed on edited file - no space?");
		fp = fbuf;
	     }
	    c = *fp++ = *lp++;
	  } while (c != 0);
	fp--; /* back over the zero */
     }

    if (fp != fbuf)
     if (write(fout, fbuf, fp-fbuf)!=fp-fbuf)
	editerror("write failed on edited file - no space?");

    close(fout); tmp_changed = false;
 }

tranline(lbuf, wbuf)
char *lbuf, *wbuf;
 { register char *rl, *rw;
    register char c;
    char cb;

    rl = lbuf;
    rw = (tbout && *rl==c_SPACE ? 0 : wbuf);

    while (c = *rl++)
      switch (ictab[c])
	{  case MODIFY:
		if (rw==0) rw = tabs(rl-lbuf-1, wbuf);
		/* convert back to {, } etc. */
		c = *rl++;
		cb = modify(c);
		if (cb!=c || cb==c_MODIFY)
		  *rw++ = cb;
		else
		 { /* bad conversion ! */
		    rl--;
		    *rw++ = c_MODIFY;
		 }
		break;

	    case CONTROL:
		if (rw==0) rw = tabs(rl-lbuf-1, wbuf);
		c = *rl++;
		if (c=='?') *rw++ = 0177;
		else if ('A'<=c && c<='_') *rw++ = c-0100;
		else if ('a'<=c && c<='z') *rw++ = c-0140;
		else if (c==0)
		 { *rw++ = '\0'; return; } /* broken line */
		else rl--; /* bad conversion - forget it */
		break;

	    case SPACE:
		if (rw != 0) *rw++ = c;
		break;

	    default:
		if (rw==0) rw = tabs(rl-lbuf-1, wbuf);
		*rw++ = c;
		break;
	 }

    if (rw==0) rw = wbuf;
    *rw++ = '\n'; *rw++ = '\0';

 }

/* procedures to set up the input file system */

int in_file;
char in_cache[512];
int in_block;
char *in_p, *inlim;

infile(fname, tname)
char *fname, *tname;
 { struct iobuf i_buf;
    char linebuf[ENOUGH];

    maketemp(tname);
    if ((in_file = open(fname, 0)) == -1)
     { if (errno == EACCES)
	  quit("cannot read file %s", fname);
	else
	  quit("ded error - errno %o in infile", errno);
    }

    in_p = in_cache;
    inblock(0);

    while(inline(linebuf))
      if (++maxl>NLINES)
	quit("too many lines (infile)");
      else
	addline(maxl, linebuf);

    close(in_file);
 }

int inline(linebuf)
char *linebuf;
 { char *lastchar, *lastin;
    register char *lp;
    register char c;
    register char *inp;
    char *lplim;
    char *lastspace, *inspace;

    lastchar=0;
    lp = linebuf; lplim = lp+ncols-1;
    inp = in_p;
    lastspace = inspace = 0;

    while (true)
     { if (inp>=inlim)
	 { inblock(in_block+1);
	    if (inlim<=in_cache)
	     { in_p = in_cache;
		if (lastchar==0)
		 { *linebuf = '\0'; return(false); }
		else
		 { *lp++ = '\0'; return(true); }
	     }
	    else inp = in_cache;
	 }

	switch (ictab[c = *inp++])
	 { case SPACE:
		if (lp>lplim) break;
		else
		 {  lastchar = lp; lastin = inp;
		    *lp++ = c;
		    if (lp<lplim)
		     { lastspace = lp-1; inspace = inp; }
		    continue;
		 }

	    case ONE:
		if (lp>lplim) break;
		else
		 { lastchar=lp; lastin=inp; *lp++ = c; continue; }

	    case MODIFY:
	    case TWO:
		if (lp>=lplim) break;
		else
		 { lastchar=lp; lastin=inp; *lp++ = c_MODIFY;
		    *lp++ = unmodify(c);
		    continue;
		 }

	    case TAB:
		if (lp+8 - ((lp-linebuf+8) % 8) > lplim) break;
		else
		 {  lastchar = lp; lastin = inp;
		    do *lp++ = c_SPACE; while ((lp-linebuf) % 8 != 0);
		    if (lp<lplim)
		     { lastspace = lp-1; inspace = inp; }
		    continue;
		 }

	    default:
		if (c == c_NL)
		 { if (lastchar==0) *linebuf = '\0';
		   else *lp++ = '\0';
		   in_p = inp;
		   return(true);
		 }
		else
		if (0<c && c<=037 || c==0177)
		 { /* display control characters as ^Z<letter> on ITT,
		     * bold-face ^<letter> on Hazeltine
		     */
		    if (lp>=lplim) break;
		    else
		     { lastchar=lp; lastin=inp; *lp++ = int_CONTROL;
			*lp++ = (c==0177 ? '?' : c + 0100);
			continue;
		     }
		 }
		else
		if (c==0) continue;
		else
		  editerror("input file contains character %o", c);

	 } /* end of switch (ictab) */

	/* come here when line is too long to fit on page */
	if (lastspace != 0)
	 /* put a zero after the last space */
	 { lp = lastspace+1; *lp++=int_CONTROL; *lp++='\0';
	    in_p = inspace;
	 }
	else
	if (lastchar!=0)
	 /* put a zero in place of the last character */
	 { lp = lastchar; *lp++ = int_CONTROL; *lp++ = '\0';
	    in_p = lastin-1;
	 }
	else
	  editerror("dropped out of loop (infile)");

	if (inp<in_p) inblock(in_block-1);
	return(true);
     } /* end of while(true) */
 } /* end of inline */

inblock(block)
int block;
 { seek(in_file, (in_block=block), 3);
    inlim = in_cache + read(in_file, in_cache, 512);
    /* note that inlim points PAST the end of in_cache */
 }
