/*
 * kick off automatic fsck from init
 */
#define say(x)	(void)write(2,x,sizeof(x)-1) 

/* char *noargs[] = { "/etc/fsck", "-p", (char*)0}; */
char *noargs[] = { "/etc/upchuck", "-w", (char*)0};
main(ac, av)
char **av;
{

	switch( run ((ac<2) ? noargs : &av[1])) {
	case 0:
		break;
	case 4:
		(void)run("/etc/reboot", "-n", (char *)0);
		break;
	case 8:
		say("Automatic reboot failed... help!\n");
		exit(1);
		break;
	case 12:
		say("Reboot interrupted\n");
		exit(1);
		break;
	default:
		say("Unknown error in reboot\n");
		exit(1);
		break;
	}
	exit(0);
}
run(av)
char **av;
{
	int x;

	if(fork() == 0) {
		execv(av[0], av);
		perror(av[0]);
		exit(1);
	}
	if(wait(&x) == -1)
		return -1;
	else
		return (x>>8) & 0377;
}
