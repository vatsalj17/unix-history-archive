#define DO_SPITBOL
#ifdef DO_SPITBOL
/*	DO_SPITBOL		August 1978.
 *
 *	Spitbol Programs:
 *		 the shell will try to execute a file as a spitbol
 *		 program (by calling /usr/bin/dospitbol) if the first
 *		 two characters in the file are 'E.' (a valid spitbol
 *		 label). The arguments are placed in a file called
 *		 progname.a
 *
 *				- Michael Rourke.
 */ 
#endif DO_SPITBOL
/*
 *	execc - call texec with appropriate bin directories
 *		prepended to "name".
 *
 *	Modified for ".path" and compatibility with pwbshell
 *						Greg Rose Feb 79.
 */ 

#include	<local-system>

#include	<passwd.h>


#define NPATHS	10
char *paths[NPATHS];
char *shell;


char *execat(buf, ebuf, s2, s3)
char *ebuf, *buf;
register char *s2, *s3;
{
	register char *s1 = buf;


	while(*s1++ = *s2++);
	if(s1 != buf+1)	/* dont put in a slash if it is null path! */ 
		s1[-1] = '/';
	else
		s1--;
	while(*s1++ = *s3++)
		if(s1 >= ebuf)
		{
			prints(2, "execc: buffer overflow\n");
			return("?");
		}

	return(buf);
}

execcinit()
{
	register unsigned uid;
	static int notdoneyet 1;
	static char pathbuf[SSIZ]	":/bin:/usr/bin\n/bin/sh";
	char pbuf[SSIZ];
	char buf[50];
	struct pwent user;
	register char *cp, **pp;
	int n, pathfd;

	if(notdoneyet)
	{
		uid = getefvt();
		if(uid == 0)
		{
			register char *p1, *p2;

			p1 = "/etc:/bin:/usr/bin:\n/bin/sh";
			p2 = pathbuf;
			while(*p2++ = *p1++);
		}
		user.pw_uid = uid;
		if(!getpwlog(&user, pbuf, sizeof pbuf))
			prints(2, "bad password file\n");
		pwclose();
		execat(buf, &buf[sizeof buf], user.pw_strings[DIRPATH], ".path");
		if((pathfd = open(buf, 0)) != -1)
		{
			n = read(pathfd, pathbuf, sizeof pathbuf);
			if(n == -1 || n >= sizeof pathbuf)
			{
				prints(2, "execc: bad .path file\n");
				return(-1);
			}
			pathbuf[n] = '\0';
			close(pathfd);
		}
		/*
		 * parse the contents of pathbuf into the appropriate fields.
		 */ 
		pp = paths;
		*pp++ = cp = pathbuf;
		for( ;(*cp) && (*cp != '\n'); cp ++)
		{
			if(*cp == ':')
			{
				*cp = '\0';
				*pp++ = cp+1;
				if(pp >= &paths[NPATHS])
				{
					prints(2, "execc: buffer overflow\n");
					return(-1);
				}
			}
		}
		*pp = 0;
		if(*cp == '\n')
		{
			*cp++ = '\0';
			shell = cp;
			while(*cp)
			{
				if(*cp == '\n')
					*cp = '\0';
				cp++;
			}
		}
		if(*shell == '\0')
			shell = "/bin/sh";
		notdoneyet = 0;
	}
}


execc(name, argv)
register char *name;
register char *argv[];
{
	char buf[50];
	register char **pp;


	if(!anyslashes(name))
	{
		execcinit();
		for(pp = paths; *pp != 0; pp++)
			texec(execat(buf, &buf[sizeof buf], *pp, name), argv);
	}
	else
		texec(name, argv);
	return(-1);
}


/*
 *	"texec" knows about command files and pascal interpreter
 */ 

#include	<errnos.h>

char PX[]	"/usr/bin/px";
char SPITBOL[]	"/usr/bin/dospitbol";

texec(name, argv)
register char *name;
char *argv[];
{
	extern errno;


	execv(name, argv);
	if(errno == ENOEXEC)
	{
		char *v[256];
		register char **vp = &v[1];

		{
			register char **avp = argv;


			*vp++ = name;
			if(!*avp++)
			{
				*vp++ = name;
				*vp = 0;
			}
			else
				while(*vp++ = *avp++)
					if(vp > &v[sizeof v/sizeof v[0]])
					{
						prints(2, "too many args\n");
						exit(1);
					}
			vp = v;
		}
		{
			register pcfd;
			/*
			 *	if file exists but not executable
			 *	it may be a pascal obj file
			 *	magic number = 0404
			 */ 


			if((pcfd = open(name, 0)) > 0)
			{
				int pcrd, pcmag;

				pcrd = read(pcfd, &pcmag, 2);
				close(pcfd);
				if(pcrd == 2)
				{
					if(pcmag == 0404)
					{
						*vp = PX + 9;
						execv(PX + 4, vp);
						execv(PX    , vp);
						prints(2, "No px!\n");
						exit(1);
					}
#ifdef DO_SPITBOL
					/*
					 *	file may be a spitbol program, try and execute it
					 *	(by calling "/usr/bin/dospitbol") if the magic
					 *	number is 'E.'
					 */ 
					else if(pcmag == 'E.')
					{
						*vp = SPITBOL + 9;;
						execv(SPITBOL + 4, vp);
						execv(SPITBOL    , vp);
						prints(2, "No dospitbol!\n");
						exit(1);
					}
#endif DO_SPITBOL
				}
			}
		}
		/*
		 *	not pascal obj must be shell cmd file
		 */ 
		*vp = "sh";
		execv(shell, vp);
		prints(2, "No shell!\n");
		exit(1);
	}
	if(errno == ENOMEM)
	{
		prints(2, name);
		prints(2, ": too large\n");
		exit(1);
	}
	if(errno == E2BIG)
	{
		prints(2, name);
		prints(2, ": arglist too long\n");
		exit(1);
	}
}


anyslashes(string)
{
	register char *p;

	p = string;
	while(*p)
		if(*p++ == '/')
			return(1);
	return(0);
}
