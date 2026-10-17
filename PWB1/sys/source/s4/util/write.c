static char Sccsid[] "@(#)write	1.1";
# include <errnos.h>

/*
	Interface to write(II) (called syswrite) which handles
	all error conditions.
	Returns number of bytes written on success,
	returns fatal(<mesg>) on failure.
*/

write(fildes,buffer,nbytes)
char *buffer;
{
	register int n;

	if (nbytes>0 && (n=syswrite(fildes,buffer,nbytes))!=nbytes)
		n = xmsg("","write");
	return(n);
}
