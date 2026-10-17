/*
 *	xlib libfile directory
 *
 *	expand an interdata object library
 */
char record[128];
int out;

char anon[] { "anon000 " };

main(argc, argv)
char *argv[];
{
	register n, in;
	register char *anonp;

	if (argc != 3)
		error("usage: xlib libfile directory");
	if ((in = open(argv[1], 0)) < 0)
		error("can't open %s", argv[1]);
	if (chdir(argv[2]) < 0)
		error("bad directory %s", argv[2]);

	anonp = &anon[6];

	while((n = read(in, record, 126)) == 126) {
		if (record[4] == 15)
			mkobj(&record[5]);
		else if (record[4] == 24) {
			mkobj(anon);
			if (++(*anonp) > '9') {
				*anonp = '0';
				if (++(anonp[-1]) > '9') {
					anonp[-1] = '0';
					(anonp[-2])++;
				}
			}
		}
		if (out)
			write(out, record, 126);
	}
}

mkobj(name)
char *name;
{
	char fn[16];
	register i, c;
	register char *p;

	for (i=0; i<8; i++)
		if ((c = name[i]) == ' ')
			break;
		else {
			if (c >= 'A' && c <= 'Z')
				c =+ 'a'-'A';
			fn[i] = c;
		}
	for (p = ".obj"; fn[i++] = *p; p++)
		;

	if (out)
		close(out);
	if ((out = creat(fn, 0666)) <= 0)
		error("can't create %s", fn);
}

error(s, x)
{
	printf(s, x);
	putchar('\n');
	exit(1);
}
