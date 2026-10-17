#
/*
 *	convert old wtmp format to AUSAM
 *
 *	Piers Lauder	Mar '78
 */

#include	<local-system>

#ifdef	AUSAM

#include	<wtmp.h>

char	etcpasswd[]	"/etc/passwd";	/* old passwd file */
#define	PASSWDZ		4000		/* size of old passwd file (should use stat) */

#define	USERTIME	60*HZ		/* fudge user times */
#define	SYSTIME		30*HZ		/* fudge system times */

struct	oldfmt
{
	union {	char type; char name[2]; } o0;
	char	o_fill0[2];
	long	o_odate;
	union { struct { char otty; char fill1; long time; } o2;
		struct { long ndate; char fill2[2]; } od;
	      } o1;
	char	o_fill1[2];
}
	old[512/16];

struct	wtmp	new[512/WTMPSIZ];
#define	NENT	((sizeof new)/(sizeof new[0]))

#define	UERROR	0177777

int	nowarn;




main( argc, argv )
{
	register struct oldfmt	*op;
	register struct wtmp	*ep;
	register		n;
		unsigned	enteruid(), fetchuid();

  if ( argc > 1 )
	nowarn++;

  ep = new;

  while ( (n = read( 0, old, (sizeof old) )) > 0 )
  {
	for ( op = old , n =/ WTMPSIZ ; n-- ; op++ )
	{
		switch ( op->o0.type )
		{

		 case '\004':						/* date change */
			ep->d_type = D_TYPE;
			ep->d_newtime = op->o1.od.ndate;
			ep->d_oldtime = op->o_odate;
			break;

		 default:
			if ( (op->o0.type > ' ') && (op->o1.o2.time) )	/* log on */
			{
				ep->u_type = U_TYPE;
				ep->u_ttyid = op->o1.o2.otty;
				fixname( op->o0.name );
				if ( (ep->u_u_id = enteruid( op )) == UERROR )
				{
					warn( "Unrecognised user" , op );
				}
				ep->u_logintime = op->o1.o2.time;
				copyname( op->o0.name, ep->u_u_name );
			}
			else
			{
				warn( "Bad entry" , op );
				continue;
			}
			break;

		 case '\0':
			if ( op->o1.o2.time )				/* log off */
			{
				if ( op->o1.o2.otty == '~' )		/* system boot */
				{
					ep->s_type = S_TYPE;
					ep->s_boottime = op->o1.o2.time;
				}
				else
				{
					ep->w_type = O_TYPE;
					ep->w_ttyid = op->o1.o2.otty;
					if ( (ep->w_uid = fetchuid( op )) == UERROR )
					{
						warn( "Unrecognised logoff", op );
					}
					ep->w_finishtime = op->o1.o2.time;
					ep->w_usertime = USERTIME;
					ep->w_systime = SYSTIME;
				}
			}
			else
			{
				warn( "Bad entry" , op );
				continue;
			}
			break;
		}

		if ( ++ep >= &new[NENT] )
		{
			write( 1, new, (sizeof new) );
			ep = new;
		}
	}
  }

  if ( n = ep - new )
	write( 1, new, n*WTMPSIZ );

  return( 0 );
}




fixname( cp )
  register char *cp;
{
	register i = 8;
	register f = 0;

  do
  {
	if ( *cp == ' ' || !*cp || f )
	{
		f++;
		*cp = '\0';
	}
	cp++;
  }
  while
	( --i );
}




copyname( cp1, cp2 )
  register char *cp1, *cp2;
{
	register i = 8;

  do  *cp2++ = *cp1++;  while ( --i );
}




struct	nm_entry
{
	unsigned	nm_uid;
	char		nm_name[8];
	char		nm_flags;
}
	ents[NUSERS];

struct	nm_entry	*ttys[0177];

int	nusers;
int	passwd, passwdz;

char	pwbuf[PASSWDZ+2]	"\n\n";		/* first char assumed by "matchname" */




