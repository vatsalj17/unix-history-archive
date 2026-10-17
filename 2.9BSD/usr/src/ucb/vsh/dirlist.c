#include "hd.h"
#include "mydir.h"
#include "command.h"

#define SHIFT1	13
#define SHIFT2	6
#define SHIFT3	3
#define MASK	07

pfstatus (fname) char * fname; {

/* long listing of a file */
static char *mchar [2] [8] = {
	"---", "--x", "-w-", "-wx",
	"r--", "r-x", "rw-", "rwx",
	"--s", "--s", "-ws", "-ws",
	"r-s", "r-s", "rws", "rws"
};

static char fformat [8] = {' ', 'c', 'd', 'b', '-', '?', '?', '?'};

register mode;

if (stat (fname, &scr_stb)) {
	printf ("Cannot Access");
	return;
}
mode = scr_stb.st_mode;

printf ("%c%c%s%s%s%s%2d",
	(mode & S_ISVTX) ? 't' : ' ',
	fformat [(mode >> SHIFT1) & MASK],
	mchar [(mode & S_ISUID) != 0] [(mode >> SHIFT2) & MASK],
	mchar [(mode & S_ISGID) != 0] [(mode >> SHIFT3) & MASK],
	mchar [0] 		      [mode & MASK],
	scr_stb.st_nlink > 99 ? "" : " ",
	scr_stb.st_nlink);
if (mode & S_IFCHR)
	printf ("%5d, %3d",
	major (scr_stb.st_rdev), minor (scr_stb.st_rdev));
else
	printf ("%10D", scr_stb.st_size);
printf (" %.24s", ctime (&scr_stb.st_mtime));
}

/* This prints out the protection modes of the files on the current
   page.  It knows the page from the global variable cpage.  The
   value NOREPLOT is always returned, to make globalcmd happy.
*/
longlist () {		/* long listing for entire page */

	register i;
	bufout ();
	for (i=0; i < pageend; i++) longfile (i);
	unbufout ();

	return CMD_DATE;	/* Run date command */
}

longfile (index) int index; {

atfile (index, 22); pfstatus (filename (index));
}
