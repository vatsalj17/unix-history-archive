/* Dkinstall, secure version.  Places all incoming packages in
   a receiving area for inspection before installation.
   use "scan" to examine and dispose of arrivals
*/
#include "decl.h"
#include <signal.h>
#include <errno.h>
#include <pwd.h>

extern "C" {
	extern int dup2 (int, int);
	extern int fork();
	extern long time(long*);
	extern int setupshares (int, void(*)() = 0);
	extern void perror (char*);
	extern int errno;
	extern int sys_nerr;
	extern char *sys_errlist[];
}

static const int bufsize = 4096;
static const int timeout = 60*10;
static const int wtimeout = 15;

static void
alarmcatch(int)
{
	signal (SIGALRM, alarmcatch);
}

static void
die (String s)
{
	String msg = s;
	if ((unsigned) errno < sys_nerr)
		msg += ":" + String(sys_errlist[errno]);
	msg += "\n";
	cout << msg;
	exit (1);
}

static void
twrite (char* buf, int len)
{
	alarm (timeout);
	int n = write (1, buf, len);
	alarm (0);
	if (n != len)
		die ("remote write failed");
}

static int
tread (char* buf, int len)
{
	alarm (timeout);
	int n = read (0, buf, len);
	alarm (0);
	if (n < 0)
		die ("remote read failed");
	return n;
}

static void
send (char* s)
{
	twrite (s, strlen (s));
}

static int
mkfile (String file)
{
	int fd = creat (file, 0600);
	if (fd < 0)
		die ("temp file creat");
	return fd;
}

main()
{
	signal (SIGALRM, alarmcatch);

	/* announce our presence */
	send ("asd\n");

	/* use daemon's share instead of root */
	struct passwd *pw;
	pw = getpwnam ("daemon");
	if (pw)
		setupshares (pw->pw_uid);
	char buf[bufsize];
	int n;
	checksum cs, rcs;

	umask (077);


	/* make a temp file to hold the data */
	Path tfile = "/usr/asd/" + String(dec(time(0)));
	int infd = mkfile (tfile);
	int outfd = 0;
	/* copy the data to the input temp file and checksum it */
	while ((n = tread (buf, bufsize)) > 0) {
		cs.combine (buf, n);
		if (write (infd, buf, n) != n)
			die ("temp file write");
	}
	lseek (infd, 0, 0);

	/* Check for normal end of data */
	if (n < 0)
		die ("remote read");

	/* Try to read the checksum */
	alarm (timeout);
	n = read (0, rcs);
	alarm (0);

	/* Do the checksums match? */
	if (n != cksize || cs != rcs) {
		cout << "checksum error\n";
		exit (1);
	}

	/* Assert we got the stuff successfully */
	twrite ("", 0);

	cout << "return code " << 0 << "\n";
	return 0;
}
