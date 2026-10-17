#include	<local-system>
#include	<param.h>
#include	<passwd.h>
#include	<stat16.h>

struct { int hiword; unsigned loword; };

main(c,v)
char *v[];
{
	struct pwent pe;
	register i;
	char buf[512];
	extern fout;
	long tdu(),l;
	unsigned used;
	int highbd,sflg;
	register char *b = buf;

	fout = dup(1);
	switch(c)
	{
    case 1:	i = 1;
		highbd = PWTABSIZE;
		break;
    case 2:
		i = atoi(v[1]);
		if( i < 0 ) goto error;
		highbd = i+1;
		if( highbd > PWTABSIZE )
			highbd = PWTABSIZE;
		sflg++;
		break;
    case 3:	i = atoi(v[1]);
		highbd = atoi(v[2]);
		if ( i < 0 ) i = 0;
		if(highbd < i ) goto error;
		if(highbd > PWTABSIZE)
			highbd = PWTABSIZE;
		break;
    default:
	error:	prints(2,"Usage: pwscan [uid] | [uidl uidh]\n");
		return(-1);
	}
	for( ; i < highbd; i++)
	{
		pe.pw_uid = i;
		if( getpwlog( &pe, b, 512) <= 0 )
		{
			if(sflg)
			{
				prints(2, "Uid not found\n");
				return(1);
			}
			continue;
		}
		if(pe.pw_dlimit == 0)
			continue;
		l = tdu(pe.pw_strings[DIRPATH]);
		used = l.hiword;
		if( used == -1 )
		{
			printf("  error");
			used = 0;
		}
		else if( used < pe.pw_dlimit )
		{
			printf("       ");
		}
		else if(  used > (pe.pw_dlimit + pe.pw_doverflw) )
		{
			printf("** Hard");
		}
		else
		{
			printf("   Soft");
		}
		printf(": Uid: %5u, TDU: %5u, Soft: %5u, Hard: %5u, Name: '%s'\n",
			i, used, pe.pw_dlimit,
			pe.pw_dlimit + pe.pw_doverflw,
			pe.pw_strings[LNAME]);
	}
	flush();
	return(0);
}
