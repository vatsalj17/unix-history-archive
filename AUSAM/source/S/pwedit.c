#

/*
 *	This program maintains and updates the passwd file
 *	under the AUSAM system. The command has the following format.
 *
 *		pwedit [-m] [-u] [-k] [-n] [-x] [-c] [-pstring] [-tstring] [filename]
 *
 *	where	-k	allows keyword specifications in the input
 *		-n	causes encryption of passwords to be suspended
 *		-x	causes extraction of all files onto standard output
 *		-m	causes initial directory creation on adds
 *		-u	allows uids not in the default range to be specified
 *		-c	causes comments to appear in password entry listings
 *			from -x option and editor temporarys...
 *		-pstring the string is to be used as the alternate password
 *			file name
 *		-tstring specifies a string to be used as a template filename
 *			that is "/usr/lib/pwetplts/string"
 *		filename specifies a file from which entries are to be taken
 *			for operation in non-interactive mode
 *
 *		Commands which may be given are; add, delete, update,
 *			change, look and default.
 *
 *
 *		Graham fix 29/11/78
 *			octal numbers in updates are recognised by a leading
 *			'0' and are also printed by pwedit thus.
 *
 */

#include	<local-system>
#include	<passwd.h>
#include	<class.h>


/*	some nice little defines to make life easier	*/ 
#define	PWETMPS		"/tmp/"			/* where tmp files are kept */
#define	PWETPLTS	"/usr/lib/pwe/tplts/"	/* where the templates are */
#define	PWEDFLTS	"/usr/lib/pwe/dflts/"	/* where the defaults are */
#define	PWETDFLT	"default"		/* the default name */

#define	SSIZE		100	/* name string lengths */

/*	some handles on difficult to hold things	*/ 
struct
{
	unsigned hiword;
	unsigned loword;
};
struct
{
	char lobyte;
	char hibyte;
};

/*	now the global data	*/ 

/*	flags	*/ 
int tflag, nflag, xflag, cflag, fflag, mflag, uflag;
int sigstop;
int status;	/* status used for waits */ 
int owuid;	/* the caller's uid */

/*	filename arrays	*/ 
char tname[SSIZE], dname[SSIZE], ename[SSIZE], fname[SSIZE];

/*	temp file templates for editor	*/ 
char *edtemp "pweaXXXXX";

/*	the global last character	*/ 
char nextc;
char c;

/*	global passwd entry	*/ 
struct pwent pe;

/*	global default entry	*/ 
struct pwent de;

/*	putc and getc buffers	*/ 
struct iobuf
{
	int fildes;
	int count;
	char *cnext;
	char buff[512];
} sibuf;
struct iobuf sobuf {
	1,
	0,
	0,
	512*0
};
struct iobuf *ibuf, *obuf;

/*	some definitions	*/ 
#define	ITEMKEY	1	/* a keyword item */
#define	ITEMNUM	2	/* a number item, may be byte,int or long */
#define	ITEMSTR	3	/* string item */
#define	ITEMBIT	4	/* a bit string */
#define	ITEMNL	5	/* an empty line */
#define	ITEMTERM 6	/* a terminal group spec in {}'s */

/*	definition of ITEMKEY command codes for internal use	*/ 
#define	ADDCOM	0	/* add command */
#define	DELCOM	1	/* delete command */
#define	CHNGCOM	2	/* change command */
#define	UPDTCOM	3	/* update command */
#define	LOOKCOM	4	/* lookat command */

/*	string to code matching	*/ 
char *commands[]
{
	"add",		/* ADDCOM */ 
	"delete",	/* DELCOM */ 
	"change",	/* CHNGCOM */ 
	"update",	/* UPDTCOM */ 
	"look",		/* LOOKCOM */
	0
};

/*	some handles to hang on to things by	*/ 
long	pnum;		/* the global read number	*/ 
int	pcom;		/* type of command, +ve if known, -ve if not */ 
int	scom;		/* a place to keep it safe */ 
int pclass[CMASKSIZE];	/* the present read class mask bits string */ 
char	*pstr;		/* the global read string */ 


main(argc, argv)
int argc;
char *argv[];
{
	extern catsig();
	register int i;

	/*	initialise io	*/ 


	ibuf = &sibuf;
	obuf = &sobuf;

#ifdef	DEBUG
	printf("main\n");
	fflush(obuf);
#endif	DEBUG
	owuid = getuid();

	/*	get args and stuff	*/ 
	getargs(argc, argv);

	/*	initialise defaults etc	*/ 
	initialise();

	/*
	 *	now have serveral possibilities.
	 *	extraction mode, file input mode, or interaction.
	 */ 
	if(xflag)
		xtract();
	else
	{
		/*	initialise nasty signals	*/ 
		for(i = 1; i < 4; i++)
			signal(i, 1);
		for(i = 14; i <= 16; i++)
			signal(i, catsig);
		if(fflag)
			readfile();
		else
			interact();
	}

}


