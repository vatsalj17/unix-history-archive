#ifdef DQS11B_0
#define DQS11
#endif
#ifdef DQS11A_0
#define DQS11
#endif
#include "../hd/tty.h"
#include "../hd/peri.h"

#ifdef RP03_0
struct {
	char *nblocks;
	int	cyloff;
} rp_sizes[8] {
	7600,	0,		/* cyl 0 thru 37 */
	36200,	38,		/* cyl 38 thru 218 */
	36200,	219,		/* cyl 219 thru 399 */
	65535,	40,		/* cyl 40 thru 367 */
	36200,	22,		/* cyl 22 thru 202 */
	40600,	203,		/* cyl 203 thru 405 */
	0,	0,
	0,	0,
};
#endif
#ifdef RP05_0
#define RP04_0
#endif
#ifdef RP04_0
struct {
	char *nblocks;
	int	cyloff;
} hp_sizes[8] {
	11286,	0,		/* cyl 0 thru 26 */
	53504,	27,		/* cyl 27 thru 154 */
	53504,	155,		/* cyl 155 thru 282 */
	53504,	283,		/* cyl 283 thru 410 */
	65535,	27,		/* cyl 27 thru 183 */
	65535,	184,		/* cyl 184 thru 340 */
	29260,	341,		/* cyl 341 thru 410 */
	0,	0,
};
#endif
#ifdef RP06_0
struct {
	char *nblocks;
	int	cyloff;
} hp_sizes[8] {
	11286,	0,		/* cyl 0 thru 26 */
	65535,	27,		/* cyl 27 thru 183 */
	53504,	155,		/* cyl 155 thru 282 */
	53504,	283,		/* cyl 282 thru 410 */
	65535,	184,		/* cyl 184 thru 340 */
	65535,	341,		/* cyl 341 thru 497 */
	65535,	498,		/* cyl 498 thru 654 */
	65535,	655,		/* cyl 655 thru 811 */
};
#endif

#ifdef DQS11
struct dqsdat dqsx[] {
#ifdef DQS11B_0
	{ 0,0,0,0,0,026402,070020,064420,0150440,067,055,075,046,03 },
#endif
#ifdef DQS11A_0
	{ 0,0,0,0,2,0102402,0130020,035420,0160440,04,0205,025,0227,0203 },
#endif
#ifdef DQS11B_1
	{ 0,0,0,0,0,026402,070020,064420,0150440,067,055,075,046,03 },
#endif
#ifdef DQS11A_1
	{ 0,0,0,0,2,0102402,0130020,035420,0160440,04,0205,025,0227,0203 },
#endif
#ifdef DQS11B_2
	{ 0,0,0,0,0,026402,070020,064420,0150440,067,055,075,046,03 },
#endif
#ifdef DQS11A_2
	{ 0,0,0,0,2,0102402,0130020,035420,0160440,04,0205,025,0227,0203 },
#endif
#ifdef DQS11B_3
	{ 0,0,0,0,0,026402,070020,064420,0150440,067,055,075,046,03 },
#endif
#ifdef DQS11A_3
	{ 0,0,0,0,2,0102402,0130020,035420,0160440,04,0205,025,0227,0203 },
#endif
};
#endif
