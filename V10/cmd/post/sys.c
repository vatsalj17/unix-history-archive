/*
 * System call module
 */

#include <fcntl.h>

char	sys_fd[1];

char *
sys_open(argv)
char	*argv[];
{
	int	fd;

	if ((fd = open(argv[0], O_RDWR)) < 0) {
		error(-1, argv[0]);
		return 0;
	}
	return sys_fd + fd;
}

sys_close(fd)
char	*fd;
{
	close(fd - sys_fd);
	return 0;
}

sys_read(fd, buf, n)
char	*fd, *buf;
{
	return read(fd - sys_fd, buf, n);
}

sys_write(fd, buf, n)
char	*fd, *buf;
{
	return write(fd - sys_fd, buf, n);
}
