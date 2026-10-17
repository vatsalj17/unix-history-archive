/*
 *	A program to print the contents of a users password entry
 *	and his limits structure from core and his TDU figure
 *
 *	Peter  Ivanov 1978.
 *	and again July 1979 for alternate passwd file.
 */ 

#include	<local-system>
#include	<passwd.h>
#include	<lnode.h>

/*
 *	some defines to configure certain things
 *	THAT IS select that which is to be printed !!
 */ 

/* no scheduling info
#define	SCHEDULE	/* for sched params */ 
/*	no CORE
#define	CORE		/* for core limits */ 
/*	no CORE_DUSAGE
#define	CORE_DUSAGE	/* if disk usage maintained in core */ 
/*	no CLASSNAMES
#define	CLASSNAMES	/* if the cmask bits are really meaningful */

#define	PAGE		/* for page limits */

struct
{
	int hiword;
	unsigned loword;
};
struct	pwent	pe;
struct	lnode	ln;
char		tbuf[8];
char		*buf;
int		bufs;
int		uflag;
char		*pflag;
int		lflag;

char		*numbers[4]
{
	"No", "One", "Two", "Three"
};
/*
 **	MAIN
 */ 

main(argc, argv)
register int argc;
register char **argv;
{
	register char *s;
	extern int fout;

	fout = dup(1);
	buf = -1;
	while((argc >= 2) && (*argv[1] == '-'))
	{
		s = *++argv;
		argc--;
		while(*++s)
		{
			switch(*s)
			{
			case 'u':
				uflag++;
				break;
			case 'p':
				pflag = ++s;
				while(*s)
					s++;
				--s;
				pwfile(pflag);
				break;
			default:
				printf("Usage: pp [-pfile] [lnames|-u uids]\n");
				return 1;
			}
		}
	}
	if(argc > 1)
	{
		argc--;
		argv++;
		/* some specific requests */ 
		while(argc-- > 0)
		{
			printu(*argv++);
		}
	}
	else
	{
		if(getlogin(getreal()))
			pent();
	}
}

/*
 **	Print user details
 */ 

pent()
{
	extern long time();

	printf("Login name: %11s%35tUid: %18u\n\n", pe.pw_strings[LNAME], pe.pw_uid);
#ifdef	CORE
	if(!lflag)
	{
		printf("Core Limit:%11u KBytes%35tCore Usage:%11u KBytes\n", ln.l_climit, ln.l_cusage);
	}
#endif
	printf("Disk Limit:%12u%35tProcess Limit:%9u\n", ln.l_dlimit, ln.l_plimit);
	printf("Disk Overshoot:%8u%35tProcesses:%13u\n", ln.l_doverflw, ln.l_refcount);
	printf("Disk Usage:");
	if(ln.l_duse == -1)
		printf("%12s", "???");
	else
		printf("%12u", ln.l_duse);
#ifdef	PAGE
	printf("%35tPrinter Units:%9u\n", pe.pw_pages);
#else
	printf("\n");
#endif
#ifdef TERMBOOK
	printf("Booking Units:%9u%35tBooking Rate:%10u\n", pe.pw_tblim&0377, pe.pw_tbrate&0377);
#endif TERMBOOK
#ifdef	SCHEDULE
	printf("Shares:%16u%35tUsage:%16D\n", ln.l_shares, ln.l_usage);
#endif
	printf("\nTime Connected: %s", ptime(pe.pw_contime, tbuf));
	printf("%35tEncrypted Password: \"%8.8s\"\n", pe.pw_pword);
	printf("Total CPU Time: %s", ptime(pe.pw_cputime/60, tbuf));
	printf("%35t%s Warning%s\n", pe.pw_warn <= 3 ? numbers[pe.pw_warn] : "Many", pe.pw_warn == 1 ? "" : "s");
	if(pe.pw_extime)
	{
		register char *p;

		printf("Last used: ");
		p = ctime(pe.pw_extime);
		if(pe.pw_extime < time() - 1555200l)
			printf("%12.12s %4.4s", p+4, p+20);
		else
			printf("%16.16s", p);
	}
	else
		printf("Account has not been used.");
	printtrm(pe.pw_tmask);
#ifdef	CLASSNAMES
	printclasses();
#endif	CLASSNAMES
	printf("\nDirectory: %s\n", pe.pw_strings[DIRPATH]);
	if(*pe.pw_strings[SHELLPATH] != 0)
	{
		printf("Initial shell: %s\n", pe.pw_strings[SHELLPATH]);
	}
	printf("Names:\t%s %s\n", pe.pw_strings[FIRSTNAME], pe.pw_strings[LASTNAME]);
	if(*(pe.pw_strings[OTHER]))
	{
		printf("Other:");
		printother(pe.pw_strings[OTHER]);
	}

	/*	check out to warn him to login again	*/ 

	if((!lflag) && ((ln.l_dlimit != pe.pw_dlimit) || (ln.l_plimit != pe.pw_plimit) || ((ln.l_flags&DLIMIT) && (ln.
		l_duse < ln.l_dlimit))))
	{
		printf("Logout and Login again to reset limits\n");
	}
	flush();
}

