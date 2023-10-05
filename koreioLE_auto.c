/*--------------------------------------------------------------------
 * koreio_test.c - KoreBot Library - KoreIO Test
 *--------------------------------------------------------------------
 * $Id: koreioLE_auto.c,v 1.1 2006/08/18 12:31:15 flambercy Exp $
 *--------------------------------------------------------------------
 * $Author: flambercy $
 * $Date: 2006/08/18 12:31:15 $
 * $Revision: 1.1 $
 *--------------------------------------------------------------------*/

#include <signal.h>
#include <khepera/khepera.h>

/*! 
 * \file   koreio_test.c KoreIO test program
 *
 * \brief 
 *         
 *
 * \author   Pierre Bureau (K-Team SA)
 *
 *
 * \note     Copyright (C) 2004 K-TEAM SA
 * \bug      none discovered.                                         
 * \todo     nothing.
 */

static int quitReq = 0;
static knet_dev_t * koreio;

/*--------------------------------------------------------------------*/
/*!
 * Make sure the program terminate properly on a ctrl-c
 */
static void ctrlc_handler( int sig ) 
{
  quitReq = 1;
}

/*--------------------------------------------------------------------*/
/*! Quit the program.
 */
int quit( int argc , char * argv[] , void * data) 
{
  quitReq = 1;
}

/*--------------------------------------------------------------------*/
/*! Read all the IO states and display them. The first digit displayed
 * is IO0 and the last is IO15. A x is displayed for IOs in output mode
 * and a p is displayed for IOs in pwm mode
 */
int readios( int argc , char * argv[] , void * data)
{

}


int help( int argc , char * argv[] , void * data);
/*--------------------------------------------------------------------*/
/*! The command table contains:
 * command name : min number of args : max number of args : the function to call
 */
static kb_command_t cmds[] = {
  { "quit"            , 0 , 0 , quit } ,
  { "exit"            , 0 , 0 , quit } ,
  { "bye"             , 0 , 0 , quit } ,
  { "help"            , 0 , 0 , help } ,
  { NULL              , 0 , 0 , NULL }
};

/*--------------------------------------------------------------------*/
/*! Display a list of available commands.
 */
int help( int argc , char * argv[] , void * data) 
{
  kb_command_t * scan = cmds;
  while(scan->name != NULL)
  {
    printf("%s\r\n",scan->name);
    scan++;
  }
  return 0;
}

/*--------------------------------------------------------------------*/
/*! Main program to process the command line. 
 *
 */
static char buf[1024];

int main( int argc , char * argv[] )
{
  int rc,ver,io,ad,pw = 0, freq;
  char newpage = 12, state;
  uint16_t val;
  uint32_t time;
  

  /* Set the libkhepera debug level - Highly recommended for development. */
  kb_set_debug_level(2);

  if((rc = kb_init( argc , argv )) < 0 )
    return 1;

  signal( SIGINT , ctrlc_handler );

  printf("K-Team KoreIO Test Program\r\n");
  
  koreio = knet_open( "KoreIOLE:Board", KNET_BUS_ANY, 0 , NULL );
  if(!koreio)
  {
    printf("Cannot open KoreIOLE device trying alternate address\r\n");
    koreio = knet_open( "KoreIOLE:AltBoard", KNET_BUS_ANY, 0 , NULL );
    if(!koreio)
    {
      printf("Cannot open KoreIOLE device\r\n");
      return 1;
    }
  }

  /* Get and display the koreio firmware version */
  kio_GetFWVersion(koreio,&ver);

  printf("KoreIOLE firmware %d.%d\r\n", (ver&0x000000F0)>>4, (ver&0x0000000F));

	/* config all the IO as a output */
  io = 0;
	while(io < 16)
	while(io < 16)
	{
	  kio_ConfigIO(koreio,io,1);
	  io++;
		usleep(1000);
	}
  	/* set one of two Output */
    io = 0;
	while(io < 16)
	{
	  kio_SetIO(koreio,io);
	  io+= 2;
	}	
	  /* clear one of two Output */
    io = 1;
	while(io < 16)
	{
	  kio_ClearIO(koreio,io);
	  io+= 2;
	}	
	
	
  
  /* parse commands */
  while (!quitReq) {
    printf("\n> ");
	  

 /* read all the analog input value */
  ad = 0;
  while(ad < 12)
  {	
  	kio_ReadAnalog(koreio,ad,&val,&time);
  	printf("read ad %d: %u at %lu mS\r\n",ad,val,time);	  
	ad++;
  }

		/* blink the output */
  io = 0;
	while(io < 16)
	{
	  kio_ChangeIO(koreio,io);
	  io++;
	}	



  /* blink the power output */
  kio_ClearPW(koreio,pw);
  if(++pw > 5)
	  pw = 0;
  kio_SetPW(koreio,pw);

  
	  
	  

    /*if ( fgets( buf , sizeof(buf) , stdin ) != NULL ) {
      buf[strlen(buf)-1] = '\0';
      kb_parse_command( buf , cmds , NULL);
    }*/
  /* clear the current page */
      usleep(200000);
	  printf("%c", newpage);

  }

  knet_close( koreio );
}