getargs(ac, av)
int ac;
char **av;
{
	/*	things with a '-' are flags, without is a filename	*/ 

#ifdef	DEBUG
	printf("getargs\n");
	fflush(obuf);
#endif	DEBUG
	while(--ac)
	{
		if(**++av == '-')
		{
			switch(av[0][1])
			{
		    case 't':
				settname(*av+2);
				tflag++;
				break;
		    case 'p':
				setpname(*av+2);
				break;
		    case 'm':
				mflag++;
				break;
		    case 'u':
				uflag++;
				break;
		    case 'n':
				nflag++;
				break;
		    case 'x':
				xflag++;
				break;
		    case 'c':
				cflag++;
				break;
		    default:
				error(1, *av);
				break;
			}
		}
		else
		{
			setfname(*av);
			fflag++;
		}
	}
#ifdef	DEBUG
	printf("tflag=%d,nflag=%d,xflag=%d\n", tflag, nflag, xflag);
	fflush(obuf);
	printf("tname->%s<-,dname->%s<-,fname->%s<-\n", tname, dname, fname);
	fflush(obuf);
#endif	DEBUG
}


initialise()
{
	/*
	 *	set default file names if necessary, open template and default files
	 *	and initialise default pwent.
	 */ 

#ifdef	DEBUG
	printf("initialise\n");
	fflush(obuf);
#endif	DEBUG
	if(!tflag)
	{
		strcpy(tname, PWETPLTS);
		strcat(tname, PWETDFLT);
		strcpy(dname, PWEDFLTS);
		strcat(dname, PWETDFLT);
	}
	getdefault();
}


settname(s)
char *s;
{
	/*	make up the filenames for templates and defaults	*/ 

#ifdef	DEBUG
	printf("settname\n");
	fflush(obuf);
#endif	DEBUG
	strcpy(tname, PWETPLTS);
	strcat(tname, s);
	strcpy(dname, PWEDFLTS);
	strcat(dname, s);
}


setpname(s)
char *s;
{
	/*	set the default password file name	*/ 
#ifdef	DEBUG
	printf("setpname\n");
	fflush(obuf);
#endif	DEBUG
	pwfile(s);
}


setfname(s)
char *s;
{
	/*	set input file name	*/ 

#ifdef	DEBUG
	printf("setfname\n");
	fflush(obuf);
#endif	DEBUG
	strcpy(fname, s);
}


xtract()
{
	/*	explode whole passwd file onto SO with adds for each entry	*/ 
	register struct iobuf *rsbuf;
	register int i;

#ifdef	DEBUG


	printf("xtract\n");
	fflush(obuf);
#endif	DEBUG
	/*	save old output	*/ 
	rsbuf = obuf;
	fflush(obuf);
	obuf = &sobuf;

	/*	output	*/ 
	for(i = 0; i < PWTABSIZE; i++)
	{
		pe.pw_uid = i;
		if(getpwlog(&pe, 0, 0) < 0)
			continue;
		/*	real entry, output it	*/ 
		if(getpw(0, &pe) < 0)
			continue;
		putpe("add", &pe, 1);
	}

	/*	restore old SO	*/ 
	fflush(obuf);
	obuf = rsbuf;
}


