#include	<local-system>
#include	<stat16.h>
#include	<passwd.h>

struct dirent
{
	unsigned d_inum;
	char d_name[14];
};
struct lpdrec
{
	char l_cntrl;
	char l_data[39];
};

struct dirent *dtab;
struct statbuf sb;
struct pwent pe;
struct lpdrec *p;
struct lpdrec *df;
char *lpd "/tmp/lpd/x";
#define lpdnum lpd[9]
char *banner,*origin,*mesg,*time,buf[SSIZ];
char *tmp,rflg,aflg,fd,ncpy,bflg,
     nxtch '\n';
int lcount,uid,size,iown,nred,maxdf;
#define MAXCF 20	/* max number of "cf" files that I can digest from one
			 * "df" file	*/
char *cfbuf[MAXCF];
int  ncfs;
comp(a,b)
	struct dirent *a,*b;
{
	char c;
	int ret;

	ret=0;
	if(a->d_inum && b->d_inum)
	{
		c=0;
		while(c<14 && !(ret=a->d_name[c] - b->d_name[c]))
			c++;
	}
	else
		ret=b->d_inum - a->d_inum;
	return(ret);
}

main(argc,argv)
	int argc;
	char **argv;
{
	extern fout,exit();
	char *arg;
	int i;

	fout=dup(1);
	uid=getreal();
	while(argc>1 && (arg=argv[1])[0]=='-')
	{
		while(*++arg)
			switch(*arg)
			{
			case '0':
				break;
			case '1':
			case '2':
			case '3':
				lpdnum= *arg;
				break;
			default:
				prints(2,"Bad Argument\n");
				exit(-1);
			case 'a':
				aflg++;
				break;
			case 'r':
				rflg++;
			}
		argv++;
		argc--;
	}
	signal(3,exit);		/* dont want cores in lpd directory */
	if(chdir(lpd)== -1)
	{
		prints(2,lpd);
		prints(2,": Can't chdir\n");
		exit(-1);
	}
	newstat("",&sb);
	if((fd=open("",0))== -1 || seek(fd,32,0)== -1 ||
		(tmp=sbrk(sb.sb_size1=- 32))== -1 ||
		read(fd,tmp,sb.sb_size1)!=sb.sb_size1)
	{
		prints(2,". unreadable\n");
		exit(-1);
	}
	dtab=tmp;
	qsort(dtab,sb.sb_size1/sizeof *dtab,sizeof *dtab,comp);
	tmp=+sb.sb_size1;
	for(lcount=0;&dtab[lcount]<tmp && dtab[lcount].d_inum;lcount++);
	df= &dtab[lcount];
	maxdf=(df-tmp)-((df-tmp)%sizeof *df);
	for(i=0;i<lcount;i++)
		process(i);
	if(!(iown || aflg))
		printf("No jobs found\n");
	fout=0;
	flush();
	if(rflg)
	{
		while(iown && (rflg=input()))
			if(rflg!=',' && rflg!='\n')
			{
				while((nxtch=getchar())!='\n');
				prints(2,"Syntax\n");
			}
	}
}

process(n)
{

	getdf(dtab[n].d_name);
	if(pe.pw_uid==uid || !uid)
		iown++;
	if(aflg || pe.pw_uid==uid)
	{
		if(!bflg)
		{
			printf("     Date Time     Owner     Banner   Size Origin  tty  Message\n");
			bflg++;
		}
		time=ctime(sb.sb_modtime);
		time[16]=0;
		if(getpwlog(&pe,buf,sizeof buf)== -1)
			pe.pw_strings[LNAME]="";
		printf("%3d%11s%11s%11s%5d%8s",n,time+8,pe.pw_strings[LNAME],banner,size*ncpy,origin);
		if(mesg)
			printf("  tty%c %s",*mesg,mesg+1);
		putchar('\n');
	}
}

mint(p)
	struct lpdrec *p;
{

	return((p->l_data[1]<<8) + (*p->l_data & 0377));
}

input()
{
	int top,bot;

	if(nxtch=='\n')
		printf("Delete: ");
	bot=top=readnum();
	if(nxtch=='-')
		bot=readnum();
	if(nxtch)
		if(0<=top && bot<lcount)
			for(;top<=bot;top++)
				delete(top);
		else
			prints(2,"Line number?\n");
	return(nxtch);
}

readnum()
{
	int i;
	char cr;

	i=cr=0;
	while((nxtch=getchar())==' ');
	while(nxtch<='9' && '0'<=nxtch)
	{
		i=i*10+nxtch-'0';
		nxtch=getchar();
		cr++;
	}
	while(nxtch==' ')
		nxtch=getchar();
	if(!cr)
		i= -1;
	return(i);
}

delete(n)
{
	getdf(dtab[n].d_name);
	if(pe.pw_uid==uid || !uid)
	{
		iown--;
		unlink(dtab[n].d_name);
		while(ncfs)
			unlink(cfbuf[--ncfs]);
	}
	else
		printf("%d Permission Denied\n",n);
}
getdf(name)
	char *name;
{
	char *t;
	int i;

	banner="";
	origin=0;
	ncfs=mesg=size=0;
	t=pe.pw_uid= -1;
	ncpy=1;

	if(newstat(name,&sb)!=-1 && sb.sb_size1>maxdf)
	{
		if( sb.sb_size0 || sb.sb_size1 & 0100000 || (t=sbrk(sb.sb_size1-maxdf))==-1)
		{
			prints(2,"Out of core\n");
			exit(-1);
		}
		maxdf=sb.sb_size1;
	}
	fd=open(name,0);
	nred=read(fd,df,maxdf)/sizeof *df;
	close(fd);
	if(nred*sizeof *df == sb.sb_size1)
		for(i=0;i<nred;i++)
		{
			p= &df[i];
			switch(p->l_cntrl | 040)
			{
			case '*':
				origin="cyber";
				banner=p->l_data;
				break;
			case 'l':
				banner= p->l_data;
				break;
			case 'u':
				if(ncfs<(MAXCF-1))
					cfbuf[ncfs++]=p->l_data;
				break;
			case 'i':
				pe.pw_uid=mint(p);
				break;
			case 's':
				size=mint(p);
				break;
			case 'b':
				origin="batch";
				banner=p->l_data;
				break;
			case 'm':
				mesg= p->l_data;
				break;
			case 'n':
				ncpy= *p->l_data-'0';
			}
		}
	if (pe.pw_uid == -1)
		origin = "NULL";
	else
		if (origin == 0) origin = "unix";
}
