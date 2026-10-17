# include	<sccs.h>

SCCSID(@(#)acc_init.c	8.1	12/31/84)

/*
**  ACC_INIT -- fake acc_init.
**
**	Modules that do not pull in the access method library will
**	choke.  So we include an acc_init here that the ctl module
**	can pick up if the access method library has not been
**	loaded.
*/

acc_init()
{
}
