char	who[] "/bin/who";
char	sync[] "/bin/sync";
main()
{
	register	*stats;

	printf("The following users are on the system\n\n");
	printf("The user Stop is this program\n\n");
	stats = fork();
		if(stats == 0){
			execl(who,sync,0);
			exit();
		}
	wait();
	printf("\nYou can halt the system now if you wish\n");
}
