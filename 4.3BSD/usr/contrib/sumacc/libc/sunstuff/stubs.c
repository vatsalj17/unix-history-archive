/* stubs for various routines
 * Per Bothner. Dec 1982.
 * This is needed so that Pascal routines will link properly.
 */

noSuchProc(name)
char *name;
{
    printf("The routine %s is not implemented!!!\n", name);
}

time() {
    noSuchProc("time");
}

ftime() {
    noSuchProc("ftime");
}

localtime() {
    noSuchProc("localtime");
}
