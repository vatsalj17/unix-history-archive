/*
 * Call server module
 */

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>
#include "server.h"

typedef struct {
	char	*tag;
	char	line[BUFSIZ];
	char	buf[BUFSIZ];
	char	*ptr, *end;
} call_t;

static int	stalk(), sgetc();
extern char	*pq_open();

char *
call_open(argv)
char	*argv[];
{
	register call_t	*call = (call_t *) malloc(sizeof (call_t));

	if (call->tag = pq_open(argv)) {
		call->ptr = call->end = call->buf;
		if (stalk(call, 0, "") != -1)
			return (char *) call;
	}
	free((char *) call);
	return 0;
}

call_close(call)
call_t	*call;
{
	register int	r1, r2;

	r1 = stalk(call, CLOSE, "");
	if ((r2 = pq_close(call->tag)) == -1)
		error(-1, "call: close");
	free((char *) call);
	return (r1 != -1 && r2 != -1) ? 0 : -1;
}

call_read(call, argv)
call_t	*call;
char	*argv[];
{
	register char	*s;
	register int	rv;

	if ((rv = stalk(call, READ, "")) > 0) {
		s = call->line;
		while (s = strchr(s, VALUE))
			*s++ = 0, *argv++ = s;
		*argv = 0;
	}
	return rv;
}

call_write(call, argv)
call_t	*call;
char	*argv[];
{
	char	line[BUFSIZ];
	char	*a, *s = line;

	while (a = *argv++) {
		*s++ = VALUE;
		while (*s++ = *a++);
		s--;
	}
	return stalk(call, WRITE, line);
}

static int
stalk(call, cmd, arg)
call_t	*call;
char	cmd;
char	*arg;
{
	register char	*s;
	register int	c;

	if (cmd) {
		s = call->line;
		*s++ = cmd;
		if (arg) {
			while (*s++ = *arg++);
			s--;
		}
		*s++ = '\n';
		c = s - call->line;
		if (pq_write(call->tag, call->line, c) != c)
			return error(-1, "call: write");
	}
	switch (c = sgetc(call)) {
	case EOF:
		return -1;
	case PROMPT:
		return 0;
	case '\n':
	case VALUE:
	case ERROR:
		s = call->line;
		while (c != EOF && c != '\n')
			*s++ = c, c = sgetc(call);
		if (c == EOF)
			return -1;
		*s = 0;
		if (sgetc(call) == PROMPT)
			return (*call->line != ERROR) ? 1
				: error(0, "%s", call->line + 1);
		/* fall thru */
	default:
		return error(0, "call: Protocol error");
	}
}

static int
sgetc(call)
register call_t	*call;
{
	if (call->ptr == call->end) {
		int n = pq_read(call->tag, call->buf, sizeof call->buf);
		if (n <= 0) {
			error(n == 0 ? EPIPE : -1, "call: read");
			return EOF;
		}
		call->ptr = call->buf;
		call->end = call->buf + n;
	}
	return *call->ptr++;
}
