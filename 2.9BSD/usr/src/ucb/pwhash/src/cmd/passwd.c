static	char	sccsid[] = "@(#)passwd.c	2.7";	/*	SCCS id keywords	*/
#include <whoami.h>

#define	SMLROOT	1	/* Small root device, forces two copies	*/
/*	#define V6CRYPT	1	/* Accept version 6 passwd as well	*/

/*
 * enter a password in the password file
 * this program should be suid with owner
 * with an owner with write permission on /etc/passwd
 */
#include <stdio.h>
#include <signal.h>
#include <pwd.h>

#define	V7PWDSIZE	13
static	char	passwd[] = "/etc/passwd";
#ifndef SMLROOT
char	temp[]	 = "/etc/ptmp";
#else
char	temp[]	 = "/tmp/pwdXXXXXX";
#endif

struct	passwd *pwd;
struct	passwd *getpwent();
int	endpwent();
char	*strcpy();
char	*crypt();
#ifdef V6CRYPT
char	*v6crypt();
#endif
char	*getpass();
char	*getlogin();
char	*pw;
char	pwbuf[10];
char	buf[512];
int	sizechanged = 0;

main(argc, argv)
char *argv[];
{
	char *p;
	int i;
	char saltc[2];
	long salt;
	int u,g,fi,fo;
	int insist;
	int ok, flags;
	int c;
	int pwlen;
	FILE *tf;
	char *uname;

	insist = 0;
	if(argc < 2) {
		if ((uname = getlogin()) == NULL) {
			printf ("Usage: passwd user\n");
			goto bex;
		} else {
			printf("Changing password for %s\n", uname);
			fflush(stdout);
		}
	} else {
		uname = argv[1];
	}
	while(((pwd=getpwent()) != NULL)&&(strcmp(pwd->pw_name,uname)!=0));
	u = getuid();
	g = getgid();
#ifdef UCB_GRPMAST
	if((pwd==NULL) || (u!=0 && u != pwd->pw_uid &&
		(u!=g || g!=pwd->pw_gid)))
#else
	if((pwd==NULL) || (u!=0 && u != pwd->pw_uid))
#endif
		{
		printf("Permission denied.\n");
		goto bex;
		}
	endpwent();
	if (pwd->pw_passwd[0] && u == pwd->pw_uid) {
		strcpy(pwbuf, getpass("Old password:"));
		pw = crypt(pwbuf, pwd->pw_passwd);
		if(strcmp(pw, pwd->pw_passwd) != 0) {
#ifdef V6CRYPT
			pw = v6crypt(pwbuf);
			if (strcmp(pw, pwd->pw_passwd) != 0) {
#endif
				printf("Sorry.\n");
				goto bex;
			}
#ifdef V6CRYPT
		}
#endif
	}
	sizechanged = (strlen(pwd->pw_passwd) != V7PWDSIZE);
tryagn:
	strcpy(pwbuf, getpass("New password:"));
	pwlen = strlen(pwbuf);
	if (pwlen == 0) {
		printf("Password unchanged.\n");
		goto bex;
	}
	ok = 0;
	flags = 0;
	p = pwbuf;
	while(c = *p++){
		if(c>='a' && c<='z') flags |= 2;
		else if(c>='A' && c<='Z') flags |= 4;
		else if(c>='0' && c<='9') flags |= 1;
		else flags |= 8;
	}
	if(flags >=7 && pwlen>= 4) ok = 1;
	if(((flags==2)||(flags==4)) && pwlen>=6) ok = 1;
	if(((flags==3)||(flags==5)||(flags==6))&&pwlen>=5) ok = 1;

	if((ok==0) && (insist<2)){
		if(flags==1)
		printf("Please use at least one non-numeric character.\n");
		else
		printf("Please use a longer password.\n");
		fflush(stdout);
		insist++;
		goto tryagn;
		}

	if (strcmp(pwbuf,getpass("Retype new password:")) != 0) {
		printf ("Mismatch - password unchanged.\n");
		goto bex;
	}

	time(&salt);
	salt += getpid();

	saltc[0] = salt & 077;
	saltc[1] = (salt>>6) & 077;
	for(i=0;i<2;i++){
		c = saltc[i] + '.';
		if(c>'9') c += 7;
		if(c>'Z') c += 6;
		saltc[i] = c;
	}
	pw = crypt(pwbuf, saltc);
	signal(SIGHUP, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);

	if (pwlock("r")) {
		printf("Temporary file busy -- try again\n");
		goto bex;
	}
#ifdef SMLROOT
	mktemp(temp);
#endif
	close(creat(temp,0600));
	if((tf=fopen(temp,"w")) == NULL) {
		printf("Cannot create temporary file\n");
		goto bex;
	}

	if((fi=open(temp,0)) < 0) {
		printf("Temp file disappeared!\n");
		goto out;
	}

/*
 *	copy passwd to temp, replacing matching lines
 *	with new password.
 */

	while((pwd=getpwent()) != NULL) {
		if(strcmp(pwd->pw_name,uname) == 0) {
			u = getuid();
			g = getgid();
#ifdef UCB_GRPMAST
		if(u!=0 && u != pwd->pw_uid &&
			(u!=g || g!=pwd->pw_gid))
#else
		if(u!=0 && u != pwd->pw_uid)
#endif
			{
				printf("Permission denied.\n");
				goto out;
			}
			pwd->pw_passwd = pw;
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

	if((fo=creat(passwd, 0644)) < 0) {
		printf("Cannot recreat passwd file.\n");
		goto out;
	}
	while((u=read(fi,buf,sizeof(buf))) > 0) write(fo,buf,u);

out:
	if (sizechanged)
#ifdef UCB_PWHASH
		pwtable();
#else
		;
#endif
	unlink(temp);
	pwunlock();
	exit(0);
bex:
	
	exit(1);
}
