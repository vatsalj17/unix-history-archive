#
/*
 *	creates an empty passwd file
 */

#include	<local-system>
#include	<passwd.h>

main(argc,argv)
char	*argv[];
{
int	fd, i;
long	buf[PWTABSIZE+PWHASHSIZE];

if(argc > 1)
	{
	/* alternate passwd file */
	if((fd = creat( *(++argv), 02604)) < 0)
		{
		printf("cant create alternate passwd file!\n");
		exit();
		}
	}
  else
	{
	/* normal passwd file */
	if((fd = creat(ETCPASSWD, 02604)) < 0)
		{
		printf("cant create passwd file!\n");
		exit();
		}
	}

writelock(fd);

for(i=0; i< (PWTABSIZE+PWHASHSIZE); i++) buf[i]=PWENTNULL;

if(write(fd,buf,PWTABLEN) != PWTABLEN)
	printf("write of passwd file failed!\n");

unlock();

}
