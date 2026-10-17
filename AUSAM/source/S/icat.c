char	searchstr[]	"#include";
char	inclnam[]	"/usr/include/";
main(c,v)
char	**v;
int	c;
{
	extern	fout;
	fout = dup(1);
	c --; v ++;
	if(c)
		do
			newfile(*v++);
		while(--c);
	else
		expand(0);
	flush();
	return(0);
}

newfile(name)
char	*name;
{
	int	fd;
	if((fd = open(name, 0)) == -1)
		perror(name),exit(1);
	expand(fd);
	close(fd);
}

expand(fd)
int	fd;
{
	register char	remember, c, *cp;
	char	*rp;
	struct {
		int	fildes;
		int	nleft;
		char	*nextp;
		char	buf[512];
	}	b;
	char	namebuf[128];
	b.fildes = fd;
	b.nleft = 0;
	cp = searchstr;	
	c = getc(&b);
	while(c != -1)
	{
		if(*cp == '\0')
		{
			/* we got an include */
			while(c == ' ' || c == '\t')
				c = getc(&b);
			if(c == '<')
			{
				strcpy(inclnam, namebuf);
				cp = namebuf + sizeof inclnam - 1;
			}
			else
				cp = namebuf;
			if(c == '<' || c == '"')
			{
				remember = (c == '<') ? '>' : '"';
				while((c = getc(&b)) != '\n' &&
					c != -1 &&
					c != remember)
					*cp++ = c;
				if(c != remember)
					prints(2, "unmatched '<' or '\"'\n");
				*cp++ = 0;
				while(c != '\n' && c != -1)
					c = getc(&b);
				if(c == '\n')	/* get next line */
					c = getc(&b);
				newfile(namebuf);
			}
			else
				prints(2, "no name on include\n");
			cp = searchstr;
		}
		else
		{
			if(c == *cp)
				cp ++;
			else
			{
				for(rp = searchstr; rp < cp; rp++)
					putchar(*rp);
				cp = searchstr;
				if(c == *cp)
					cp ++;
				else
					putchar(c);
			}
			c = getc(&b);
		}
	}
}


strcpy(f,t)
register char	*f, *t;
{
	while(*f)
		*t++ = *f++;
}
