/*
 *   SMTPD.SUN.C
 *
 *	Special version for braindamaged SUN version of INETD.
 *
 *	Server process that is designed to be called by SUN/INETD,
 *	the stupid Internet Daemon of Daemons.  It calls SMTPSRVR
 *	with the proper arguments.
 *
 *	Expects arguments in form:
 *
 *	(1) smtpd sourcehost.sourceport
 *		for SUN
 *
 *	(2) smtpd - smtpserver channel
 *		for others or smart suns (better to use smtpd.4.3.c)
 *
 *		R E V I S I O N   H I S T O R Y
 *   
 */


#include "util.h"
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

extern int errno;
extern char *cmddfldir;
extern char *rindex(), *sprintf();
extern void bomb();	/* advance declaration -- routine in this file */

char localhost[256];
char remotehost[256];
char smtpserv[256] = "";
char *channel =  "smtp";	/* not configurable on SUN */

main(argc,argv)
int argc;
char **argv;
{
    register int valid = 0;
    int opt;

    if (argc >= 2)
    {

	/* now which kind of inetd called us? */

	if (*argv[1] == '-') 
	    valid = setup1(argc-2,argv+2);
	else 
	    valid = setup2(argc-1,argv+1);
    }

    if (!valid)
    {
	/* bomb calls exit() */
	bomb("Usage: %s [- smtpsrv channels] [rmthost.port]", argv[0]);
    }

    if (smtpserv[0] == '\0')
    {
	/* only if not user configured... */
	mmdf_init();
	(void) sprintf(smtpserv,"%s/smtpsrvr",chndfldir);
    }

    opt = 1;
    (void) setsockopt (0, SOL_SOCKET, SO_KEEPALIVE, (char *)&opt, sizeof(opt));

    execl(smtpserv,"smtpsrvr",remotehost,localhost,channel,(char *)0);

    /* bomb calls exit() */
    bomb("server exec error (%d)",errno);
}

setup1(count,args)
int count;
char **args;
{
    register int i;
    register struct hostent *he;
    struct sockaddr_in rmt;
    int size;

    for(i=0; i < count; i++)
    {
	switch(i)
	{
	    case 0:
		(void) strcpy(smtpserv,args);
		break;

	    case 1:
		channel = args[i];
		break;

	    default:
		break;
	}
    }

    /* now learn remote address */
    size = sizeof(rmt);
    if (getpeername(0,&rmt,&size) != 0)
	return(0);

    he = gethostbyaddr((char *)&rmt.sin_addr,sizeof(rmt.sin_addr),rmt.sin_family);

    if ((he == NULL) || !isstr(he->h_name))
    {
	char *cp = (char *)&rmt.sin_addr.s_addr;

	(void)sprintf(remotehost,"%u.%u.%u.%u",
		(int)cp[0],(int)cp[1],(int)cp[2],(int)cp[3]);
    }
    else
	(void)strcpy(remotehost,he->h_name);

     /* now local host name */
     size = sizeof(localhost);
     if (gethostname(localhost,&size) != 0)
	(void)strcpy(localhost,"LocalHost");


    return(1);
}

setup2(count,args)
int count;
char **args;
{
    register char *cp;
    register struct hostent *he;
    int size;
    struct in_addr in;

    if (count != 1)
	return(0);

    /* strip off trailing port value */
    if ((cp = rindex(args[0],'.')) == 0)
	return(0);

    *cp = 0;

    /* now get symbolic name if we can */

    (void) sscanf(args[0],"%x",&in.s_addr);
    he = gethostbyaddr((char *)&in,4,AF_INET);

    if ((he != 0) && isstr(he->h_name))
	(void)strcpy(remotehost,he->h_name);
    else
	(void)strcpy(remotehost,args[0]);

    printf("remotehost = %s\n",remotehost);

     size = sizeof(localhost);
     if (gethostname(localhost,&size) != 0)
	(void)strcpy(localhost,"LocalHost");

    printf("localhost = %s\n",localhost);

    return(1);
}

/**************************************************************************/
/* this works for both SUN and VAX -- more creativity may be required w/  */
/* other types of machines.                                               */
/**************************************************************************/

/* VARARGS1 */
void bomb(fmt,args)
char *fmt;
int args;
{
    char buf[BUFSIZ];

    setbuf(stdout,buf);

    printf("451 ");

    _doprnt(fmt,&args,stdout);
    printf("\r\n");

    (void) fflush(stdout);

    exit(99);
}
