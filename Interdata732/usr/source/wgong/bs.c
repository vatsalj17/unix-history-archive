/*
 * Perform backspace-delete function as in tty.c
 *	- useful after an edit session when you forgot stty
 */

main()
{
	register i, c, col;
	extern fout;
	static line[256];

	fout = dup(1);
	col = 0;

	while (c = getchar()) {
		switch (c) {

		case '\n':
			for (i=0; i<col; i++)
				putchar(line[i]);
			putchar('\n');
			col = 0;
			break;

		case '\b':
			if (col)
				col--;
			break;

		default:
			line[col++] = c;
			break;

		}
	}
	flush();
}
