/*
 * Minimal testing of STREAMS functionality; open an echo stream, push the
 * dump module and do a read, write, putmsg and getmsg.
 */

#include <stropts.h>
#include <fcntl.h>
#include <stdio.h>
#include <poll.h>
#include <errno.h>

int main (argc, argv)
int		argc;
char	      *	argv [];
{
	int		fd;
	char		buf [20];
	int		cnt;
	struct pollfd	fds [1];
	struct strbuf	databuf;

	if ((fd = open ("/dev/echo", O_RDWR)) == -1) {

		perror ("Couldn't open /dev/echo");
		return -1;
	}

	fprintf (stderr, "Returned %d\n", fd);

	fprintf (stderr, "PUSH ioctl () says %d\n", ioctl (fd, I_PUSH, "dump"));
	fprintf (stderr, "SRDOPT ioctl () says %d\n", ioctl (fd, I_SRDOPT, RMSGN));

	fprintf (stderr, "write () says %d\n", write (fd, "Foo!", 5));

	fds->fd = fd;
	fds->events = POLLIN;
	if (poll (fds, 1, 0) == -1) {
		fprintf (stderr, "Got error from poll ()\n");
		return -1;
	}
	fprintf (stderr, "Got event 0x%x\n", fds->revents);

	cnt = read (fd, buf, sizeof (buf));
	buf [cnt] = 0;

	fprintf (stderr, "Read %d bytes, result = %s\n", cnt, buf);

	databuf.maxlen = sizeof (buf);
	databuf.len = 12;
	databuf.buf = "STREAM this!";

	errno = 0;
	fprintf (stderr, "putmsg says %d\n", putmsg (fd, NULL, & databuf, 0));

	if (errno != 0)					    
		fprintf (stderr, "errno says %d\n", errno);

	errno = 0;
	cnt = 0;
	databuf.buf = buf;
	fprintf (stderr, "getmsg says %d\n", getmsg (fd, NULL, & databuf,
						     & cnt));

	if (errno != 0)					    
		fprintf (stderr, "errno says %d\n", errno);

	fprintf (stderr, "len = %d flag = %d\n", databuf.len, cnt);
	close (fd);

	return 0;
}
