/* character values
 * ^A is WRIGHT
 * ^B is WLEFT
 * ^C means back to UNIX
 * ^D is END OF FILE
 * ^E is DOWN
 * ^F is TAILERASE
 * ^G is WRUBOUT, as well as c_BELL */

#define c_BELL          007     /* ^G */

/* ^H is unused ?? */

#define c_TAB           011     /* ^I */
#define c_NL            012     /* ^J */
#define c_POSITION      013     /* ^K */
#define c_CLEAR         014     /* ^L */
#define c_CR            015     /* ^M */

/* ^N is unused ??
 * ^O is STOP OUTPUT
 * ^P is unused ??
 * ^Q is RIGHT
 * ^R is CERASE
 * ^S is UP
 * ^T is 'system up' message
 * ^U is LEFT
 * ^V is SEND
 * ^W is WORDERASE
 * ^X is TAILERASE
 * ^Y is CHMODE
 */

#define int_CONTROL     032     /* ^Z */
#define c_ESCAPE        033     /* ^[ */

/* and the rest (034-037) are too hard to type on an ITT
*/

#define c_SPACE         040
extern char c_MODIFY; /* initially backslash */
extern char c_BLOB, c_CONTROL; /* set differently for itt and hazeltine */
#define c_LEADIN        0176
#define c_RUBOUT 0177
