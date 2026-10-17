#include	<local-system>

char	mesg[3000];
int	msize;

#include	<utmp.h>
struct	utmp	utmp[256];

main(argc, argv)
char *argv[];
{
	register i, *p;
	int f;

	f = open("/etc/utmp", 0);
	if(f < 0)
	{
		perror("utmp");
		return 1;
	}
	read(f, utmp, sizeof utmp);
	close(f);
	f = 0;
	if(argc >= 2)
	{
		f = open(argv[1], 0);
		if(f < 0)
		{
			perror(argv[1]);
			return 1;
		}
	}
	while((i = read(f, &mesg[msize], sizeof mesg - msize)) > 0)
		msize =+ i;
	if(f) close(f);
	if( i = fork())
	{
		if( i == -1)
		{
			perror();
		}
		else
			return 0;
	}
	for(i=0; i<sizeof utmp/sizeof utmp[0]; i++)
	{
		p = &utmp[i];
#ifndef	AUSAM
		if(p->u_name[0] == 0)
			continue;
#else
		if(p->u_type == 0)
			continue;
#endif	AUSAM
		sendmes(p->u_ttyid);
	}
	return 0;
}

sendmes(tty)

{
	register i;
	register char *s;

	s = "/dev/ttyx";
	s[8] = tty;
	i = open(s, 1);
	if(i < 0)
	{
		perror(s);
		return;
	}
	write(i, "\n\n\007\007\007AGSM Broadcast Message ...\n\n", 33);
	write(i, mesg, msize);
	close(i);
	return;
}
