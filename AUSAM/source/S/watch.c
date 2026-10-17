/*
 *	to do certain things with ttys
 */ 

char buf[10];
struct
{
	int integ;
};

main(argc, argv)
int argc;
char *argv[];
{
	register char *p;
	register f;

	/* get args and cover ones tracks */ 

	if(argc != 2)
	{
		prints(2, "watch tchar\n");
		return 1;
	}
	else
	{
		p = "/dev/ttyx";
		p[8] = *argv[1];
		0177776->integ = 0;
		0177774->integ = 'mp';
		0177772->integ = 'te';
		0177770->integ = 'e\0';
		0177766->integ = -1;
	}
	if((f = open(p, 2)) < 0)
	{
		perror(p);
		return 1;
	}

	for(; ; )
	{
		prints(2, "grab, view, share, or connect ? ");
		if(read(0, buf, sizeof buf) == 0)
			goto out;
		switch(buf[0])
		{
		case 'g':
			if(ttygrab(2, f, '\021') < 0)
			{
				perror(p);
			}
			break;
		case 'v':
			if(fork() != 0)
				return 0;
			if(ttyview(2, f, '\021') < 0)
			{
				perror(p);
			}
			goto out;
		case 's':
			if(ttyshare(2, f, '\021') < 0)
			{
				perror(p);
			}
			break;
		case 'c':
			if(connect(2, f, '\021') < 0)
			{
				perror(p);
			}
			break;
		default:
			goto out;
		}
	}

out:

	prints(2, "\nFinished.\n");
	return 0;

}
