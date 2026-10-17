#include	<passwd.h>
#include	<lnode.h>
setlimits( lp )
  register struct lnode *lp;
{
  setuid( lp->l_uid );
  return( 0 );
}
