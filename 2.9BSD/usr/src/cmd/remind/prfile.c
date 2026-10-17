#include "remind.h"

main()
{
    register struct rmdblock *rp = &m;
    register FILE *fp;
    register i;
    char buf[BUFSIZ];
    char d[1000];
    char *cp;

    setbuf(stdout,buf);
    if((fp=fopen(reminders,"r"))==NULL)
    {
	fprintf(stderr,"cannot open %s\n",reminders);
	exit(1);
    }
    while((fread(rp,sizeof m, 1,fp))==1)
    {
	printf("::::::::::::::::::::::\n");
	printf("deliver time: %s",ctime(&rp->tdeliver));
	printf("nobytes: %d\n",rp->nobytes);
	printf("sent: %s",ctime(&rp->tsent));
	printf("sender%8s\n",rp->sender);
	printf("bits: %d\n",rp->bits&0377);
	printf("dirsize: %d\n",rp->dirsize&0377);
	printf("nrcvrs: %d\n",rp->nrcvrs&0377);
	printf("msgbytes: %d\n",rp->msgbytes);
	if((fread(d,rp->nobytes,1,fp))!=1)
	{
	    fprintf(stderr,"fread error\n");
	    exit(1);
	}
	printf("receivers: ");
	for(i=0,cp=d;i<8*rp->nrcvrs;i++)
	{
	    putchar(*cp++);
	    if((i+1)%8 == 0)
		putchar(' ');
	}
	printf("\n");
	if(rp->dirsize)
	{
	    printf("directory: ");
	    for(i=0;i<rp->dirsize;i++)
		putchar(*cp++);
	    printf("\n");
	}
	printf("message:\n");
	for(i=0;i<rp->msgbytes;i++)
	    putchar(*cp++);
    }
}