/*
 **	Get the relevant entries from passwords and lnodes
 */ 

printu(p)
register unsigned p;
{
	if(uflag)
	{
		/* have asked to treat arg as uid */ 
		if(getlogin(atoi(p)))
			pent();
		if(buf != -1)
		{
			free(buf);
			buf = -1;
		}
	}
	else
	{
		/* have an arg to try as login name */ 
		if(getuserid(p))
			pent();
		if(buf != -1)
		{
			free(buf);
			buf = -1;
		}
	}
}

printtrm(u)
register unsigned u;
{
	register i;

	if(u == 0)
		printf("%35tAccount may not use any terminal\n");
	else if(u == -1)
		printf("%35tAccount may use any terminal\n");
	else
	{
		printf("%35tValid terminal groups: ");
		for(i = 0; i < 16; i++)
		{
			if(u&01)
				putchar('a'+i);
			u =>> 1;
		}
		putchar('\n');
	}
}

/*
 **	Get login name
 */ 

getlogin(uid)
register unsigned uid;
{
	lflag = 0;
	pe.pw_uid = uid;
	if(getpwlog(&pe, 0, 0) == 0)
	{
		put();
		return(1);
	}
	else
	{
		printf("Uid does not exist !?!\n");
		return(0);
	}
}

/*
 **	Set up user limits and strings
 */ 

put()
{
	ln.l_uid = pe.pw_uid;
	buf = alloc(bufs = sum());
	getpwlog(&pe, buf, bufs);
	if(lflag = (limits(&ln, L_OTHLIM) < 0))
	{
		ln.l_shares = pe.pw_shares;
		ln.l_usage = pe.pw_shares;
		ln.l_dlimit = pe.pw_dlimit;
		ln.l_doverflw = pe.pw_doverflw;
		ln.l_plimit = pe.pw_plimit;
		ln.l_climit = pe.pw_climit;
		ln.l_refcount = 0;
	}
#ifndef	CORE_DUSAGE
	if(pe.pw_dlimit)
		ln.l_duse = tdu(pe.pw_strings[DIRPATH]);
#endif	CORE_DUSAGE
}
/*
 **	Get user user-id
 */ 

getuserid(s)
register char *s;
{
	lflag = 0;
	pe.pw_strings[LNAME] = s;
	if(getpwuid(&pe, 0, 0) == 0)
	{
		put();
		return(1);
	}
	else
	{
		printf("Login name does not exist !?!\n");
		return(0);
	}
}

/*
 **	Add up string lengths for password entry
 */ 

sum()
{
	register int i;
	register int s;

	s = 0;
	for(i = 0; i < PWSLENCNT; i++)
	{
		s =+ pe.pw_strings[i];
	}
	return(s);
}

#ifdef	CLASSNAMES
#include	<class.h>
printclasses()
{
	register i, j, k;

	printf("Classes:");
	j = k = 0;
	for(i = 0; i < CMASKSIZE*16; i++)
	{
		if(pe.pw_cmask[classes[i].c_word]&classes[i].c_mask)
		{
			if(j == 0 && k != 0)
				putchar('\t');
			k++;
			printf("%10s", classes[i].c_name);
			if(++j == 7)
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
#endif	CLASSNAMES

printother(s)
register char *s;
{
	register char *r;

	do
	{
		r = s;
		while(*s && *s != '\n')
			s++;
		if(*s != 0)
			*s++ = 0;
		printf("\t%s\n", r);
	}
	while(*s);
}
