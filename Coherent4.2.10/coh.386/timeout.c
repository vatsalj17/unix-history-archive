/* $Header: /ker/coh.386/RCS/timeout.c,v 2.4 93/10/29 00:56:00 nigel Exp Locker: nigel $ */
/* (lgl-
 *	The information contained herein is a trade secret of Mark Williams
 *	Company, and  is confidential information.  It is provided  under a
 *	license agreement,  and may be  copied or disclosed  only under the
 *	terms of  that agreement.  Any  reproduction or disclosure  of this
 *	material without the express written authorization of Mark Williams
 *	Company or persuant to the license agreement is unlawful.
 *
 *	COHERENT Version 2.3.37
 *	Copyright (c) 1982, 1983, 1984.
 *	An unpublished work by Mark Williams Company, Chicago.
 *	All rights reserved.
 -lgl) */
/*
 * Timeout management.
 *
 * $Log:	timeout.c,v $
 * Revision 2.4  93/10/29  00:56:00  nigel
 * R98 (aka 4.2 Beta) prior to removing System Global memory
 * 
 * Revision 2.3  93/08/19  03:27:01  nigel
 * Nigel's r83 (Stylistic cleanup)
 * 
 * Revision 2.2  93/07/26  14:29:27  nigel
 * Nigel's R80
 * 
 * Revision 1.4  93/04/14  10:08:12  root
 * r75
 * 
 * Revision 1.3  92/07/16  16:33:38  hal
 * Kernel #58
 * 
 * Revision 1.2  92/01/06  12:01:05  hal
 * Compile with cc.mwc.
 * 
 * Revision 1.2	89/08/01  13:56:42 	src
 * Bug:	#include <timeout.h> not accurate; timeout.h now in /usr/include/sys.
 * Fix:	#include <sys/timeout.h> now used. (ABC)
 * 
 * Revision 1.1	88/03/24  08:14:38 	src
 * Initial revision
 * 
 * 87/07/23	Allan Cornish		/usr/src/sys/coh/timeout.c
 * Timeout2 function now cancels timer if delay value is 0.
 *
 * 87/07/08	Allan Cornish		/usr/src/sys/coh/timeout.c
 * Timeout2 function added to support long timeouts.
 *
 * 87/07/07	Allan Cornish		/usr/src/sys/coh/timeout.c
 * Support for multiple timing queues ported from RTX.
 *
 * 86/11/24	Allan Cornish		/usr/src/sys/coh/timeout.c
 * Added support for new t_last field in tim struct.
 */

#include <sys/coherent.h>
#include <common/_tricks.h>
#include <kernel/_timers.h>
#include <stddef.h>

#include <coh/timeout.h>


/*
 * Given a pointer to a timeout structure, `tp', call the function `f'
 * with integer argument `a' in `n' ticks of the clock. The list is
 * searched to see if the specified timeout structure is already in a
 * list, and it is removed if already there.
 */

#if __USE_PROTO__
void timeout(TIM * tp, unsigned int n, int (*f)(int a), int a)
#else
void
timeout(tp, n, f, a)
register TIM *tp;
unsigned n;
int (*f)();
int a;
#endif
{
	TIM ** qp;
	int s;

	/*
	 * Already on a timing queue.
	 */
	s = sphi ();
	if ((qp = tp->t_last) != NULL) {
		tp->t_last = NULL;
		if ((* qp = tp->t_next))
			tp->t_next->t_last = qp;
	}
	spl (s);

	if ((tp->t_func = f) == NULL)
		return;

	/*
	 * Calculate clock tick at which timeout is to occur.
	 * Record function and argument to be invoked upon timeout.
	 */

	tp->t_lbolt = lbolt + n;
	tp->t_farg  = a;

	/*
	 * Identify timeout queue.
	 */

	qp = timq + (tp->t_lbolt % __ARRAY_LENGTH (timq));

	/*
	 * Insert at head of timeout queue.
	 */

	s = sphi ();

	if ((tp->t_next = * qp) != NULL)
		tp->t_next->t_last = tp;
	tp->t_last = qp;
	* qp = tp;

	spl (s);
}

#if __USE_PROTO__
void timeout2(TIM * tp, long n, int (*f)(int a), int a)
#else
void
timeout2(tp, n, f, a)
register TIM *tp;
long n;
int (*f)();
int a;
#endif
{
	TIM ** qp;
	int s;

	/*
	 * Already on a timing queue.
	 */

	s = sphi ();
	if ((qp = tp->t_last) != NULL) {
		tp->t_last = NULL;
		if ((* qp = tp->t_next))
			tp->t_next->t_last = qp;
	}
	spl (s);

	/*
	 * Do not schedule new timer if no function or delay interval.
	 */
	if (f == NULL || n == 0)
		return;

	/*
	 * Calculate clock tick at which timeout is to occur.
	 * Record function and argument to be invoked upon timeout.
	 */

	tp->t_lbolt = lbolt + n;
	tp->t_func  = f;
	tp->t_farg  = a;

	/*
	 * Identify timeout queue.
	 */

	qp = timq + (tp->t_lbolt % __ARRAY_LENGTH (timq));

	/*
	 * Insert at head of timeout queue.
	 */
	s = sphi ();

	if ((tp->t_next = * qp) != NULL)
		tp->t_next->t_last = tp;
	tp->t_last = qp;
	* qp = tp;

	spl (s);
}
