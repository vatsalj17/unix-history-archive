#
#include	<local-system>
/* the copy command
	copy [-a] [-ad] [-l] [-n] [-o] [-r] [-v] [-m] sources destination
	written by Keith Davis Feb 13, 1976

	Ian Johnstone added -m july 18, 1977

	Ian Johnstone added -s novemeber 30, 1977

*/




unsigned par_uid;     /* the parent process id for the mkdir forks */

	/* the device type */
struct
{
	int     dev;
	int     inumber;/* the inode number */
	int     flags;  /* the flags */
#ifndef	AUSAM
	char    nlinks; /* number of links to the file */
	char    uid;    /* the uid of the file */
	char    gid;    /* the group id of the file */
#else	AUSAM
	unsigned uid;	/* 16-bit uid */
	char	nlinks;	/* no. of links */
#endif	AUSAM
	char    size0;  /* high byte of the 24 bit size */
	int     size1;  /* low word of the 24 bit size */
	int     addr[8];/* first address */
	long	actime; /* time of last access */
	long	modtime;/* time of last modifiction */
}
       stat1, stat2;

int     type1, type2;

char name1[256],name2[272]; /* buffers for source and destination names */
char *name1l, *name2l;  /* points to the last char '\0' in name1 and name2 */

int     direntry[9];    /* buffer for directory entries */

int     askflag;        /* ask before doing anything */
int     askdirflag;     /* ask before doing anything to directories */
int     linkflag;       /* if on then links are used where possible */
int     newonlyflag;    /* if on then override will not be done */
int     ownerflag;      /* if on and super user then will change owner on created files etc */
int     recursiveflag;  /* if on then will branch down directory structure */
int     verboseflag;    /* prints lots of information */
int     suflag;         /* 0 if super user otherwise 1 */
int	moveflag;	/* if on then remove file after copy */
int	smdatflag;	/* set modify date on destination files
			   to preserve inc dump information */



main(argc, argv)
int argc;
char **argv;
{
	extern int	fout;
	register int	i;
	register char	*s, *t;
	char		*name2ptr; /* remembers loc of last arg */
	int		k = 0;

	fout = 2;
#ifndef	AUSAM
	suflag = getuid()&0377; /* set suflag id */
#else	AUSAM
	suflag = getuid();	/* set suflag=0 if super-user */
#endif	AUSAM
	for(i=3;i<16;i++) close(i); /* close unneeded files */
	for(i=1;i<argc;i++)
	{
		if(argv[i][0] == '-') switch(argv[i][1])
		{
			case 'a': if (argv[i][2] == 'd') askdirflag = 1;
				  else askflag = askdirflag = 1;
				  break;
			case 'l': linkflag = 1;  break;
			case 'm': moveflag = 1; linkflag = 1;  break;
			case 's': if (suflag == 0) smdatflag = 1; 
				  else printf("only super user may set -s flag\n");
				  break;
			case 'n': newonlyflag = 1;  break;
			case 'o': if (suflag == 0) ownerflag = 1;
				  else printf("only super user may set -o flag\n");
				  break;
			case 'r': recursiveflag = 1;  break;
			case 'v': verboseflag = 1;  break;
		}
		else name2ptr = argv[k++] = argv[i]; /* copy the arg */
	}
	if(--k < 1)  /* k now is the number of sources */
	{
		if (k)
		{
			printf("Error: arg count\n");
			exit(0);
		}
		k++; /* if only one source, use working directory */
		name2ptr = ".";
	}
	for(i=0;i<k;i++)   /* this code solves the multiple sources problem */
	{
		/* get a source */
		s = name1;  t = argv[i];
		while(*s++ = *t++);
		name1l = --s; /* point to the '\0' in char string */
		while (*--s == '/') 	/* extra / */
		{
			*s = 0; name1l = s;
		}

		/* get the destination */
		s = name2;  t = name2ptr; /* always the same file */
		while(*s++ = *t++);
		name2l = --s;
		while (*--s == '/') 	/* extra / */
		{
			*s = 0; name2l = s;
		}

		getstat();
		/* only do directories if they are the only argument or '-r' given */
		if (type1 == 2 && type2 != 0 && (recursiveflag || k == 1)) cd();
		else    /* do the cp command */
			/* make entry in the direntry for the cp command */
		{
			s = &direntry[1];  t = name1;
			while (*s = *t++) if(*s++ == '/') s = &direntry[1];
			cp(); /* do the cp command */
		}
	}
}


