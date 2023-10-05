/*--------------------------------------------------------------------
 * koreio_test.c - KoreBot Library - KoreIO Test
 *--------------------------------------------------------------------
 * $Id: koreio_gripper.c,v 1.1 2006/08/18 12:31:15 flambercy Exp $
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

/*--------------------------------------------------------------------*/
/*! Configure the given digital IO 
 *  The first argument is the IO number
 *  The second argument is:
 *    0 to set as input
 *    1 to set as ouput
 *    2 to set as pwm
 */
int configio(int argc, char * argv[], void * data)
{
  unsigned config;
  int rc;
  char * msg[] = {"input", "output", "pwm"};

  rc = kio_ConfigIO(koreio,atoi(argv[1]),atoi(argv[2]));
  
  if(rc == -1)
    printf("wrong io\r\n");
  else if(rc == -2)
    printf("wrong config\r\n");
  else
    printf("Configured io %d in %s mode\r\n",atoi(argv[1]),msg[atoi(argv[2])]);
}

/*--------------------------------------------------------------------*/
/*! Reset the given digital output value
 */
int resetio( int argc, char * argv[], void * data)
{
  int rc;

  rc = kio_ClearIO(koreio,atoi(argv[1]));

  if(rc < 0)
    printf("wrong io\r\n");
  else
    printf("Cleared io: %d\r\n",atoi(argv[1]));
}

/*--------------------------------------------------------------------*/
/*! Change the LED state
 */
int changeled( int argc, char * argv[], void * data)
{
  int rc;

  rc = kio_ChangeLed(koreio,atoi(argv[1]));

  if(rc < 0)
    printf("wrong state\r\n");
  else
    printf("Changed LED state\r\n");
}

/*--------------------------------------------------------------------*/
/*! Change the given digital output value
 */
int changeio( int argc, char * argv[], void * data)
{
  int rc;

  rc = kio_ChangeIO(koreio,atoi(argv[1]));

  if(rc < 0)
    printf("wrong io\r\n");
  else
    printf("Changed io: %d\r\n",atoi(argv[1]));
}

/*--------------------------------------------------------------------*/
/*! Set the given digital output value
 */
int setio( int argc, char * argv[], void * data)
{
  int rc;

  rc = kio_SetIO(koreio,atoi(argv[1]));

  if(rc < 0)
    printf("wrong io\r\n");
  else
    printf("Set io: %d\r\n",atoi(argv[1]));
}

/*--------------------------------------------------------------------*/
/*! Read the given analog input value
 */
int readad( int argc, char * argv[], void * data)
{
  uint16_t val;
  uint32_t time;
  int rc;
  
  rc = kio_ReadAnalog(koreio,atoi(argv[1]),&val,&time);

  if(rc < 0)
    printf("wrong ad\r\n");
  else
    printf("read ad %d: %u at %lu mS\r\n",atoi(argv[1]),val,time);
}


/*--------------------------------------------------------------------*/
/*! Read the can message in the buffer and erase it
 *  */
int readcan(int argc, char * argv[], void * data)
{
    char can_status, len;
    uint32_t can_data1, can_data2;
    uint32_t time, id;
    int rc;
    rc = kio_ReadCAN(koreio,&id,&len,&can_data1,&can_data2,&can_status,&time);

    if(rc == -1)
       printf("Buffer empty\r\n");
    else if(rc == -2)
       printf("Error communication with turret\r\n");
    else
    {
       printf("ID: 0x%lx, len: %d, Data: 0x%lx%lx, status: 0x%x, at %lu mS\r\n",id,len,can_data1, can_data2,can_status,time);
       printf("%d messages remaining in buffer\r\n", rc);
    }
}

/*-------------------------------------------------------------------*/
/*! Write on the CAN bus and change the 7 seg value (work only with the
 * CAN developement kit)
 */
int can7seg(int argc, char * argv[], void * data)
{
#define id 	0x00000400
#define null	0x1E7F4000
#define one	0x1E7F7900
#define two 	0x1E7F2400
#define three 	0x1E7F3000
#define four	0x1E7F1900
#define five 	0x1E7F1200
#define six	0x1E7F0200
#define seven 	0x1E7F7800
#define eight	0x1E7F0000
#define nine	0x1E7F1000
#define can_status  0x02

  int rc;
  uint32_t can_data = 0;
  char len = 3;

  switch (atoi(argv[1]))
  {
    case 0:
      can_data = null;
      break;
    case 1:
      can_data = one;
      break;
    case 2:
      can_data = two;
      break;
    case 3:
      can_data = three;
      break;      
    case 4:
      can_data = four;
      break;      
    case 5:
      can_data = five;
      break;      
    case 6:
      can_data = six;
      break;      
    case 7:
      can_data = seven;
      break;      
    case 8:
      can_data = eight;
      break;      
    default:
      can_data = nine;
      break;      
}
  
  rc = kio_SendCAN(koreio, id, can_data, can_data, len, can_status);

  if(rc != 0)
    printf("CAN write failed\r\n");
  else
    printf("Number %d set on the 7 seg\r\n",atoi(argv[1]));
}

