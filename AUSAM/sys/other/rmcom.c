/*
 *	a programme to read std input and write std output
 *	in the process deleting all C comments and successive '\n's
 *	and strips blanks and tabs from front of lines.
 *	this is to speed up compilations !!!!!
 *
 *				ian J
 *
 */

main()
{
	extern fin,fout;
	register c;
	register nl = 0;	/* zero when not skipping newlines */
	register bf = 0;	/* zero when not skipping blanks/tabs */
	int incomment = 0;

	fin = dup(0);
	fout = dup(1);

	while( ( c=getchar() ) != 0 ) {
		if( incomment )
			while( c=='*' ) {
				c = getchar();
				if( c=='/' ) {
					incomment=0;
					break;
				}
			}
		 else {
			while( c=='/' ) {
				c = getchar();
				if( c=='*' ) {
					incomment++;
					goto loop;
				}
				putchar('/');
			}
			/* delete successive blanks tabs newlines ... */
			if( nl == 0) {
				if( c == '\n') {
					nl++;
					putchar(c);
				} else if( c == ' ' || c == '\t' ) bf++;
					else {
						if( bf ) putchar(' ');
						bf = 0;
						putchar(c);
					}
			} else if( (c != ' ') && (c != '\t') && (c != '\n')) {
					nl = 0; bf = 0;
					putchar(c);
				}
		}
	loop:;
	}
	flush();
}
