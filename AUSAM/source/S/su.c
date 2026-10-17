#include	<param.h>
#include	<local-system>
#include	<passwd.h>
#include	<wtmp.h>
#include	<stdio.h>

/*
 * su -- set user id
 *
 *   su [username OR integeruid]
 *
 *    sets your uid to that of the username given or to the integer given.
 * if the name is not found in the password file and the arg is not an integer
 * or there is no arg given it sets your uid to 0.
 *
 *	Written By	GREG KABLE
 */
char	password[100];
#ifndef	AUSAM16
char	pwbuf[100];
#else
struct	pwent	pe,tpe;
char	wtmpf[]	"/usr/adm/wtmp";
#endif
int	ttybuf[3];

main(c,v)
int	c;
char	**v;
{
	register char *p, *q, *t;
	int	count	0,
		uid	0;
	extern	fin;
	extern  long time();

#ifndef	AUSAM16
	if(getpw(0, pwbuf))
		error("bad password file");
	(&fin)[1] = 0;
	p = pwbuf;
	while(*p != ':')
		if(*p++ == '\0')
			error("bad password file");
	if(*++p != ':' &&(getuid()&0377 || c<=1))
#else
	uid = getreal();
	pe.pw_uid = 0;
	if(getpwlog(&pe,0,0))
		error("Bad Password File");
	p = pe.pw_pword;
	if(c>1)
	{
		tpe.pw_strings[LNAME] = v[1];
		if(getpwuid(&tpe,0,0))
			pe.pw_uid = atoi(v[1]);
		else
			pe.pw_uid = tpe.pw_uid;
	}
	if(*p != 0 &&(uid || c<=1))
#endif
	{
		gtty(0, ttybuf);
		ttybuf[2] =& ~010;
		stty(0, ttybuf);
		printf("password: ");
		if((count = read(0,password,sizeof password - 1)) <= 1)
			password[0] = '\0';
		else
			password[count - 1] = '\0';
		ttybuf[2] =| 010;
		stty(0, ttybuf);
		putc('\n',stdout);
		q = crypt(password);
#ifndef	AUSAM16
		t = p;
		while(*++t != ':' && *t);
		*t = '\0';
		if(strcmp(p,q)) error("sorry");
#else
		while(*q++ == *p++);
		if(p < &pe.pw_pword[8]) error("sorry");
#endif
	}
#ifndef	AUSAM16
	if(c > 1) uid = find(*(++v));
	setuid(uid);
#else
	if ( uid >= SYSUSERS && pe.pw_uid < SYSUSERS )
	{
		struct sutmp safe;

		safe.su_type = SU_TYPE;
		safe.su_olduid = uid;
		safe.su_newuid = pe.pw_uid;
		safe.su_usetime = time();
		if((uid = open(wtmpf, 1)) != -1)
		{
			seek( uid, 0, 2 );
			write( uid, &safe, sizeof safe );
			close( uid );
		}
	}
	setuid( pe.pw_uid );
#endif
	execl("/bin/sh", "-", 0);
	error("cannot execute shell");
}

error(str)
{
	fputs(str,stderr);
	putc('\n',stderr);
	exit(1);
}
#ifndef	AUSAM16

find(user)
char	*user;
{
	FILE	*tfil;
	char	*tpnt,userbuf[30];

	if(!(tfil = fopen("/etc/passwd","r"))) error("bad password file");
	while(!feof(tfil))
	{
		tpnt = userbuf;
		while((*tpnt = getc(tfil)) != ':' && *tpnt++ >= 0);
		*tpnt = '\0';
		if(!strcmp(userbuf,user)) break;
		while((*tpnt = getc(tfil)) >= 0 && *tpnt != '\n');
	}
	if(feof(tfil)) return(atoi(user));
	else
	{
		while((*tpnt = getc(tfil)) >= 0 && *tpnt != ':');
		tpnt = userbuf;
		while((*tpnt = getc(tfil)) >= '0' && *tpnt <= '9') tpnt++;
		*tpnt = '\0';
		return(atoi(userbuf));
	}
	close(tfil);
}
#endif
