#

/*
 * A program to enable shared-data programs to be linked
 */ 

char tmpfile[]	"tmpfile";
char ld[]	"/bin/ld";
char aout[]	"a.out";

main(argc, argv)
int argc;
char *argv[];
{
	register int k;
	register int j;
	register int i;
	int fidi;
	int fido;
	int hdri[8];
	int zero;
	int bsssize;
	int datsize;
	int txtsize;
	int totsize;
	int iobuf[12000];
	int ioleng;
	int textid;


	zero = 0;
	if(argc < 3)
		error("Usage: lked id text.o data.o [ld args]\n");

	/* i.d. */ 
	textid = *argv[1];

	/* get at text file */ 
	if((fidi = open(argv[2], 0)) < 0)
	{
		perror(argv[2]);
		exit(1);
	}
	read(fidi, hdri, sizeof hdri);
	if(hdri[0] != 0407 || hdri[7] != 0)
	{
		prints(2, argv[2]);
		error(" not relocatable\n");
	}
	if((fido = creat(tmpfile, 0600)) < 0)
	{
		perror(tmpfile);
		exit(1);
	}
	txtsize = hdri[1];
	datsize = hdri[2];
	bsssize = hdri[3];
	hdri[1] =+ hdri[3]+hdri[2];	/* merge text, data and bss */ 
	hdri[2] = hdri[3] = 0;
	write(fido, &hdri, sizeof hdri);
	totsize = txtsize+datsize;
	/* copy text */ 
	for(i = totsize; i >= 0; )
	{
		i =- 512;
		ioleng = read(fidi, iobuf, i < 0?i+512:512);
		write(fido, iobuf, ioleng);
	}
	/* make bss look real */ 
	for(i = 0; i < bsssize; i =+ 2)
		write(fido, &zero, 2);

	/* adjust relocation factor */ 
	for(i = totsize; i >= 0; )
	{
		i =- 512;
		ioleng = read(fidi, iobuf, i < 0?i+512:512);
		if(j = ioleng/2)
		{
			do
			{
				if(iobuf[j-1] == 006 || iobuf[j-1] == 004)
					iobuf[j-1] = 002;
				else if(iobuf[j-1] == 007 || iobuf[j-1] == 005)
					iobuf[j-1] = 003;
			}
			while(--j);
		}
		write(fido, iobuf, ioleng);
	}
	/* fix bss relocation */ 
	for(i = 0; i < bsssize; i =+ 2)
		write(fido, &zero, 2);

	/* copy and adjust symbol table */ 
	while(read(fidi, iobuf, 12) == 12)
	{
		if(iobuf[4] == 004 || iobuf[4] == 003)
			iobuf[4] = 002;
		else if(iobuf[4] == 044 || iobuf[4] == 043)
			iobuf[4] = 042;
		write(fido, iobuf, 12);
	}
	close(fidi);
	close(fido);

	if(fork() == 0)
	{
		argv[0] = ld + 5;
		argv[1] = "-rX";
		argv[2] = "tmpfile";
		argv[argc] = 0;
		execv(ld, argv);
		perror(ld);
		exit(1);
	}
	waitx(iobuf);
	if(unlink(tmpfile) < 0)
	{
		perror(tmpfile);
		exit(1);
	}

	if((fidi = open(aout, 2)) < 0)
		error("Can't open a.out\n");
	read(fidi, &hdri, 16);
	if(hdri[0] != 0407 || hdri[7] != 0)
		error("ld output incorrect\n");
	hdri[2] =+ hdri[1]-txtsize;
	hdri[1] = txtsize;
	seek(fidi, 0, 0);
	write(fidi, &hdri, 16);
	totsize = 2*(hdri[1]+hdri[2]);
	if((totsize > sizeof iobuf) || read(fidi, iobuf, totsize) != totsize)
		error("resultant text and data unsuitable\n");
	totsize =/ 4;
	for(k = 0; k < totsize; k++)
	{
		if(iobuf[totsize+k] == 2 && iobuf[k] >= txtsize)
			iobuf[totsize+k] = 4;
		else if(iobuf[totsize+k] == 3)
		{
			if((iobuf[k]+2*k+2) >= txtsize)
				iobuf[k+totsize] = 5;
		}
	}
	seek(fidi, 16, 0);
	totsize =* 4;
	if(write(fidi, iobuf, totsize) != totsize)
		error("write error a.out\n");
	/*
	 * fix up those symbols we changed before
	 */
	while(read(fidi, iobuf, 12) == 12)
	{
		if((iobuf[4] == 002 || iobuf[4] == 042) && iobuf[5] >= txtsize)
		{
			iobuf[4] =+ 1;
			seek(fidi, -12, 1);
			write(fidi, iobuf, 12);
		}
	}
	close(fidi);
	if(fork() == 0)
	{
		argv[0] = ld + 5;
		argv[1] = "-n";
		argv[2] = aout;
		argv[3] = 0;
		execv(ld, argv);
		perror(ld);
		exit(1);
	}
	waitx(iobuf);
	if((fidi = open(aout, 2)) < 0)
	{
		perror(aout);
		exit(1);
	}
	read(fidi, &hdri, 16);
	if(hdri[0] != 0410)
		error("a.out not suitable\n");
	hdri[6] = textid;
	hdri[0] = 0412;
	seek(fidi, 0, 0);
	write(fidi, &hdri, 16);
	close(fidi);
	return(0);
}

error(arg)
{
	prints(2, arg);
	exit(1);
}