#ifndef	AUSAM
getstat() /* gets the status on the two files */
{
	if (stat(name1,&stat1) == -1) type1 = 4;
	else    type1 = ((stat1.flags)>>13)&3;
	if (stat(name2,&stat2) == -1) type2 = 4;
	else    type2 = ((stat2.flags)>>13)&3;
}
#else	AUSAM
getstat() /* gets the status on the two files */
{
	if (newstat(name1,&stat1) == -1) type1 = 4;
	else    type1 = ((stat1.flags)>>13)&3;
	if (newstat(name2,&stat2) == -1) type2 = 4;
	else    type2 = ((stat2.flags)>>13)&3;
}
#endif	AUSAM



cd() /* copy a directory */
{
	/* source-directory file descriptor */
	register int dirfd;

	int pid,status,mode,modeflags,name1s,name2s;
	long savedmodtime;

	mode = type2;  modeflags = stat1.flags; /* save for later use */
	savedmodtime = stat1.modtime; /* save for later use */

	if (type2 == 4) /* does not exist so create the directory */
	{
		do if((pid=fork()) == 0)
		{
			execl("/bin/mkdir","mkdir",name2,0);
			perror( "/bin/mkdir" );
			kill(9,par_uid);
			exit(0);
		}  while(pid == -1);
		while(pid != waitx(&status));
		getstat();
	}
	if (type2 != 2) /* if two then directory otherwise error */
	{
		if (type2 == 4) printf("Error: could not mkdir %s\n",name2);
		else            printf("Error: %s is not a directory\n",name2);
		return;
	}
	if ((dirfd = open(name1,0)) == -1)
	{
		perror( name1 );
		return;
	}
	name1s = name1l;  name2s = name2l; /* save pointers */
	getentry(dirfd); /* skip first two entries */
	getentry(dirfd); /* because they are the "." & ".." entries */
	while(getentry(dirfd))  /* get an entry */
	{
		/* and push it onto the name1 & name2 */
		push_name();
		getstat();
		switch (type1)
		{
			/* just a file */
			case 0:
				if (ask(askflag,"copy file",name1)) cp();
				break;
			case 1: /* character-type special file */
			case 3: /* block-type special file */
				if (ask(askdirflag,"copy special file",name1)) sf();
				break;
			case 2: /* directory */
				if (recursiveflag) /* ignore if not */
					if (ask(askdirflag,"examine directory",name1))
						cd(); /* copy the directory */
				break;
			default:/* error */
				printf("Error: %s does not exist",name1);
				return;
		}
		name1l = name1s;  *name1l = 0; /* pop name1 */
		name2l = name2s;  *name2l = 0; /* pop name2 */
	}
	close(dirfd); /* close the file source-directory */
	if (mode == 4) /* if directory was created then change */
	{
		chmod(name2,modeflags);
#ifndef	AUSAM
		if (ownerflag) chown(name2,(stat1.gid<<8)|(stat1.uid));
#else	AUSAM
		if ( ownerflag )
			chown( name2, stat1.uid );
#endif	AUSAM
		if (smdatflag) smdate(name2,savedmodtime);
	}
	if (moveflag)
	{
		if( !fork() )
		{
			execl("/bin/rmdir","rmdir",name1,0);
			perror( "/bin/rmdir" );
			exit(1);
		}
		waitx( &status );
	}
	return;
}


