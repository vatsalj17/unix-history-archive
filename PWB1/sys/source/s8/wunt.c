struct {
	char	name[8];
	char	tty;
	char	fill;
	int	time[2];
	int	wfill;
} wtmp;

main()
{
	register int f;
	f = open("/usr/adm/wtmp", 1);
	uname(wtmp.name);
	wtmp.tty = 'x';
	time(wtmp.time);
	seek(f, 0, 2);
	write(f, &wtmp, 16);
}
