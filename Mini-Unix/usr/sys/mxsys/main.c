#
#include "param.h"
#include "user.h"
#include "systm.h"
#include "proc.h"
#include "inode.h"

#define	CLOCK1	0177546
#define CLOCK2	0172540

/*
 * Icode is the octal bootstrap
 * program executed in user mode
 * to bring up the system.
 */
int	icode[]
{
	0104413,	/* sys exec; init; initp */
	TOPSYS+014,
	TOPSYS+010,
	0000777,	/* br . */
	TOPSYS+014,	/* initp: init; 0 */
	0000000,
	0062457,	/* init: </etc/init\0> */
	0061564,
	0064457,
	0064556,
	0000164,
};

/*
 * Initialization code.
 * Called from mch.s as
 * soon as a stack
 * has been established.
 * Functions:
 *	find which clock is configured
 *	hand craft 0th process
 *	call all initialization routines
 *	fork - process 0 to schedule
 *	     - process 1 execute bootstrap
 *
 * panic: no clock -- neither clock responds
 * loop at loc 6 in user mode -- /etc/init
 *	cannot be executed.
 */
main()
{
	extern schar;
	register i, *p;

	updlock = 0;

	/*
	 * determine clock
	 */

	if(fuword(lks = CLOCK1) == -1)
		lks = CLOCK2;

	/*
	 * set up system process
	 */

	proc[0].p_stat = SRUN;
	proc[0].p_flag =| SLOAD;
	u.u_procp = &proc[0];

	/*
	 * set up 'known' i-nodes
	 */

	*lks = 0115;
	cinit();
	binit();
	iinit();
	rootdir = iget(rootdev, ROOTINO);
	rootdir->i_flag =& ~ILOCK;
	u.u_cdir = iget(rootdev, ROOTINO);
	u.u_cdir->i_flag =& ~ILOCK;

	/*
	 * make init process
	 */

	copyout(icode, TOPSYS, sizeof icode);

	/*
	 * Return goes to loc. 0 of user init
	 * code just copied out.
	 */
}
