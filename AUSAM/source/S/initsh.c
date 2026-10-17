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
		sleep(5);
		return;
	}
	puts("\n    Welcome to the Department of Computer Science's PDP11/70 Computer.");
	puts("    We trust your stay will be a pleasant one.\n");
	for (;;)
	{
		setraw();
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
			sleep(5);
			continue;
		}
		fputs("Now type it in again, just to be sure: ", stdout);
		cp = buff;
		while ((c = getc(stdin)) != '\n') if (*cp++ != c) noway++;
		if (noway || *cp)
		{
			 puts("That is not what you typed in before.\nTry again.\n");
			sleep(5);
			continue;
		}
		fixup(crypt(buff));
		puts("\n    Now you will have to log in again.  This time, as in  the  future,");
		puts("    you  will be asked to supply your password.  If you wish to change");
		puts("    your  password  in  the  future,  type  `passwd'  and  follow  the");
		puts("    instructions (you should do this regularly).\n\n");
		sleep(15);
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
			puts("\nLike I said, '#', '@', and Back-Space are not allowed.");
			return 1;
		}
	if (p - psswd <= 8)
	{
		puts("\nMore than 8 characters, please.");
		return 1;
	}
	if (strcmp(psswd, pe.pw_strings[FIRSTNAME]) == 0 ||
		strcmp(psswd, pe.pw_strings[LASTNAME]) == 0 ||
		strcmp(psswd, pe.pw_strings[LNAME]) == 0 ||
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
	if (strcmp(psswd, bigbuf) == 0) return 1;
	strcpy(bigbuf, s2);
	strcat(bigbuf, " ");
	strcat(bigbuf, s1);
	if (strcmp(psswd, bigbuf) == 0) return 1;
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
		sleep(5);
		exit(1);
	}
}

waffle()
{
	puts("    Your first chore (now that you have got this far) will be to  give");
	puts("    your  new  account  a  password.  This is necessary, so that other");
	puts("    people cannot use your account, or corrupt  your  files,  or  copy");
	puts("    your  assignments,  etc.  In a moment you will be asked to enter a");
	puts("    password.  You should make this something that you will  remember,");
	puts("    but not too easy or obvious.  For example, DON'T use your names or");
	puts("    login number.  There are  other  less  scrupulous  people  on  the");
	puts("    system  who  derive  pleasure from cracking the passwords of other");
	puts("    accounts (such people can expect no  mercy  when  caught!).   Your");
	puts("    password  should be at least 8 characters long, and preferably two");
	puts("    or  more  words.   It  should  NOT  contain  any  of  the  special");
	puts("    characters  '#', '@', or Back-Space (BS), and should be lower-case");
	puts("    only.  It is ended by a Carriage-Return.  Whenever you  are  asked");
	puts("    to  type  your password, it will not be echoed, i.e., you will not");
	puts("    see it.  This is so that others do not see it either.\n");
}
