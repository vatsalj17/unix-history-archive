/*
 * pv - list the perversities of all the users (foul)
 *
 *
 * pv [-wdsoupaniclxrTt] [passwdfile] [lowuid [..hiuid]]
 *	w: who --> give first name last name
 *	d: give initial directory path
 *	s: give initial shell
 *	o: give other info
 *	u: gives uid
 *	p: gives encrypted password
 *	a: == wdsup
 *	n: print number of users
 *	i: people without passwords and initial shells
 *	c: classes
 *	l: give a list of all his/hers files (login directory only)
 *	r: with the lflag gives a recursive look at the directories
 *	x: people without passwords
 *	t: users terminal groups
 *	T: print usage times
 *
 *		Written Bryan David Palmer,
 *		    Inspired by Peter (-wdsourpanicltTx) Ivanov
 *			P.S: don't blame me for it -- BDP fix
 */

#include <local-system>
#include <passwd.h>
#include <class.h>
struct pwent pe;
char buf[SSIZ];
#include <stat16.h>
struct statbuf se;



int	wflag,
	sflag,
	dflag,
	oflag,
	pflag,
	uflag,
	xflag,
	tflag,
	iflag,
	lflag,
	nflag,
	rflag,
	Tflag,
	cflag;

char initsh[] "/usr/logins/initsh";

compar(s1,s2)
register char *s1, *s2;
{
	while((*s1++ == *s2++) && (s1[-1]));
	return(*--s1-*--s2);
}

struct dirbuf
{
	int	dd_inode;
	char	dd_name[14];
}dirbuf;


#define	IFMT	060000
#define	DIR	0100000
#define	CHR	020000
#define	BLK	040000
#define	ISARG	01000
#define	LARGE	010000
#define	STXT	010000
#define	SUID	04000
#define	SLOK	02000
#define	ROWN	0400
#define	WOWN	0200
#define	XOWN	0100
#define	AUTOLOK	010
#define	ROTH	04
#define	WOTH	02
#define	XOTH	01
#define	RSTXT	01000

#define MAXDIRSZ  1000
char direct[MAXDIRSZ][14];

long tell();
char *ctime();

lscomp(s1,s2)
register char *s1, *s2;
{
	register len;

	len = 14;
	while((*s1++ == *s2++) && (--len));
	return(*--s1-*--s2);
}

printtimes(conect, cpu)
long conect, cpu;
{
	char wundabuf[8];	/* all is revealed */
	char eveni_wunda[8];	/* see wot i mean */

	printf("Connect time: %s\tCpu time: %s",
	    ptime(conect, wundabuf),
	    ptime(cpu/60, eveni_wunda));
}
printfiles(directory,indent)
register char *directory;
{
	int fd;
	long place;
	register int i, j;
	char *tm;

	if((chdir(directory)) == -1)
	{
		printf("%s: cannot chdir\n",directory);
		return(1);
	}
	if((fd = open("",0)) == -1)
	{
		printf("%s: can't open\n",directory);
		return(0);
	}
	i = 0;
	while((read(fd,&dirbuf,16) == 16) && (i < MAXDIRSZ))
	{
		if(dirbuf.dd_inode)
		{
			for(j = 0; j < 14; j++)
				direct[i][j] = dirbuf.dd_name[j];
			i++;
		}
	}
	qsort(direct,i,14,lscomp);
	for(j = 0; j < i; j++)
	{
		if(newstat(direct[j],&se) != -1)
		{
			if((se.sb_flags & IFTYP) == IFDIR)
				putchar('d');
			else if(se.sb_flags & CHR)
				putchar('c');
			else if(se.sb_flags & BLK)
				putchar('b');
			else
				putchar('-');
			if(se.sb_flags & ROWN)
				putchar('r');
			else
				putchar('-');
			if(se.sb_flags & WOWN)
				putchar('w');
			else
				putchar('-');
			if(se.sb_flags & SUID)
				putchar('s');
			else if(se.sb_flags & XOWN)
				putchar('x');
			else
				putchar('-');
			printf("---");
			if(se.sb_flags & ROTH)
				putchar('r');
			else
				putchar('-');
			if(se.sb_flags & WOTH)
					putchar('w');
			else
				putchar('-');
			if(se.sb_flags & XOTH)
				putchar('x');
			else
				putchar('-');
			if(se.sb_flags & RSTXT)
				putchar('T');
			else if((se.sb_flags & AUTOLOK) && (se.sb_flags & SLOK))
				putchar('A');
			else if(se.sb_flags & SLOK)
				putchar('L');
			else	putchar(' ');
			tm = ctime(se.sb_modtime);
			tm[24] = 0;
			printf(" %3d %4d\t%s\t %s %14s\n",
				se.sb_nlinks, se.sb_uid
				,locv(se.sb_size0, se.sb_size1)
				,tm,direct[j]);
		}
	}
	if(rflag)
	{
		seek(fd,32,0);	/* seek past '.' and '..' */
		while(read(fd,&dirbuf,16) == 16)
		{
			if(dirbuf.dd_inode)
			{
				newstat(dirbuf.dd_name,&se);
				if(IFDIR & se.sb_flags)
				{
					place = tell(fd);
					close(fd);
					printf("./");
					for(i = 0; i < indent; i++)
						putchar('/');
					printf("%s:\n",dirbuf.dd_name);
					if(printfiles(dirbuf.dd_name,indent+1) != 1)
						chdir("..");
					fd = open("",0);
					lseek(fd,place,0);
				}
			}
		}
	}
	close(fd);
}

