#
#include	<local-system>

/*
 *	lpr -- spool printout for online printer.
 *		has been modified extensively from distrib
 *
 */


#ifdef	AUSAM
#include	<stat.16.h>
struct	statbuf	statbuf;
#include	<passwd.h>

#include	<printers.h>
int ptype;		/* index in printer struct*/
struct pwent pe[1];
#endif	AUSAM

#define privlusr 49		/* the elite can have prty print */

char	tfnmh[]		"/tmp/lpd/tfaXXXXXX";
char	cfnmh[]		"/tmp/lpd/cfaXXXXXX";
char	dfnmh[]		"/tmp/lpd/x/dfxXXXXXXXXX";
#define	xposn		dfnmh[9]
#define	cposn		cfnmh[10]
char	lockfile[]	"/tmp/lpd/lock\0";
#define	lockposn	lockfile[13]

 
int	nact;
int	tff;
int	quick;
int	iden;
int	pageno;
char	*arg;
#ifndef	AUSAM
char	statbuf[20];
int	buf[256];
#endif
char	remmsg[]	": can't remove\n";
unsigned ncopies	0;

struct {
	int	hi_int;
	int	lo_int;
};

struct {
	char	lo_byte;
	char	hi_byte;
}

main(argc, argv)
int argc;
char *argv[];
{
	int lockfd,lpdpid;
	register char *cb;
	int rflg=0,xban=0,push=0,yflg=0;
	char *xmsg=0;
	int register f;
	extern rubout();
#ifdef	AUSAM
	char namebuf[100];
	int buf[20];		/* BDP fix */
#endif	AUSAM

	if( ! (signal(2, 1) & 01) ) signal(2, rubout);
	mktemp(cfnmh);
	tff = creat(mktemp(tfnmh), 0600);
	if(tff < 0)
	{
		perror(tfnmh);
		exit(1);
	}
	while (argc > 1 && (arg = argv[1])[0] == '-')
	{
		switch (arg[1])
		{

	    case 'r':
			rflg++;
			break;

	    case 'e':
			if(arg[2] > '0' && arg[2] <= '9')
				quick =+ arg[2]-'0';
			else quick++;
			break;

	    case 's':
			quick--;
			break;
	    case 'i':
#ifdef	AUSAM
			cb = &buf[1];	/* BDP fix */
#else
			cb = &buf[100];
#endif	AUSAM
			arg++;
			while(*cb++ = *++arg);
#ifndef	AUSAM
			cb[-1] = ':';
#endif	AUSAM
			iden++;
			break;
	    case 'x':
			xban++;
			break;
	    case 'p':
			push++;
			break;
	    case 'm':
			xmsg = arg+1;
			break;
	    case 'n':
			ncopies = &arg[2];
			break;
	    case 'y':
			yflg++;
			break;		/* cause supression of page feeds
					   when printing */
	    case '1':
	    case '2':
	    case '3':
			lockposn = xposn = cposn = arg[1];
#ifdef	AUSAM
			ptype = arg[1] - '0';
#endif	AUSAM
			break;		/* select another printer */
		}
		argc--;
		argv++;
	}
#ifndef	AUSAM
	f = getuid();
#else
	f = getreal();
#endif	AUSAM
	buf[0] = f;	/* BDP fix */
	if( ( f == 0 ) && ( quick >= 0 ) ) quick=9;	/* ensure us super user types dont wait */
	if(f <= privlusr)
		quick++;
	else	if(quick > 0) quick = 0;
	if(quick < -1) quick = -1;
	if(quick >= 9) quick = 9-(f!=0);
#ifndef	AUSAM
	card('I',&buf->hi_byte);
#endif
#ifdef	AUSAM
	card('I', buf);		/* BDP fix */
	pe->pw_uid = f;
	if( getpwlog(pe, namebuf, sizeof namebuf) < 0)
	{
		pe->pw_strings[LNAME] = "PHANTOM";
		pe->pw_pages = 5;	/* pretty bloody small */
	}
	else
	{
		if(namebuf[0] >= '0' && namebuf[0] <= '9')
			pe->pw_strings[LNAME] = pe->pw_strings[LASTNAME];
	}
	if( xban == 0)
	{
		if(iden)
			card('L', &buf[1]);	/* BDP fix */
		else
			card('L', pe->pw_strings[LNAME]);
	}
#else
	if(xban==0) ident();
#endif	AUSAM
	if(push) card('P',"p");
	if( ncopies ) card('N', ncopies);
	if( yflg) card('Y',"y");
	if(argc == 1)
		copy(0);
	while(--argc)
	{
		arg = *++argv;
		if( access(arg, 4) )
		{
			perror(arg);
			continue;
		}
#ifdef	AUSAM
		if(*arg == '/' && pe->pw_uid == 0)
#else
		if(*arg == '/')
#endif	AUSAM
		{
			if( (f=open(arg,0)) <= 0 )
			{
				perror(arg);
				continue;
			}
			close(f);
			card('F', arg);
			if(rflg)
				if(remvp(arg) == 0)
					card('U',arg);
				else
				{
					prints(2, arg);
					prints(2, remmsg);
				}
			nact++;
			continue;
		}
		f = open(arg, 0);
		if(f < 0)
		{
			perror(arg);
			continue;
		}
#ifndef	AUSAM
		copy(f);
#else
		if( copy(f))
			break;
#endif	AUSAM
		if(rflg)
			if( remvp(arg) == 0 )	/* check for remove permission */
				f = unlink(arg);
			else
			{
				prints(2, arg);
				prints(2, remmsg);
			}
	}

	if( xmsg ){
		*xmsg = ttyn(2);
		card('M', xmsg);
	}
	buf[0]=pageno+1;
	card('S',buf);
	if(nact)
	{
		dfnmh[13] =- quick;
		if(link(tfnmh,nextname(dfnmh)))
		{
			prints(2, "can't rename in LPD\n");
			exit();
		}
		if( ! (
			((lockfd=open(lockfile,0)) >= 0) &&
			(read(lockfd,&lpdpid,2)==2)	     &&
			(kill(lpdpid,4)!= -1)
		 )) prints(2, "LPD not accessible tell a guru\n");
	}
	unlink(tfnmh);
}

