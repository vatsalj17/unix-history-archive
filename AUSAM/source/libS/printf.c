#include	"stdio.h"

printf(fmt, args)
{
	_doprnt(fmt, &args, stdout);
}
