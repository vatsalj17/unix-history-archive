#include "ap_lex.h"
#include "util.h"
#include "conf.h"

/* $Header: ap_lxtable.c,v 1.3 84/09/05 21:31:52 dpk Exp $ */
/* $Log:	ap_lxtable.c,v $
 * Revision 1.3  84/09/05  21:31:52  dpk
 * Added #include of util.h
 * 
 * Revision 1.2  83/11/18  17:05:31  reilly
 * Steve Kille's version
 *  */

/* mappings of lexical symbols to ascii values for address parser */

char    ap_lxtable[] =
{
    LT_EOD, LT_ERR, LT_ERR, LT_ERR,
				  /*    000-003          nul            */
    LT_ERR, LT_ERR, LT_ERR, LT_ERR,
				  /*    004-007                         */
    LT_LTR, LT_SPC, LT_SPC, LT_ERR,
				  /*    010-013          bs tab lf      */
    LT_ERR, LT_ERR, LT_ERR, LT_ERR,
				  /*    014-017                         */
    LT_ERR, LT_ERR, LT_ERR, LT_ERR,
				  /*    020-023                         */
    LT_ERR, LT_ERR, LT_ERR, LT_ERR,
				  /*    024-027                         */
    LT_ERR, LT_ERR, LT_ERR, LT_ERR,
				  /*    030-033                         */
    LT_ERR, LT_ERR, LT_ERR, LT_ERR,
				  /*    034-037                         */
    LT_SPC, LT_XTR, LT_QOT, LT_XTR,
				  /*    040-043          sp !  "  #     */
#ifdef JNTMAIL
				  /*         In JNT domain treat % as @ */
    LT_XTR, LT_AT, LT_XTR, LT_XTR,
#else
    LT_XTR, LT_XTR, LT_XTR, LT_XTR,
				  /*    044-047          $  %  &  '     */
#endif
    LT_LPR, LT_RPR, LT_XTR, LT_XTR,
				  /*    050-053          (  )  *  +     */
    LT_COM, LT_XTR, LT_XTR, LT_XTR,
				  /*    054-057          ,  -  .  /     */
    LT_NUM, LT_NUM, LT_NUM, LT_NUM,
				  /*    060-063          0  1  2  3     */
    LT_NUM, LT_NUM, LT_NUM, LT_NUM,
				  /*    014-067          4  5  6  7     */
    LT_NUM, LT_NUM, LT_COL, LT_SEM,
				  /*    070-073          8  9  :  ;     */
    LT_LES, LT_XTR, LT_GTR, LT_XTR,
				  /*    074-077          <  =  >  ?     */
    LT_AT, LT_LTR, LT_LTR, LT_LTR,
				  /*    100-103          @  A  B  C     */
    LT_LTR, LT_LTR, LT_LTR, LT_LTR,
				  /*    014-107          D  E  F  G     */
    LT_LTR, LT_LTR, LT_LTR, LT_LTR,
				  /*    110-114          H  I  J  K     */
    LT_LTR, LT_LTR, LT_LTR, LT_LTR,
				  /*    115-117          L  M  N  O     */
    LT_LTR, LT_LTR, LT_LTR, LT_LTR,
				  /*    120-123          P  Q  R  S     */
    LT_LTR, LT_LTR, LT_LTR, LT_LTR,
				  /*    124-127          T  U  V  W     */
    LT_LTR, LT_LTR, LT_LTR, LT_LSQ,
				  /*    130-133          X  Y  Z  [     */
    LT_SQT, LT_RSQ, LT_XTR, LT_XTR,
				  /*    134-137          \  ]  ^  _     */
    LT_XTR, LT_LTR, LT_LTR, LT_LTR,
				  /*    140-143          `  a  b  c     */
    LT_LTR, LT_LTR, LT_LTR, LT_LTR,
				  /*    144-147          d  e  f  g     */
    LT_LTR, LT_LTR, LT_LTR, LT_LTR,
				  /*    150-153          h  i  j  k     */
    LT_LTR, LT_LTR, LT_LTR, LT_LTR,
				  /*    154-157          l  m  n  o     */
    LT_LTR, LT_LTR, LT_LTR, LT_LTR,
				  /*    160-163          p  q  r  s     */
    LT_LTR, LT_LTR, LT_LTR, LT_LTR,
				  /*    164-167          t  u  v  w     */
    LT_LTR, LT_LTR, LT_LTR, LT_XTR,
				  /*    170-173          x  y  z  {     */
    LT_XTR, LT_XTR, LT_XTR, LT_ERR,
				  /*    174-177          |  }  ~  del   */
};
