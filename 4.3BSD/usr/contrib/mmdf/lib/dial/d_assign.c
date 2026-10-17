#
/*
 *	D _ A S S I G N . C
 *
 *	Doug Kingston, BRL, 4 August 81
 *
 *	Takes the name of an assignable device as an argument.
 *	The function calles the program /bin/assign to get a device
 *	for exclusive use.
 */
#include "d_returns.h"

#ifdef DOASSIGN
struct {
	char lobyte;
	char hibyte;
};

extern	int	d_errno;

d_assign (devname)
char *devname;
{
	int status;

	if ((status = d_tryfork()) == -1) {
		d_log ("d_assign", "couldn't fork");
		d_errno = D_FORKERR;
		return( D_FATAL );
	}

	if (status == 0) {
		/* CHILD */
		(void) close (1);
		open ("/dev/null", 1);
		execl ("/bin/assign", "mmdf-assign", devname, (char *)0);
		exit (99);
	}
	else {
		/* PARENT */
		wait (&status);
		return (status.hibyte == 0 ? D_OK : D_NO);
	}
}


d_deassign (devname)
char *devname;
{
	int status;

	if ((status = d_tryfork()) == -1) {
		d_log ("d_deassign", "couldn't fork");
		return( D_NO );
	}

	if( status == 0 ) {
		/* CHILD */
		(void) close (1);
		open ("/dev/null", 1);
		execl ("/bin/assign", "mmdf-assign", "-", devname, (char *)0);
		exit (99);
	}
	else {
		/* PARENT */
		wait (&status);
		return (status.hibyte == 0 ? D_OK : D_NO);
	}
}

#else

/* Fake routine for systems without "assign" */

d_assign() {
	return( D_OK );
}

d_deassign() {
	return( D_OK );
}

#endif
