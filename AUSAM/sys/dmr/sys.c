#
/*
 */

/*
 *	indirect driver for controlling tty.
 */
#include "../defines.h"
#include "../param.h"
#include "../conf.h"
#include "../user.h"
#include "../tty.h"
#include "../proc.h"

syopen(dev, flag)
{
	register struct tty *tp;	/* fix000 */

	if(tp = syttyp())
	(*cdevsw[tp->t_dev.d_major].d_open)(tp->t_dev, flag);
}

syread(dev)
{
	register struct tty *tp;	/* fix000 */

	if(tp = syttyp())
	(*cdevsw[tp->t_dev.d_major].d_read)(tp->t_dev);
}

sywrite(dev)
{
	register struct tty *tp;	/* fix000 */

	if(tp = syttyp())
	(*cdevsw[tp->t_dev.d_major].d_write)(tp->t_dev);
}

sysgtty(dev, flag)
{
	register struct tty *tp;	/* fix000 */

	if(tp = syttyp())
	(*cdevsw[tp->t_dev.d_major].d_sgtty)(tp->t_dev, flag);
}

syttyp()
{
	register struct tty *tp;	/* fix000 */

	tp = u.u_procp->p_ttyp;
	if(tp == NULL)
		u.u_error = ENXIO;
	return(tp);
}
