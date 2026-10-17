#include "remind.h"

trysend(rcvr,tty) /* Returns 0 if can't send */
char *rcvr, *tty;
{       register FILE *screen;
	char ttnm[20];
	char scr_buf[BUFSIZ];

	if(m.msgbytes == 0)
		return(1);
	strcpy(ttnm,ttname);
	strcat(ttnm,tty);

	if ((screen = fopen(ttnm,"w")) == NULL)
		return(0);

	setbuf(screen,scr_buf);
	if ((m.bits&PRIORITY)==0)
	{       if (fstat(fileno(screen),&ttstat) == -1)
		{
			fclose(screen);
			return(1);	/* Treat no stat as delivered */
		}
		if ((ttstat.st_mode&02) == 0)
		{
			fclose(screen);
			return(0);	/* No write permission */
		}
	}

	fprintf(screen,"%c",7);
	send(screen,rcvr);  /* Closes screen */
	fprintf(screen,"%c",7);
	fclose(screen);
	return(1);
}

sendmail(rcvr)
char *rcvr;
{       register int f, tries;
	char ch, rec[9];
	int fd[2], fdin[2];
	FILE *pip;

	for (f = 0; (rec[f] = rcvr[f]) && ++f < 8; ) ;
	rec[f] = '\0';  /* Terminator for 8 char receiver name */
	tries = 5;
	do
	{       pipe(fd);
		pipe(fdin);
		if (spawn())
		{       close (fd[0]);
			close (fdin[1]);
			if((pip=fdopen(fd[1],"w"))==NULL)
			    error("bad pipe\n");
			send(pip,rcvr);
			fclose(pip);
			f = read(fdin[0], &ch, 1);
			close (fdin[0]);
		}
		else
		{       close (fd[1]);
			close (fdin[0]);
			if (fd[0])
			{       close(0);
				dup(fd[0]);
				close(fd[0]);
			}
			if (fdin[1] != 1)
			{       close(1);
				dup(fdin[1]);
				close(fdin[1]);
			}
			if(Uid != -1)
			    setuid(Uid);
			execl("/bin/mail","remindmail",rec,0);
			exit(1);
		}
	} while (f && tries--);
	return;
}

send(fp,rcvr)
FILE *fp;
char *rcvr;
{       register char *p;
	register int i, t;
	struct sgttyb tt;
	int fd = fileno(fp);
	char *t_name;
	char *ttyname();

	if ((t_name=ttyname(fd)) != NULL)
	{
		ioctl(fd,TIOCGETP,&tt);
		if(((t=tt.sg_flags) & CRMOD) == 0)
		{
			tt.sg_flags = t | CRMOD;
			ioctl(fd,TIOCSETP,&tt);
		}
	}
	fprintf (fp,"\n*** REMINDER - ");
	fprintf (fp," From %.8s to %.8s\n", m.sender, rcvr);
	if (m.nrcvrs > 1)
	{       fprintf (fp,"   List: ");
		for (i = 0; i < m.nrcvrs; )
		{       if (i%4 == 0 && i)
				fprintf (fp,"\n         ");
			fprintf (fp,"%-9.8s ", rcvrlist[i++]);
		}
		fputc('\n',fp);
	}
	fprintf (fp,"  Sent ........... %.24s\n", ctime(&m.tsent));
	fprintf (fp,"  For delivery ... %.24s\n", ctime(&m.tdeliver));
	if (m.dirsize)
		fprintf(fp,"   The following was started as requested...\n\n");
	p = msg;
	for (i = m.msgbytes; i--; )
		fputc(*p++,fp);
	fprintf (fp,"\n*** END\n");
	fflush(fp);
	if (t_name && t != tt.sg_flags)
	{
		tt.sg_flags = t;
		ioctl(fd,TIOCSETP,&tt);
	}
	return(1);
}
