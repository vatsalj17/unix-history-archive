/* Fortran alarm PLWard 3/20/80 */
long alarm_(secs,remaining)
    int *secs,*remaining;
{
    unsigned seconds;
    seconds = *secs;
    *remaining = alarm(seconds);
}