interact()
{
	/*
	 *	it interacts with the cretins
	 */ 
	register int ri, rf;
	struct iobuf *ribuf, *robuf;
	char cc;

#ifdef	DEBUG
	printf("interact\n");
	fflush(obuf);
#endif	DEBUG
	while(1)
	{
		/* major loop */ 
		if(sigstop)
			break;
		rf = 0;
		if( owuid == 0)
		{
			do
			{
				putchar('>');
				fflush(obuf);
			}
			while(((ri = getitem()) != ITEMKEY) && (ri != 0));
			if(ri == 0)
				break;
			scom = pcom;
		}
		else
			scom = LOOKCOM;

		if(scom != ADDCOM)
		{
			/* only need uid and lname if not an add command */ 
			printf("lname: ");
			fflush(obuf);
			if((ri = getitem()) == 0)
				break;
			if(ri == ITEMSTR)
			{
				pe.pw_strings[LNAME] = pstr;
				rf =| 02;
			}
			if( rf == 0 || scom == DELCOM)
			{
				printf("uid: ");
				fflush(obuf);
				if((ri = getitem()) == 0)
					break;
				if(ri == ITEMNUM)
				{
					/* got a number, not just nothing */ 
					if(!bcheck(2))
					{
						error(15, 0);
						continue;
					}
					pe.pw_uid = pnum.loword;
					rf =| 01;
				}
			}
		}

		/* now have command and other data, just use it */ 
		if(scom == DELCOM)
		{
			/* delete command, must have both uid and lname */ 
			if((rf&03) != 03)
			{
				error(17, 0);
			}
			else
			{
				if(delpwent(&pe) < 1)
					error(9, 0);
			}
		}
		else
		{
			/* for add command or flags 01|02 must go on */ 
			if((scom == ADDCOM) || (rf&03))
			{
				fflush(obuf);
				robuf = obuf;
				strcpy(ename, PWETMPS);
				strcat(ename, edtemp);
				mktemp(ename);
				obuf = copen(ename, 0600);

				if(scom == ADDCOM)
				{
					/* for an add must copy over template */ 
					ribuf = ibuf;
					ibuf = copen(tname, 0);
					while((cc = getc(ibuf)) != -1)
						putc(cc, obuf);
					cclose(ibuf);
					ibuf = ribuf;
				}
				else
				{
					/* must be change or update */ 
					if(rf&01)
					{
						if(getpw(0, &pe) < 0)
							rf =| 04;
					}
					else
					{
						if(getpw(1, &pe) < 0)
							rf =| 04;
					}

					/* free up the interactive login name */ 

					if(!(rf&04))
						putpe(commands[scom], &pe, 1);
				}

				/* finished with obuf */ 
				fflush(obuf);
				cclose(obuf);
				obuf = robuf;

				if(!(rf&04))
				{
					/* return input absorbed into buffer */
					seek(ibuf->fildes, -(ibuf->count), 1);
					ibuf->count = 0;
					/* now spawn an elfic ed */ 
					if(fork())
					{
						/* parent */ 
						wait(&status);
					}
					else
					{
						/* child */ 
						if(execl("/bin/em", "pwede", "-e", ename, 0) < 0)
						{
							error(16, 0);
						}
					}

					/* now have returned so validate results */ 
					if(scom != LOOKCOM)
					{
						ribuf = ibuf;
						ibuf = copen(ename, 0);
						if(getpe(&pe) < 0)
						{
							error(5, 1);
							rf =| 04;
						}
						else
							if(putpw(&pe) < 0)
								error(6, 1);
						cclose(ibuf);
						ibuf = ribuf;
					}
				}
				unlink(ename);
			}
		}
	}
}


putpe(rs, rpe, ff)
register char *rs;
register struct pwent *rpe;
int ff;
{
	/*
	 *	Output the pwent struct at *rpe with the string at *rs as the
	 *	first line of output. Uses printf calling putchar calling putc
	 *	an ff of 1 causes freeing of string area after output
	 */ 
	register int i;

#ifdef	DEBUG
	printf("putpe\n");
	fflush(obuf);
#endif	DEBUG
	/*	command out	*/ 
	printf("%s\n", rs);

	/*	now all other entries	*/ 
	if(cflag)
	{
		printf("%d	/* uid */\n", rpe->pw_uid);
	}
	else
	{
		printf("%d\n", rpe->pw_uid);
	}
	if(cflag)
	{
		printf("%d	/* shares */\n", rpe->pw_shares);
	}
	else
	{
		printf("%d\n", rpe->pw_shares);
	}
	if(cflag)
	{
		printf("%ld	/* usage */\n", rpe->pw_usage);
	}
	else
	{
		printf("%ld\n", rpe->pw_usage);
	}

	printclass(rpe);

	printterms(rpe);

	if(cflag)
	{
		printf("0%o	/* flags */\n", rpe->pw_flags);	/* Graham fix	*/
	}
	else
	{
		printf("0%o\n", rpe->pw_flags);		/* Graham fix */
	}
	if(cflag)
	{
		printf("%d	/* dlimit */\n", rpe->pw_dlimit);
	}
	else
	{
		printf("%d\n", rpe->pw_dlimit);
	}
	if(cflag)
	{
		printf("%d	/* doverflw */\n", rpe->pw_doverflw);
	}
	else
	{
		printf("%d\n", rpe->pw_doverflw);
	}
	if(cflag)
	{
		printf("%d	/* plimit */\n", rpe->pw_plimit);
	}
	else
	{
		printf("%d\n", rpe->pw_plimit);
	}
	if(cflag)
	{
		printf("%d	/* climit */\n", rpe->pw_climit);
	}
	else
	{
		printf("%d\n", rpe->pw_climit);
	}
	if(rpe->pw_pword[0] == 0)
		printf("\"\"\n");
	else
	{
		printf("\"%c%c%c%c%c%c%c%c\"\n", rpe->pw_pword[0], rpe->pw_pword[1], rpe->pw_pword[2], rpe->pw_pword[3],
			 rpe->pw_pword[4], rpe->pw_pword[5], rpe->pw_pword[6], rpe->pw_pword[7]);
	}
	if(cflag)
	{
		printf("%ld	/* contime */\n", rpe->pw_contime);
	}
	else
	{
		printf("%ld\n", rpe->pw_contime);
	}
	if(cflag)
	{
		printf("%ld	/* cputime */\n", rpe->pw_cputime);
	}
	else
	{
		printf("%ld\n", rpe->pw_cputime);
	}
	if(cflag)
	{
		printf("%ld	/* extime */\n", rpe->pw_extime);
	}
	else
	{
		printf("%ld\n", rpe->pw_extime);
	}
	if(cflag)
	{
		printf("%d	/* warn */\n", rpe->pw_warn);
	}
	else
	{
		printf("%d\n", rpe->pw_warn);
	}
	if(cflag)
	{
		printf("%d	/* pages */\n", rpe->pw_pages);
	}
	else
	{
		printf("%d\n", rpe->pw_pages);
	}
#ifdef	TERMBOOK
	if(cflag)
	{
		printf("%d	/* tblim */\n", rpe->pw_tblim & 0377);
	}
	else
	{
		printf("%d\n", rpe->pw_tblim & 0377);
	}
	if(cflag)
	{
		printf("%d	/* tbrate */\n", rpe->pw_tbrate);
	}
	else
	{
		printf("%d\n", rpe->pw_tbrate);
	}
#endif	TERMBOOK

	for(i = 0; i < PWSLENCNT; i++)
		if(rpe->pw_strings[i] != 0)
		{
			printf("\"%s\"\n", rpe->pw_strings[i]);
		}
		else
		{
			error(2, i);
		}
	if(ff)
	{
		free(rpe->pw_strings[0]);
		for(i = 0; i < PWSLENCNT; i++)
			rpe->pw_strings[i] = 0;
	}
}


