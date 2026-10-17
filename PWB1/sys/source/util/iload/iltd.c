int	buf[2560];
char	unix[14];
char	ans[256];
int	*boot;
int	rp03boot(), rp04boot();
int	(*read)(), (*write)();
int	tmread(), htread(), rpwrite(), hpwrite();
main()
{
	register count;
	int	tapa, disa,c;
	int	diskunit, tapeunit;

	unix[0] = 0;
	strcat(&unix,"unix");
	printf("\nPWB UNIX License Version\n");
	printf("Initial Load: Tape-to-Disk\n");
	printf("\nThe disk drive type which will be used for the Root file system\n");
	printf("and the tape drive type which will be used for the Initial Load Tape\n");
	printf("must be specified below.\n");
	printf("\nAnswer the questions with a 'y' or 'n' followed by\n");
	printf("a carriage return or line feed.\n");
	printf("The character '@' will kill the entire line\n");
	printf("and the character '#' will erase the last character typed.\n");
	printf("To restart the program during the question phase,\n");
	printf("type the DEL character.\n\n");
	if (yes("RP03 at address 176710")) {
		write = &rpwrite;
		boot = &rp03boot;
		strcat(&unix,"rp");
	} else
	if (yes("RP04, RP05, or RP06 at address 176700")) {
		write = &hpwrite;
		boot = &rp04boot;
		strcat(&unix, "hp");
	} else
	{
		printf("PWB only supports the above disk drives\n");
		printf("restart and answer correctly\n\n");
		exit(1);
	}
	diskunit = number("Drive number (0-7)");
	if (diskunit>7) {
		printf("Out of range, 0 assumed\n");
		diskunit = 0;
	}
	printf("Disk drive %d selected\n",diskunit);
	printf("\nMount formatted pack on drive %d\n",diskunit);
	while (!yes("Ready"))
		printf("So what's the matter?\n");
	putchar('\n');
	if (yes("TU10/TM11 at address 172520")) {
		read = &tmread;
		strcat(&unix, "tm");
	} else
	if (yes("TU16 at address 172440")) {
		read = &htread;
		strcat(&unix, "ht");
	} else
	{
		printf("PWB only supports the above tape drives,\n");
		printf("restart and answer correctly\n\n");
		exit(1);
	}
	tapeunit = number("Drive number (0-7)");
	if (tapeunit>7) {
		printf("Out of range, 0 assumed\n");
		tapeunit = 0;
	}
	printf("Tape drive %d selected\n",tapeunit);
	printf("\nThe tape on drive %d will be read from the current position\n",tapeunit);
	printf("at 800bpi, 5120 characters(10 blocks) per record\n");
	printf("and written onto the pack on drive %d starting at block 0.\n\n",diskunit);
	while (!yes("Ready"))
		printf("Why not?\n");
	tapa = 0;
	disa = 0;
	buf[1+256] = 0;
	if((*read)(tapa,&buf,sizeof buf,tapeunit)){
		printf("hard tape error - abort\n");
		exit(1);
	}
	if(buf[1+256])
		printf("Size of filesystem to be copied is %l blocks.\n",buf[1+256]);
	else	{
			printf("Bad format on tape - abort\n");
			exit(1);
		}
	printf("What is the pack volume label? (e.g. p0001): ");
	getline(&ans);
	if (ans[0]=='\0')
		strcpy(&ans,"p0001");
	ans[5] = '\0';
	printf("The pack will be labelled %s\n",&ans);
	strcpy(&buf[256+253],&ans);
	count = (buf[1+256]+9)/10;
	for(;;) {
		if ((*write)(disa,&buf,sizeof buf,diskunit)){
			printf("hard disc error - abort\n");
			exit(1);
		}
		disa =+ 10;
		tapa++;
		if(--count<=0)
			break;
		if ((*read)(tapa,&buf,sizeof buf,tapeunit)) {
			printf("hard tape error - abort\n");
			exit(1);
		}
	}
	printf("disk boot block for your disk drive type will be installed now\n");
	if ((*write)(0,boot,512,diskunit)) {
		printf("can not write boot block - warning\n");
	}
	printf("\nThe file system copy is now completed.\n");
	printf("\nTo boot the basic unix for your disk and tape drives\n");
	printf("as indicated above, mount this pack on drive 0\n");
	printf("and read in the boot block (block 0) using\n");
	printf("whatever means you have available. See romboot(VIII), 70boot(VIII).\n");
	printf("\nThen boot the program %s using diskboot(VIII).\n",&unix);
	printf("Normally:    #00=%s\n\n",&unix);
	printf("Remember to come up single-user by setting the\n");
	printf("console switches to 0173030.\n");
	printf("\nAfter UNIX is up, link the file %s to unix using ln(I)\n",&unix);
	printf("      % ln /%s /unix\n",&unix);
	printf("and set the date(I).\n");
	printf("\nGood Luck!\n\n");
	printf("The tape will now be rewound.\n\n");
	(*read)(0,0,0,tapeunit);
	exit(0);
}
yes(qs)
char	*qs;
{
	printf("%s?: ",qs);
	getline(&ans);
	if (ans[0]=='y')
		return(1);
	if (ans[0]=='n' || ans[0]=='\0')
		return(0);
	printf("assumed 'n'\n");
	return(0);
}

number(qs)
char	*qs;
{
	register c, num;

	num = 0;
	printf("%s?: ",qs);
	getline(&ans);
	c = ans[0];
	if('0'<=c && c<='9')
		num = num*10 + c - '0';
	return(num);
}

getline(s)
register char *s;
{
	register char *bp, c;

	bp = s;
	while ((c=getchar())!='\n') {
		if (c=='@')
			s = bp; else
		if (c=='#')
			s--; else
		if (c==0177)
			exit(1); else
		*s++ = c;
	}
	*s = '\0';
}
