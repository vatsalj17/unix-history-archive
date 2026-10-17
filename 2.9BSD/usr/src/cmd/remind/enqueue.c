
#include "remind.h"
char *notemp	= "Unable to create /rmd/rmdtemp. Remind is dead.";

enqueue()
{
	register int slptime = 0;
	register i;
	register FILE *pf;
	FILE *rem, *temp;
	int procid;
	time_t now;
	char endrem, written;
	struct rmdblock mm;
	char rmddata[8*MAXRCVRS + MAXMSG + DIRSIZE];
	struct passwd *pw;

#ifndef DEBUG
	signal(SIGINT,SIG_IGN);          /* Ignore Interrupts */
	signal(SIGQUIT,SIG_IGN);          /* Ignore Quits */
	signal(SIGTERM,SIG_IGN);          /* Ignore Termination */
#endif DEBUG

	/* Wait for file to be usable */
	lock();

	/* Kill any existing reminder process that may be "sleeping" */
	if ((pf = fopen(rmdproc,"r")) != NULL)
	{
		if(fscanf(pf,"%d",&procid)<= 0)
			error("/rmd/rmdproc unreadable");
		fclose(pf);
		kill (procid,9);
	}
	if((pf=fopen(rmdproc,"w"))==NULL)
	{
		fprintf(stderr,"remind: cannot open %s for writing\n",rmdproc);
		error("");
	}

	procid = getpid();
	fprintf(pf,"%d\n",procid);
	fclose(pf);

	/* Put new reminder into reminder file (temp) in delivery order */
	if ((rem=fopen(reminders,"r"))==NULL)
	{
		endrem = -1;
		mm.tdeliver = -1L;
	}
	else
		endrem = (fread((char *)&mm, sizeof mm, 1, rem) != 1);
	if ((slptime = tdiff(&m.tdeliver,&m.tsent))
		> (i = tdiff(&mm.tdeliver,&m.tsent)))
			slptime = i;
	if ((temp = fopen(rmdtemp,"w")) == NULL)
		error (notemp);
	else
		if(chmod(rmdtemp,0600) < 0)
		    error("Cannot chmod");

	for (written = 0;;)
	{       if (!endrem &&
			(written||(tdiff(&mm.tdeliver, &m.tdeliver) <= 0)))
		{
			fwrite ((char *)&mm, sizeof mm, 1, temp);
			fread (rmddata, mm.nobytes,1,rem);
			fwrite (rmddata, mm.nobytes, 1, temp);
			endrem = (fread((char *)&mm, sizeof mm, 1, rem) != 1);
		}
		else if (!written)
		{
			fwrite ((char *)&m, sizeof m, 1, temp);
			fwrite ((char *)rcvrlist, 8*m.nrcvrs, 1, temp);
			if (m.dirsize)
			    fwrite (exdir, m.dirsize&0377, 1, temp);
			fwrite (msg, m.msgbytes, 1, temp);
			written++;
		}
		else break;
	}
	fclose (temp);
	if(rem)
	    fclose (rem);
	/*
	 * Dummy all std files
	 */
	freopen(dummy,"r",stdin);
	freopen(dummy,"w",stdout);
	freopen(dummy,"w",stderr);
	cleanup();

	for (;;)
	{	if (slptime > 0)
			sleep(slptime);
		lock();
		now = time((time_t *)0);
		if ((rem = fopen(reminders,"r")) == NULL)
			bye();
		do
		{       if (fread((char *)&m, sizeof m, 1, rem) != 1)
				bye();
			if((pw=getpwnam(m.sender))==NULL)
			{
			    fprintf(stderr,"Unknown user: %s\n",m.sender);
			    Uid = -1;
			}
			else
			    Uid = pw->pw_uid;
			if ((slptime = tdiff(&m.tdeliver, &now)) <= 0)
			{
				fread ((char *)rcvrlist, 8*m.nrcvrs, 1, rem);
				if (m.dirsize)
				    fread(exdir, m.dirsize&0377, 1, rem);
				fread (msg, m.msgbytes, 1, rem);
				if (spawn() == 0)
				{	fclose(rem);
					deliver(); /* Child delivers */
				}
			}
		} while (slptime <= 0);

		/* Copy, removing delivered messages */
		if ((temp = fopen(rmdtemp,"w")) == NULL)
			bye();
		else
			chmod(rmdtemp,0600);
		do
		{
			fread (rmddata, m.nobytes, 1, rem);
			fwrite ((char *)&m, sizeof m, 1, temp);
			fwrite (rmddata, m.nobytes, 1, temp);
		} while (fread((char *)&m, sizeof m, 1, rem) == 1);
		fclose (rem);
		fclose (temp);
		cleanup();
	}
}
