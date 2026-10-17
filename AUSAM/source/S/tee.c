#include <stat.h>

int open[15] { 1 };	/* 15 is a little optimistic */
int t;			/* set if any char outputs */
int n 1;		/* number of open files in open */

main(argc, argv)
char **argv;
{
	int register r, w, p;
	char in[512], out[512];
	struct statb buf;

	fstat(1, &buf);
	t = (buf.i_mode&IFMT) == IFCHR;
	while(argc-- > 1)
	{
		if((open[n] = creat(argv[1], 0600)) == -1)
		{
			perror(argv[1]);
		}
		else
		{
			n++;
			if(stat(argv[1], &buf) != 0)
				if((buf.i_mode&IFMT) == IFCHR)
					t++;
		}
		argv++;
	}
	r = w = 0;
	for(;;)
	{
		for(p = 0; p < 512; )
		{
			if(r >= w)
			{
				if(t > 0 && p > 0)
					break;
				w = read(0, in, 512);
				r = 0;
				if(w <= 0)
				{
					stash(p,out);
					return;
				}
			}
			out[p++] = in[r++];
		}
		stash(p,out);
	}
}

stash(p,out)
char *out;
{
	register int k, i, d;

	d = t?10:p;
	for(i = 0; i < p; i =+ d)
		for(k = 0; k < n; k++)
			write(open[k], out+i, d < p-i ? d : p-i);
}
