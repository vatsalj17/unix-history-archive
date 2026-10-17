/* Fortran chdir PLWard 3/20/80 */
long chdir_(cmd,returns)
     char cmd[];
     int *returns;
{
     *returns = chdir(cmd);
}
