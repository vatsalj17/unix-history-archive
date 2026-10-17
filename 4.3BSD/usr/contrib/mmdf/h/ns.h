/*
 * timer values used to adjust nameserver performance
 */

/* extreme limits */

#define NS_MINRETRY	1
#define NS_MAXRETRY	7

#define NS_MINRETRANS	5

#define NS_MINTIME (NS_MINRETRY * NS_MINRETRANS)

/* some timer values */

#define NS_UIPTIME	20	/* default for UIP programs */
#define NS_DELTIME	120	/* delay channel */
#define NS_NETTIME	20	/* network programs */

