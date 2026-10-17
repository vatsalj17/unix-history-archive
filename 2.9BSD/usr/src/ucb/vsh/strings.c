#
/* Usefull string handeling routines */
#
#include "hd.h"
#include "strings.h"

/* Scanspace reads in the standard input and returns the first character
   which is not a space.  Note this could be an end of file.  */

scanspace () {
	register ch;

	while (WHITESPACE (ch = getch ()));
	return ch;
}

/* Scanend scans to the end of a line */

scanend () {
	register ch;

	while (!ENDLINE (ch =  getch ()));
	return ch;
}

/* Xgetword inputs a string up to clim - 1 chars long */
/* String is returned in argument;  Length is return value.  */

/* Use getword (char_array) instead.  Clim is filled in for you. */

xgetword (input, clim) char *input; int clim; {

#define LASTCHAR	(input + clim - 1)

register ch;  register char *cp, *lchar;

ch = scanspace ();
cp = input;  lchar = LASTCHAR;

while (cp < lchar &&
	!ENDLINE (ch) &&
	!WHITESPACE (ch)) {

	*cp++ = ch;  ch = getch ();
}

while (!ENDLINE (ch)) ch = getch ();
*cp = 0;
return (cp - input);
}

/* Xgetline inputs a string up to clim - 1 chars long */
/* String is returned in argument;  Length is return value.  */

/* Use getline (char_array) instead.  Clim is filled in for you. */
/* Use fgetline (stream, char_array) to get a line from a file   */
/* other than stdin. */

xgetline (stream, input, clim) FILE *stream; char *input;  int clim; {

	register ch;  register char *cp, *lchar;
	cp = input;  lchar = LASTCHAR;

	do {
		ch = fgetc (stream);
		if (cp > lchar) cp = lchar;
		*cp++ = ch;
	} while (!ENDLINE (ch));

	*--cp = 0;
	return (cp - input);
}

getrtn () {		/* Ask person to press return */
	printf ("  Press -Return-");
	scanend ();
}

putch (ch) int ch; {putchar (ch);}
getch () {return getchar ();}
