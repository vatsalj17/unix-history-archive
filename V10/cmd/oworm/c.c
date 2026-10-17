main()
{
	int i;

	for(i = 6*1024; brk(i*1024) >= 0; i++)
		;
	perror("brk");
	print("largest break is %d\n", i*1024);
}