sf() /* copy a special file */
{
	if (suflag)
	{
		printf("Error: Only super user can copy special files.\n");
		return;
	}
	mknod(name2,stat1.flags,stat1.addr[0]); /* make the node */
#ifndef	AUSAM
	if (ownerflag) chown(name2,(stat1.gid<<8)|stat1.uid);
#else	AUSAM
	if ( ownerflag )
		chown( name2, stat1.uid );
#endif	AUSAM
	if (smdatflag) smdate(name2,stat1.modtime);
	getstat();
	if(type1        != type2        ||
	   stat1.flags  != stat2.flags  ||
	   stat1.addr[0]   != stat2.addr[0]   )
	{
		printf("Error: could not create special file %s\n",name2);
		unlink(name2);
	}
	else if (moveflag) unlink(name1);
}



getentry(df)
int df; /* the directory file descriptor */
{
	do
	{
		if(read(df,direntry,16) != 16) return(0);
	} while (direntry[0] == 0);
	return(1);
}


push_name()
{
	register char *s, *t;

	/* add entry onto name1 */
	if (name1l+17 > &name1[256])
	{
		printf("Error file name too long %s\n",name1);
		exit(1);
	}
	s = name1l;  t = &direntry[1];  *s++ = '/';
	while (*s++ = *t++);
	name1l = --s;

	/* add entry onto name2 */
	if (name2l+17 > &name2[256])
	{
		printf("Error file name too long %s\n",name2);
		exit(1);
	}
	s = name2l;  t = &direntry[1];  *s++ = '/';
	while (*s++ = *t++);
	name2l = --s;
}


cp()    /* works just like the cp command */
{
	int errflag;
	static int buf[256];
	int fold, fnew;
	register int n;
	register char *s, *t;

	/* is target a directory? */
	if (type2 == 2) /* if so then pad name2 with direntry */
	{
		/* set to last char in destination name */
		s = name2l;
		*s++ = '/';        /* push a '/' */
		t = &direntry[1];  /* points to begin of source string */
		while(*s++ = *t++);/* add last part of name2 */
		getstat();
	}
	if (type2 != 4)
	{
		/* cannot process if flag is on and old */
		if (newonlyflag)
		{
			printf("Error: cannot overwrite %s\n",name2);
			return;
		}
		if (stat1.dev==stat2.dev)
			/* done if same device and inode */
		{
			if (stat1.inumber==stat2.inumber)
				return;
			if (linkflag && unlink(name2) == -1)
			{
				printf("Error: cannot remove %s\n",name2);
				return;
			}
		}
	}
	errflag = 0;
	if (linkflag == 0 || link(name1,name2) == -1)
	{
		if((fold = open(name1,0)) == -1)
		{
			perror( name1 );
			errflag=1;
			return;
		}
		if ((fnew = creat(name2,stat1.flags)) == -1)
		{
			perror( name2 );
			errflag=1;
			close(fold); return;
		}
		if (ownerflag && type2 == 4)
#ifndef	AUSAM
			chown(name2,(stat1.gid<<8)|(stat1.uid));
#else	AUSAM
			chown( name2 , stat1.uid );
#endif	AUSAM
		while(n = read(fold,buf,512))
		{
			if(n == -1)
			{
				perror( name1 );
				errflag=1;
				unlink(name2); break;
			}
			else if(write(fnew, buf, n) != n)
			{
				perror( name2 );
				errflag=1;
				unlink(name2); break;
			}
		}
		close(fold);
		close(fnew);
	}
	if( (moveflag) && (!errflag) ) unlink(name1);
	if( (smdatflag) && (!errflag) ) smdate(name2,stat1.modtime);
	return;
}


ask(f,s,t) int f;
{
	char ch;
	register flg;

	if (f)  /* if flag true then ask question */
	{
		printf("%s %s? ",s,t);
		if (read(0,&ch,1) != 1) return(0);
		flg = ch == 'y';
		while (ch != '\n' && read(0,&ch,1) == 1) ;
		return(flg);
	}
	else if (verboseflag) printf("%s %s\n",s,t);
	return(1);
}
