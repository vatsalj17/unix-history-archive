#
/*
 *	Converts the null terminated string pointed to by "s" into an
 *	integer value modulo PWHASHSIZE.
 *	Modified 19-7-79 so as not to include longops for multiplication.
 *	Mod by Graham S. checked PI (saves > 300. bytes).
 */

#include	<local-system>
#include	<passwd.h>
pwhash(s)
register	char	*s;

{

long		sum;
struct
	{
	int		hiword;
	unsigned	loword;
	};
register	unsigned	lwd;
register	char	c;

sum.loword = 0;
while(c = *s++)
	{
	sum.hiword = 0;
	lwd = sum.loword;
	if(c >= '0' && c <= '9')
		{
		sum.loword = lwd * 10;
		sum.hiword = hmul(lwd,10);
		if(sum.hiword < 0)
			sum.hiword =+ 10;
		sum =+ c - '0';
		}
	else
		if(c >= 'a' && c <= 'z')
			{
			sum.loword = lwd * 26;
			sum.hiword = hmul(lwd,26);
			if(sum.hiword < 0)
				sum.hiword =+ 26;
			sum =+ c - 'a';
			}
		else
			{
			sum =<< 7;
			sum =+ c;
			}
	sum.loword =+ sum.hiword;
	}
return((sum.loword & 077777) % PWHASHSIZE);
}
