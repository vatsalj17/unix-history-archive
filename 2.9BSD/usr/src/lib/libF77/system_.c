/* Fortran system call PLWard 3/20/80 */
int system_(cmd,returns)
     char cmd[];
     int *returns;
{
     *returns = system(cmd);
}
