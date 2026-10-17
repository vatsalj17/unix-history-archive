#include	<stdio.h>
#include	<local-system>
#include	<defines.h>
#include	<passwd.h>
#include	<gtty.h>

char buff[256];
struct pwent pe;
char pwstrngs[256];

main()
{
	register char c, *cp;
	register noway;

	pe.pw_uid = getreal();
	if ((noway = getpwlog(&pe, pwstrngs, sizeof pwstrngs)) < 0 || noway >= sizeof pwstrngs)
	{
		puts("Error - seek help!!");
		return;
	}
	setraw();
	puts("\nRecently your password was devined using a simple program\n\n");
	for (;;)
	{
		noway = 0;
		waffle();
		fputs("Type in your new password: ", stdout);
		if (gets(buff) == 0)
		{
			puts("Error on input");
			return;
		}
		if (check(buff))
		{
			puts("Try again.\n");
			continue;
		}
		fputs("Now type it in again, just to be sure: ", stdout);
		cp = buff;
		while ((c = getc(stdin)) != '\n') if (*cp++ != c) noway++;
		if (noway || *cp)
		{
			 puts("That is not what you typed in before.\nTry again.\n");
			continue;
		}
		fixup(crypt(buff));
		return;
	}
}

setraw()
{
	struct sgttyb ttybuf;

	if (gtty(0, &ttybuf) < 0)
	{
		perror("gtty");
		exit(1);
	}
	ttybuf.mode =| RAW;
	ttybuf.mode =& ~ECHO;
	if (stty(0, &ttybuf) < 0)
	{
		perror("stty");
		exit(1);
	}
	stdin->_flag =| _IONBF;		/* don't buffer input */
}

check(psswd)
char *psswd;
{
	register char *p;

	for (p = psswd; *p; p++)
		if (*p == '#' || *p == '@' || *p == '\b')
		{
			puts("\n '#', '@', and Back-Space are not allowed.");
			return 1;
		}
	if (p - psswd <= 8)
	{
		puts("\nMore than 8 characters, please.");
		return 1;
	}
	if (scmp(psswd, pe.pw_strings[FIRSTNAME]) == 0 ||
		scmp(psswd, pe.pw_strings[LASTNAME]) == 0 ||
		scmp(psswd, pe.pw_strings[LNAME]) == 0 ||
		comp(psswd, pe.pw_strings[FIRSTNAME], pe.pw_strings[LASTNAME]) ||
		comp(psswd, pe.pw_strings[LNAME], pe.pw_strings[FIRSTNAME]) ||
		comp(psswd, pe.pw_strings[LNAME], pe.pw_strings[LASTNAME]))
	{
		puts("\nI said not your names or login number.");
		return 1;
	}
	return 0;
}

comp(psswd, s1, s2)
char *psswd, *s1, *s2;
{
	char bigbuf[256];

	strcpy(bigbuf, s1);
	strcat(bigbuf, " ");
	strcat(bigbuf, s2);
	if (scmp(psswd, bigbuf) == 0) return 1;
	strcpy(bigbuf, s2);
	strcat(bigbuf, " ");
	strcat(bigbuf, s1);
	if (scmp(psswd, bigbuf) == 0) return 1;
	return 0;
}

fixup(psswd)
register char psswd[8];
{
	register fred;

	for (fred = 0; fred < 8; fred++) pe.pw_pword[fred] = psswd[fred];
	pe.pw_strings[SHELLPATH] = "";
	if (chngpwent(&pe) != 1)
	{
		puts("Error - seek help!");
		exit(1);
	}
}

waffle()
{
	puts("  You must now enter a NEW password.  This  is  necessary,  so");
	puts("  that  other  people cannot use your account, or corrupt your");
	puts("  files etc. In  a  moment  you  will  be  asked  to  enter  a");
	puts("  password.  You  should  make  this  something  that you will");
	puts("  remember, but not too easy or obvious.  For  example,  DON'T");
	puts("  use  your names or login number.  Your password should be at");
	puts("  least 8 characters long, and preferably two or  more  words.");
	puts("  If  you  wish to have a single word password then check that");
	puts("  it is not in the dictionary.  It should NOT contain  any  of");
	puts("  the  special  characters  '#',  '@', or Back-Space (BS), and");
	puts("  should be lower-case only.\n");
}

/* upper/lower case compare routine for strings */

scmp( s1 , s2 )
register char *s1, *s2;
{
	char c1, c2;

	while( *s1 && *s2 )
	{
		c1 = *s1++;
		if( 'A' <= c1 &&  c1 <='Z' )
			c1 =- 'A' - 'a';
		c2 = *s2++;
		if( 'A' <= c2 &&  c2 <='Z' )
			c2 =- 'A' - 'a';
		if( c1 != c2 )
			return 1;
	}
	return *s1 != *s2;
}
