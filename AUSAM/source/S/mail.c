/*
 *	Mail: System Postal Service
 *
 *			Rewritten (in readable code) by
 *				Steven Fraser Oct '79
 */

#include	<local-system>
#include	<passwd.h>
#include	<stat16.h>

struct pwent pe;

char buf[512];

char mbox[]	"mbox";
char mail[]	".mail";
char slmail[]	"/.mail";
char letter[]	"dead.letter";
char cantcr[]	": can't open or create"; /* the only two duplicated */
char permdn[]	": permission denied";

main(argc, argv)
register int argc;
register char **argv;
{
	register n;

	pe.pw_uid = getreal();
	if ((n = getpwlog(&pe, buf, sizeof buf)) == -1)
		panic("", "Who are you?");
	if (n == sizeof buf)
		panic("", "Buffer too small - see a guru");
	if ((chdir(pe.pw_strings[DIRPATH])) == -1)
		panic("Can't find ", pe.pw_strings[DIRPATH]);

	if (argc <= 1 || (argc == 2 && argv[1][0] == '-'))
		readmail(argc, argv[1][1]);
	else
		sendmail(argc, argv);
	exit(0);
}

readmail(argc, answer)
char answer;
{
	register int mf, save;

	if ((mf = open(mail, 0)) == -1)
	{
		prints(2, "No mail.\n");
		exit(0);
	}

	while ((save = read(mf, buf, sizeof buf)) > 0)
		write(1, buf, save);

	if (argc == 2)
		save = answer != 'n';
	else
	{
		if (gtty(1, buf) != -1)
		{
			prints(1, "\nsave (y-n)? ");
			read(0, buf, sizeof buf);
			save = *buf != 'n';
		}
		else
			save = 0;
	}
	if (save)
	{
		if ((save = postpend(mf, mbox)) == 1)
			panic(mbox, permdn);
		else if (save == -1)
			panic(mbox, cantcr);
	}
	unlink(mail);
}

sendmail(argc, argv)
int argc;
char **argv;
{
	register char *ml;
	register lt, sav, n;

	sav = 0;
	unlink(letter);

	/*
	 * this stops silly stuff with links & also,
	 * allows "mail person < dead.letter" without
	 * clobbering dead.letter first
	 */

	if ((lt = creat(letter, 0600)) == -1)
		panic(lt, cantcr);
	chown(letter, pe.pw_uid);

	/*
	 * because creat only opens for reading
	 * we must close and reopen (yuk)
	 */

	close(lt);
	lt = open(letter, 2);

	prints(lt, "From ");	/* print post mark */
	prints(lt, pe.pw_strings[LNAME]);
	prints(lt, " ");
	prints(lt, ijtime());
	while ((sav = read(0, buf, sizeof buf)) > 0)
		write(lt, buf, sav);
	prints(lt, "\n");

	while (--argc)
	{
		pe.pw_strings[LNAME] = argv[argc];
		if ((n = getpwuid(&pe, buf, sizeof buf)) == -1 ||
			n == sizeof buf)
		{
			say("Can't send to ", argv[argc]);
			sav++;		/* have to write mail "dead.letter" */
		}
		else
		{
			ml = join(pe.pw_strings[DIRPATH], slmail);
			if ((n = postpend(lt, ml)) == 0)
				continue;
			if (n == 1)
				say(ml, permdn);
			else if (n == -1)
				say(ml, cantcr);
			sav++;
		}
	}
	if (sav)
		prints(2, "Letter saved in 'dead.letter'.\n");
	else
		unlink(letter);
}

panic(s1, s2)
char *s1, *s2;
{
	say(s1, s2);
	exit(1);
}

say(s1, s2)
char *s1, *s2;
{
	prints(2, s1);
	prints(2, s2);
	prints(2, "\n");
}

postpend(fd1, fname)	/* append to end of file */
int fd1;
char *fname;
{
	register n;
	register fd2;
	struct statbuf s;

	seek(fd1, 0, 0);
	if ((fd2 = open(fname, 1)) == -1)
	{
		fd2 = creat(fname, 0600);
		chown(fname, pe.pw_uid);
		if (fd2 == -1) return -1;
	}
	else
	{
		newfstat(fd2, &s);
		if (pe.pw_uid != s.sb_uid || s.sb_nlinks > 1)
			return 1;
	}
	seek(fd2, 0, 2);
	while ((n = read(fd1, buf, sizeof buf)) > 0)
		write(fd2, buf, n);

	close(fd2);
	return 0;
}

join(p, q)
register char *p, *q;
{
	register char *t;

	t = buf;
	while (*t = *p++) t++;
	while (*t = *q++) t++;
	*t = 0;
	return buf;
}
