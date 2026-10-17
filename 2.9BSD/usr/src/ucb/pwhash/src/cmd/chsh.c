	char *sccsid[] = "@(#)chsh.c	2.4";
/*
 * chsh
 */
#include <whoami.h>
#include <stdio.h>
#include <signal.h>
#include <pwd.h>

#define	NSHELLS	2	/* Number of shells in valid list */
#define SMLROOT		/* Not enough space in /etc for duplicate of passwd */

char	pwfile[] = "/etc/passwd";
#ifndef SMLROOT
char	temp[] = "/etc/ptmp";
#else
char	temp[] = "/tmp/pwdXXXXXX";
#endif

char	*shells[NSHELLS] = {		/* First in list is default shell */
		"/bin/csh",
		"/bin/sh"
	};
char	*defsh;
struct	passwd *pwd;
struct	passwd *getpwent();
int	endpwent();
char	*crypt();
char	*getpass();
char	buf[BUFSIZ];

main(argc, argv)
char *argv[];
{
	int u,fi,fo;
	int i;
	FILE *tf;
#ifdef UCB_GRPMAST
	int g;
#endif

	defsh = shells[0];
	if(argc < 2 || argc > 3) {
		printf("Usage: chsh user [ shell ]\n");
		goto bex;
	}
	if (argc == 2)
		argv[2] = defsh;
	u = getuid();
#ifdef UCB_GRPMAST
	g = getgid();
#endif
	if (u) {
		for (i=0; i<NSHELLS; i++)
			if (!strcmp(argv[2], shells[i]))
				goto doit;
		printf("Invalid shell.  Valid shells are:\n");
		for(i=0; i<NSHELLS; i++)
			printf("\t%s\n", shells[i]);
		exit(1);
	}

    doit:
	if (!strcmp(argv[2], defsh))
		argv[2] = "";
	while((pwd=getpwent()) != NULL){
		if(strcmp(pwd->pw_name,argv[1]) == 0){
			if(u!=0 && u != pwd->pw_uid
#ifdef UCB_GRPMAST
			    && !(u==g && g==pwd->pw_gid)
#endif
			    ){
				printf("Permission denied.\n");
				goto bex;
			}
			if (u) {
				if (*pwd->pw_shell == '\0')
					pwd->pw_shell = defsh;
				for (i=0; i<NSHELLS; i++)
					if (!strcmp(pwd->pw_shell, shells[i]))
						goto ok;
				printf("Can't change shell for %s\n", pwd->pw_name);
			}
		    ok:
			break;
			}
		}
	endpwent();
	signal(SIGHUP, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);

	if (pwlock("w")) {
		fprintf(stderr,"Password file busy -- try again");
		goto bex;
	}
#ifdef SMLROOT
	mktemp(temp);
#endif
	if((tf=fopen(temp,"w")) == NULL) {
		printf("Cannot create temporary file\n");
		goto bex;
	}

/*
 *	copy passwd to temp, replacing matching lines
 *	with new shell.
 */

	while((pwd=getpwent()) != NULL) {
		if(strcmp(pwd->pw_name,argv[1]) == 0) {
			u = getuid();
			if(u!=0 && u != pwd->pw_uid
#ifdef UCB_GRPMAST
			    && !(u==g && g==pwd->pw_gid)
#endif
			    ){
				printf("Permission denied.\n");
				goto out;
			}
			pwd->pw_shell = argv[2];
		}
		fprintf(tf,"%s:%s:%u:%u:%s:%s:%s\n",
			pwd->pw_name,
			pwd->pw_passwd,
			pwd->pw_uid,
			pwd->pw_gid,
			pwd->pw_gecos,
			pwd->pw_dir,
			pwd->pw_shell);
	}
	endpwent();
	fclose(tf);

/*
 *	copy temp back to passwd file
 */

	if((fi=open(temp,0)) < 0) {
		printf("Temp file disappeared!\n");
		goto out;
	}
	if((fo=creat(pwfile, 0644)) < 0) {
		printf("Cannot recreat passwd file.\n");
		printf("Not removing temp file.\n");
		goto bex;
	}
	while((u=read(fi,buf,sizeof(buf))) > 0) write(fo,buf,u);

out:
	unlink(temp);
	pwunlock();

bex:
	exit(1);
}
