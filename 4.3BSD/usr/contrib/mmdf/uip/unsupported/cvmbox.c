/*
 *	SYNOPSIS
 *		cvmbox file ...
 *	DESCRIPTION
 *		This program expects its input (either the argument files
 *		or standard input) to be in the old mailbox format.  It
 *		writes to standard output a mailbox file in the new format.
 *		This program was based on a program called "mailsplit"
 *		written by Dick Grune, Vrije Universiteit, Amsterdam.
 *	AUTHOR	Sjoerd Mullender, Vrije Universiteit, Amsterdam
 *	VERSION	Fri Jan 31 13:05:35 MET 1986
 */

#include	<stdio.h>
extern char *sprintf();
extern char *strcat(), *strcpy();

char mail_delim[] = "\1\1\1\1\n";

char *weekdays[] = {
	"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", 0
};

char *months[] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec", 0
};

char *word(), *search_text();

char *iname;			/* name of input file */
FILE *ifile;			/* its stream */
int lncnt;			/* its line count */
char date[40];			/* our version of the Date: line */
char from[1024];		/* our version of the From: line */
int date_present;		/* there was a Date: line in the header */
int from_present;		/* there was a From: line in the header */

main(argc, argv)
char *argv[];
{
	if (argc == 1) {
		iname = "stdin";
		ifile = stdin;
		lncnt = 0;
		process();
	}
	else
	while (argc > 1) {
		iname = argv[1];
		ifile = fopen(iname, "r");
		if (ifile == NULL)
			error("cannot open %s", iname);
		lncnt = 0;
		process();
		fclose(ifile);
		argc--, argv++;
	}
	
	return 0;
}

process()
{
	if (!get_new_line())		/* one-line read-ahead */
		return;
	do {
		date_present = 0;
		from_present = 0;
		fputs(mail_delim, stdout);
		copy_header();
		copy_letter();
		fputs(mail_delim, stdout);
	} while (is_header_line());
}

char line[1024];

int
get_new_line()
{
	if (fgets(line, sizeof line, ifile) == NULL)
		return 0;
	lncnt++;
	return 1;
}

int
is_header_line()
{
	return strncmp(line, "From ", 5) == 0;
}

copy_header()
{
	condense_From_line();
	make_date();
	while (get_new_line() && line[0] != '\n')
		copy_header_line();
	if (from_present)
		printf("Apparently-%s", from);
	else
		fputs(from, stdout);
	if (!date_present)
		fputs(date, stdout);
}

copy_header_line()
{
	int ch;
	
	if (strncmp(line, "Date: ", 6) == 0)
		date_present = 1;
	if (strncmp(line, "From: ", 6) == 0)
		from_present = 1;
	fputs(line, stdout);
}

copy_letter()
{
	do	fputs(line, stdout);
	while (get_new_line() && !is_header_line());
}

condense_From_line()
{
	/* will try to read successive >From lines and compose
	 * a summary line.
	 */
	char name[1024];
	char prefix[1024];
	int ch;
	
	dissect_line(name, prefix, date);
	while ((ch = getc(ifile)) == '>') {
		if (!get_new_line())
			error("Abrupt EOF", (char *)0);
		dissect_line(name, &prefix[strlen(prefix)], date);
	}
	ungetc(ch, ifile);		/* regrettable */
	sprintf(from, "From: %s%s\n", prefix, name);
}

dissect_line(nm, pr, dt)
	char *nm, *pr, *dt;
{
	/* dissects the line into a name, in nm, a prefix (from
	 * the 'remote from' part, in pr, and the date in dt,
	 * if dt is not NULL.
	 */
	char *wd = word(line, NULL);		/* skip From */
	char *rm = search_text(line, "remote from ");
	
	line[strlen(line) - 1] = '\0';		/* remove NL */
	wd = word(wd, nm);			/* the name */
	if (rm) {
		char *rn = word(word(rm, NULL), NULL);
		if (!rn)
			/*bad_format()*/;
		rn = word(rn, pr);		/* the prefix */
		if (rn)
			/*bad_format()*/;
		strcat(pr, "!");
		while (rm[-1] == ' ')
			*--rm = '\0';
	}
	else
		*pr ='\0';			/* no prefix */
	if (dt)
		strcpy(dt, wd);			/* the date */
}

make_date()
{
	/* Parses the date part of a From line and makes a Date: line out of
	 * it.  This date has the following format:
	 * DDD MMM d[d] hh:mm[:ss] yyyy [ZZZ]
	 * where DDD is the day of the week, MMM is the month, d[d] is the
	 * day of the month, hh:mm:ss is the time, yyyy is the year and ZZZ
	 * is the timezone.
	 */
	char timezone[10];
	int weekday;
	int year = 0, month = 1, day = 0;
	int hour = 0, min = 0, sec = 0;
	char *wd = date;
	int n;

	weekday = wordpos(wd, weekdays);
	if (weekday == 0)
		formaterr(iname, lncnt);
	wd = word(wd, NULL);
	
	month = wordpos(wd, months);
	if (month == 0)
		formaterr(iname, lncnt);
	wd = word(wd, NULL);
	
	if (sscanf(wd, "%d", &day) != 1)
		formaterr(iname, lncnt);
	wd = word(wd, NULL);
	
	if ((n = sscanf(wd, "%d:%d:%d", &hour, &min, &sec)) != 3 && n != 2)
		formaterr(iname, lncnt);
	if (n == 2)
		sec = 0;
	wd = word(wd, NULL);
	
	if ('A' <= *wd && *wd <= 'Z')		/* time zone indicator */
		wd = word(wd, timezone);
	else
		timezone[0] = 0;
	if (sscanf(wd, "%d", &year) != 1)
		formaterr(iname, lncnt);
	wd = word(wd, NULL);
	
	sprintf(date, "Date: %s, %d %s %d %02d:%02d:%02d %s\n",
		weekdays[weekday-1], day, months[month-1],
		year-1900, hour, min, sec, timezone);
}

char *
word(s, bf)
	char *s, *bf;
{
	/* returns the address of the first new word after s,
	 * or NULL otherwise.
	 * If bf is not 0, the word is copied to the buffer bf.
	 */
	if (!s) {
		if (bf)
			*bf = '\0';
		return NULL;
	}
	while (*s && *s != ' ' && *s != '\t') {
		if (bf)
			*bf++ = *s;
		s++;
	}
	if (bf)
		*bf = '\0';
	while (*s && (*s == ' ' || *s == '\t'))
		s++;
	if (!*s)
		return NULL;
	return s;
}

char *
search_text(ln, txt)
	char *ln, *txt;
{
	/* returns a pointer to the text txt in the array ln,
	 * or NULL otherwise.
	 */
	int length = strlen(txt);
	
	while (*ln) {
		if (strncmp(ln, txt, length) == 0)
			return ln;
		ln++;
	}
	return NULL;
}

int
wordpos(wd, lst)
	char *wd, *lst[];
{
	/* the index (starting at 1) of the word in the list or
	 * 0 otherwise
	 */
	int pos = 0;
	
	if (!wd)
		return 0;
	while (lst[pos] && strncmp(wd, lst[pos], strlen(lst[pos])) != 0)
		pos++;
	if (!lst[pos])
		return 0;
	return pos+1;
}

formaterr(inm, lc)
	char *inm;
{
	char buff[300];
	
	sprintf(buff, "\"%s\", line %d: improper date format", inm, lc);
	error("%s", buff);
}

error(fmt, str)
	char *fmt, *str;
{
	fprintf(stderr, fmt, str);
	fprintf(stderr, "\n");
	exit(1);
}
