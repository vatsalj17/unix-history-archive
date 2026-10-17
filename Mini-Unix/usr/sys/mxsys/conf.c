/*
 */

int	(*bdevsw[])()
{
			&nulldev,	&nulldev,	&rkstrategy, 	&rktab,   
/*	&nulldev,	&nulldev,	&rfstrategy, 	&rftab, */
/*	&nulldev,	&tcclose,	&tcstrategy, 	&tctab, */
/*	&tmopen,	&tmclose,	&tmstrategy, 	&tmtab, */
/*	&htopen,	&htclose,	&htstrategy, 	&httab, */
/*			&nulldev,	&nulldev,	&rpstrategy, 	&rptab,    */
/*			&nulldev,	&nulldev,	&hpstrategy, 	&hptab,    */
	0,
};

int	(*cdevsw[])()
{
	&klopen,   &klclose,   &klread,   &klwrite,   &klsgtty,
	&nulldev,  &nulldev,   &mmread,   &mmwrite,   &nodev,
/*	&lpopen,   &lpclose,   &nodev,    &lpwrite,   &nodev, */
/*	&dcopen,   &dcclose,   &dcread,   &dcwrite,   &dcsgtty, */
/*	&dhopen,   &dhclose,   &dhread,   &dhwrite,   &dhsgtty, */
/*	&dpopen,   &dpclose,   &dpread,   &dpwrite,   &nodev, */
/*	&dnopen,   &dnclose,   &nodev,    &dnwrite,   &nodev, */
	0,
};
