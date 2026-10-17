#define	CHPERL	 11	/* characters per line */
#define	LNPERCH	 12	/* lines per character */
#define	CHR	'@'	/* the character printed out */

char letters[] "/usr/lib/letters";
int line[LNPERCH * CHPERL];
char **gargv;
int colcnt;		/* current printing column */
int blanks;		/* blanks since last character */

main(ac, av)
char *av[];
{
	register char c;
	register cnt, *wp;
	int fd, count, linec;

	if ((fd = open(letters, 0)) < 0)
	{
		perror(letters);
		return 1;
	}
	if (ac == 1) prints(2, "\007Go Ahead (11 chars/line; 5 lines/page)\n");
	else gargv = ++av;
	for (;;)
	{
		for (count = 0; count < CHPERL; count++)
		{
			if ((c = getch()) == '\n' || c == 0) break;
			if (c < ' ')
			{
				prints(2, "Control-chrs. not allowed.\n");
				c = ' ';
			}
			seek(fd, (c - ' ') * 2 * LNPERCH, 0);
			if (read(fd, &line[count * LNPERCH], 2 * LNPERCH) != 2 * LNPERCH)
			{
				perror(letters);
				return 1;
			}
		}
		if (c == 0 && count == 0) break;
		if (c && c != '\n') while ((c = getch()) && c != '\n');
		for (linec = 0; linec < LNPERCH; linec++)
		{
			wp = &line[linec];
			for (cnt = 0; cnt < count; cnt++)
			{
				print(*wp);
				wp =+ LNPERCH;
			}
			putchar('\n');
			blanks = 0;
			colcnt = 0;
		}
		if (c == 0) break;
	}
}


getch()
{
	char c;

	if (gargv)
	{
		if (gargv == -1) return 0;
		if (**gargv) return *(*gargv)++;
		if (*++gargv == -1) return 0;
		return '\n';
	}
	if (read(0, &c, 1) != 1) return 0;
	return c;
}


print(wd)
register wd;
{
	register mask, col;

	mask = 04000;
	do
		if ((wd & mask) == 0) blanks++;
		else
		{
			col = colcnt; colcnt =+ blanks; blanks = col;
			while ((col = (col + 8) & ~07) <= colcnt)
			{
				putchar('\t');
				blanks = col;
			}
			for (col = blanks; col < colcnt; col++) putchar(' ');
			blanks = 0;
			colcnt++;
			putchar(CHR);
		}
	while (mask =>> 1);
}