unsigned enteruid( op )
  register struct oldfmt *op;
{
	register struct nm_entry	*ep, *fp;
		struct nm_entry		**tp;
		unsigned		uid, matchname();

  if ( *(tp = &ttys[op->o1.o2.otty&0177]) )
  {
/*	struct wtmp fix;

	fix.w_type = O_TYPE;		/* log off previous user */
/*	fix.w_ttyid = op->o1.o2.otty;
	fix.w_uid = (*tp)->nm_uid;
	fix.w_finishtime = op->o1.o2.time;
	fix.w_usertime = USERTIME;
	fix.w_systime = SYSTIME;
	write( 1, &fix, (sizeof fix) );
 */
	warn( "Multi-logon", op );
  }

  for ( fp = 0 , ep = ents ; ep <= &ents[nusers] ; ep++ )
	if ( eqname( op->o0.name , ep->nm_name ) )
		goto found;
	else
		if ( !fp && !ep->nm_flags )
			fp = ep;

  if ( !(ep = fp) )
  {
	prints( 2, "Too many users!\n" );
	exit( -1 );
  }
  if ( !passwd )
  {
	if ( (passwd = open( etcpasswd, 0 )) < 0 )
	{
		prints( 2, etcpasswd );
		prints( 2, ": cannot open.\n" );
		exit( -1 );
	}
	if ( (passwdz = read( passwd, &pwbuf[2], (sizeof pwbuf)-2 )) == ((sizeof pwbuf)-2) )
	{
		prints( 2, etcpasswd );
		prints( 2, ": too large!\n" );
		exit( -1 );
	}
  }

  if ( (uid = matchname( op->o0.name )) != UERROR )
  {
	nusers++;
	copyname( op->o0.name, ep->nm_name );
	ep->nm_uid = uid;
	ep->nm_flags++;
found:
	*tp = ep;
	return( ep->nm_uid );
  }

  return( UERROR );
}




unsigned fetchuid( op )
  struct oldfmt *op;
{
	register struct nm_entry *ep;
	register char tty = op->o1.o2.otty & 0177;

  if ( ep = ttys[tty] )
  {
	ttys[tty] = 0;
	return( ep->nm_uid );
  }

  return( UERROR );
}




int eqname( cp1, cp2 )
  register char *cp1, *cp2;
{
	register i = 8;

  do
	if ( *cp1++ != *cp2++ )
		return( 0 );
  while
	( --i );

  return( 1 );
}




unsigned matchname( np )
  char *np;
{

  static char mname[10] "\n";	int dvec[(sizeof mname)+1], mnsiz;

  {
	/*
	 *	initialise match string
	 */

	register i;	register char *cp1, *cp2;

    for ( cp1 = np , cp2 = &mname[1] , i = 0 ;
		(*cp1 != '\0') && (++i <= 8) ;
			*cp2++ = *cp1++ );
    *cp2 = ':';
    mnsiz = i+2;
  }

  {
	/*
	 *	initialise match vector
	 */

	register i = 0;	register d_hb = 0;

    while ( d_hb != mnsiz )
    {
	register j = 0;	i++;

	while ( (j < (mnsiz - i)) && (mname[j] == mname[i+j]) )
		j++;
	while ( d_hb < (j+i) )
		dvec[++d_hb] = i;
    }
  }

  {
	/*
	 *	match the string
	 */

	register r = 0;	register k = 0;	register N = mnsiz;
	int M_N = passwdz - N;

    while ( r <= M_N )
    {
	while ( (k != N) && (mname[k] == pwbuf[r+k]) )
		k++;

	if ( k == N )
	{
		/*
		 *	extract uid
		 */

		struct { char *cp; };

		N.cp = &pwbuf[r+k];
		while ( *N.cp++ != ':' );	/* skip password */
		return( atoi( N.cp ) );
	}
	else if ( k )
	{
		r =+ dvec[k];
		k =- dvec[k];
	}
	else
		r++;
    }

    return( UERROR );
  }
}




warn( s, op )
  char *s;
  register struct oldfmt *op;
{
	char n[9];

  if ( nowarn )
	return;

  prints( 2, s );
  prints( 2, " -" );

  if ( op->o0.type > ' ' )
  {
	prints( 2, " name: '" );
	copyname( op->o0.name, n );
	n[8] = '\0';
	prints( 2, n );
ptty:
	prints( 2, "' tty: '" );
	op->o1.o2.fill1 = '\0';
	prints( 2, &op->o1.o2.otty );
  }
  else
  {
	prints( 2, " type: '" );
	op->o0.name[1] = '\0';
	op->o0.type =+ '0';
	prints( 2, &op->o0.type );
	if ( op->o0.type == '0' )
		goto ptty;
  }

  prints( 2, "' on: " );
  prints( 2, ctime( op->o1.o2.time ) );
}

#endif
