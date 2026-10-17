/* Fortran iargc or get number of args in command line PLWard 3/20/80 */
int numarg_()
{
extern int xargc;
return ( xargc - 1 );
}
