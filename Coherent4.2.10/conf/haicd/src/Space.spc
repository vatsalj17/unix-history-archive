/*
 * Configurable information for "hai" SCSI CD ROM driver.
 */

#define __KERNEL__	 1

#include <sys/haiscsi.h>
#include "conf.h"

/*
 * Attach CD ROM device function table to SCSI kit.
 */
extern dca_t cdrdca;
dca_t * cdrdcap = & cdrdca;
