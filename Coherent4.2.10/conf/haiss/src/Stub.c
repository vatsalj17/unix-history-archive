/*
 * haiss/src/Stub.c
 *
 * Stub to use when TMC-950/9C50 host adapter routines are not configured
 * into the kernel.
 */

#include <stddef.h>
#include <sys/coherent.h>
#include <sys/haiscsi.h>

haft_t		* ss_haftp = NULL;

/* End of file */
