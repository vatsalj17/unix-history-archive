/*  definitions for channel programs */

/*  the following are used to indicate the state condition of a
 *  channel program.  this may be passed to qu_fakrply() to keep
 *  Deliver happy if the foreign site goes away.
 */

#define SND_ABORT   0             /* terminate on error                 */
#define SND_RINIT   1             /* goto qu_rinit                     */
#define SND_RDADR   2             /* goto qu_radr                      */
#define SND_ARPLY   3             /* goto qu_wrply, for address        */
#define SND_TRPLY   4             /* goto qu_wrply, for msg text       */