/*-------------------------------------------------------------------*/
/*! Change the state of the three led on the CAN developpement kit
 */
int canled( int argc, char * argv[], void *data)
{
#define id_led     0x00000300
#define can_data1  0x1E

  int rc;
  uint32_t can_data = 0;
  char len = 3;
  char status = 0x02;
  char on_off = 0xFF;

  if (atoi(argv[2]))
    on_off = 0;

  can_data = (((((can_data | can_data1) << 8) | atoi(argv[1])) << 8) | on_off) << 8;
  rc = kio_SendCAN(koreio, id_led, can_data, can_data, len, can_status);

  if(rc != 0)
     printf("CAN write failed\r\n");
  else
     printf("Leds %d set\r\n",atoi(argv[1]));
  
}


/*-------------------------------------------------------------------*/
/*! Change the given analog output value
 */
int setad( int argc, char * argv[], void * data)
{
  int rc;
  float val;

  rc = kio_SetANValue(koreio,atoi(argv[1]), atoi(argv[2]));

  val = (atoi(argv[2]) - 139) *0.035;
  if(rc < 0)
    printf("wrong analog output\r\n");
  else
    printf("ananlog output %d: %f V\r\n",atoi(argv[1]), val);

}

/*--------------------------------------------------------------------*/
/*! Set the PWM channel ratio
 */
int setratio( int argc, char * argv[], void * data)
{
  int rc;

  rc = kio_ChangePWM_ratio(koreio, atoi(argv[1]), atoi(argv[2]));

  if(rc < 0)
    printf("wrong PWM output\r\n");
  else
    printf("channel %d set to %u%%\r\n",atoi(argv[1]), atoi(argv[2]));
}

/*--------------------------------------------------------------------*/
/*! Move all the axes of the gripper 3 times 
 */
int sequence( int argc, char * argv[], void * data)
{
  int rc, val;

  val = 50 + 82;
  rc = kio_ChangePWM_ratio(koreio, 0 , val);
  val = (255 - val) + 82 ;
  rc = kio_ChangePWM_ratio(koreio, 1 , val);


  val = 123 + 82;
  rc = kio_ChangePWM_ratio(koreio, 2 , val);
  val = (255 - val) + 82 ;
  rc = kio_ChangePWM_ratio(koreio, 3 , val);

  val = 100 + 82;
  rc = kio_ChangePWM_ratio(koreio, 4 , val);
  val = (255 - val) + 82 ;
  rc = kio_ChangePWM_ratio(koreio, 5 , val);

  val = 0;
  rc = kio_ChangePWM_ratio(koreio, 6 , val);

  val = 0;
  rc = kio_ChangePWM_ratio(koreio, 7 , val);

////////////////////////////////////////////

  usleep(2000000);
  val = 82;
  rc = kio_ChangePWM_ratio(koreio, 4 , val);
  val = (255 - val) + 82 ;
  rc = kio_ChangePWM_ratio(koreio, 5 , val);

  usleep(200000);
  
 val = 8+ 82;
  rc = kio_ChangePWM_ratio(koreio, 0 , val);
  val = (255 - val) + 82 ;
  rc = kio_ChangePWM_ratio(koreio, 1 , val);


  val = 255;
  rc = kio_ChangePWM_ratio(koreio, 2 , val);
  val = (255 - val) + 82 ;
  rc = kio_ChangePWM_ratio(koreio, 3 , val);

  val = 0;
  rc = kio_ChangePWM_ratio(koreio, 6 , val);

  val = 0;
  rc = kio_ChangePWM_ratio(koreio, 7 , val);

////////////////////////////////////////////

  usleep(2000000);

 val = 170 + 82;
  rc = kio_ChangePWM_ratio(koreio, 0 , val);
  val = (255 - val) + 82 ;
  rc = kio_ChangePWM_ratio(koreio, 1 , val);


  val = 255;
  rc = kio_ChangePWM_ratio(koreio, 2 , val);
  val = (255 - val) + 82 ;
  rc = kio_ChangePWM_ratio(koreio, 3 , val);

  val = 170 + 82;
  rc = kio_ChangePWM_ratio(koreio, 4 , val);
  val = (255 - val) + 82 ;
  rc = kio_ChangePWM_ratio(koreio, 5 , val);

  val = 0;
  rc = kio_ChangePWM_ratio(koreio, 6 , val);

  val = 0;
  rc = kio_ChangePWM_ratio(koreio, 7 , val);

////////////////////////////////////////////



  if(rc < 0)
    printf("wrong PWM output\r\n");
  else
    printf("Gripper moved");
}

