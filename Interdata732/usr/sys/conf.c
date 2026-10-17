		/*** Interdata unix configuration file ***/

/*
 * block device switch
 */

int (*bdevsw[])() 
{
	&nulldev, &nulldev, &dskstrategy, &dsktab,	/* 10-mb disc */
	&mtopen, &mtclose, &mtstrategy, &mtab,		/* 800 bpi mag tape */
	&nulldev, &nulldev, &msmstrategy, &msmtab,	/* 67-mb disc */
	0
};

int	nblkdev;	/* number of block device types in system */

/*
 * character device switch
 */

int (*cdevsw[])() 
{
	&vduopen, &vduclose, &vduread, &vduwrite, &vdusgtty,	/* vdu on pals */
	&nulldev, &nulldev, &mmread, &mmwrite, &nulldev,	/* memory */
	&nulldev, &nulldev, &dskread, &dskwrite, &nodev,	/* raw disk */
	&syopen, &nulldev, &syread, &sywrite, &sysgtty,		/* control tty */
	&mtopen, &mtclose, &mtread, &mtwrite, &mtsgtty,		/* raw magtape */
/***	&cliopen, &cliclose, &cliread, &cliwrite, &clisgtty,	/* current loop */
	&nodev, &nodev, &nodev, &nodev, &nodev,
	&lpopen, &lpclose, &nodev, &lpwrite, &lpsgtty,		/* line printer */
	&nulldev, &nulldev, &msmread, &msmwrite, &nodev,	/* raw 67-mb disk */
	0
};

int	nchrdev;	/* number of character device types in system */

/*
 * root and swap device definitions are now in low.s
 */
