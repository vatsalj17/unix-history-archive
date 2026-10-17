#include	"mac.h"
#include	"mac.x"


getlin()
{
	register int nchar;
	register int cc;
	register int quote;

	p = buf;
	nchar = 0;
	while ((cc = getc(&ibuf)) >= 0)  {

		if (nchar > MAXBUF)  {
			printf("buffer overflow\n");
			exit(1);
			}

		if (cc >= 'A'  &&  cc <= 'Z')
			cc =+ ('a' - 'A');

		buf[nchar++] = cc;
		if (cc == '\n')  {
			return(nchar);
			}
		}

	buf[0] = '\0';
	return(0);
}


getch()
{
	register cc1;
	register cc2;

	cc1 = *p++;
	if (cc1 != '\\')
		return(cc1);

	cc2 = *p++;
	switch (cc2)  {

		case 'r':
			return('\r');

		case 'f':
			return('\f');

		case 't':
			return('\t');

		case '\\':
			return('\\');

		case '0':
			return('\0');

		case 'n':
			return('\n');

		case 'b':
			return('\b');

		default:
			p--;
			return(cc1);

		}
}
