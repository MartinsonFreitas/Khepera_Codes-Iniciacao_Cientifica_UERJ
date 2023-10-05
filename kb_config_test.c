/*-------------------------------------------------------------------------------
 * Project: Khepera Library	
 * $Author: pbureau $
 * $Date: 2004/11/22 17:54:23 $
 * $Revision: 1.5 $
 * 
 * 
 * $Header: /home/cvs/libkhepera/src/tests/kb_config_test.c,v 1.5 2004/11/22 17:54:23 pbureau Exp $
 */

/*--------------------------------------------------------------------*/
/*! 
 * \file   kb_config_test.c khepera lib config module test
 *
 * \brief
 * 	   dump the content of the Korebot library configuration files.
 * 	   (usually located in /etc/khepera to the standard output.
 * 	   useful for debug purpose.
 *
 * \author   Cédric Gaudin (K-Team SA)
 * \author   Pierre Bureau (K-Team SA)
 * \note     Copyright (C) 2004 K-TEAM SA
 */

#include <khepera/khepera.h>

int print_register( const char * name ,
		    kb_register_config_t * reg ,
		    void * context )
{
  kb_msg( "Register: %s\n" , name );
  return 0;
}

int print_device( const char * name ,
		   kb_device_config_t * device ,
		   void * context )
{
  kb_msg( "Device: %s\n" , name );

  return 0;
}

int print_section( const char * name ,
		   kb_section_config_t * section ,
		   void * context )
{
  kb_msg( "Section: %s\n" , name );
  kb_enum_device( name , print_device , 0 );
  kb_enum_register( name , print_register , 0 );

  return 0;
}

int main( int argc , char * argv[] )
{
  kb_device_config_t * dev;
  kb_register_config_t * reg;
  knet_dev_t *koala;
  int rc;
  unsigned char buf[10];
  unsigned char a,b,c,d;

  kb_set_debug_level( 2 );

  if((rc = kb_init( argc , argv )) < 0 )
    return 1;

  kb_enum_section( print_section , 0);

	dev = kb_lookup_device( "Khepera4:dsPic" );

   if (dev != NULL) 
    printf("dev=ok\n");
  else
    printf("dev=KO\n");


  /*
  dev = kb_lookup_device( "KoreBot:XScaleVCoreDAC" );
  

  

  reg = kb_lookup_register( "KoreMotor:Filter");

  printf("%X\n" , reg );

  if (dev != NULL) 
    printf("dev=ok\n");
  else
    printf("dev=KO\n");
  */
  
  return 0;
}