/*--------------------------------------------------------------------*/
/*! Move all the axes of the gripper 
 */
int gripper( int argc, char * argv[], void * data)
{
  int rc, val;

  val = atoi(argv[1]);
  if (val > 173) val = 255;
  else if (val < 0) val = 82;
  else val += 82;
  rc = kio_ChangePWM_ratio(koreio, 0 , val);
  
  val = (255 - val) + 82 ;
  rc = kio_ChangePWM_ratio(koreio, 1 , val);


  val = atoi(argv[2]);
  if (val > 173) val = 255;
  else if (val < 0) val = 82;
  else val += 82;
  rc = kio_ChangePWM_ratio(koreio, 2 , val);
  
  val = (255 - val) + 82 ;
  rc = kio_ChangePWM_ratio(koreio, 3 , val);

  val = atoi(argv[3]);
  if (val > 173) val = 255;
  else if (val < 0) val = 82;
  else val += 82;
  rc = kio_ChangePWM_ratio(koreio, 4 , val);
  
  val = (255 - val) + 82 ;
  rc = kio_ChangePWM_ratio(koreio, 5 , val);

  val = atoi(argv[4]);
  if (val > 255) val = 255;
  else if (val < 55) val = 55;
  rc = kio_ChangePWM_ratio(koreio, 6 , val);

  val = atoi(argv[5]);
  if (val > 255) val = 255;
  else if (val < 55) val = 55;
  rc = kio_ChangePWM_ratio(koreio, 7 , val);




  if(rc < 0)
    printf("wrong PWM output\r\n");
  else
    printf("Gripper moved");
}



/*--------------------------------------------------------------------*/
/*! Move the two servo of the shoulder   
 */
int shoulder( int argc, char * argv[], void * data)
{
  int rc, val;

  val = atoi(argv[1]);
  if (val > 173) val = 255;
  else if (val < 0) val = 82;
  else val += 82;
  rc = kio_ChangePWM_ratio(koreio, 0 , val);
  
  val = (255 - val) + 82 ;
  rc = kio_ChangePWM_ratio(koreio, 1 , val);



  if(rc < 0)
    printf("wrong PWM output\r\n");
  else
    printf("poignet set to  %u\r\n",val);
}


/*--------------------------------------------------------------------*/
/*! Move the two servo of the elbow
 */
int elbow( int argc, char * argv[], void * data)
{
  int rc, val;

  val = atoi(argv[1]);
  if (val > 173) val = 255;
  else if (val < 0) val = 82;
  else val += 82;
  rc = kio_ChangePWM_ratio(koreio, 2 , val);
  
  val = (255 - val) + 82 ;
  rc = kio_ChangePWM_ratio(koreio, 3 , val);



  if(rc < 0)
    printf("wrong PWM output\r\n");
  else
    printf("poignet set to  %u\r\n",val);
}



/*--------------------------------------------------------------------*/
/*! Move the two servo of the wrist 
 */
int wrist( int argc, char * argv[], void * data)
{
  int rc, val;

  val = atoi(argv[1]);
  if (val > 173) val = 255;
  else if (val < 0) val = 82;
  else val += 82;
  rc = kio_ChangePWM_ratio(koreio, 4 , val);
  
  val = (255 - val) + 82 ;
  rc = kio_ChangePWM_ratio(koreio, 5 , val);



  if(rc < 0)
    printf("wrong PWM output\r\n");
  else
    printf("poignet set to  %u\r\n",val);
}

/*--------------------------------------------------------------------*/
/*! Move the servo of the fingers's rotation
 */
int rotate( int argc, char * argv[], void * data)
{
  int rc, val ;

  val = atoi(argv[1]);
  if (val > 255) val = 255;
  else if (val < 55) val = 55;
  rc = kio_ChangePWM_ratio(koreio, 6 , val);

  if(rc < 0)
    printf("wrong PWM output\r\n");
  else
    printf("rot set to  %u\r\n",atoi(argv[1]));
}
/*--------------------------------------------------------------------*/
/*! Open or close the fingers
 */
