/*
 *  S E N D . C 
 *
 *  EE/CIS Computer Lab
 *  Department of Computer and Information Sciences
 *  Department of Electrical Engineering
 *  University of Delaware
 *
 *  REVISION HISTORY:
 *
 *  $Revision: 1.4 $
 *
 *  $Log:	send.c,v $
 * Revision 1.4  86/01/14  16:29:24  galvin
 * Change send() to check for From line instead of believing it is there.
 * 
 * Change sendmail() to extract()/detract() the To field.
 * 
 * mail1() needed some tuning and cleaning.  Basically change it to
 * interface to the MMDF ml_* routines.  One other major change was to
 * take out the call to verify() and depend on MMDF to do it.  We have
 * to do this anyway since we are allowing address forms that choke Mail.
 * 
 * Change savemail() to include MMDF delimiters.
 * 
 * Revision 1.3  86/01/02  14:29:57  galvin
 * Add another parameter to send to indicate whether or not this message
 * should be delimited by MMDF message delimiters.
 * 
 * Revision 1.2  85/12/18  03:00:41  galvin
 * Added comment header for revision history.
 * 
 *
 */

/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
static char *sccsid = "@(#)send.c	5.2 (Berkeley) 6/21/85";
#endif not lint

#include "./rcv.h"
#include <ctype.h>
#include <sys/stat.h>

#include "./mmdf.h"

/*
 * Mail -- a mail program
 *
 * Mail to others.
 */

/*
 * Send message described by the passed pointer to the
 * passed output buffer.  Return -1 on error, but normally
 * the number of lines written.  Adjust the status: field
 * if need be.  If doign is set, suppress ignored header fields.
 * If delim is set, output MMDF delimiters around message.
 */
send(mailp, obuf, doign, delim)
	struct message *mailp;
	FILE *obuf;
	int delim;
{
	register struct message *mp;
	long c;
	FILE *ibuf;
	char line[LINESIZE], field[BUFSIZ];
	int lc, ishead, infld, fline, dostat;
	char *cp, *cp2;

	mp = mailp;
	ibuf = setinput(mp);
	c = mp->m_size;
	ishead = 1;
	dostat = 1;
	infld = 0;
	fline = 1;
	lc = 0;
	if (delim)
		fputs(delim1, obuf);
	while (c > 0L) {
		fgets(line, LINESIZE, ibuf);
		c -= (long) strlen(line);
		lc++;
		if (ishead) {
			/* 
			 * If first line is the "From line", then just print.
			 * Since we are dealing with MMDF, it probably isn't.
			 */
			if (fline) {
				fline = 0;
				if (!strncmp(line, "From ", 5))
				goto writeit;
			}
			/*
			 * If line is blank, we've reached end of
			 * headers, so force out status: field
			 * and note that we are no longer in header
			 * fields
			 */
			if (line[0] == '\n') {
				if (dostat) {
					statusput(mailp, obuf, doign);
					dostat = 0;
				}
				ishead = 0;
				goto writeit;
			}
			/*
			 * If this line is a continuation (via space or tab)
			 * of a previous header field, just echo it
			 * (unless the field should be ignored).
			 */
			if (infld && (isspace(line[0]) || line[0] == '\t')) {
				if (doign && isign(field)) continue;
				goto writeit;
			}
			infld = 0;
			/*
			 * If we are no longer looking at real
			 * header lines, force out status:
			 * This happens in uucp style mail where
			 * there are no headers at all.
			 */
			if (!headerp(line)) {
				if (dostat) {
					statusput(mailp, obuf, doign);
					dostat = 0;
				}
				putc('\n', obuf);
				ishead = 0;
				goto writeit;
			}
			infld++;
			/*
			 * Pick up the header field.
			 * If it is an ignored field and
			 * we care about such things, skip it.
			 */
			cp = line;
			cp2 = field;
			while (*cp && *cp != ':' && !isspace(*cp))
				*cp2++ = *cp++;
			*cp2 = 0;
			if (doign && isign(field))
				continue;
			/*
			 * If the field is "status," go compute and print the
			 * real Status: field
			 */
			if (icequal(field, "status")) {
				if (dostat) {
					statusput(mailp, obuf, doign);
					dostat = 0;
				}
				continue;
			}
		}
writeit:
		fputs(line, obuf);
		if (ferror(obuf))
			return(-1);
	}
	if (delim)
		fputs(delim2, obuf);
	if (ferror(obuf))
		return(-1);
	if (ishead && (mailp->m_flag & MSTATUS))
		printf("failed to fix up status field\n");
	return(lc);
}

/*
 * Test if the passed line is a header line, RFC 733 style.
 */
headerp(line)
	register char *line;
{
	register char *cp = line;

	while (*cp && !isspace(*cp) && *cp != ':')
		cp++;
	while (*cp && isspace(*cp))
		cp++;
	return(*cp == ':');
}

/*
 * Output a reasonable looking status field.
 * But if "status" is ignored and doign, forget it.
 */
