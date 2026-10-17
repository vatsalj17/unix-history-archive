#include "util.h"
#include "mmdf.h"

/* fake user program tailoring, in case the user program does not
 * use it or it is not a user program.  This way, everyone can just
 * call mmdf_init().
 */

uip_tai (argc, argv)            /* tailor some uip parameter */
    int argc;                   /* number of values     */
    char *argv[];               /* list of values       */
{
    return (NO);
}
