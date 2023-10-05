/*--------------------------------------------------------------------
 * gpio_test_all.c - Khepera Library - GPIO Test
 *--------------------------------------------------------------------
 * $Id: gpio_test_all.c,v 1.2 2011/11/18 14:51:53 jtharin Exp $
 *--------------------------------------------------------------------
 * $Author: jtharin $
 * $Date: 2011/11/18 14:51:53 $
 * $Revision: 1.0 $
 *--------------------------------------------------------------------*/
#include <signal.h>
#include <khepera/khepera.h>

#include <math.h>    // round (add -std=gnu99 to Makefile)
#include <unistd.h>  // usleep


static int quitReq = 0;

const int FREQUENCIES [] = {16384, 50000}; // (max PWM0: 16384 @ and 50% ; PWM1: 7500000 and 50%);
const int DUTIES [] = {50,25};


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
/*! Configure the given digital IO 
 *  The first argument is the IO number
 *  The second argument is:
 *    1 to set as input
 *    0 to set as output
 */
int configio(int argc, char * argv[], void * data)
{

	printf("Configure GPIO %d for %s\n",atoi(argv[1]),atoi(argv[2])?"input":"output");

	/* Set gpio to IO mode */
	kb_gpio_function(atoi(argv[1]),0);

  kb_gpio_dir(atoi(argv[1]),atoi(argv[2]));
}

/*--------------------------------------------------------------------*/
/*! Reset the given digital output value
 */
int resetio( int argc, char * argv[], void * data)
{
   kb_gpio_clear(atoi(argv[1]));

}




/*--------------------------------------------------------------------*/
/*! Set the given digital output value
 */
int setio( int argc, char * argv[], void * data)
{

  kb_gpio_set(atoi(argv[1]));

}

/*--------------------------------------------------------------------*/
/*! Read the given digital input value
 */
int readio( int argc, char * argv[], void * data)
{

  int rc;
  
  rc = kb_gpio_get(atoi(argv[1]));

  if(rc < 0)
    printf("wrong io or not in input mode (rc = %d)\r\n",rc);
  else
    printf("read io %d: %d\r\n",atoi(argv[1]),rc);

}

/*--------------------------------------------------------------------*/
/*! Test all the pwms
 */
int pwms_test_on( int argc, char * argv[], void * data)
{

  int rc,i;
  

  
  for (i=0;i<2;i++)
  {

			rc = kb_pwm_period(i,FREQUENCIES[i]); 
						
			if(rc != 0)
				printf("\nERROR: Cannot set frequency %d Hz to pwm %d!\n",FREQUENCIES[i],i);
			else
			{

				rc = kb_pwm_duty(i,DUTIES[i]); 

				if(rc != 0) {
					printf("\nERROR: Cannot set duty cycle %d %% to pwm %d!\n",DUTIES[i],i);
				} else
				{
					rc = kb_pwm_activate(i);
		

					if(rc != 0)
						printf("\nERROR: Cannot activate pwm %d!\n",i);
					else
					{
						printf("pwm %d:   ->  duty %3d%%  freq %7d\n",i,DUTIES[i], FREQUENCIES[i]);
					}	
					
					
					
			}
		}

  }
 
    
}

/*--------------------------------------------------------------------*/
/*! Stop all the pwms
 */
int pwms_test_off( int argc, char * argv[], void * data)
{

  int rc,i;
  
  for (i=0;i<2;i++)
  {

		rc = kb_pwm_desactivate(i);

		if(rc != 0)
		  printf("\nERROR: Cannot desactivate pwm %d!\n",i);
	

  }
 
    
}


/*--------------------------------------------------------------------*/
/*! Configure all the DIO for 0 output or 1 input
 */
int configio_all( int argc, char * argv[], void * data)
{

  int i;
  
  printf("Configure GPIO %d to %d for %s\n",GPIO_FIRST,GPIO_LAST,atoi(argv[1])?"input":"output");
  
  for (i=GPIO_FIRST;i<=GPIO_LAST;i++)
  {

		/* Set gpio i to IO mode */
		kb_gpio_function(i,0);

		// set direction to output 
		kb_gpio_dir(i,atoi(argv[1]));	

  }     
}


/*--------------------------------------------------------------------*/
/*! set all the DIO to 1
 */
int setio_all( int argc, char * argv[], void * data)
{

  int i;
  
  for (i=GPIO_FIRST;i<=GPIO_LAST;i++)
  {		
		kb_gpio_set(i);
  }     
}