statusput(mp, obuf, doign)
	register struct message *mp;
	register FILE *obuf;
{
	char statout[3];

	if (doign && isign("status"))
		return;
	if ((mp->m_flag & (MNEW|MREAD)) == MNEW)
		return;
	if (mp->m_flag & MREAD)
		strcpy(statout, "R");
	else
		strcpy(statout, "");
	if ((mp->m_flag & MNEW) == 0)
		strcat(statout, "O");
	fprintf(obuf, "Status: %s\n", statout);
}


/*
 * Interface between the argument list and the mail1 routine
 * which does all the dirty work.
 */

mail(people)
	char **people;
{
	register char *cp2;
	register int s;
	char *buf, **ap;
	struct header head;

	for (s = 0, ap = people; *ap != (char *) -1; ap++)
		s += strlen(*ap) + 1;
	buf = salloc(s+1);
	cp2 = buf;
	for (ap = people; *ap != (char *) -1; ap++) {
		cp2 = copy(*ap, cp2);
		*cp2++ = ' ';
	}
	if (cp2 != buf)
		cp2--;
	*cp2 = '\0';
	head.h_to = buf;
	head.h_subject = NOSTR;
	head.h_cc = NOSTR;
	head.h_bcc = NOSTR;
	head.h_seq = 0;
	mail1(&head);
}


/*
 * Send mail to a bunch of user names.  The interface is through
 * the mail routine below.
 */

sendmail(str)
	char *str;
{
	struct header head;

	if (blankline(str))
		head.h_to = NOSTR;
	else
		head.h_to = detract(elide(extract(str, GTO)), GTO);
	head.h_subject = NOSTR;
	head.h_cc = NOSTR;
	head.h_bcc = NOSTR;
	head.h_seq = 0;
	mail1(&head);
	return(0);
}

/*
 * Mail a message on standard input to the people indicated
 * in the passed header.  (Internal interface).
 */

mail1(hp)
	struct header *hp;
{
	register char *cp;
	int gotcha;
	struct name *to, *np;
	struct stat sbuf;
	FILE *mtf, *postage;
	int remote = rflag != NOSTR || rmail;

	/*
	 * Collect user's mail from standard input.
	 * Get the result as mtf.
	 */

	if ((mtf = collect(hp)) == NULL)
		return;
	hp->h_seq = 1;
	if (hp->h_subject == NOSTR)
		hp->h_subject = sflag;
	if (intty && value("askcc") != NOSTR)
		grabh(hp, GCC);
	else if (intty) {
		printf("EOT\n");
		fflush(stdout);
	}

	/*
	 * Now, get the user names from the combined to and cc lists
	 */

	senderr = 0;
	to = usermap(cat(extract(hp->h_bcc, GBCC),
	    cat(extract(hp->h_to, GTO), extract(hp->h_cc, GCC))));
	if (to == NIL) {
		printf("No recipients specified\n");
		goto topdog;
	}

	/*
	 * Look through the recipient list for names with /'s
	 * in them which we write to as files directly.
	 */

	to = outof(to, mtf, hp);
	if (senderr && !remote) {
topdog:
		rewind(mtf);
		if ((mtf = infix(hp, mtf)) == NULL) {
			fprintf(stderr, ". . . message lost, sorry.\n");
			return;
		}
		if (fsize(mtf) != 0) {
			remove(deadletter);
			exwrite(deadletter, mtf, 1);
		}
		goto out;
	}
	for (gotcha = 0, np = to; np != NIL; np = np->n_flink)
		if ((np->n_type & GDEL) == 0) {
			gotcha++;
			break;
		}
	if (!gotcha)
		goto out;
	to = elide(to);
	if (count(to) > 1)
		hp->h_seq++;
	hp->h_to = detract(to, GTO|GCOMMA);
	hp->h_cc = detract(to, GCC|GCOMMA);
	hp->h_bcc = detract(to, GBCC|GCOMMA);
	if (hp->h_seq > 0 && !remote) {
		if (fsize(mtf) == 0)
		    if (hp->h_subject == NOSTR)
			printf("No message, no subject; hope that's ok\n");
		    else
			printf("No message; hope that's ok\n");
		}
	if (debug) {
		printf("Recipients of message:\n");
		if (hp->h_to != NOSTR)
			printf(" \"%s\"", hp->h_to);
		if (hp->h_cc != NOSTR)
			printf(" \"%s\"", hp->h_cc);
		if (hp->h_bcc != NOSTR)
			printf(" \"%s\"", hp->h_bcc);
		printf("\n");
		fflush(stdout);
		return;
	}
	rewind(mtf);
	if (ml_init(YES, NO, NOSTR, hp->h_subject) != OK)
		goto topdog;
	if (!isnull(hp->h_to) && ml_adr(hp->h_to) != OK)
		goto topdog;
	if (!isnull(hp->h_cc) && ml_cc() != OK || ml_adr(hp->h_cc) != OK)
		goto topdog;
	if (!isnull(hp->h_bcc) && ml_adr(hp->h_bcc) != OK)
		goto topdog;
	if (ml_aend() != OK)
		goto topdog;
	if (ml_tinit() != OK)
		goto topdog;
	if (ml_file(mtf) != OK)
		goto topdog;
	if (ml_end(OK) != OK)
		goto topdog;
	rewind(mtf);
	if ((mtf = infix(hp, mtf)) == NULL) {
		fprintf(stderr, ". . . message lost, sorry.\n");
		return;
	}

