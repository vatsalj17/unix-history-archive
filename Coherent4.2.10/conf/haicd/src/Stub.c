/*
 * conf/haicd/src/Stub.c
 */

#define __KERNEL__	 1

#include <sys/haiscsi.h>

/*
 * Don't attach CD ROM function table to SCSI kit.
 */
dca_t * cdrdca = NULL;