copy(f)
int f;
{
	int register i;	long nc;
	int ff, fn;

	ff = creat(fn=ranname(cfnmh),0666);
	if(ff < 0) {
		prints(2, "can't create in LPD spool\n");
		return;
	}
#ifdef	AUSAM
	if( speed(f, ff, pe->pw_pages, ptype))
	{
		prints(2, "Page limit exceeded, print cancelled\n");
		close(f);
		close(ff);
		unlink(fn);
		return 1;
	}
#else
	nc = 0;
	while((i = read(f, buf, 512)) > 0) {
		write(ff, buf, i);
		nc =+ i;
		if(nc.hi_int > TLIMIT) {
			prints(2, "copy file is too large\n");
			break;
		}
	}
#endif	AUSAM
	close(ff);
	close(f);
	card('F', fn);
	card('U', fn);
	nact++;
	return 0;
}

card(c, s)
int c;
char s[];
{
	write(tff,&c,1);
	write(tff,s,39);	/* lpd record size is 40 chars */
}

#ifndef	AUSAM
ident()
{
	register char *b1p, *b2p;

	b2p = &buf[100];
	if(!iden && getpw(getuid(), b2p)) b2p = "pdp:";
	b1p = b2p;
	while(*b2p++ != ':') ;
	b2p[-1] = 0;
	card('L', b1p);
}
#endif	AUSAM

ranname(s)
char s[];
{

loop:
#ifndef	AUSAM
	if(stat(s, statbuf))
#endif
#ifdef	AUSAM
	if ( newstat( s, &statbuf ) )
#endif
		return(s);
	s[11]++;
	goto loop;
}

remvp(file)
char	*file;
{
	int	wflag;
	register char *p,*q,*r;

	wflag = 0;
#ifdef	AUSAM
	newstat(file, &statbuf);
	if(statbuf.sb_flags & IFTYP)
		return -1;
#else
	stat(file, statbuf);
	if(statbuf[5] & 0140)
		return -1;
#endif	AUSAM
	p = file;
	r = p+1;
	if( *p == 0 )
		return -1;
	if( *p == '/' )
		q = "/";
	else
		q = "";
	while( *r)
	{
		if( *r++ == '/' )
		{
			while( *r == '/' )
				r++;
			if( *r )
				p = r;
			q = file;
		}
	}
	if( q == file )
	{
		*--p = 0;
		wflag = access( q, 2);
		*p = '/';
	}
	else
		wflag = access(q, 2);
	return wflag;
}

#ifdef	AUSAM
struct iobuf
{
	int fdes;
	int count;
	char *cptr;
	char buff[512];
}	ibuf, obuf;
#define NLINES 20 /* should be a dynamic allocation */
#define	LINESPERPAGE	60
char	*line[NLINES];
int	hiwater[NLINES];
int	lineno;


