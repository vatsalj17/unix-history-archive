char	pq_error[1024];

error(err, fmt, a, b, c, d, e)
char	*fmt;
{
	extern int	sys_nerr, errno;
	extern char	*sys_errlist[];
	register char	*s = pq_error;

	s += sprintf(s, fmt, a, b, c, d, e);
	if (err) {
		if (err == -1)
			err = errno;
		s += sprintf(s, ": ");
		s += sprintf(s, (err < sys_nerr) ?
			sys_errlist[err] : "Error %d", err);
	}
	return -1;
}
