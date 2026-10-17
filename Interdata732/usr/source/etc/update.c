main()
{
	if (fork())
		exit();

	for(;;) {
		sleep(30);
		sync();
	}
}