getpw(ct, rpe)
int ct;
register struct pwent *rpe;
{
	/*
	 *	using call type, ct, gets an entry into *rpe. cts are:
	 *		0	by uid
	 *		1	by login name
	 */ 
	register int rsum, ri;
	char *as, *ss;

#ifdef	DEBUG
	printf("getpw\n");
	fflush(obuf);
#endif	DEBUG
	/*	first determine the string sizes	*/ 
	ss = rpe->pw_strings[0];

	switch(ct)
	{
    case 0:	/* by uid */ 
		if(getpwlog(rpe, 0, 0) < 0)
		{
			error(12, 0);
			return(-1);
		}
		break;
    case 1:	/* by login name */ 
		if(getpwuid(rpe, 0, 0) < 0)
		{
			error(13, 0);
			return(-1);
		}
		break;
	}

	for(ri = 0, rsum = 0; ri < PWSLENCNT; ri++)
	{
		rsum =+ rpe->pw_strings[ri];
	}

	if((as = alloc(rsum)) < 0)
	{
		error(14, 0);
		return(-1);
	}

	switch(ct)
	{
    case 0:	/* by uid we get it */ 
		if(getpwlog(rpe, as, rsum) != rsum)
		{
			error(12, 0);
			return(-1);
		}
		break;
    case 1:	/* by login name */ 
		rpe->pw_strings[0] = ss;
		if(getpwuid(rpe, as, rsum) != rsum)
		{
			error(13, 0);
			return(-1);
		}
		break;
	}

}


putpw(rpe)
register struct pwent *rep;
{
	/*
	 *	add change or update a passwd file entry using the stuff in *rep
	 */ 
	register int ri;

#ifdef	DEBUG
	printf("putpw\n");
	fflush(obuf);
#endif	DEBUG
	switch(scom)
	{
    case 0:	/* add */ 
		/* enforce the bounds limits for this site */ 
		if(!uflag)
			rpe->pw_uid = UIDLB;
		if(addpwent(rpe) < 0)
			error(8, rpe->pw_strings[LNAME]);
		else
		/* make directories etc if needed */ 
		if(mflag)
			mkit();
		break;
    case 2:	/* change */ 
		if(chngpwent(rpe) < 0)
			error(10, 0);
		break;
    case 3:	/* update */ 
		if(updtpwent(rpe) < 0)
			error(11, 0);
		break;
	}

	for(ri = 0; ri < PWSLENCNT; ri++)
	{
		free(rpe->pw_strings[ri]);
		rpe->pw_strings[ri] = 0;
	}

}


