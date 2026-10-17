/*
 *      activity phase-recording
 *
 *      use files to timestamp activity phases for channels
 */

#define PHS_CNSTRT  1           /* start connecting to site */
#define PHS_CNGOT   2           /* got connection to site */
#define PHS_CNEND   3           /* end connection to site */
#define PHS_RESTRT  4           /* start reading (sending) mail from site */
#define PHS_REMSG   5           /* finished reading one message */
#define PHS_REEND   6           /* end reading (sending) mail from site */
#define PHS_WRSTRT  7           /* start writing (picking up) mail to site */
#define PHS_WRMSG   8           /* finished writing one message  */
#define PHS_WREND   9           /* end writing (picking up) mail to site */


extern time_t phs_get ();
