#
/*
**  	who   [-d] [-s] [-f] [-l] [-o] [ wtmp ]|[ am i ]
** 
**
**	oct  77		piers lauder
**	       		new wtmp/utmp formats
**	feb  78		piers lauder
**			ausam
**	apr  79		davec
**			C flag
**	june 79		michael coorey
**			rewritten, output reformatted
**
**
**
**	lists names and details of unix users
**	flags	d	initial directory
**		s	initial shell
**		f	firstname
**		l	lastname
**		C	class
**		o	other information
**
**	usually accesses files 	/etc/utmp
**			    or	/usr/adm/wtmp
**
**
**
**
**		output format:
**
**	type	name	uid		ttyid	time		newtime
**					newuid
**	1-8	9-24	25-29	30-32	33-40	41-52	53-56	57-68
**	( who on default file is left-shifted 8 chars (does not include type or uid) )
*/





#include	<local-system>		/* system paramters */
#include	<utmp.h>		/* utmp/wtmp formats */
#include	<passwd.h>		/* user details on passwd file */
#include	<class.h>		/* user class info */
#include	<stat16.h>		/* format for "fstat" */





struct	utmp	buf[512/UTMPSIZ];	/* buffer for utmp/wtmp records */
int		fin,fout;		/* buffered input/output file descriptors */
char		utmpf[]		"/etc/utmp";





main(argc,argv)

int	argc;
char	*argv[];

