/*
 *	hand-crafted from mkconf output
 */

#include	"../defines.h"
#include	"../param.h"

#include	"../user.h"	/* fix023 */

#ifdef	FPU
char *uufsav	&u.u_fsav;	/* fix023 - used in m70.s */
char *uufec	&u.u_fec;	/* fix023 - used in m70.s */
#endif FPU
char *uuerror	&u.u_error;	/* fix023 - used by powerf.s & m70.s */

int	(*bdevsw[])()
{
	&nulldev,	&nulldev,	&hpstrategy,	&hptab,	/* ms - 0 */
	&rkopen,	&rkclose,	&rkstrategy,	&rktab,	/* rk - 1 */
	&tmopen,	&tmclose,	&tmstrategy,	&tmtab,	/* tm - 2 */
	0
};

int	(*cdevsw[])()
{
	&klopen,   &klclose,  &klread,  &klwrite,  &klsgtty, &kl11,	/* kl - 0 */
#ifdef	ERROR_LOG
	&elopen,   &elclose,  &elread,  &nodev,    &nulldev,     0,	/* el - 1 */
#endif
#ifndef	ERROR_LOG
	&nodev,    &nodev,    &nodev,   &nodev,    &nodev,       0,	/*    - 1 */
#endif
	&lpopen,   &lpclose,  &nodev,   &lpwrite,  &lpsgtty,     0,	/* lp - 2 */
	&nulldev,  &nulldev,  &hpread,  &hpwrite,  &nodev,       0,	/* hp - 3 */
	&nulldev,  &nulldev,  &mmread,  &mmwrite,  &nodev,       0,	/* mem- 4 */
	&rkopen,   &rkclose,  &rkread,  &rkwrite,  &nodev,       0,	/* rk - 5 */
	&tmopen,   &tmclose,  &tmread,  &tmwrite,  &nodev,       0,	/* tm - 6 */
	&nodev,    &nodev,    &nodev,   &nodev,    &nodev,       0,	/* dj - 7 */
	&syopen,   &nulldev,  &syread,  &sywrite,  &sysgtty,     0,	/* sys- 8 */
	&dhopen,   &dhclose,  &dhread,	&dhwrite,  &dhsgtty, &dh11,	/* dh - 9 */
	0
};

int	rootdev	 { (001<<8) | 000 } ;	/* RK drive 0 */
int	swapdev	 { (001<<8) | 000 } ;	/* RK drive 0 */
int	swplo	4000 ;			/* must not be zero */
int	nswap	 872 ;