	rewind(mtf);
	if ((cp = value("record")) != NOSTR)
		savemail(expand(cp), mtf);

		if (!stat(POSTAGE, &sbuf))
			if ((postage = fopen(POSTAGE, "a")) != NULL) {
				fprintf(postage, "%s %d %d\n", myname,
				    count(to), fsize(mtf));
				fclose(postage);
			}
out:
	fclose(mtf);
	return;
}

#ifdef NOT_USED
/*
 * Fix the header by glopping all of the expanded names from
 * the distribution list into the appropriate fields.
 * If there are any ARPA net recipients in the message,
 * we must insert commas, alas.
 */

fixhead(hp, tolist)
	struct header *hp;
	struct name *tolist;
{
	register int f;
	register struct name *np;

	for (f = 0, np = tolist; np != NIL; np = np->n_flink)
		if (any('@', np->n_name)) {
			f |= GCOMMA;
			break;
		}

	if (debug && f & GCOMMA)
		fprintf(stderr, "Should be inserting commas in recip lists\n");
	hp->h_to = detract(tolist, GTO|f);
	hp->h_cc = detract(tolist, GCC|f);
}
#endif NOT_USED

/*
 * Prepend a header in front of the collected stuff
 * and return the new file.
 */

FILE *
infix(hp, fi)
	struct header *hp;
	FILE *fi;
{
	extern char tempMail[];
	register FILE *nfo, *nfi;
	register int c;

	rewind(fi);
	if ((nfo = fopen(tempMail, "w")) == NULL) {
		perror(tempMail);
		return(fi);
	}
	if ((nfi = fopen(tempMail, "r")) == NULL) {
		perror(tempMail);
		fclose(nfo);
		return(fi);
	}
	remove(tempMail);
	puthead(hp, nfo, GTO|GSUBJECT|GCC|GNL);
	c = getc(fi);
	while (c != EOF) {
		putc(c, nfo);
		c = getc(fi);
	}
	if (ferror(fi)) {
		perror("read");
		return(fi);
	}
	fflush(nfo);
	if (ferror(nfo)) {
		perror(tempMail);
		fclose(nfo);
		fclose(nfi);
		return(fi);
	}
	fclose(nfo);
	fclose(fi);
	rewind(nfi);
	return(nfi);
}

/*
 * Dump the to, subject, cc header on the
 * passed file buffer.
 */

puthead(hp, fo, w)
	struct header *hp;
	FILE *fo;
{
	register int gotcha;

	gotcha = 0;
	if (hp->h_to != NOSTR && w & GTO)
		fmt("To: ", hp->h_to, fo), gotcha++;
	if (hp->h_subject != NOSTR && w & GSUBJECT)
		fprintf(fo, "Subject: %s\n", hp->h_subject), gotcha++;
	if (hp->h_cc != NOSTR && w & GCC)
		fmt("Cc: ", hp->h_cc, fo), gotcha++;
	if (hp->h_bcc != NOSTR && w & GBCC)
		fmt("Bcc: ", hp->h_bcc, fo), gotcha++;
	if (gotcha && w & GNL)
		putc('\n', fo);
}

/*
 * Format the given text to not exceed 72 characters.
 */

fmt(str, txt, fo)
	register char *str, *txt;
	register FILE *fo;
{
	register int col;
	register char *bg, *bl, *pt, ch;

	col = strlen(str);
	if (col)
		fprintf(fo, "%s", str);
	pt = bg = txt;
	bl = 0;
	while (*bg) {
		pt++;
		if (++col >72) {
			if (!bl) {
				bl = bg;
				while (*bl && !isspace(*bl))
					bl++;
			}
			if (!*bl)
				goto finish;
			ch = *bl;
			*bl = '\0';
			fprintf(fo, "%s\n    ", bg);
			col = 4;
			*bl = ch;
			pt = bg = ++bl;
			bl = 0;
		}
		if (!*pt) {
finish:
			fprintf(fo, "%s\n", bg);
			return;
		}
		if (isspace(*pt))
			bl = pt;
	}
}

/*
 * Save the outgoing mail on the passed file.
 */

savemail(name, fi)
	char name[];
	FILE *fi;
{
	register FILE *fo;
	register int c;
	time_t time();
	time_t now;
	char *ctime();
	char *n;

	if ((fo = fopen(name, "a")) == NULL) {
		perror(name);
		return;
	}
	time(&now);
	n = rflag;
	if (n == NOSTR)
		n = myname;
	fputs(delim1, fo);
	fprintf(fo, "From %s %s", n, ctime(&now));
	rewind(fi);
	for (c = getc(fi); c != EOF; c = getc(fi))
		putc(c, fo);
	fprintf(fo, "\n");
	fputs(delim2, fo);
	fflush(fo);
	if (ferror(fo))
		perror(name);
	fclose(fo);
}
