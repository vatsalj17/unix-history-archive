#
/*
 *	Program turns on (or off) the bits in the class mask
 *	which correspond to the "class" mentioned in the first arg
 *	for all the people in following args.
 *
 *		[un]class class user[s]
 */

#include	<local-system>
#include	<passwd.h>
#include	<class.h>

/*	global vars	*/
int	unflg;

struct pwent	pe;

main(argc, argv)
int	argc;
char	**argv;
{
register int	rn;
unsigned *cp;

if(argc < 3)
	{
	printf("[un]class classname lname ...\n");
	exit(0);
	}

if( **(argv++) == 'u')
	unflg++;
argc--;

if(( cp= match(*argv)) == 0)
	{
	printf("no such class\n");
	exit(0);
	}

argv++;
argc--;

for( ; argc--; argv++)
	{
	/* now for all the people, set the bits........ */
	pe.pw_strings[LNAME] = *argv;
	if( getpwuid(&pe, 0, 0) < 0)
		{
		printf("%s	no such user\n",*argv);
		continue;
		}

	for(rn=0; rn<CMASKSIZE; rn++)
		if(unflg)
			{
			/* clear the bits */
			pe.pw_cmask[rn] =& ~(cp[rn]);
			}
		  else
			{
			/* set the bits */
			pe.pw_cmask[rn] =| cp[rn];
			}

	if(updtpwent(&pe) != 1)
		{
		printf("%s	update failed\n", *argv);
		continue;
		}
	}
}

/*****************************************************************************/

unsigned	cm[CMASKSIZE];

match(c)
char	*c;
{
/*
 * pattern match the classes, and return composite mask
 */

register char	*rc, *rcm;
register int	rn;
int	miss;
int	hit;

/* have we a null class to look up ? */
if(*c == '\0') return(0);

/* clear the global mask */
for(rn=0; rn<CMASKSIZE; cm[rn++] = 0);

hit=0;

for(rn=0; rn<CMASKSIZE*16; rn++)
	{
	/*
	 *	scan down classes
	 *
	 *	ignore the null ones
	 */

	if(classes[rn].c_name[0] == '\0')
		miss = 1;
	  else
		miss = 0;

	for(rc=c, rcm=classes[rn].c_name;
		(miss == 0) && (*rc != '\0');
			rc++, rcm++)
		{
		if(*rc != *rcm) miss++;
		}

	if(miss == 0)
		{
		cm[classes[rn].c_word] =| classes[rn].c_mask;
		hit++;
		}
	}

if(hit) return(&cm);
return(0);
}
