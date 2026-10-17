#
#include "param.h"
#include "user.h"
#include "systm.h"
#include "proc.h"
#include "seg.h"
#include "inode.h"
#include "buf.h"
#include "text.h"

extern int	freemem;
extern icode();		/*** bootstrap program to load init process ***/

/*
 * Initialization code.
 * Called from *** start.s ***
 * soon as a stack and segmentation
 * have been established.
 * Functions:
 *	clear and free user core
 ***	start clock
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
	register m;		/***/

	/*
	 * zero and free all of core
	 */

	updlock = 0;
	m = ((*ka6 & SEGMASK) >> 8) + USIZE;	/*** Start of user memory ***/
	for (i = m; (i<<8) < memtop; i++) {
		clearseg(i);
		maxmem++;
		freemem++;
		mfree(coremap, 1, i);
	}
	printf("Memory = %l.%l K\n", maxmem/4, (maxmem%4)*25);
	printf("RESTRICTED RIGHTS\n\n");
	printf("Use, duplication or disclosure is subject to\n");
	printf("restrictions stated in Contracts with Western\n");
	printf("Electric Company, Inc.");
	printf(" and University of Wollongong.\n");

	maxmem = min(maxmem, MAXMEM);
	mfree(swapmap, nswap, swplo);

	/*
	 * start the clock 
	 */

	clkstart();

	/*
	 * set up system process
	 */

	proc[0].p_addr = (*ka6 & SEGMASK) >> 8;;
	proc[0].p_size = USIZE;
	proc[0].p_stat = SRUN;
	proc[0].p_flag=| SLOAD|SSYS;
	u.u_procp = &proc[0];

	/*
	 * set up 'known' i-nodes
	 */

	cinit();
	binit();
	iinit();
	rootdir = iget(rootdev, ROOTINO);
	rootdir->i_flag =& ~ILOCK;
	u.u_cdir = iget(rootdev, ROOTINO);
	u.u_cdir->i_flag =& ~ILOCK;

	/*
	 * make init process
	 * enter scheduling loop
	 * with system process
	 */

	if(newproc()) {
		expand(USIZE+3);
		estabur(0, 2, 1, 0);
		copyout(icode, 0, 256);		/*** icode had better
						be <= 256 bytes long! ***/
		/*
		 * Return goes to loc. 0 of user init
		 * code just copied out.
		 */
		return;
	}

	sched();
}

/*
 * Load the user hardware segmentation
 * registers from the software prototype.
 * The software registers must have
 * been setup prior by estabur.
 */
sureg()				/***/
{
	register *up, *rp, a, t;

	a = (u.u_procp->p_addr) << 8;
	if ((up=u.u_procp->p_textp) != NULL)
		t = (up->x_caddr)<<8;
	up = &u.u_uisa[16];
	rp= &uisa->r[16];
	while (--up >= &u.u_uisa[0])
		*--rp = *up + (*up & SEGWP ? t : a);
}

/*
 * Set up software prototype segmentation
 * registers to implement the 3 pseudo
 * text,data,stack segment sizes passed
 * as arguments.
 * The argument sep specifies if the
 * text and data+stack segments are to
 * be separated.
 */
estabur(nt, nd, ns, sep)
{
	register a, *ap, *dp;

	if(sep) 
		goto err;
	else
		if(nseg(nt)+nseg(nd)+nseg(ns) > 15)
			goto err;
	if(nt+nd+ns+USIZE > maxmem)
		goto err;
	a = 0;
	ap = &u.u_uisa[0];
	while(nt >= 256) {
		*ap++ = (a<<8) | (255<<20) | SEGPRES|SEGWP;
		a =+ 256;
		nt =- 256;
	}
	if(nt) {
		*ap++ = (a<<8) | ((nt-1)<<20) | SEGPRES|SEGWP;
	}
	a = USIZE;
	while(nd >= 256) {
		*ap++ = (a<<8) | (255<<20) | SEGPRES;
		a =+ 256;
		nd =- 256;
	}
	if(nd) {
		*ap++ = (a<<8) | ((nd-1)<<20) | SEGPRES;
		a =+ nd;
	}
	while(ap < &u.u_uisa[15]) {
		*ap++ = 0;
	}
	a =+ ns;
	while(ns >= 256) {
		a =- 256;
		ns =- 256;
		*--ap = (a<<8) | (255<<20) | SEGPRES;
	}
	if(ns) {
		a =- ns;
		*--ap = (a<<8) | ((ns-1)<<20) | SEGPRES;
	}
	sureg();
	return(0);

err:
	u.u_error = ENOMEM;
	return(-1);
}

/*
 * Return the arg/256 rounded up.
 */
nseg(n)
{

	return((n+255)>>8);
}
