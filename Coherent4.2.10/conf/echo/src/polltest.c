/*
 * Test out STREAMS polling; open several streams and pipes, then fork. The
 * child will write to a random channel, the parent has to pick it up.
 */

#include <common/tricks.h>
#include <sys/compat.h>
#include <stdio.h>
#include <unistd.h>
#include <poll.h>
#include <fcntl.h>
#include <stdarg.h>
#include <time.h>
#include <signal.h>

/*
 * The server will wait a second.
 */

#define	TEST_SERVER	1000


/*
 * Here we record all the file descriptors.
 */

#define	TEST_MAX	20

struct pollfd	fds [TEST_MAX];
int		max_fd;


/*
 * Add a file descriptor.
 */

#if	USE_PROTO
int add_fd (int fd)
#else
int
add_fd (fd)
int		fd;
#endif
{
	if (max_fd >= ARRAY_LENGTH (fds)) {
		printf ("add_fd: trying to set up too many files.\n");
		return -1;
	}

	fds [max_fd].events = POLLIN;
	fds [max_fd ++].fd = fd;
	return 0;
}


/*
 * Create an echo stream.
 */

#if	USE_PROTO
int create_echo (void)
#else
int
create_echo ()
#endif
{
	int		fd;

	if ((fd = open ("/dev/echo", O_RDWR)) == -1) {

		perror ("Couldn't open /dev/echo");
		return -1;
	}

	return add_fd (fd);
}


/*
 * Create a pipe pair.
 */

#if	USE_PROTO
int create_pipe (void)
#else
int
create_pipe ()
#endif
{
	int		pipes [2];

	if (pipe (pipes) < 0) {
		perror ("could not create pipes");
		return -1;
	}

	return add_fd (pipes [0]) >= 0 && add_fd (pipes [1]) >= 0 ? 0 : -1;
}


/*
 * Statically set up the I/O buffers for the client and server; since each
 * will get a separate copy, this saves some stuffing around.
 */

char		control_information [256];
char		data_information [256];

#if	0
struct strbuf	ctlbuf = {
	sizeof (control_information), 0, control_information
};
struct strbuf	databuf = {
	sizeof (data_information), 0, data_information
};
#endif


/*
 * Client/server output routine.
 */

#if	USE_PROTO
void report (CONST char * fmt, ...)
#else
void
report (fmt)
CONST char    *	fmt;
#endif
{
	va_list		args;
	struct tm     *	tm;
	time_t		now;

	(void) time (& now);
	tm = localtime (& now);

	va_start (args, fmt);
	(void) fprintf (stderr, "%5d %02d:%02d:%02d: ", getpid (),
			tm->tm_hour, tm->tm_min, tm->tm_sec);
	(void) vfprintf (stderr, fmt, args);
	va_end (args);
}


/*
 * Server half: input sink.
 */

#if	USE_PROTO
void run_server (void)
#else
void
run_server ()
#endif
{
	for (;;) {
		int		i;

		report ("server: waiting\n");

		if ((i = poll (fds, max_fd, TEST_SERVER)) == 0) {
			report ("server: no events\n");
			continue;
		}

		report ("server: got %d events\n", i);

		for (i = 0 ; i < max_fd ; i ++) {
			int		flags;
			int		retval;

			if (fds [i].revents == 0)
				continue;
			report ("server: event on channel %d\n", i);

			/*
			 * Flush the data. For now, pipes are filesystem
			 * objects so we must use read ().
			 */

			fcntl (fds [i].fd, F_SETFL,
			       fcntl (fds [i].fd, F_GETFL) | O_NONBLOCK);

			while (read (fds [i].fd, data_information,
				     sizeof (data_information)) != -1)
				/* DO NOTHING */ ;

			fcntl (fds [i].fd, F_SETFL,
			       fcntl (fds [i].fd, F_GETFL) & ~ O_NONBLOCK);
		}
	}
}


/*
 * Signal-catching function for the client; we use this as a backup to ensure
 * that poll () really is waiting for the right amount of time.
 */

#if	USE_PROTO
void catch_alarm (int sig)
#else
void
catch_alarm (sig)
int		sig;
#endif
{
	signal (SIGALRM, catch_alarm);
	report ("Caught alarm signal!\n");
}


/*
 * Client half: periodically generate output.
 */

#if	USE_PROTO
void run_client (void)
#else
void
run_client ()
#endif
{
	signal (SIGALRM, catch_alarm);

	for (;;) {
		int		i;
		int		written;

		/*
		 * Delay a random amount, between 0.5 and 1.5 seconds.
		 */

		alarm (2);
		i = rand () % 1000 + 500;
		report ("client: waiting %d msec\n", i);

		if (poll (fds, 0, i) != 0)
			continue;

		/*
		 * Generate some output, and send it someplace.
		 */

		i = rand () % max_fd;
		report ("client: writing to channel %d\n", i);

		strcpy (data_information, ctime ());
		written = write (fds [i].fd, data_information,
				 strlen (data_information));

		if (written < 0)
			report ("client: error writing to channel %d\n", i);
	}
}


/*
 * Main program; create a channels and fork into client and server.
 */

#if	USE_PROTO
int main (int argc, char * argv [])
#else
int
main (argc, argv)
int		argc;
char	      *	argv [];
#endif
{
	int		i;

	srand (clock ());

	create_echo ();
	create_echo ();
	while (max_fd < TEST_MAX - 1)
		create_pipe ();

	if (fork ())
		run_server ();
	else
		run_client ();

	return 0;
}