{
	static struct flags			/* flags */
	{
		char	d,s,f,l,o,C,locking;
		int	yes;
	}
	flg;
	register struct flags	*flag	= &flg;
	struct pwent		pe;		/* password entry for user */
	int			pwentry;	/* true if there exists a pwent for current user */
	register struct pwent	*pwe;
	char			pbuf[SSIZ];	/* strings from password entry */
	register struct utmp	*p;		/* pointer to utmp/wtmp record */
	char			tty;		/* users tty */
	int			status;		/* exit status */
	int			tmpsiz;		/* size of input buffer */
	char			*ifile;		/* input file name */
	char			c;





	/* set flag arguments */
	flag->yes = 0;
	while ( argc > 1 && argv[1][0] == '-' )
	{
		while ( c = *(++argv[1]) ) 
		{
			switch( c )
			{
			case 'd':	
				flag->d++;	
				break;
			case 's':	
				flag->s++;	
				break;
			case 'f':	
				flag->f++;	
				break;
			case 'l':	
				flag->l++;	
				break;
			case 'o':	
				flag->o++;	
				break;
			case 'C':	
				flag->C++;	
				break;
			default:	
				printf("bad flag  %c\n",c);
				continue;
			}
			flag->yes++;
		}
		argc--;
		argv++;
	}



	/* set and open input file */
	if(argc == 2)
		ifile = argv[1];
	else	ifile = utmpf;
	fin = open(ifile, 0);
	if( fin < 0 )
	{
		perror(ifile);
		exit( 1 );
	}

	{
		struct statbuf	sbuf;

		fstat( fin, &sbuf );
		if ( sbuf.sb_flags & IFLOCK )
		{
			flag->locking++;
		}
	}

	/* set output file descriptor to 3 so output buffered */
	fout = dup(1);
	close(1);

	/* set user tty */
	if (argc==3)
		tty = ttyn(0);





	/* step thru utmp/wtmp file */
	while( (tmpsiz=read(fin,buf,sizeof buf)) > 0 )
	{
		if ( flag->locking )
			if ( unlock() == -1 )
			{
				prints( 2, "unlock - " );
				perror( ifile );
				status = 1;
				goto finish;
			}

		/* step thru buffer */
		for( p=buf; (tmpsiz =- UTMPSIZ)>=0; p++ )
		{
			/* - who am i - irelevant record */
			if( (argc==3)   &&   (tty!=p->u_ttyid) )
				continue;
			/*  -  who  -  who /etc/utmp  -  irrelevant record */
			if( (ifile==utmpf) && (p->u_type != U_TYPE) )
				continue;
			switch( p->u_type )
			{
			case U_TYPE:	/* user record */
			case O_TYPE:	/* log off record */

				/* print type except for default listing */
				if( !(argc==1) )
					if( p->u_type == U_TYPE )
						putchar( '\t' );
					else	printf( "logoff\t" );

				if ( p->u_type == U_TYPE )
					/* print name */
					if( !flag->yes && p->u_u_name[0] )
						/* no options and name in utmp */
					{
						char	name[9],*s;
						int	j;
						/* ensure name is \0 terminated */
						s=name;
						for( j=0; j<8; j++ )
							*s++ = p->u_u_name[j];
						*s = '\0';
						pname( name );
					}
					else
						/* get name from passwd file */
					{
						pwe = &pe;
						if(pwe->pw_uid != p->u_u_id)
						{
							pwe->pw_uid = p->u_u_id;
							pwentry = (getpwlog(pwe,pbuf,SSIZ) >0);
						}
						if(pwentry)
							pname(pwe->pw_strings[LNAME]);
						else	pname("");
					}
				else
				{
					printf( "\t\t" );
					if(flag->yes && pwe->pw_uid != p->u_u_id)
					{
						pwe->pw_uid = p->u_u_id;
						pwentry = (getpwlog(pwe,pbuf,SSIZ)>0);
					}
				}

				if ( argc != 1 )
					/* print uid */
					printf( "%-5d\t", p->u_u_id );

				/* print tty time */
				printf( "tty%c\t%12.12s",
				p->u_ttyid,ctime(p->u_logintime)+4 );

				/* options */
				if( flag->yes && pwentry )
				{
					if( flag->f | flag->l | flag->d | flag->s | flag->C )
					{
						putchar( '\t' );
						if( flag->f )
							printf( "%s ",pwe->pw_strings[FIRSTNAME] );
						if( flag->l )
							printf( "%s ",pwe->pw_strings[LASTNAME] );
						if( flag->d )
							printf( "%s ",pwe->pw_strings[DIRPATH] );
						if( flag->s )
							if( *(pwe->pw_strings[SHELLPATH]) == '\0' )
								printf( "/bin/sh " );
							else   printf( "%s ",pwe->pw_strings[SHELLPATH] );
						if( flag->C )
							/* print class name */
						{
							int	word,bit;
							for( word=0; word!=CMASKSIZE; word++ )
								for( bit=15; bit>=0; --bit )
									if( pwe->pw_cmask[word] & (1<<bit) )
										printf( "%s ",classes[word*16+15-bit].c_name );
						}
						putchar( '\n' );
					}
					if( flag->o )
						printf( "\t%s\n\n",pwe->pw_strings[OTHER] );
				}
				else
					putchar( '\n' );


				/*  - who am i -   done */
				if( argc == 3 )
					goto finish;
				break;



			case W_TYPE:	/* async process */
				printf( "async\t\t\t%-5d\t\t%12.12s\n",
				p->w_uid,ctime(p->w_finishtime)+4 );
				break;
			case SU_TYPE: /* super user */
				printf( "SUPER\t\t\t%-5d > %-5d\t%12.12s\n",
				p->su_olduid,p->su_newuid,ctime(p->su_usetime)+4 );
				break;
			case D_TYPE:	/* date change */
				printf( "DATE\t\t\t\t\t%12.12s",
				ctime(p->d_oldtime)+4 );
				printf( " to %12.12s\n",
				ctime(p->d_newtime)+4 );
				break;
			case S_TYPE:	/* system boot */
				printf( "BOOT\t\t\t\t\t%12.12s\n",
				ctime(p->s_boottime)+4 );
				break;
			default:	
				write(2,"bad wtmp\n",9);
				status = 1;
				goto finish;
			}
		}

		if ( flag->locking )
		{
			pwclose();	/* passwd file maybe */
			if ( readlock( fin ) == -1 )
			{
				prints( 2, "lock - " );
				perror( ifile );
				status = 1;
				goto finish;
			}
		}
	}


	/* check for error */
	if( tmpsiz < 0 )
	{
		perror(ifile);
		status=1;
		goto finish;
	}



	/*  - who am i -  failed */
	if( argc==3 )
	{
		printf( "nobody\t\tbad utmp\n" );
		status = 1;
	}



finish:	/* exit with status */
	flush();
	exit(status);
}





/* print user name */
pname(s)
char *s;
{
	register int i;
	register char *p;

	p=s;
	if( *p == '\0' )
	{
		*p++ = '?';
		*p-- = '\0';
	}
	for(i=0; *p++ != '\0'; i++);

	printf("%-.15s",s);

	if( i<8 ) putchar('\t');
	putchar('\t');
}
