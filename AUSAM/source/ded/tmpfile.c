#include "ded.h"
#include "char.h"
#include "file.h"

/* file handler */

int f_tmp;
int f_write;
char f_cache[512];

#define fpLIM &f_cache[511]
int f_cblock;           /* block offset of file cache */

int tmp_block, tmp_offset; /* where the end of the temporary file is */
int l_block[NLINES], l_offset[NLINES]; /* addresses of lines in temp file */

/* data structure to help with definition of ranges of lines */
int b_range[26]
 { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1
 };
int e_range[26]
 { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1
 };

/* create the temporary file */
maketemp(tname)
char *tname;
 { if ((f_tmp = creat(tname, 0666)) == -1)
      quit("cannot create temporary file %s", tname);
    close(f_tmp);
    if ((f_tmp = open(tname, 2)) == -1)
      quit("cannot re-open temporary file %s", tname);

    maxl = -1;
    tmp_block = tmp_offset = 0;
 }

getline(fline,linebuf)
int fline;
char *linebuf;
 { if (topl<=fline && fline<=topl+LASTINROW)
      copyrow(fline-topl,0,linebuf);
    else
      getfline(fline,linebuf);
 }

/* gets a line from disk, in disk format,
 * into store, in display format
 */
getfline(fline,linebuf)
int fline;
char *linebuf;
 { register char *lp, *fp;
    int block, offset;

    block = l_block[fline];
    offset = l_offset[fline];

    if (offset == -1 || fline>maxl)
      linebuf[0] = 0;
    else
     { getblock(block);
	lp = linebuf;
	fp = f_cache+offset;

	while ((*lp++ = *fp++) != 0)
	    if (fp > fpLIM)
	     {  getblock(++block);
		 fp = f_cache;
	     }

	*lp = 0;
     }
    return;

}

/* writes the line in buf
 * onto the end of the file
 */

addline(fline,linebuf)
int fline;
char *linebuf;
{
    register char *fp, *lp;
    int block, offset;
    register char c;
    char cb;
    int count;

    if (linebuf==0 || *linebuf==0)
      l_block[fline] = l_offset[fline] = -1;
    else
     { block = tmp_block;
	offset = tmp_offset;
	getblock(block); f_write++;
	fp = f_cache + offset;
	lp = linebuf;

	do
	 { *fp++ = c = *lp++;

	    if (fp > fpLIM)
	     { getblock(++block);
		f_write++;
		fp = f_cache;
	     }
	 } while (c != 0);

	l_block[fline] = tmp_block; tmp_block = block;
	l_offset[fline] = tmp_offset; tmp_offset = (fp-f_cache);
     }
}

emptyline(fline)
int fline;
 { return(topl<=fline && fline<=topl+LASTINROW ? emptyrow(fline-topl)
						: l_offset[fline] == -1);
 }


/* gets the block into the cache
 *
 */
getblock(block)
{

	if (f_cblock != block) {
		if (f_write) {
			seek(f_tmp, f_cblock, 3);
			write(f_tmp, f_cache, 512);
		}
		seek(f_tmp, f_cblock=block, 3);
		read(f_tmp, f_cache, 512);
		f_write = 0;
	}
}

/* procedure to move lines up or down to make or delete space */
shuffile(fline,incr)
int fline, incr;
 { xshuffile(fline, incr); adj_maxl(); }

xshuffile(fline, incr)
int fline, incr;
 { register int i;

    if (incr > 0) /* moving up */
     { if (maxl+incr > NLINES)
	  editerror("too many lines in file");

	for (i=maxl; i>=fline; i--)
	 { l_block[i+incr] = l_block[i];
	    l_offset[i+incr] = l_offset[i];
	 }

	for (i='a'; i<='z'; i++)
	 { if (b_range[i-'a']>=fline) b_range[i-'a'] =+ incr;
	    if (e_range[i-'a']>=fline) e_range[i-'a'] =+ incr;
	 }

	maxl =+ incr;
     }
    else
    if (incr < 0) /* moving down */
     { if (fline-incr < 0)
	  editerror("attempt to shuffle lines off top of file");

	for (i=fline; i<=maxl; i++)
	 { l_block[i+incr] = l_block[i];
	    l_offset[i+incr] = l_offset[i];
	 }

	for (i='a'; i<='z'; i++)
	 { if (b_range[i-'a']>=fline) b_range[i-'a'] =+ incr;
	    else
	    if (b_range[i-'a']>=fline+incr) b_range[i-'a'] = fline+incr;

	    if (e_range[i-'a']>=fline) e_range[i-'a'] =+ incr;
	    else
	    if (e_range[i-'a']>=fline+incr) e_range[i-'a'] = fline+incr;
	 }

	if (maxl+incr>=topl+LASTINROW) maxl =+ incr;
	else
	 { for (i=maxl+incr+1; i<=topl+LASTINROW; i++)
	     l_block[i] = l_offset[i] = -1;
	    maxl = topl+LASTINROW;
	 }
     }

    tmp_changed = true;
 }

/* strip trailing blank lines from the file -
 * but not if there are any refs to the last line
 */
adj_maxl()
{ register int i;
   while (maxl>topl+LASTINROW && l_offset[maxl]<0)
    { for (i='a'; i<='z'; i++)
	 if (b_range[i-'a']==maxl || e_range[i-'a']==maxl) return;
       maxl--;
    }
 }

/* procedures to shift lines about */
copyline(source, destination)
int source, destination;
 { register int block, offset;

    while(source>maxl) inc_maxl();
    block = l_block[source]; offset = l_offset[source];
    /* don't let shuffile alter any lines */
    xshuffile(destination, 1);
    l_block[destination] = block; l_offset[destination] = offset;
 }

moveline(source, destination)
int source, destination;
 { register int i;

    copyline(source, destination);
    if (source >= destination) source++;

    for (i='a'; i<='z'; i++)
     { if (b_range[i-'a']==source) b_range[i-'a'] = destination;
	if (e_range[i-'a']==source) e_range[i-'a'] = destination;
     }

    /* don't let shuffile alter any lines */
    xshuffile(source+1, -1);
 }
