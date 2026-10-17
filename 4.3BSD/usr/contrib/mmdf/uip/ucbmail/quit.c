/*
 *  Q U I T . C 
 *
 *  EE/CIS Computer Lab
 *  Department of Computer and Information Sciences
 *  Department of Electrical Engineering
 *  University of Delaware
 *
 *  REVISION HISTORY:
 *
 *  $Revision: 1.3 $
 *
 *  $Log:	quit.c,v $
 * Revision 1.3  85/12/18  13:25:49  galvin
 * Add another argument to send to indicate whether or not this
 * message should be delimited by MMDF message delimiters.
 * 
 * Change all but "temp" file opens/closes to use MMDF locking routines.
 * 
 * Revision 1.2  85/12/18  03:18:48  galvin
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
static char *sccsid = "@(#)quit.c	5.2 (Berkeley) 6/21/85";
#endif not lint

#include "./rcv.h"
#include <sys/stat.h>
#include <sys/file.h>
#include "./mmdf.h"

/*
 * Rcv -- receive mail rationally.
 *
 * Termination processing.
 */

/*
 * Save all of the undetermined messages at the top of "mbox"
 * Save all untouched messages back in the system mailbox.
 * Remove the system mailbox, if none saved there.
 */

quit()
{
	int mcount, p, modify, autohold, anystat, holdbit, nohold;
	FILE *ibuf, *obuf, *fbuf, *rbuf, *readstat, *abuf;
	register struct message *mp;
	register int c;
	extern char tempQuit[], tempResid[];
	struct stat minfo;
	char *id;

	/*
	 * If we are read only, we can't do anything,
	 * so just return quickly.
	 */

	if (readonly)
		return;
	/*
	 * See if there any messages to save in mbox.  If no, we
	 * can save copying mbox to /tmp and back.
	 *
	 * Check also to see if any files need to be preserved.
	 * Delete all untouched messages to keep them out of mbox.
	 * If all the messages are to be preserved, just exit with
	 * a message.
	 *
	 * If the luser has sent mail to himself, refuse to do
	 * anything with the mailbox, unless mail locking works.
	 */

	if ((fbuf = lk_fopen(mailname, "r", (char *) 0, (char *) 0, 5)) == NULL)
		goto newmail;
	rbuf = NULL;
	if (fstat(fileno(fbuf), &minfo) >= 0 && minfo.st_size > mailsize) {
		printf("New mail has arrived.\n");
		rbuf = fopen(tempResid, "w");
		if (rbuf == NULL || fbuf == NULL)
			goto newmail;
#ifdef APPEND
		fseek(fbuf, mailsize, 0);
		while ((c = getc(fbuf)) != EOF)
			putc(c, rbuf);
#else
		p = minfo.st_size - mailsize;
		while (p-- > 0) {
			c = getc(fbuf);
			if (c == EOF)
				goto newmail;
			putc(c, rbuf);
		}
#endif
		fclose(rbuf);
		if ((rbuf = fopen(tempResid, "r")) == NULL)
			goto newmail;
		remove(tempResid);
	}

	/*
	 * Adjust the message flags in each message.
	 */

	anystat = 0;
	autohold = value("hold") != NOSTR;
	holdbit = autohold ? MPRESERVE : MBOX;
	nohold = MBOX|MSAVED|MDELETED|MPRESERVE;
	if (value("keepsave") != NOSTR)
		nohold &= ~MSAVED;
	for (mp = &message[0]; mp < &message[msgCount]; mp++) {
		if (mp->m_flag & MNEW) {
			mp->m_flag &= ~MNEW;
			mp->m_flag |= MSTATUS;
		}
		if (mp->m_flag & MSTATUS)
			anystat++;
		if ((mp->m_flag & MTOUCH) == 0)
			mp->m_flag |= MPRESERVE;
		if ((mp->m_flag & nohold) == 0)
			mp->m_flag |= holdbit;
	}
	modify = 0;
	if (Tflag != NOSTR) {
		if ((readstat = fopen(Tflag, "w")) == NULL)
			Tflag = NOSTR;
	}
	for (c = 0, p = 0, mp = &message[0]; mp < &message[msgCount]; mp++) {
		if (mp->m_flag & MBOX)
			c++;
		if (mp->m_flag & MPRESERVE)
			p++;
		if (mp->m_flag & MODIFY)
			modify++;
		if (Tflag != NOSTR && (mp->m_flag & (MREAD|MDELETED)) != 0) {
			id = hfield("article-id", mp);
			if (id != NOSTR)
				fprintf(readstat, "%s\n", id);
		}
	}
	if (Tflag != NOSTR)
		fclose(readstat);
	if (p == msgCount && !modify && !anystat) {
		if (p == 1)
			printf("Held 1 message in %s\n", mailname);
		else
			printf("Held %2d messages in %s\n", p, mailname);
		lk_fclose(fbuf, mailname, (char *) 0, (char *) 0);
		if (rbuf != NULL)
			fclose(rbuf);
		return;
	}
	if (c == 0) {
		if (p != 0) {
			lk_fclose(fbuf, mailname, (char *) 0, (char *) 0);
			writeback(rbuf);
			return;
		}
		goto cream;
	}

	/*
	 * Create another temporary file and copy user's mbox file
	 * darin.  If there is no mbox, copy nothing.
	 * If he has specified "append" don't copy his mailbox,
	 * just copy saveable entries at the end.
	 */

	mcount = c;
	if (value("append") == NOSTR) {
		if ((obuf = fopen(tempQuit, "w")) == NULL) {
			perror(tempQuit);
			lk_fclose(fbuf, mailname, (char *) 0, (char *) 0);
			return;
		}
		if ((ibuf = fopen(tempQuit, "r")) == NULL) {
			perror(tempQuit);
			remove(tempQuit);
			fclose(obuf);
			lk_fclose(fbuf, mailname, (char *) 0, (char *) 0);
			return;
		}
		remove(tempQuit);
		if ((abuf = fopen(mbox, "r")) != NULL) {
			while ((c = getc(abuf)) != EOF)
				putc(c, obuf);
			fclose(abuf);
		}
		if (ferror(obuf)) {
			perror(tempQuit);
			fclose(ibuf);
			fclose(obuf);
			lk_fclose(fbuf, mailname, (char *) 0, (char *) 0);
			return;
		}
		fclose(obuf);
		close(creat(mbox, sentprotect));
		if ((obuf = fopen(mbox, "r+")) == NULL) {
			perror(mbox);
			fclose(ibuf);
			lk_fclose(fbuf, mailname, (char *) 0, (char *) 0);
			return;
		}
	}
	if (value("append") != NOSTR)
		if ((obuf = fopen(mbox, "a")) == NULL) {
			perror(mbox);
			lk_fclose(fbuf, mailname, (char *) 0, (char *) 0);
			return;
		}
	for (mp = &message[0]; mp < &message[msgCount]; mp++)
		if (mp->m_flag & MBOX)
			if (send(mp, obuf, 0, 1) < 0) {
				perror(mbox);
				if (ibuf != NULL)
				fclose(ibuf);
				fclose(obuf);
				lk_fclose(fbuf, mailname, (char *)0, (char *)0);
				return;
			}

	/*
	 * Copy the user's old mbox contents back
	 * to the end of the stuff we just saved.
	 * If we are appending, this is unnecessary.
	 */

	if (value("append") == NOSTR) {
		rewind(ibuf);
		c = getc(ibuf);
		while (c != EOF) {
			putc(c, obuf);
			if (ferror(obuf))
				break;
			c = getc(ibuf);
		}
		fclose(ibuf);
		fflush(obuf);
	}
	if (ferror(obuf)) {
		perror(mbox);
		fclose(obuf);
		lk_fclose(fbuf, mailname, (char *) 0, (char *) 0);
		return;
	}
	fclose(obuf);
	if (mcount == 1)
		printf("Saved 1 message in mbox\n");
	else
		printf("Saved %d messages in mbox\n", mcount);

	/*
	 * Now we are ready to copy back preserved files to
	 * the system mailbox, if any were requested.
	 */

	if (p != 0) {
		lk_fclose(fbuf, mailname, (char *) 0, (char *) 0);
		writeback(rbuf);
		return;
	}

	/*
	 * Finally, remove his /usr/mail file.
	 * If new mail has arrived, copy it back.
	 */

cream:
	if (rbuf != NULL) {
		lk_fclose(fbuf, mailname, (char *) 0, (char *) 0);
		if ((abuf = lk_fopen(mailname, "w+", (char *) 0, (char *) 0, 5)) == NULL)
			goto newmail;
		while ((c = getc(rbuf)) != EOF)
			putc(c, abuf);
		fclose(rbuf);
		lk_fclose(abuf, mailname, (char *) 0, (char *) 0);
		alter(mailname);
		return;
	}
	demail();
	lk_fclose(fbuf, mailname, (char *) 0, (char *) 0);
	return;

newmail:
	printf("Thou hast new mail.\n");
	if (fbuf != NULL)
		lk_fclose(fbuf, mailname, (char *) 0, (char *) 0);
}