struct iobuf *copen(s, m)
char *s;
int m;
{
	/*
	 *	open (or if m>0 creat) a file with name "*s" and make
	 *	associated buffer area for getc and putc.
	 */ 
	register int rfd;
	register struct iobuf *rsbuf;

#ifdef	DEBUG
	printf("copen of %s\n", s);
	fflush(obuf);
#endif	DEBUG
	if(m > 0)
	{
		/*	creat the file	*/ 
		if((rfd = creat(s, m)) < 0)
		{
			error(3, s);
		}
	}
	else
	{
		/*	open only	*/ 
		if((rfd = open(s, 0)) < 0)
		{
			error(0, s);
		}
	}
	if((rsbuf = alloc(sizeof sobuf)) < 0)
		error(4, s);
	rsbuf->fildes = rfd;
	rsbuf->count = 0;
	rsbuf->cnext = 0;
	return(rsbuf);
}


readfile()
{
	/*
	 *	opens file called "*fname" and decodes input to form
	 *	new and modified passwd entries. Sets up input buffers also.
	 */ 
	register struct iobuf *rsbuf;
	register int ri, rn;

	/*	save old input buffer	*/ 
#ifdef	DEBUG
	printf("readfile\n");
	fflush(obuf);
#endif	DEBUG
	rsbuf = ibuf;
	/*	open and use the new stuff	*/ 
	ibuf = copen(fname, 0);
	rn = 0;
	while(++rn)
	{
		if(sigstop)
			break;
		if((ri = getpe(&pe)) < 0)
		{
			error(5, rn);
			break;
		}
		if(putpw(&pe) < 0)
			error(6, rn);
		if(ri == 0)
			break;
	}

	/*	flush and restore to exit	*/ 
	fflush(obuf);
	cclose(ibuf);
	ibuf = rsbuf;
}


getdefault()
{
	/*
	 *	open files etc and read defaults into "&de".
	 *	Note that defaults of zero force entries to be requested for them
	 */ 
	register struct iobuf *rsbuf;

#ifdef	DEBUG
	printf("getdefault\n");
	fflush(obuf);
#endif	DEBUG
	rsbuf = ibuf;
	ibuf = copen(dname, 0);
	if(getpe(&de) < 0)
		error(5, 0);
	cclose(ibuf);
	ibuf = rsbuf;
}