/*--------------------------------------------------------------------*/
/*! clear all the DIO to 0
 */
int cleario_all( int argc, char * argv[], void * data)
{

  int i;
  
  for (i=GPIO_FIRST;i<=GPIO_LAST;i++)
  {
		kb_gpio_clear(i);

  }     
}


/*--------------------------------------------------------------------*/
/*! read all IO */
int readio_all( int argc, char * argv[], void * data)
{

  int i;
 
  kb_change_term_mode(1);
  
  kb_clrscr();
  
  
  printf("Push anykey to stop!\n\n");
  
  while(! kb_kbhit())
  {
  	
		for (i=GPIO_FIRST;i<=GPIO_LAST;i++)
		{

			
			if (i<GPIO_FIRST+10)
				printf("\033[%d;%dH",(i-GPIO_FIRST)/2+2,0);            // Move the cursor to line and colomn .
			else // add a line between the 2 first groups
			if (i<GPIO_FIRST+16)
					printf("\033[%d;%dH",(i-GPIO_FIRST)/2+2+1,0);            // Move the cursor to line and colomn .
		  else // add a line between for the last groups
		  		printf("\033[%d;%dH",(i-GPIO_FIRST)/2+2+2,0);            // Move the cursor to line and colomn .
		  
		  i++; // print odd first 
			printf(" gpios: %d:%d | ",i,kb_gpio_get(i));
			i--;
			printf("%d:%d\n",i,kb_gpio_get(i));
			i++;
				
		}
		usleep(200000);
  } 
  
   kb_change_term_mode(0);    
}


/*--------------------------------------------------------------------*/
/*! Activate the given pwm 
 */
int pwm_on( int argc, char * argv[], void * data)
{

  int rc;
  
  rc = kb_pwm_activate(atoi(argv[1]));

  if(rc != 0)
    printf("wrong pwm (error: %d)\r\n",rc);
   

}

/*--------------------------------------------------------------------*/
/*! Desactivate the given pwm 
 */
int pwm_off( int argc, char * argv[], void * data)
{

  int rc;
  
  rc = kb_pwm_desactivate(atoi(argv[1]));
  if(rc != 0)
    printf("wrong pwm\r\n");  

}

/*--------------------------------------------------------------------*/
/*! Change the ratio of the given pwm 
 */
int pwm_ratio( int argc, char * argv[], void * data)
{

  int rc;
  
  rc = kb_pwm_duty(atoi(argv[1]),atoi(argv[2]));

  if(rc != 0)
    printf("wrong pwm\r\n");
  

}

/*--------------------------------------------------------------------*/
/*!  Change the frequency the given pwm 
 */
int pwm_freq( int argc, char * argv[], void * data)
{

  int rc;
  
  rc = kb_pwm_period(atoi(argv[1]), atoi(argv[2]));

  if(rc != 0)
    printf("wrong pwm\r\n");
  
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
  { "readio"          , 1 , 1 , readio },
  { "setio"           , 1 , 1 , setio },
  { "cleario"         , 1 , 1 , resetio },
  { "configio"        , 2 , 2 , configio },
  { "pwms_test_on"        , 0 , 0, pwms_test_on },
  { "pwms_test_off"        , 0 , 0, pwms_test_off },
  { "configio_all"        , 1 , 1 , configio_all },
  { "setio_all"        , 0 , 0, setio_all },
  { "cleario_all"        , 0 , 0, cleario_all },
  { "readio_all"        , 0 , 0, readio_all},
  { "pwm_on"        , 1 , 1 , pwm_on },
  { "pwm_off"        , 1 , 1 , pwm_off },
  { "pwm_ratio"        , 2 , 2 , pwm_ratio},
  { "pwm_freq"        , 2 , 2 , pwm_freq },
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
  int rc,ver;

  /* Set the libkorebot debug level - Highly recommended for development. */
  kb_set_debug_level(2);

  if((rc = kb_init( argc , argv )) < 0 )
    return 1;

  signal( SIGINT , ctrlc_handler );

  printf("K-Team GPIO Test Program 2\r\n");
  
  /* Initialize the GPIO and the PWM modules */
  kb_pwm_init();
  
  kb_gpio_init();
  
  /* parse commands */
  while (!quitReq) {
    printf("\n> ");

    if ( fgets( buf , sizeof(buf) , stdin ) != NULL ) {
      buf[strlen(buf)-1] = '\0';
      kb_parse_command( buf , cmds , NULL);
    }
  }

}
