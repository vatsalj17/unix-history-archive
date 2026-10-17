/*
 * filter for removing white space from C programs.
 * is intelligent about string quotes etc.
 *
 *	a finite state algorithm - well almost ...
 *
 *				Dave Horsfall, CSU.
 */

/* states */

#define	NORMAL	0
#define	SKPSPC	1
#define	STCMNT	2
#define	INCMNT	3
#define	ENDCMNT	4
#define	QUOTES	5
#define	SLOSHED	6

main()
{
	register state;
	register c;		/* char read */
	register savc;		/* space or newline char */
	char quote;		/* ' or " */
	extern	fin, fout;

	fin = dup(0);
	fout = dup(1);
	state = SKPSPC;
	savc = -1;		/* first-time kludge */

	c = getchar();
	while (c != 0)
	{
		switch (state)
		{

		case NORMAL:
			switch (c)
			{
			case '\t':
				c = ' ';
			case ' ':
			case '\n':
				savc = c;
				state = SKPSPC;
				break;

			case '/':
				state = STCMNT;
				break;

			case '\'':
			case '"':
				quote = c;
				state = QUOTES;
			default:
				if (savc)
				{
					if (savc > 0)
						putchar(savc);
					savc = 0;
				}
				putchar(c);
				break;
			}
			break;

		case SKPSPC:
			if (c == ' ' || c == '\t' || c == '\n')
				break;
			state = NORMAL;
			continue;	/* avoids the getchar at the bottom */

		case STCMNT:
			if (c == '*')
				state = INCMNT;
			else
			{
				putchar('/');
				putchar(c);
				savc = 0;
				state = NORMAL;
			}
			break;


		case INCMNT:
			if (c == '*')
				state = ENDCMNT;
			break;

		case ENDCMNT:
			if (c == '/')
			{
				state = SKPSPC;
				if (savc >= 0)
					savc = '\n';
			}
			else
				if (c != '*')
					state = INCMNT;
			break;

		case QUOTES:
			putchar(c);
			if (c == quote)
				state = NORMAL;
			else
				if (c == '\\')
					state = SLOSHED;
			break;

		case SLOSHED:
			putchar(c);
			state = QUOTES;
			break;
		}

		c = getchar();
	}

	putchar('\n');
	flush();
	exit(0);
}