getpe(rpe)
register struct pwent *rpe;
{
	/*
	 *	reads and validates the entry from "ibuf" into "*rpe".
	 */ 
	register char *rpw;
	register int rn;

#ifdef	DEBUG
	printf("getpe\n");
	fflush(obuf);
#endif	DEBUG
	/*	get command	*/ 
	if(getitem() != ITEMKEY)
		return(-1);
	if((scom = pcom) < 0)
		return(-1);

	/*	get uid	*/ 
	if(getitem() != ITEMNUM)
		return(-1);
	if(!bcheck(2))
		return(-1);
	rpe->pw_uid = pnum.loword;

	/*	get shares	*/ 
	if(getitem() != ITEMNUM)
		return(-1);
	if(!bcheck(2))
		return(-1);
	rpe->pw_shares = pnum.loword;

	/*	get usage	*/ 
	if(getitem() != ITEMNUM)
		return(-1);
	if(!bcheck(4))
		return(-1);
	rpe->pw_usage = pnum;

	/*	read class mask bit string	*/ 
	if(getitem() != ITEMBIT)
		return(-1);
	for(rn = 0; rn < CMASKSIZE; rn++)
		rpe->pw_cmask[rn] = pclass[rn];

	/*	read terminal groups string	*/ 
	if(getitem() != ITEMTERM)
		return(-1);
	rpe->pw_tmask = pnum.loword;

	/*	get flags	*/ 
	if(getitem() != ITEMNUM)
		return(-1);
	if(!bcheck(2))
		return(-1);
	rpe->pw_flags = pnum.loword;

	/*	get dlimit	*/ 
	if(getitem() != ITEMNUM)
		return(-1);
	if(!bcheck(2))
		return(-1);
	rpe->pw_dlimit = pnum.loword;


	/*	get doverflw	*/ 
	if(getitem() != ITEMNUM)
		return(-1);
	if(!bcheck(2))
		return(-1);
	rpe->pw_doverflw = pnum.loword;
	/*	get plimit	*/ 
	if(getitem() != ITEMNUM)
		return(-1);
	if(!bcheck(1))
		return(-1);
	rpe->pw_plimit = pnum.loword.lobyte;

	/*	get climit	*/ 
	if(getitem() != ITEMNUM)
		return(-1);
	if(!bcheck(1))
		return(-1);
	rpe->pw_climit = pnum.loword.lobyte;

	/*	get passwd	*/ 
	if(getitem() != ITEMSTR)
		return(-1);
	if(*pstr == 0)
	{
		/* null passwd */ 
		for(rn = 0; rn < 8; rn++)
			rpe->pw_pword[rn] = 0;
	}
	else if((scom != ADDCOM) || (nflag))
	{
		/* no encryption */ 
		for(rn = 0; rn < 8; rn++)
			rpe->pw_pword[rn] = pstr[rn];
	}
	else
	{
		/* encrypt */ 
		rpw = crypt(pstr);
		for(rn = 0; rn < 8; rn++)
			rpe->pw_pword[rn] = rpw[rn];
	}

	/*	get contime	*/ 
	if(getitem() != ITEMNUM)
		return(-1);
	if(!bcheck(4))
		return(-1);
	rpe->pw_contime = pnum;

	/*	get cputime	*/ 
	if(getitem() != ITEMNUM)
		return(-1);
	if(!bcheck(4))
		return(-1);
	rpe->pw_cputime = pnum;

	/*	get extime	*/ 
	if(getitem() != ITEMNUM)
		return(-1);
	if(!bcheck(4))
		return(-1);
	rpe->pw_extime = pnum;

	/*	get warn	*/ 
	if(getitem() != ITEMNUM)
		return(-1);
	if(!bcheck(1))
		return(-1);
	rpe->pw_warn = pnum.loword.lobyte;

	/*	get pages	*/ 
	if(getitem() != ITEMNUM)
		return(-1);
	if(!bcheck(1))
		return(-1);
	rpe->pw_pages = pnum.loword.lobyte;

#ifdef TERMBOOK
	/*	get tblim	*/
	if(getitem() != ITEMNUM)
		return(-1);
	if(!bcheck(1))
		return(-1);
	rpe->pw_tblim = pnum.loword.lobyte;

	/*	get tbrate	*/
	if(getitem() != ITEMNUM)
		return(-1);
	if(!bcheck(1))
		return(-1);
	rpe->pw_tbrate = pnum.loword.lobyte;

#endif	TERMBOOK

	/*	finally get strings	*/ 
	for(rn = 0; rn < PWSLENCNT; rn++)
	{
		if(getitem() != ITEMSTR)
			return(-1);
		if(rpe->pw_strings[rn] != 0)
		{
			free(rpe->pw_strings[rn]);
		}
		rpe->pw_strings[rn] = pstr;
		pstr = 0;
	}

}


getitem()
{
	/*
	 *	returns a number corresponding to the type of item read.
	 *	return values are define previously. Returns the negative of the
	 *	normal return for an error item. Returns 0 for eof.
	 */ 
	register int finished;
	register int rv;
	register int ri;

#ifdef	DEBUG
	printf("getitem\n");
	fflush(obuf);
#endif	DEBUG
	finished = 0;

	c = getc(ibuf);

	while(!finished)
	{
		if(c == -1)
			return(0);
		if((c <= 'z') && (c >= 'a'))
		{
			/*	a keyword	*/ 
			rv = ITEMKEY;
			nextc = c;
			pstr = getstring(pstr, 0);
			for(ri = 0; (strcmp(commands[ri], pstr) != 0); ri++)
			{
				if(commands[ri] == '\0')
				{
					ri = -1;
					break;
				}
			}
			pcom = ri;
			finished = 1;
		}
		else if((c <= '9') && (c >= '0'))
		{
			/*	a number of some form	*/ 
			rv = ITEMNUM;
			if(c == '0')	/* Graham fix,we're not interested in null's!	*/
			{
				if(getnum(8) < 0)
					rv =* -1;
			}
			else
			{
				if(getnum(10) < 0)
					rv =* -1;
			}
			finished = 1;

		}
		else
			switch(c)
			{
		    case '"':	/*	a string	*/ 
				rv = ITEMSTR;
				pstr = getstring(pstr, 1);
				finished = 1;
				break;
		    case '/':	/*	a comment	*/ 
				flushline();
				break;
		    case '`':	/*	a bit string	*/ 
				rv = ITEMBIT;
				if(getclass() < 0)
					rv =* -1;
				finished = 1;
				break;
		    case '{':	/* a terminal group spec */ 
				rv = ITEMTERM;
				if(getterms() < 0)
					rv = -rv;
				finished = 1;
				break;
		    case '\n':	/* a null line */ 
				rv = ITEMNL;
				finished = 1;
				break;
		    default:	/*	read another char and do again	*/ 
				c = getc(ibuf);
				break;
			}
	}

	flushline();
	return(rv);
}