int finger( int argc, char * argv[], void * data)
{
  int rc, val;

  val = atoi(argv[1]);
  if (val > 255) val = 255;
  else if (val < 55) val = 55;
  rc = kio_ChangePWM_ratio(koreio, 7 , val );

  if(rc < 0)
    printf("wrong PWM output\r\n");
  else
    printf("pince set to  %u\r\n",atoi(argv[1]));
}







/*--------------------------------------------------------------------*/
/*! Set the PWM channel frequence
 */
int setfreq(int argc, char * argv[], void * data)
{
  int rc;

  rc = kio_ChangePWM_freq(koreio, atoi(argv[1]));

  if(rc < 0)
    printf("Wrong value\r\n");
  else
    printf("Frequence set to %lu Hz\r\n", atoi(argv[1]));
}

/*--------------------------------------------------------------------*/
/*! Read the given IO state
 */
int readio( int argc, char * argv[], void * data)
{
  int rc;
  
  rc = kio_ReadIO(koreio,atoi(argv[1]));

  if(rc < 0)
    printf("wrong io or not in input mode\r\n");
  else
    printf("read io %d: %d\r\n",atoi(argv[1]),rc);
}

/*--------------------------------------------------------------------*/
/*! Change the given power output value
 */
int changepw( int argc, char * argv[], void * data)
{
  int rc;

  rc = kio_ChangePW(koreio,atoi(argv[1]));

  if(rc < 0)
    printf("wrong pw\r\n");
  else
    printf("Changed pw: %d\r\n",atoi(argv[1]));
}

/*--------------------------------------------------------------------*/
/*! Set the given power output value
 */
int setpw( int argc, char * argv[], void * data)
{
  int rc;

  rc = kio_SetPW(koreio,atoi(argv[1]));

  if(rc < 0)
    printf("wrong pw\r\n");
  else
    printf("Set pw: %d\r\n",atoi(argv[1]));
}

/*--------------------------------------------------------------------*/
/*! read on the secondary I2C bus n_times (max 4)
 */
int readi2c(int argc, char * argv[], void * data)
{
  int rc;
  uint32_t i2c_values;
  char temp;
  char i;
  char n_read;
  
  kio_i2c_StartRead(koreio,atoi(argv[1]), atoi(argv[2]), atoi(argv[3]));

  usleep(10000);

  rc = kio_i2c_ReturnRead(koreio,atoi(argv[3]), &i2c_values);

  n_read = atoi(argv[3]);
  if (n_read > 4)
    n_read = 4;
  
  for(i = 0; i < n_read; i++)
  { 
    temp = i2c_values;    
    printf("Read n°%d : 0x%x\r\n",i,temp);
    i2c_values = i2c_values >> 8;
  }
  
}
  
/*--------------------------------------------------------------------*/
/*! Write on the secondary I2C bus
 */
int writei2c(int argc, char * argv[], void * data)
{
  
  kio_i2c_Write(koreio, atoi(argv[1]), atoi(argv[2]), atoi(argv[3]));

  printf("Write 0x%x on device 0x%x at register 0x%x\r\n", atoi(argv[3]), atoi(argv[1]), atoi(argv[2]));
}


/*--------------------------------------------------------------------*/
/*! Start an address Scan on the secondary I2C bus. The list of answering
 * devices must be consulted using readlist.
 */
int startscan(int argc, char * argv[], void * data)
{
  kio_i2c_StartScan(koreio);
  printf("I2C address scan started\r\n");
}

/*--------------------------------------------------------------------*/
/*!
 */
int listscan(int argc, char * argv[], void * data)
{
  char addlist[128];
  int rc,i;
  
  rc = kio_i2c_ListScan(koreio, addlist);
  
  if(rc < 0)
    printf("List scan error\r\n");
  else
    printf("Secondary bus scan: %d device found\r\n",rc);

  for(i=0; i<rc; i++)
    printf("device %d: 0x%x\r\n",i,addlist[i]);

}

/*--------------------------------------------------------------------*/
/*! Reset the given power output value
 */
int resetpw( int argc, char * argv[], void * data)
{
  int rc;

  rc = kio_ClearPW(koreio,atoi(argv[1]));

  if(rc < 0)
    printf("wrong pw\r\n");
  else
    printf("Cleared pw: %d\r\n",atoi(argv[1]));
}

