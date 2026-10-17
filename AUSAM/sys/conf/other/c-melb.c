/*
 */

int	(*bdevsw[])()
{
	&hpopen,	&nulldev,	&hpstrategy, 	&hptab,	/*  0  ->  hp          */
	&htopen,	&htclose,	&htstrategy, 	&httab,	/*  1  ->  ht          */
	0
};

int	(*cdevsw[])()
{
	&klopen,   &klclose,  &klread,   &klwrite,  &klsgtty,	/*  0  ->  console     */
	&lpopen,   &lpclose,  &nodev,    &lpwrite,  &nodev,	/*  1  ->  lp          */
	&djopen,   &djclose,  &djread,   &djwrite,  &djsgtty,	/*  2  ->  dj          */
	&nulldev,  &nulldev,  &mmread,   &mmwrite,  &nodev,	/*  3  ->  mem         */
	&hpopen,   &nulldev,  &hpread,   &hpwrite,  &nodev,	/*  4  ->  hp          */
	&htopen,   &htclose,  &htread,   &htwrite,  &nodev,	/*  5  ->  ht          */
	&craopen,  &crclose,  &crread,   &nodev,    &nodev,	/*  6  ->  cr          */
	&crbopen,  &crclose,  &crread,   &nodev,    &nodev,	/*  7  ->  crb         */
	&syopen,   &nulldev,  &syread,   &sywrite,  &sysgtty,	/*  8  ->  sys         */
	0
};

int	rootdev	 { (000<<8) | 000 };
int	swapdev	 { (000<<8) | 000  };
int	swplo	  	  4599;
int	nswap	  { (13*418)-(11*418) } ;