getnum(rbase)
register int rbase;
{
	/*
	 *	get a number terminated by a blank in base rbase
	 */ 
	char cc;

#ifdef	DEBUG
	printf("getnum\n");
	fflush(obuf);
#endif	DEBUG
	pnum = 0;

	while(((cc = c-'0') >= 0) && (cc < rbase))
	{
		pnum = (pnum*rbase)+cc;
		c = getc(ibuf);
	}

	return(0);
}


printclass(rpe)
register struct pwent *rpe;
{
	/*
	 *	prints the bitwise representation of the pw_cmask
	 */ 
	register int rn, ri;

#ifdef	DEBUG
	printf("printclass\n");
	fflush(obuf);
#endif	DEBUG
	putc('`', obuf);
	for(rn = 0; rn < CMASKSIZE; rn++)
	{
		/* int by int */ 
		for(ri = 0; ri < 16; ri++)
		{
			/* bit by bit */ 
			if(((rpe->pw_cmask[rn]<<ri)&0100000) == 0)
				putc('0', obuf);
			else
				putc('1', obuf);
		}
	}
	putc('`', obuf);
	putc('\n', obuf);
}

printterms(rpe)
register struct pwent *rpe;
{
	/*
	 *	prints the groupwise representation of pw_tmask
	 */ 
	register rn, ri;

#ifdef	DEBUG
	printf("printterms\n");
	fflush(obuf);
#endif	DEBUG

	rn = rpe->pw_tmask;
	putc('{', obuf);
	for(ri = 0; ri < 16; ri++)
	{
		if(rn&01)
		{
			putc('a'+ri, obuf);
		}
		rn =>> 1;
	}
	putc('}', obuf);
	putc('\n', obuf);
}


getclass()
{
	/*	read class mask into pclass	*/ 
	register int ri, rn;

#ifdef	DEBUG
	printf("getclass\n");
	fflush(obuf);
#endif	DEBUG
	for(rn = 0; rn < CMASKSIZE; rn++)
		pclass[rn] = 0;

	for(rn = 0; rn < CMASKSIZE; rn++)
	{
		for(ri = 16; --ri >= 0; )
		{
			c = getc(ibuf);
			if((c == '0') || (c == '1'))
			{
				if(c == '0')
					pclass[rn] =& ~(1<<ri);
				else
					pclass[rn] =| (1<<ri);
			}
			else
				return(-1);
		}
	}

	return(0);
}

getterms()
{
	/*
	 *	return terminal mask in pnum.loword
	 */ 
	register rc, ri;

#ifdef	DEBUG
	printf("getterms\n");
	fflush(obuf);
#endif	DEBUG

	ri = 0;
	while((rc = getc(ibuf)) != -1 && rc != '\n')
	{
		if(rc == '}')
		{
			pnum.loword = ri;
			return 0;
		}
		if(rc >= 'a' && rc <= 'p')	/* only valid groups so far */ 
		{
			ri = ri^(1<<(rc-'a'));
		}
		else if(rc == '!')
			ri = ~ri;
		else
			return -1;
	}
	return -1;
}


flushline()
{
	/* flushes ibuf until a '\n' or eof are reached */ 

#ifdef	DEBUG
	printf("flushline\n");
	fflush(obuf);
#endif	DEBUG
	while((c != '\n') && (c != -1))
	{
		c = getc(ibuf);
	}

}


bcheck(ri)
register int ri;
{
	/*
	 *	checks to see if the global long pnum has a number small enough
	 *	to fit in "ri" bytes.
	 */ 
#ifdef	DEBUG
	printf("bcheck\n");
	fflush(obuf);
#endif	DEBUG
	return(1);
}


cclose(rsbuf)
register struct iobuf *rsbuf;
{
	/*
	 *	close the file
	 *	and free the buffer area
	 */ 
#ifdef	DEBUG
	printf("cclose\n");
	fflush(obuf);
#endif	DEBUG
	close(rsbuf->fildes);
	free(rsbuf);
}


char readc(tm)
int tm;
{
	/*
	 *	reads characters from the ibuf file one by one.
	 *	returns characters, '\0' for EOF or end of string
	 *	specified by value of tm.
	 */ 

#ifdef	DEBUG
	printf("readc\n");
	fflush(obuf);
#endif	DEBUG
	if(nextc)
	{
		c = nextc;
		nextc = 0;
	}
	else
		c = getc(ibuf);

	if(c == '\\')
	{
		c = getc(ibuf);
		return(c);
	}

	if(c == -1)
		return('\0');
	if(tm)
	{
		/* terminate with '"' */ 
		if(c == '"')
			return('\0');
	}
	else
	{
		/* terminate with ' ' or '\n' */ 
		if((c == ' ') || (c == '\n'))
			return('\0');
	}

	return(c);
}


