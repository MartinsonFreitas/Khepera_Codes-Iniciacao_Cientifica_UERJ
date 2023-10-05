/*--------------------------------------------------------------------
 * knet_test.c - KNET Test
 *--------------------------------------------------------------------
 * $Id: knet_test.c,v 1.1 2004/07/29 10:57:20 cgaudin Exp $
 *--------------------------------------------------------------------
 * $Author: cgaudin $
 * $Date: 2004/07/29 10:57:20 $
 * $Revision: 1.1 $
 *--------------------------------------------------------------------*/

#include <khepera/khepera.h>

/* main */

int main( int argc , char * argv[] )
{
  int rc;
  knet_dev_t * dev;

  if ((rc = kb_init( argc , argv )) < 0 )
    return 1;

  dev=knet_open( argv[1] , KNET_BUS_ANY , 0 , NULL );
  
  if ( dev != NULL ){
    kb_msg( "OK\n");
    knet_close( dev );
  }
  else {
    kb_msg("ER\n");
  }

  return 0;
}
