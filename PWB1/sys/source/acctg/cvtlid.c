/*
 * cvtlid(lid)
 * 	char *lid
 *	given a pointer to a login id, return the integer uid
 *	return -1 if bad lid
 *
 * parts of this code is stolen from login.c
 */
cvtlid(lid)
char *lid;
{
char pwbuf[128]; /* buffer for a record from /etc/passwd */
extern fin[2];
register char *lidp, *bufp;
int sfin,c,ret;
/*
 *
 * open /etc/passwd
 * 
 */
sfin = fin[0];
ret = 0;
if ( (fin[0]=open("/etc/passwd",0)) < 0)
	{printf("can't open /etc/passwd\n");
	 fin[0] = sfin;
	 exit();
	}
/*
 * search /etc/passwd for 'lid' entry
 */
loop:  
	lidp = lid;
	bufp = pwbuf;
	while (  (c=getchar()) != '\n')
		{if (c == '\0')
			{ret = -1;
			 goto retn;
			}
		 *bufp++ = c;
		}
	*bufp++ = '\0'; /* terminate pwbuf string */
/*  is this record the lid entry? */
	bufp = pwbuf;
	while ( *lidp++ == *bufp++)  ;
	if (  (*--lidp != '\0' && lidp < lid + 8)
	      || *--bufp != ':')
		goto loop;
/*  got the 'lid' entry -- grab its uid */
	bufp = colon(colon(pwbuf));
	while (*bufp != ':')
		{
		ret = ret*10 + *bufp++ - '0';
		}
retn:
	close(fin[0]);
	fin[1]=0;
	fin[0] = sfin;
	return(ret);
}
/*
 :
 * return a pointer to first character after a colon
 *
 */
colon(p)
char *p;
{
register char *rp;
rp = p;
while (*rp != ':')  rp++;
*rp++ = '\0';
return(rp);
}
