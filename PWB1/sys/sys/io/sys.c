#
/*
 *	indirect driver for controlling tty.
 */
#include "../hd/param.h"
#include "../hd/user.h"
#include "../hd/conf.h"
#include "../hd/tty.h"

syopen(dev, flag)
{

	if(u.u_ttyp == NULL) {
		u.u_error = ENXIO;
		return;
	}
	(*cdevsw[major(u.u_ttyd)].d_open)(minor(u.u_ttyd), flag);
}

syread(dev)
{

	(*cdevsw[major(u.u_ttyd)].d_read)(minor(u.u_ttyd));
}

sywrite(dev)
{

	(*cdevsw[major(u.u_ttyd)].d_write)(minor(u.u_ttyd));
}

sysgtty(dev, flag)
{

	(*cdevsw[major(u.u_ttyd)].d_sgtty)(minor(u.u_ttyd), flag);
}
