#include <local-system>
#include <passwd.h>
#include <ttygroup.h>

/*
 *	Validate or unvalidate a particular user to or from
 *	a certain tty group. Usage is
 *		val lname [+][-]ttygroups
 *
 *	where	lname is the users login name
 *		the optional + or - mean add or remove
 *			permissions for the following ttygroups
 *			default is add
 *		ttygroups are letters (abc etc) corresp to
 *			thoses in ttygroups.h
 *
 */ 
char add 1;

main(c, v)
int c;
char **v;
{
	struct pwent pe;
	char *term;

	if(c == 3)
	{
		v++;
		pe.pw_strings[LNAME] = *v;
		if(getpwuid(&pe, 0, 0) != -1)
		{
			switch(*(term = *++v))
			{
			case '-':
				add = 0;
			case '+':
				term++;
			}
			while(*term)
				if(add)
					pe.pw_tmask =| tflag(*term++);
				else
					pe.pw_tmask =& ~tflag(*term++);
			if(updtpwent(&pe) < 0)
				printf("val: passwd update error\n");
		}
		else
			printf("val: %s no such lname\n", *v);
	}
	else
	{
		printf("usage: val lname [+][-]ttys\n");
		printf("       ttys - list of ttygroup letters (abc..)\n");
		printf("       +   add ttygroup\n       -   delete ttygroup\n");
	}
}
tflag(term)
char term;
{
	int i;

	for(i = 0; i < NTTYGRPS; i++)
		if(term == tty_group[i].tchar)
			return(tty_group[i].tmask);
	printf("val: %c no such ttygroup\n", term);
	return(0);
}
