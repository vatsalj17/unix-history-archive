#include <local-system>
#include <passwd.h>

main(argc,argv)
int argc;
char **argv;
{
	struct pwent pe;
	char sbuf[512];

	if( argc != 1 )
	{
		prints(2,"arg count\n");
		exit(1);
	}
	pe.pw_uid = getreal();
	if( getpwlog( &pe , sbuf , sizeof sbuf ) == -1 )
	{
		prints(2,"getpwlog error\n");
		exit(1);
	}
	execl("/bin/log", "log", pe.pw_strings[LNAME], 0);
	perror("log");
}