printclasses()	/* FLOGGED CODE from 'pp.c' */
{
	register i,j,k;

	printf("\nClasses:");
	j = k = 0;
	for(i = 0; i < CMASKSIZE*16; i++)
	{
		if( pe.pw_cmask[classes[i].c_word] & classes[i].c_mask)
		{
			if(j == 0 && k != 0)
				putchar('\t');
			k++;
			printf("%10s", classes[i].c_name);
			if( ++j == 7)
			{
				j = 0;
				putchar('\n');
			}
		}
	}
	if(k == 0)
		printf("      None\n");
	else if(j != 0)
		putchar('\n');
}


printterms(u)	/* more code from pp */
register unsigned u;
{
	register i;

	printf("\nTerminals:     ");
	if(u == 0)
		printf("None\n");
	else if(u == -1)
		printf("Any\n");
	else
	{

		for(i = 0; i < 16; i++)
		{
			if(u&01)
				putchar('a'+i);
			u =>> 1;
		}
		putchar('\n');
	}
}


extern fout;
main(argc,argv)
char **argv;
{
	register int uid;
	char *s;
	register struct pwent *pee;
	int louid, hiuid, count;

	fout = dup(1);
	while((argc >= 2) && (*argv[1] == '-'))
	{
		s = *++argv;
		argc--;
		while(*++s)
		{
			switch(*s)
			{
		    case 's' :	sflag++;
				break;
		    case 'w' :	wflag++;
				break;
		    case 'T' :  Tflag++;
				break;
		    case 'd' :	dflag++;
				break;
		    case 'o' :	oflag++;
				break;
		    case 'p' :	pflag++;
				break;
		    case 'n' :	nflag++;
				break;
		    case 'u' :	uflag++;
				break;
		    case 'r' :	rflag++;
				break;
		    case 'x' :  xflag++;
				break;
		    case 'c' :	cflag++;
				break;
		    case 'a' :	wflag++;
				dflag++;
				sflag++;
				uflag++;
				pflag++;
				break;
		    case 'i' :	iflag++;
				xflag++;
				break;
		    case 't' :  tflag++;
				break;
		    case 'l' :	lflag++;
				break;
		    default :	printf("pv [-wdsourpaniclxtT] [optpwfile] [louid[..hiuid]]\n");
				flush();
				exit(1);
			}
		}
	}
	pee = &pe;
	louid = 0;
	hiuid = PWTABSIZE;
	/* see if a new password file is needed */
	if((argc >= 2) && ((argv[1][0] < '0') || (argv[1][0] > '9')))
	{
		argc--, argv++;
		pwfile(*argv);
	}

	if(argc == 2)	/* assume a subrange was entred */
	{
		s = argv[1];
		louid = atoi(s);
		while((*s >= '0') && (*s <= '9'))	s++;
		if(*s)
		{
			while(*s++ == '.');
			if(*--s)
				hiuid = atoi(s);
		}
		else hiuid = louid;
	}
	count = 0;
	for(uid = louid; uid <= hiuid; uid++)
	{
		pee->pw_uid = uid;
		if((getpwlog(pee,buf,sizeof buf)) != -1)
		{
			if(nflag)
			{
				count++;
				goto sloop;	/*WELL*/
			}
			if((xflag) && (*pee->pw_pword))
				goto sloop;	/*YUK*/
			if((iflag) && (compar(initsh,pee->pw_strings[SHELLPATH]) == 0))
				goto sloop;	/*GROTTY*/
			printf("%s",pee->pw_strings[LNAME]);
			if(uflag)
				printf(":%d",pee->pw_uid);
			if(pflag)
			{
				putchar(':');
				if(pee->pw_pword[0])
					printf("%8.8s",pee->pw_pword);
				else
					printf("        ");
			}
			if(wflag)
				printf(":%s %s",pee->pw_strings[FIRSTNAME]
					,pee->pw_strings[LASTNAME]);
			if(sflag)
				printf(":%s",pee->pw_strings[SHELLPATH]);
			if(dflag)
				printf(":%s",pee->pw_strings[DIRPATH]);
			if(cflag)
				printclasses();
			if(tflag)
				printterms(pee->pw_tmask);
			if(oflag)
			{
				printf("\nOther:\n");
				printf("\n%s\n",pee->pw_strings[OTHER]);
			}
			if(lflag)
			{
				printf("\nFiles:\n");
				printfiles(pee->pw_strings[DIRPATH],0);
			}
			if(Tflag)
				printtimes(pee->pw_contime, pee->pw_cputime);
			putchar('\n');
sloop:;
		}
	}
	if(nflag)
		printf("%d\n",count);
	flush();
	exit(0);
}
