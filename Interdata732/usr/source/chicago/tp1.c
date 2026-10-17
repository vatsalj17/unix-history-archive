#define	EXTERN	
#include "tp.h"
char	tc[]	"/dev/tapx";
char	mt[]	"/dev/mt0";
int	oflag;

main(argc,argv)
char **argv;
{
	register char c,*ptr;
	extern cmd(), cmr(),cmx(), cmt();

	command = &cmr;
	flags = 020;		/* default is flu */
	if ((narg = rnarg = argc) < 2)	narg = 2;
	else {
		ptr = argv[1];	/* get first argument */
		parg = &argv[2];	/* pointer to second argument */
		while (c = *ptr++) switch(c)  {
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
				tc[8] = c;
				mt[7] = c;
				continue;
			case 'c':
				flags =| flc;  continue;
			case 'd':
				setcom(&cmd);  continue;
			case 'f':
				flags =| flf;  continue;
			case 'i':
				flags =| fli;  continue;
			case 'm':
				flags =| flm;  continue;
			case 'o':
				oflag++;	continue;
			case 'r':
				flags =& ~flu;  setcom(&cmr);  continue;
			case 't':
				setcom(&cmt);  continue;
			case 'u':
				flags =| flu;  setcom(&cmr);  continue;
			case 'v':
				flags =| flv;  continue;
			case 'w':
				flags =| flw;  continue;
			case 'x':
				setcom(&cmx);  continue;
			default:
				useerr();
		}
	}
/**/
	optap();
	nameblk = nptr = sbrk(0);		/* establish top */
	top = nameblk+NMSIZE*1024;
	brk(top);
	(*command)();
}

optap()
{
	extern cmr();

	if (oflag) {		/* disc file */
		tapsiz = 32767;
		ndirent = MDIRENT;
		if (narg <= 2)
			fio = -1;
		else {
			if (flags & flc)
				fio = creat(*parg++, 0666);
			else
				fio = open(*parg++, 2);
			narg--;
			rnarg--;
		}
	}
	else if ((flags & flm) == 0) {	/*  DECTAPE */
		tapsiz = 578;
		ndirent = 192;
		fio =open(tc,2);
	} else {			/* MAGTAPE */
		tapsiz = 32767;
		ndirent = MDIRENT;
		if (command == &cmr)
			fio = open(mt,1);
		else
			fio = open(mt,0);
	}
	if (fio < 0)  {
		printf("Tape open error\n");
		done();
	}
	ndentd8 = ndirent>>3;
	edir = &dir[ndirent];
}

setcom(newcom)
{
	extern cmr();

	if (command != &cmr)  	useerr();
	command = newcom;
}

useerr()
{
	printf("Bad usage\n");
	done();
}

/*/* COMMANDS */

cmd()
{
	extern delete();

	if (flags & (flm|flc|flf))	useerr();
	if (narg <= 2)			useerr();
	rddir();
	gettape(&delete);
	wrdir();
	check();
}

cmr()
{
	if (flags & (flc|flm))		clrdir();
	else				rddir();
	getfiles();
	update();
	check();
}

cmt()
{
	extern taboc();

	if (flags & (flc|flf|flw))	useerr();
	rddir();
	if (flags & flv)
		printf("   mode    uid gid tapa    size   date    time name\n");
	gettape(&taboc);
	check();
}

cmx()
{
	extern extract();

	if (flags & (flc|flf))		useerr();
	rddir();
	gettape(&extract);
	done();
}

check()
{
	usage();
	done();
}

done()
{
	printf("End\n");
	exit();
}

encode(pname,dptr)	/* pname points to the pathname
			 * nptr points to next location in nameblk
			 * dptr points to the dir entry		   */
{
	register  char *np, *sp;

	sp = pname;
	dptr->d_namep = np = nptr;
	if (np > top - NMSIZE)  {
			printf("Out of core\n");
			done();
	}
	while (*np++ = *sp++);
	if (np <= nptr + NMSIZE)  nptr = np;
	else {
		printf("Pathname too long - %s\nFile ignored\n",nptr);
		clrent(dptr);
	}
}

decode(pname,dptr)	/* dptr points to the dir entry
			 * name is placed in pname[] */
{

	register char *p1, *p2;

	p1 = pname;
	p2 = dptr->d_namep;
	while (*p1++ = *p2++);
}
