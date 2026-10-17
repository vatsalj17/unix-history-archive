/*
 * IPC module
 */

#include <ipc.h>

char *
ipc_open(argv)
char	*argv[];
{
	extern char	sys_fd[];
	int	fd;

	if ((fd = ipcopen(ipcpath(argv[0], "dk", ""), "light")) < 0) {
		error(-1, "ipc %s: %s", argv[0], errstr);
		return 0;
	}
	return sys_fd + fd;
}
