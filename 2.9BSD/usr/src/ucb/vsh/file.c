#include "hd.h"
#include "mydir.h"
#include "strings.h"
#include "classify.h"

/* File gets a file name, converts it into a full path name, and
   selects that file.  */

file (argv) char **argv; {	/* Select specific file */

	char nname [STRMAX];

	if (*argv == CNULL) putmsg ("File:  ");
	if (getfname (*argv, nname) == FAILURE) return FAILURE;

	return (enterfile (nname));
}

/* Create accepts a file name, creats that file, and then enters it.  */
create () {

	char nname [STRMAX], oname [STRMAX];
	register char ch;  char *cp; int class;

	putmsg ("Create:  ");
	if (getfname (CNULL, nname) == FAILURE) return FAILURE;

	if (!access (nname, 0)) {
		putmsg (nname);  printf (":  Already exists");
		return FAILURE;
	}

	todotdot (nname);
	class = classify (nname);

	if (class == CL_NULL || access (nname, 3)) {
		myperror (nname);  return FAILURE;
	}
	else if (class != CL_DIR) {
		putmsg (nname);  printf (":  Not a directory");
		return FAILURE;
	}

	/* Un do effect of todotdot */
	for (cp = nname; *cp++;);
	*--cp = '/';

	erase ();  printf ("Creating %s\n\nSelect:\n\n", nname);

	printf
	("1  Text\n2  Directory\n3  Copy a file\n4  Link a file\n\n");

	ch = getch ();  putch (LF);
	if (ch == '1') {
		f_exec (EDITOR, EDITOR, nname, 0);
	}
	else if (ch == '2') {
		if (f_exec ("/bin/mkdir", "mkdir", nname, 0)) getrtn ();
		return enterdir (nname) | REPLOT;
	}
	else if (ch == '3' || ch == '4') {
		printf ("From:  ");
		if (getfname (CNULL, oname) == FAILURE) return REPLOT;

		switch (classify (oname)) {

			case CL_NULL:  case CL_PROTPLN:
			myperror (oname);  break;

			case CL_DIR:
			printf ("Cannot copy or link to directories");
			break;

			default:
			if (ch == '3') f_exec
				("/bin/cp", "cp", oname, nname, 0);
			if (ch == '4') {
				printf ("Linking\n");
				if (link (oname, nname))
					myperror ("Link failed");
			}
			at (1501);
			if (!access (nname, 0)) {
				printf ("File created.\
  Do you wish to examine it?  ");
				if (getch() == 'y') {
					putch (LF);
					return
					enterfile (nname) | REPLOT;
				}
				return REPLOT;
			}
		}
		at (1501);
		printf ("File not created. ");
		getrtn ();
	}
	return REPLOT;
}

/* Getfname takes two character arrays as parameters.
   Inname is the partial pathname of a file.  If inname == CNULL,
   getfname will instead read the partial pathname from the terminal.
   The full pathname is returned in outname.
   Getfname has a return value of SUCCESS or FAILURE.
*/
getfname (inname, outname) char *inname, *outname; {
	char inword [STRMAX];  int ilen;

	if (inname == CNULL) {
		tty_push (COOKEDMODE);
		ilen = getword (inword);
		tty_pop ();
		if (ilen <= 0) {
			clearmsg (0);
			return FAILURE;
		}
		inname = inword;
	}
	if (pathgen (wdname, inname, outname)) {
		putmsg ("Path name too long");
		return FAILURE;
	}
	return SUCCESS;
}

/* If one is in dir with pathname "old", and does a chdir "change",
   one ends up in directory "new".  Exception: ".." always breaks
   through to the root.
*/
pathgen (old, change, new) char *old, *change, *new; {

char element [DIRSIZ + 1];
char chgbuf  [STRMAX];
register len;

if (change [0] == '/') strcpy (new, SLASH);
else strcpy (new, old);

strcpy (chgbuf, change);	/* So change is not modified */

while (*chgbuf) {

	extract (element, chgbuf);
	if (compe (DOT, element));
	else if (compe (DOTDOT, element)) todotdot (new);

	else {
		len = strlen (new);
		if (len > LPLEN) return 1;
		else if (len > 1) strcat (new, SLASH);
		strcat (new, element);
	}
}
return 0;
}

extract (element, path) char *element, *path; {

register char *cp;
int eltlen;

for (cp = path; *cp != 0 && *cp != '/'; cp++);

eltlen = cp - path;
if (eltlen == 0) {
	strcpy (element, DOT);
}
else {
	strncpy (element, path, DIRSIZ);
	element [min (eltlen, DIRSIZ)] = 0;
}
if (*cp) shift (path, eltlen + 1);
else path [0] = 0;
}

shift (path, length) char *path;  int length; {

register char *cp;

for (cp = path + length; cp [-1];) *path++ = *cp++;
}
