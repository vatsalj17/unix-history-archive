/*
 *  trbas.c
 *
 *	translates basic multi-line RSTS programs
 *	usable as a filter
 */

char obuff[500], *obp, *last_, nlseen;

main()

{	char c;

	obp = obuff;

	while( c = getchar() )

	{	switch(c)

		{	case 015 :	break;

			case '\n' :	last_ = obp;
					*obp++ = '_';
					nlseen = 1;
					*obp++ = c;
					break;

			default :	if(nlseen) if( ('0'<=c) && (c<='9') ) dump();
					nlseen = 0;
					*obp++ = c;
		}
	}

	dump();
}

dump()

{	int n;

	if(last_) *last_ = ' ';
	last_ = 0;
	n = obp - obuff;
	if(n)
	{	if(write(1,obuff,n) != n) errexit();
		obp = obuff;
	}
}

errexit()

{	write(2,"Write error - run aborted\n",26);
	exit();
}