speed(inf,outf,pagelim,indx)
{
	register c, column, i;
	char	*p;
	char	*j;
	int	trns, plim, pwidth;

	plim = pagelim * printer[indx].pagemult;
	trns = printer[indx].translate;
	pwidth = printer[indx].pagewidth;
	ibuf.fdes = inf;
	ibuf.count = 0;
	obuf.fdes = outf;
	obuf.cptr = obuf.buff;
	i = NLINES;
	switch(getw(&ibuf))
	{
	case 0407:
	case 0410:
	case 0411:
	case 0412:
	case 0177555:
	case 0177545:
	case 01:
	case 0404:
	case 017437:
		if(inf)
			prints(2,arg);
		prints(2,": Illegal file type\n");
		unlink(cfnmh);
		nact--;
		return 0;
	}
	ibuf.cptr=- 2;
	ibuf.count=+ 2;
	do	line[i-1] = 0;	while(--i);
	column = 0;
	for(; ; )
	{
		switch(c = getc(&ibuf))
		{
		case -1:
			fflush(&obuf);
			return 0;
		case ' ':
			column++;
			break;
		case '\t':
			column = (column+8)&~7;
			break;
		case '\n':
			flushline('\n');
			column = 0;
			if(++lineno > LINESPERPAGE)
			{
				goto morepage;
			}
			break;
		case '\f':
			flushline('\f');
			column = 0;
		morepage:
			lineno = 0;
			if(plim >= 0 && ++pageno == plim)
				return -1;
			break;
		case '\r':
			column = 0;
			break;
		case '\b':
			if(--column < 0)
				column = 0;
			break;
		case '{':
			if(!trns) goto Normal;
			c = '(';
			goto Escape;
		case '}':
			if(!trns) goto Normal;
			c = ')';
			goto Escape;
		case '~':
			if(!trns) goto Normal;
			c = '^';
			goto Escape;
		case '|':
			if(!trns) goto Normal;
			c = '!';
			goto Escape;
		case '`':
			if(!trns) goto Normal;
			c = '\'';
			goto Escape;
		Escape:
			if(column < pwidth)
			{
				for(i = 0; i != NLINES; i++)
					if(p = line[i])
					{
						if(!p[column])
						{
							p[column] = '-';
							if(column >= hiwater[i])
								hiwater[i] = column + 1;
							goto	restofchar;
						}
					}
					else
					{
						p = spush(pwidth);
						line[i] = p;
						for(j = &p[pwidth]; j != p;)
							*--j = 0;
						p[column] = '-';
						hiwater[i] = column + 1;
						goto	restofchar;
					}
				if(i == NLINES)
				{
					flushline('\r');
					line[0][column] = '-';
					hiwater[0] = column + 1;
				}
			}
		Normal:
			if( (c < ' ') || (c == '\177') ) 	/* unprintable -- ignore */
				break;
		default:
			if(column < pwidth)
			{
			restofchar:
				for(i = 0; i != NLINES; i++)
					if(p = line[i])
					{
						if(!p[column])
						{
							p[column] = c;
							if(column >= hiwater[i])
								hiwater[i] = column + 1;
							break;
						}
					}
					else
					{
						p = spush(pwidth);
						line[i] = p;
						for(j = &p[pwidth]; j != p;)
							*--j = 0;
						p[column] = c;
						hiwater[i] = column + 1;
						break;
					}
				if(i == NLINES)
				{
					flushline('\r');
					line[0][column] = c;
					hiwater[0] = column + 1;
				}
			}
			column++;
			break;
		}
	}
}
flushline(c)
{
	register i, j, nblanks;
	int needcr;

	for(i = 0; hiwater[i] && i != NLINES; i++)
	{
		for(j = 0; j != hiwater[i]; j++)
		{
			if(line[i][j])
			{
				putc(line[i][j], &obuf);
				line[i][j] = 0;
			}
			else
				putc(' ', &obuf);
		}
		hiwater[i] = 0;
		if(hiwater[i + 1])
			putc('\r', &obuf);
	}
	putc(c, &obuf);
}
#endif	AUSAM

rubout()
{
	unlink(tfnmh);
	unlink(cfnmh);
	unlink(dfnmh);
	exit(1);
}


char tnchars[] ",.0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

nextname(s)
char	*s;
{
	extern long time();
	int	pid;
	long	systime;
	register char *p,*q;

	for(p = s; *p ; p ++);
	pid = getpid();
	for(q = p-3; p != q; )
	{
		*--p = tnchars[pid & 077];
		pid =>> 6;
	}
	systime = time();
	for(q = p-6; p != q; )
	{
		*--p = tnchars[systime & 077];
		systime =>> 6;
	}
	return s;
}
