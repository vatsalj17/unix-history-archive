#
/*
 *	This prints info on those people whose class is the 1st arg
 *
 *		printc class
 */ 

#include	<local-system>
#include	<passwd.h>
#include	<class.h>

/*	global vars	*/ 

struct pwent pe;

main(argc, argv)
int argc;
char **argv;
{
	register int rn, rm;
	register int rsum;
	char *area;

	extern fout;


	fout = dup(1);

	if(argc-- != 2)
	{
		printf("printc classname\n");
		flush();
		exit(0);
	}
	argv++;

	if((match(*argv)) == 0)
	{
		printf("no such class\n");
		flush();
		exit(0);
	}

	for(rm = 0; rm < PWTABSIZE; rm++)
	{
		/* now for all the people, check the bits........ */ 
		pe.pw_uid = rm;

		if((getpwlog(&pe, 0, 0) < 0) || !maskcomp())
		{
			continue;
		}

		area = 0;
		rsum = 0;
		for(rn = 0; rn < PWSLENCNT; rn++)
		{
			rsum =+ pe.pw_strings[rn];
		}

		if((area = alloc(rsum)) == -1)
		{
			printf("no more core\n");
			flush();
			exit(1);
		}

		if(getpwlog(&pe, area, rsum) <= 0)
		{
			printf("password file corrupted\n");
			flush();
			exit(1);
		}
		else
		{
			output();
		}

		free(area);
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

/*****************************************************************************/ 

maskcomp()
{
	register int rn;


	for(rn = 0; rn < CMASKSIZE; rn++)
		if(pe.pw_cmask[rn]&cm[rn])
			return(1);

	return(0);
}

/*****************************************************************************/ 

output()
{
	register char *rc;
	/*	output the buggers	*/ 



	for(rc = pe.pw_strings[OTHER]; *rc != '\0'; rc++)
		if(*rc == '\n')
			*rc = ',';
	printf("%s%25t%s%60t%s\n", pe.pw_strings[LASTNAME], pe.pw_strings[FIRSTNAME], pe.pw_strings[LNAME]);
	flush();
}
