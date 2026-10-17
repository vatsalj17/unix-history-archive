#include "hd.h"
#include "mydir.h"

#define	curmtime	wd_stb.st_mtime

/* See dir.h for information about these variables */
int tfiles, tpages, cpage, pageend;

struct stat wd_stb;

int wdfile;
char wdname [MPLEN + 1];

struct direct dirbuf [mfiles + 1];

char *d_namep [mfiles];

int dir_status;
struct stat scr_stb;

char *dirmsg [] = {
	"", "System Error", "Too big to Load", 
	"Executable but not Readable"
};

/*  Enterdir -- Enter into new directory.
    The parameter newdir is the file to change to.  Pass a single
    element file name or a full path name. "file", "..", or "."
    are OK.  "../..", "/a/b/..", "a/", or "a/." must be passed
    to the file procedure instead.

    On success, there is a new current directory, and wdname has been
    modified to reflect the new path name.  ENTERDIR | REPLOT is the
    return value.  On failure, FAILURE is returned.  Nothing is changed.
*/

enterdir (newdir)  char * newdir;  {

int entermode;		/* Type of entry */
#define	FOREWARD	0	/* Deaper into dir */
#define	BACKWARD	1	/* Previous dir (..) */
#define	STOP		2	/* Stay in same place (.) */
#define LOAD		3	/* Load new path name */

if (compe (newdir, DOT)) entermode = STOP;
else if (compe (newdir, DOTDOT)) entermode = BACKWARD;
else if (newdir[0] == '/') entermode = LOAD;
else entermode = FOREWARD;

if (entermode == FOREWARD && strlen (wdname) > LPLEN) {
	putmsg ("Cannot chdir -- Pathname too long");
	return FAILURE;
}

if (chdir (newdir)) {
	myperror (newdir);
	return FAILURE;
}

if (entermode == STOP);			/* change to "." */
else if (entermode == BACKWARDS) {	/* change to ".." */
	todotdot (wdname);
	if (ISROOT (wdname)) chdir (SLASH);
}
else if (entermode == LOAD) strcpy (wdname, newdir);

else {
	if (!ISROOT (wdname)) strcat (wdname, SLASH);
	strcat (wdname, newdir);	/* go deaper into dir */
}

close (wdfile);  wdfile = open (DOT, 0);  dir_status = unloaded;
cpage = 1;
return ENTERDIR | REPLOT;
}

/* loaddir loads dir and sets assoc parameters */
loaddir () {

tpages = tfiles = 0;
if (wdfile < 0) {dir_status = protected;  return;}
else dir_status = unloaded;

fstat (wdfile, &wd_stb);

if (wd_stb.st_size > max_dir_size) {
	dir_status = toobig;  return;
}
lseek (wdfile, 0L, 0);		/* read in entire file */
read (wdfile, dirbuf, (int) wd_stb.st_size);

sortdir ();

tpages = tfiles / nfpp + ((tfiles % nfpp) > 0);
dir_status = loaded;
return;
}

/* sortdir sorts the directory entries.  when done, the following is true:
    1)  tfiles contains the number of files available
    2)  the d_namep array will contain pointers to the files.
	these will be sorted assending.
*/
sortdir () {

struct direct *maxent, *dirp;
int dircmp ();

tfiles = 0;
maxent = & dirbuf [wd_stb.st_size / dirsize];
for (dirp = dirbuf; dirp < maxent; dirp++) {
	if (dirp->d_ino) d_namep [tfiles++] = dirp->d_name;
	dirp->d_ino = 0;
}
qsort (d_namep, tfiles, sizeof d_namep [0], dircmp);
}

dircmp (a, b) char **a, **b; {
	return strcmp (*a, *b);
}

/*  Dispdir displays a page of the directory.
    W A R N I N G.  Dispdir modifies global data.  If the dir is not
    loaded, or is out of date, dispdir will call on loaddir.
    Cpage can be adjusted to conform to the current dir.
    An out of date dir is reloaded only if reload is true.

    In general, the goal of dispdir is to make sure the internal
    representation of the directory is consistent with the real
    directory, and what is displayed is consistent with the internal
    directory.
*/

dispdir (reload) int reload; {

int dirx;		/* index into dirbuf */
int dirchar;		/* char to select file assoc. with dirx */

long lastmtime;		/* last time dir was modified */

bufout ();  clearmsg (-1);
if (reload) {
	lastmtime = curmtime;
	fstat (wdfile, &wd_stb);
	if ((lastmtime != curmtime) || dir_status) loaddir ();
}

cpage = max (1, min (cpage, tpages));
pageend = 0;

erase ();
printf ("Directory = %s  %s", wdname, dirmsg [dir_status]);

if (tfiles == 0) {
	unbufout ();  return;
}

if (tpages > 1) {
	at ((strlen (wdname) > 50) ? 268 : 168);
	printf ("Page %d / %d    ", cpage, tpages);
}

pageend = tfiles % nfpp;
if (cpage != tpages || pageend == 0) pageend = nfpp;

for (dirx = 0, dirchar = 'a'; dirx < pageend;  dirx++, dirchar++) {
	atfile (dirx, 3);
	printf ("%c  %s", dirchar, filename (dirx));
}
unbufout ();
}

/*	Change dir to father of dir  */
todotdot (dir) char *dir; {

	register char *cp;

	for (cp = dir; *cp; cp++);	/* Scan to end of name */
	while (*--cp != '/');		/* Scan back to a slash */
	if (cp == dir) cp++;		/* Must handle root specially */
	*cp = 0;
}