char *getstring(ptr, tm)
char *ptr;
int tm;
{
	/*
	 *	read a string from the input source specified by present ibuf.
	 *	free the area pointed to by ptr (if non-zero). return a pointer
	 *	to a newly alloced string.
	 *	tm specifies the termination method, as in 1 for a """ terminator
	 *	and 0 for normal ones like " ", "\n" etc
	 */ 
	register char *string;
	register s, slen, finished;

#ifdef	DEBUG
	printf("getstring\n");
	fflush(obuf);
#endif	DEBUG
	s = 0;
	finished = 0;

	if(ptr)
	{
		free(ptr);
		ptr = 0;
	}

	string = alloc(slen = SSIZE);

	while(!finished)
	{
		while(s < slen)
		{
			if((string[s++] = readc(tm)) <= 0)
			{
				finished = 1;
				break;
			}
		}
		if(finished)
			string = expand(string, s, slen);
		else
			string = expand(string, slen, slen =+ SSIZE);
	}

	return(string);
}


char *expand(old, initial, final)
register char *old;
{
	/*
	 *	modifies size of area pointed to by "old" from "initial" to "final"
	 *	and returns "new" pointer
	 */ 
	register char *new;
	register int i;

#ifdef	DEBUG
	printf("expand\n");
	fflush(obuf);
#endif	DEBUG
	new = alloc(final);

	for(i = 0; i < initial; i++)
	{
		new[i] = old[i];
	}

	free(old);

	return(new);
}


error(ri, rp)
register int ri;
register unsigned *rp;
{
	/*	error output and actions	*/ 
	register struct iobuf *rsbuf;

	rsbuf = obuf;
	obuf = &sobuf;

	printf("pwedit: ");

	switch(ri)
	{
    case 0:
		printf("cannot open file %s\n", rp);
		fflush(obuf);
		exit(0);
		break;
    case 1:
		printf("unknown argument %s\n", rp);
		break;
    case 2:
		printf("not used at present\n");
		fflush(obuf);
		exit(0);
		break;
    case 3:
		printf("cannot creat file %s\n", rp);
		fflush(obuf);
		exit(0);
		break;
    case 4:
		printf("cannot alloc buffer area for file %s\n", rp);
		fflush(obuf);
		exit(0);
		break;
    case 5:
		printf("error in input file at item %d\n", rp);
		break;
    case 6:
		printf("error writing passwd file at item %d\n", rp);
		fflush(obuf);
		exit(0);
		break;
    case 7:
		printf("invalid character is %c\n", *rp);
		fflush(obuf);
		exit(0);
		break;
    case 8:
		printf("passwd add error login name %s\n", rp);
		break;
    case 9:
		printf("passwd delete error\n");
		break;
    case 10:
		printf("passwd change error\n");
		break;
    case 11:
		printf("passwd update error\n");
		break;
    case 12:
		printf("passwd getpwlog error\n");
		break;
    case 13:
		printf("passwd getpwuid error\n");
		break;
    case 14:
		printf("cannot alloc string area\n");
		fflush(obuf);
		exit(0);
		break;
    case 15:
		printf("uid too large\n");
		break;
    case 16:
		printf("editor execl failed\n");
		fflush(obuf);
		exit(0);
		break;
    case 17:
		printf("not enough data for interactive action requested\n");
		break;
    case 18:
		printf("cannot fork mkdir\n");
		break;
    case 19:
		printf("cannot exec mkdir for %s\n", rp);
		fflush(obuf);
		exit(0);
    case 20:
		printf("mkdir failed for %s\n", rp);
		break;
    case 21:
		printf("chown failed for %s\n", rp);
		break;
    default:
		printf("spurious error call\n");
		fflush(obuf);
		exit(0);
		break;
	}

	fflush(obuf);
	obuf = rsbuf;
}


putchar(ac)
char ac;
{
	putc(ac, obuf);
}


catsig()
{
	register int i;

	for(i = 14; i <= 16; i++)
		signal(i, 1);

	sigstop++;
}


mkit()
{
	register int cpid;


	if(cpid = fork())
	{
		/* parent */ 
		if(cpid == -1)
		{
			error(18, 0);
		}
		else
		{
			wait(&status);
			if(status&0177400)
				error(20, pe.pw_strings[DIRPATH]);
			else
			{
				if(chown(pe.pw_strings[DIRPATH], pe.pw_uid) < 0)
					error(21, pe.pw_strings[DIRPATH]);
			}
		}
	}
	else
	{
		/* child */ 
		execl("/bin/mkdir", "mkdir", pe.pw_strings[DIRPATH], 0);
		error(19, 1);
	}
}


