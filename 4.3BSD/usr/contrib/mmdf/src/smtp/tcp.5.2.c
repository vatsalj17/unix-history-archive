#include <fcntl.h>
#include "util.h"
#include "mmdf.h"
#include "netlib.h"
#include "net/con.h"

#define NETFILE "/dev/net"

/* open a tcp connect.  based on code from bbn */


tc_uicp (addr, socket, timeout, fds)
	long addr;
	long socket;            /* absolute socket number       */
	int timeout;            /* time to wait for open        */
	Pip *fds;
{
    struct con openparam, netcon;
    int sock;
    register int fd;

    sock = socket;

    openparam.c_fcon = addr;
    mkanyhost(openparam.c_lcon);

    openparam.c_lport = 0;
    openparam.c_fport = sock;
    openparam.c_timeo = timeout;
    openparam.c_mode = (CONACT | CONTCP);
    openparam.c_sbufs =
    openparam.c_rbufs = 1;
    openparam.c_proto = 0;

    netcon.c_mode = (CONTCP | CONOBLOK);
    if ((fd = open (NETFILE, O_RDWR)) < 0)
	return (RP_LIO);

    if (ioctl(fd, NETOPEN, &netcon) <0) {
	close(fd);
	return(RP_DHST);
    }

    if (ioctl (fd, NETSETE, NULL) < 0) {
	close(fd);
	return (RP_LIO);
    }

    fds -> pip.prd = fd;
    fds -> pip.pwrt = fd;
    return (RP_OK);
}
