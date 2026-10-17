#
/*
 *	passwd [name [[old password] [new password] [check]]]
 *
 *	Piers Lauder	Mar '78
 */ 
/*
 *	you would have to be mad to change your password as an arg to
 *	this program.
 *
 *	Modified to allow verification of the password.
 *
 *	Bryan Palmer	Sept '78
 *
 */ 

#include	<local-system>

#ifdef	AUSAM

#include	<passwd.h>
#include	<gtty.h>

struct pwent pe;
struct sgttyb tty;
int	vsuppress;	/* suppress verification of the password */
char *args[4];

main(argc, argv)
register argc;
char **argv;
{
	register char **argp = args;
	char sbuf[4][100];
	extern finish();
	extern cmp();	/* string comparison */ 

	signal(2, finish);
	signal(3, finish);

	if(argc > 1 && argv[1][0] == '-' && argv[1][1] == 'v')
	{
		vsuppress ++;
		argc --;	argv ++;
	}

	{
		register i = argc;

		do
			*argp++ = *argv++;
		while(--i);
	}

	{
		register unsigned uid = getreal();
		if(argc < 2)
		{
			pe.pw_uid = uid;
			if(getpwlog(&pe, sbuf[0], (sizeof sbuf[0])) < 0)
			{
				prints(2, "Cannot access password!\n");
				return(-1);
			}
			*argp++ = sbuf[0];
		}
		else
		{
			pe.pw_strings[LNAME] = args[1];
			if((getpwuid(&pe, 0, 0) < 0) || (uid && (uid != pe.pw_uid)))
			{
				prints(2, "Who?\n");
				return(-1);
			}
		}

		/*  echo off  */
		gtty(0, &tty);
		tty.mode =& ~ECHO;
		stty(0, &tty);

		if(argc < 3)
		{
			if(uid == pe.pw_uid && pe.pw_pword[0])
				*argp = getstring("Old password: ", sbuf[1]);
			argp++;
			if((args[2]) && pwcmp(crypt(args[2]), pe.pw_pword))
			{
				prints(2, "Sorry.\n");
				finish();
			}
		}

		if(argc < 4)
		{
			*argp = getstring("New password: ", sbuf[2]);
			if(**argp == '\0')
				*argp = 0;
			argp++;
		}
		if( !vsuppress)
		{
			getstring("Check: ", sbuf[3]);
			if(cmp(sbuf[2], sbuf[3]))
			{
				prints(2, "Try Again.\n");
				finish();
			}
		}
	}

	{
		register char *cp;


		if(cp = *--argp)
			cp = crypt(cp);
		else
			cp = "\0\0\0\0\0\0\0";
		for(argc = 0; argc < 8; )
			pe.pw_pword[argc++] = *cp++;
		if(updtpwent(&pe) <= 0)
			prints(2, "Unable to change password!\n");
	}

	finish();
}




pwcmp(cp1, cp2)
register char *cp1, *cp2; {
	register i = 8;
	do
		if(*cp1++ != *cp2++)
			return(-1);
	while(--i);

	return(0);
}







char *getstring(s, buf)
char *s;
register char *buf;
{

	prints(1, s);

	{
		register char *cp = buf;


		do
			if(read(0, cp, 1) <= 0)
				exit(0);
		while(*cp++ != '\n');

		*--cp = '\0';
	}


	return(buf);
}




cmp(s1, s2)
register char *s1, *s2;
{
	while(*s1++ == *s2++ && s1[-1]);
	return(*--s1-*--s2);
}


finish()
{
	gtty(0, &tty);
	tty.mode =| ECHO;
	stty(0, &tty);
	exit(-1);
}

#endif
