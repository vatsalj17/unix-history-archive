/* Fortran sleep  3/20/80 */
sleep_(secs)
int *secs;
{
	sleep((unsigned) *secs);
}
