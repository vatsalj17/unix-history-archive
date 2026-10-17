sgset_(signo, func)
int *signo;
int (*func)();
{
	signal(*signo, func);
}