/*
 * Preserve all the appropriate messages back in the system
 * mailbox, and print a nice message indicated how many were
 * saved.  On any error, just return -1.  Else return 0.
 * Incorporate the any new mail that we found.
 */
writeback(res)
	register FILE *res;
{
	register struct message *mp;
	register int p, c;
	FILE *obuf;

	p = 0;
	if ((obuf = lk_fopen(mailname, "w+", (char *) 0, (char *) 0, 5)) == NULL) {
		perror(mailname);
		return;  /* error */
	}
#ifndef APPEND
	if (res != NULL)
		while ((c = getc(res)) != EOF)
			putc(c, obuf);
#endif
	for (mp = &message[0]; mp < &message[msgCount]; mp++)
		if ((mp->m_flag&MPRESERVE)||(mp->m_flag&MTOUCH)==0) {
			p++;
			if (send(mp, obuf, 0, 1) < 0) {
				perror(mailname);
				lk_fclose(obuf, mailname, (char *) 0, (char *) 0);
				return;   /* error */
			}
		}
#ifdef APPEND
	if (res != NULL)
		while ((c = getc(res)) != EOF)
			putc(c, obuf);
#endif
	fflush(obuf);
	if (ferror(obuf)) {
		perror(mailname);
		lk_fclose(obuf, mailname, (char *) 0, (char *) 0);
		return;   /* error */
	}
	if (res != NULL)
		fclose(res);
	lk_fclose(obuf, mailname, (char *) 0, (char *) 0);
	alter(mailname);
	if (p == 1)
		printf("Held 1 message in %s\n", mailname);
	else
		printf("Held %d messages in %s\n", p, mailname);
	return;
}