/*--------------------------------------------------------------------*/
/*! Handle the Analog measurement timestamp timer
 * Argument 1 is:
 * 		  0 to reset the timer
 * 		  1 to stop the timer
 * 		  2 to launch the timer
 */
int timer( int argc, char * argv[], void * data)
{
  int rc;
  char * msg[] = {"reset", "stopped", "started"};

  rc = kio_Timer(koreio,atoi(argv[1]));

  if(rc < 0)
    printf("wrong action\r\n");
  else
    printf("Timer is %s\r\n",msg[atoi(argv[1])]);
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
  { "sequence"        , 0 , 0 , sequence},
  { "gripper"         , 5 , 5 , gripper},
  { "shoulder"        , 1 , 1 , shoulder },
  { "elbow"           , 1 , 1 , elbow },
  { "wrist"           , 1 , 1 , wrist },
  { "rotate"          , 1 , 1 , rotate },
  { "finger"          , 1 , 1 , finger },
  { "readios"         , 0 , 0 , readios },
  { "readio"          , 1 , 1 , readio },
  { "setio"           , 1 , 1 , setio },
  { "cleario"         , 1 , 1 , resetio },
  { "setratio"	      , 2 , 2 , setratio },
  { "setfreq"	      , 1 , 1 , setfreq },
  { "setpw"           , 1 , 1 , setpw },
  { "clearpw"         , 1 , 1 , resetpw },
  { "changepw"        , 1 , 1 , changepw },
  { "changeio"        , 1 , 1 , changeio },
  { "changeled"       , 1 , 1 , changeled },
  { "configio"        , 2 , 2 , configio },
  { "readad"          , 1 , 1 , readad },
  { "setad"	      , 2 , 2 , setad },
  { "startscan"       , 0 , 0 , startscan},
  { "listscan"        , 0 , 0 , listscan},
  { "readi2c" 	      , 3 , 3 , readi2c },
  { "writei2c"	      , 3 , 3 , writei2c },
  { "readcan"	      , 0 , 0 , readcan},
  { "can7seg"	      , 1 , 1 , can7seg},
  { "canled"          , 2 , 2 , canled},
  { "timer"           , 1 , 1 , timer},
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

  /* Set the libkhepera debug level - Highly recommended for development. */
  kb_set_debug_level(2);

  if((rc = kb_init( argc , argv )) < 0 )
    return 1;

  signal( SIGINT , ctrlc_handler );

  printf("K-Team KoreIO Test Program\r\n");
  
  koreio = knet_open( "KoreIO:Board", KNET_BUS_ANY, 0 , NULL );
  if(!koreio)
  {
    printf("Cannot open KoreIO device trying alternate address\r\n");
    koreio = knet_open( "KoreIO:AltBoard", KNET_BUS_ANY, 0 , NULL );
    if(!koreio)
    {
      printf("Cannot open KoreIO device\r\n");
      return 1;
    }
  }

  /* Get and display the koreio firmware version */
  kio_GetFWVersion(koreio,&ver);

  printf("KoreIO firmware %d.%d\r\n", (ver&0x000000F0)>>4, (ver&0x0000000F));

  /* Init of the servo for the gripper */
  kio_ChangePWM_freq(koreio, 0);
/*  kio_ChangePWM_ratio(koreio, 0, 82);
  kio_ChangePWM_ratio(koreio, 1, 255);
  kio_ChangePWM_ratio(koreio, 2, 82);
  kio_ChangePWM_ratio(koreio, 3, 255);
  kio_ChangePWM_ratio(koreio, 4, 82);
  kio_ChangePWM_ratio(koreio, 5, 255);
  kio_ChangePWM_ratio(koreio, 6, 255);
  kio_ChangePWM_ratio(koreio, 7, 255);
*/

  kio_ConfigIO(koreio,0,2);
  kio_ConfigIO(koreio,1,2);
  kio_ConfigIO(koreio,2,2);
  kio_ConfigIO(koreio,3,2);
  kio_ConfigIO(koreio,4,2);
  kio_ConfigIO(koreio,5,2);
  kio_ConfigIO(koreio,6,2);
  kio_ConfigIO(koreio,7,2);
 


  
  /* parse commands */
  while (!quitReq) {
    printf("\n> ");

    if ( fgets( buf , sizeof(buf) , stdin ) != NULL ) {
      buf[strlen(buf)-1] = '\0';
      kb_parse_command( buf , cmds , NULL);
    }
  }

  knet_close( koreio );
}
