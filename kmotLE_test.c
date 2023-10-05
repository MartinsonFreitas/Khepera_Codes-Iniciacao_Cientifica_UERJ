/*--------------------------------------------------------------------
 * kmot_test.c - KoreBot Library - KoreMotor Test
 *--------------------------------------------------------------------
 * $Id: kmotLE_test.c,v 1.1 2006/10/27 08:41:10 flambercy Exp $
 *--------------------------------------------------------------------
 * $Author: flambercy $
 * $Date: 2006/10/27 08:41:10 $
 * $Revision: 1.1 $
 *--------------------------------------------------------------------*/

#include <signal.h>
#include <khepera/khepera.h>

/*! 
 * \file   kmot_test.c KoreMotor utility program
 *
 * \brief 
 *         kmot_test provides a small utility program to communicate with the
 *         the KoreMotor and send commands to the motor controllers. Use the              
 *         help command to get a list of available controls.
 *         
 *
 * \author   Pierre Bureau (K-Team SA)
 * \author   Cédric Gaudin (K-Team SA)                               
 *
 *
 * \note     Copyright (C) 2004 K-TEAM SA
 * \bug      none discovered.                                         
 * \todo     nothing.
 */

static int quitReq = 0;
static int stopReq = 0;
static int currentCommand = 0;

static FILE * logfile;
static knet_dev_t * motor;
static pthread_t log_task;

/*!
 * Make sure the program terminate properly on a ctrl-c
 */
static void ctrlc_handler( int sig ) 
{
  stopReq = 1;
}

/*--------------------------------------------------------------------*/
/*! Quit the program.
 */
int quit( int argc , char * argv[] , void * data) 
{
  quitReq = 1;
}

/*--------------------------------------------------------------------*/
/*! Stop the current motor (set mode to stop motor mode).
 */
int stop( int argc , char * argv[] , void * data) 
{
  kmot_SetMode( motor , kMotModeStopMotor );
}

/*--------------------------------------------------------------------*/
/*! Set the SampleTime register (refer to KoreMotor documentation).
 *  syntax: setsampletime sampletime
 */ 
int setsampletime( int argc , char * argv[] , void * data)
{
  int sampletime = atoi( argv[1] );

  printf("Set sample time to %d\n" , sampletime ); 
  
  kmot_SetSampleTime( motor , sampletime );
}

/*--------------------------------------------------------------------*/
/*! Set the speed profile for the current controller. The speed profile
 * is only used for the regSpeedProfile mode (refer to KoreMotor documentation).
 * syntax: setspeedprofile "max speed" acceleration
 */
int configspeedprofile( int argc , char * argv[] , void * data)
{
  int vmax , accel;

  vmax = atoi(argv[1]);
  accel = atoi(argv[2]);

  printf("Set speed profile (Vmax,accel) = (%d,%d)\n" , vmax , accel );
  kmot_SetSpeedProfile( motor , vmax , accel );
}

/*--------------------------------------------------------------------*/
/*! Set the PID controller gains for the given motor.
 * syntax: setpid "regulation type" Kp Ki Kd
 *
 * regulation types are: pos, posprofile, speed, speedprofile, and torque.
 */
int setpid( int argc , char * argv[] , void * data)
{
  int regType[] = { 
    kMotRegPos , 
    kMotRegPosProfile , 
    kMotRegSpeed ,
    kMotRegSpeedProfile ,
    kMotRegTorque 
  };

  char * regTypeStr[] = { 
    "pos" , 
    "posprofile" , 
    "speed" , 
    "speedprofile" ,
    "torque" ,
    NULL 
  };

  int reg, kp, ki, kd;
  
  for (reg=0; regTypeStr[reg] != NULL; reg++) {
    if (!strcasecmp( argv[1] , regTypeStr[reg] )) {
      
      kp = atoi(argv[2]);
      ki = atoi(argv[3]);
      kd = atoi(argv[4]);

      printf("Set (P,I,D) to (%d,%d,%d)\n" , kp , ki , kd );
      kmot_ConfigurePID( motor , regType[reg] , kp , kd , ki );
    }
  }
}



#define timercpy(a,b) \
 (a)->tv_sec = (b)->tv_sec; \
 (a)->tv_usec = (b)->tv_usec; 

/*--------------------------------------------------------------------*/
/*! The data logging task
 */
void * kmot_log_task(void * arg)
{
  int32_t position,speed,current;

  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
  
  logfile = fopen(arg,"w+");
  if(!logfile)
  {
    fprintf(stderr,"Error: Unable to open %s, no data logging.\r\n",arg);
    pthread_exit(NULL);
  }
  printf("Opened logfile: %s\r\n",arg);

  //gettimeofday(&clocksav,&timez);

  while(1)
  {
#if 0
    do
    {
      gettimeofday(&clock,&timez);
      timersub(&clock,&clocksav,&clockdif);
    }
    while(clockdif.tv_usec < 500);

    timercpy(&clocksav,&clock);
#endif
    fprintf(logfile,"%d\t",currentCommand);

    /* Read the motor measurements continuously */
    speed = kmot_GetMeasure(motor, kMotMesSpeed);
    fprintf(logfile,"%ld\t",speed);
    position = kmot_GetMeasure(motor, kMotMesPos);
    fprintf(logfile,"%ld\t",position);
    current = kmot_GetMeasure(motor, kMotMesCurrent);
    fprintf(logfile,"%ld\r\n",current);
    //printf("set: %7ld ",kmot_I2cRead32(motor,MOT_SetPointLL));
    //kmot_GetStatus(motor,&status,&erreur);
    //printf("err: 0x%x stat: 0x%x\r\n",erreur,status);
    //i2c_read8(&i2c,0x61,0x20,&val);
    //printf("sens 0x%x\r\n",val);
    //printf("mode: %x\r\n",kmot_I2cRead(&motor,0x28));
    fflush(logfile);
  }

}

/*--------------------------------------------------------------------*/
/*! Stop logging motor measurements.
 */
int stoplog(int argc, char * argv[], void * data)
{
  if(pthread_cancel(log_task))
  {
    KB_FATAL("stoplog", KB_ERROR_PTHREAD, "kmot_log_task");
  }
}

/*--------------------------------------------------------------------*/
/*! Start logging motor measurements to the given file.
 */
int startlog(int argc, char * argv[], void * data)
{
  const char * defaultfile = "motorlog.dat";

  if(argc == 2)
  {
    if(pthread_create(&log_task, NULL, kmot_log_task, (void*)argv[1]))
    {
      KB_FATAL("startlog", KB_ERROR_PTHREAD, "kmot_log_task");
    }
  }
  else
    if(pthread_create(&log_task, NULL, kmot_log_task, (void*)defaultfile))
    {
      KB_FATAL("startlog", KB_ERROR_PTHREAD, "kmot_log_task");
    }
}

/*--------------------------------------------------------------------*/
/*! Initialize all the parameters for the current controller. This command 
 * must be called before using any other command, and the parameters should 
 * be modified for the type of motors and encoder used before using the program.
 */
int init( int argc , char * argv[] , void * data) 
{
  kmot_SetMode( motor , kMotModeIdle );
  kmot_SetSampleTime( motor , 1550);
  kmot_SetMargin( motor , 6 );
  kmot_SetOptions( motor , 0x0 , kMotSWOptWindup | kMotSWOptStopMotorBlk );

  kmot_ResetError( motor );
  kmot_SetBlockedTime( motor , 10 );
  kmot_SetLimits( motor , kMotRegCurrent , 0 , 500 );
  kmot_SetLimits( motor , kMotRegPos , -10000 , 10000 );

  /* PID  */
  kmot_ConfigurePID( motor , kMotRegSpeed , 768 , 0 , 18 );
  kmot_ConfigurePID(motor,kMotRegPos,70,50,10);

  kmot_SetSpeedProfile(motor,30,10);
  return 0;
}

/*--------------------------------------------------------------------*/
/*! Set a new speed regulation target for the current motor. The 
 * controller must be properly initialized before using regulation.
 * syntax: setspeed "target speed"
 */
int setspeed( int argc , char * argv[] , void * data) 
{
  int speed = atoi(argv[1]);
  
  currentCommand = speed;
  kmot_SetPoint( motor , kMotRegSpeed , speed);
  return 0;
}

/*--------------------------------------------------------------------*/
/*! Set a new position regulation target for the current motor. The
 * position will be reached using the speed profile and speed PID. The
 * controller must be properly initialized before using regulation.
 * syntax: setpos "target poistion"
 */

int setspeedprofile( int argc , char * argv[] , void * data)
{
   int speed = atoi(argv[1]);

   currentCommand = speed;
   kmot_SetPoint( motor , kMotRegSpeedProfile , speed);

   return 0;
}





/*--------------------------------------------------------------------*/
/*! Set a new velocity reductionprescaler for the current motor. The
 * controller must be properly initialized before using regulation.
 * syntax: velprescaler "value[0..3]"
 * - 0   = 1:1 
 * - 1   = 1:4
 * - 2   = 1:16
 * - 3   = 1:64
 */
int velprescaler( int argc , char * argv[] , void * data)
{
  int prescale = atoi(argv[1]);

  currentCommand = prescale;
  kmot_SetVelocityPrescale( motor , prescale);
  return 0;
}

/*--------------------------------------------------------------------*/
/*! Set a new speed prescaler for the current motor. The 
 * controller must be properly initialized before using regulation.
 * syntax: prescaler "value[0..3]"
 */
int prescaler( int argc , char * argv[] , void * data) 
{
  int prescale = atoi(argv[1]);
  
  currentCommand = prescale;
  kmot_SetPrescale( motor , prescale);
  return 0;
}

/*--------------------------------------------------------------------*/
/*! Set a new speed multiplierr for the current motor. The
 * controller must be properly initialized before using regulation.
 * syntax: multiplier "value[1..32768]"
 */
int setmultiplier( int argc , char * argv[] , void * data)
{
  int mult = atoi(argv[1]);

  currentCommand = mult;
  
  kmot_SetSpeedMultiplier( motor, mult );
  return 0;
}

/*--------------------------------------------------------------------*/
/*! Get the speed multiplier for the current motor. The
 * controller must be properly initialized before using regulation.
 * syntax: getmultiplier
 */
int multiplier( int argc, char * argv[], void * data )
{
  short mult;

  kmot_GetSpeedMultiplier( motor ,  &mult );

  printf("speed multiplier = %d\r\n", mult);
}
	

/*--------------------------------------------------------------------*/
/*! Set a new torque regulation target for the current motor. The 
 * controller must be properly initialized before using regulation.
 * syntax: settorque "target torque"
 */
int settorque( int argc , char * argv[] , void * data) 
{
  int torque = atoi(argv[1]);
  
  currentCommand = torque;
  kmot_SetPoint( motor , kMotRegTorque , torque);
  return 0;
}

/*--------------------------------------------------------------------*/
/*! Set a new open loop command for the current motor. The 
 * controller must be properly initialized.
 * syntax: openloop command
 */
int openloop( int argc , char * argv[] , void * data) 
{
  int loop = atoi(argv[1]);

  currentCommand = loop;
  kmot_SetPoint( motor , kMotRegOpenLoop, loop);
  return 0;
}


/*--------------------------------------------------------------------*/
/*! Set a new position regulation target for the current motor. The 
 * controller must be properly initialized before using regulation.
 * syntax: setpos "target position"
 */
int setpos( int argc , char * argv[] , void * data) 
{
  kmot_SetPosition( motor , atoi(argv[1]) );

  return 0;
}

/*--------------------------------------------------------------------*/
/*! Set a new position regulation target for the current motor. The 
 * position will be reached using the speed profile and speed PID. The 
 * controller must be properly initialized before using regulation.
 * syntax: setpos "target position"
 */

int moveat( int argc , char * argv[] , void * data) 
{
  int pos = atoi(argv[1]);

  currentCommand = pos;
  //kmot_SetPoint( motor , kMotRegPosProfile , pos);
  kmot_SetPoint( motor , kMotRegPos, pos);

  return 0;
}

/*--------------------------------------------------------------------*/
/*! Set a new position regulation target for the current motor. The
 * position will be reached using the speed profile and speed PID. The
 * controller must be properly initialized before using regulation.
 * syntax: setpos "target position"
 */

int moveatprofile( int argc , char * argv[] , void * data)
{
   int pos = atoi(argv[1]);

   currentCommand = pos;
   kmot_SetPoint( motor , kMotRegPosProfile , pos);

   return 0;
}


/*--------------------------------------------------------------------*/
/*! Get a set of measures from the current motor. This control returns
 * the speed, position and current for the motor.
 */
int measure( int argc , char * argv[] , void * data) 
{
  int v[3];
  
  v[0] = kmot_GetMeasure( motor , kMotMesPos );
  v[1] = kmot_GetMeasure( motor , kMotMesSpeed );
  v[2] = kmot_GetMeasure( motor , kMotMesCurrent );
  
  printf( "pos: %d, speed: %d, current: %d\n" , 
	  v[0] , v[1] , v[2] );
  return 0;
}

/*--------------------------------------------------------------------*/
/*! Launch a test routine which oscillates from (current pos + delta) 
 * and (current pos - delta). The position changes every 2 seconds. The 
 * routine can be stoped using the "stop" command.
 * syntax: test delta  
 */
int test( int argc , char * argv[] , void * data) 
{
  int curPos;
  int deltaPos = atoi(argv[1]);

  stopReq = 0;

  curPos = kmot_GetMeasure( motor , kMotMesPos );

 
  while ( !stopReq ) {
    kmot_SetPoint( motor , kMotRegPos , curPos + deltaPos );
    usleep( 200000 );

    status ( 0 , NULL );

    kmot_SetPoint( motor , kMotRegPos , curPos );
    usleep( 200000 );

    status ( 0 , NULL );

    kmot_SetPoint( motor , kMotRegPos , curPos - deltaPos );
    usleep ( 200000 );

    status( 0 , NULL  );

  }

  kmot_SetPoint( motor , kMotRegPos , curPos );

  stopReq = 0;
  return 0;
}


/*--------------------------------------------------------------------*/
/*! Reset the error register of the motor controller.
 */
int statusclear( int argc , char * argv[] , void * data)
{
  kmot_ResetError(motor);
}

/*--------------------------------------------------------------------*/
/*! Set the controller Software and Hardware options. Check the kmot.c 
 *  documentation for details about option flags.
 *  Syntax: setoption hardware software
 */
int setoption( int argc , char * argv[] , void * data)
{
  kmot_SetOptions( motor , atoi(argv[1]), atoi(argv[2]));

  return 0;
}




/*--------------------------------------------------------------------*/
/*! Read and print the option settings of the current controller. The content 
 * of the software option register and the hardware option register 
 * will be displayed.
 */
int option( int argc , char * argv[] , void * data)
{
  unsigned char software , hardware;

  kmot_GetOptions( motor , &software, &hardware);

  printf( "hardware=%02X software=%02X\n" , 
	  hardware , software ); 

  if ( hardware & kMotHWOptNormal)
    printf( "Normal Mode\n" );
  else
    printf( "Idle Mode\n" );

  if ( hardware & kMotHWOptAnSetPtInEn)
    printf("Analog Setpoint option active\r\n");

  if ( hardware & kMotHWOptEncRes1x)
    printf( "Encoder resolution set to 100%\n" );
  else
    printf( "Encoder resolution set to 25%\n" );
  
  if ( hardware & kMotHWOptTorqueInv)
    printf( "Torque Inversion option active\n" );

  if ( software & kMotSWOptSepD)
    printf("Separated D option active\r\n");
  
  if ( software & kMotSWOptWindup)
    printf("AntiWindup option active\r\n");
  
  if ( software & kMotSWOptSoftStopMin)
    printf("Softstop min option active\r\n");
 
  if ( software & kMotSWOptSoftStopMax)
    printf("Softstop max option active\r\n");

  if ( software & kMotSWOptSoftStopErr)
    printf("Softstop error option active\r\n");

  if ( software & kMotSWOptStopMotorBlk)
    printf("Block Motor option active\r\n");

  return 0;
}
/*--------------------------------------------------------------------*/
/*! Read and print the status of the current controller. The content of
 * the status register and error register will be displayed.
 */
int status( int argc , char * argv[] , void * data)
{
  unsigned char error , status;

  kmot_GetStatus( motor , &status , &error );

  printf( "status=%02X error=%02X\n" , 
	  status , error ); 

  if ( status & kMotStatusMoveDet )
    printf( "Movement detect!\n" );

  printf( "Direction %s !\n" , 
	     (status&kMotStatusDir) ? "Negative" : "Positive" );
  
  if ( status & kMotStatusOnSetPt )
    printf( "On Set Point!\n" );

  if ( status & kMotStatusNearSetPt )
    printf( "Near Set Point !\n" );
  
  if ( status & kMotStatusCmdSat )
    printf( "Command saturated !\n" );
  
  if ( status & kMotStatusWindup )
    printf( "Antireset Wind up active !\n" );
  
  if ( status & kMotStatusSoftCurCtrl )
    printf( "Software Current Control Active !\n" );
  
  if ( status & kMotStatusSoftStop )
    printf( "Software Stop Active !\n" );
  
  if ( error & kMotErrorSampleTimeTooSmall )
    printf( "Sample Time Too Small !\n" );
  
  if ( error & kMotErrorWDTOverflow )
    printf( "WatchDot Timer Overflow !\n" );
 
  if ( error & kMotErrorBrownOut )
    printf( "Brown-out !\n" );
 
  if ( error & kMotErrorSoftStopMotor )
    printf( "Software Stopped Motor !\n" );
  
  if ( error & kMotErrorMotorBlocked )
    printf( "Motor Blocked !\n" );
  
  if ( error & kMotErrorPosOutOfRange )
    printf( "Position Out of Range !\n" );
  
  if ( error & kMotErrorSpeedOutOfRange )
    printf( "Speed Out of Range !\n" );
 
  if ( error & kMotErrorTorqueOutOfRange )
    printf( "Torque Out of Range !\n" );

  return 0;
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
  { "stop"            , 0 , 0 , stop } ,
  { "init"            , 0 , 0 , init } , 
  { "stoplog"         , 0 , 0 , stoplog} , 
  { "startlog"        , 0 , 1 , startlog} ,
  { "setspeed"        , 1 , 1 , setspeed } ,
  { "setspeedprofile" , 1 , 1 , setspeedprofile },
  { "settorque"       , 1 , 1 , settorque } ,
  { "prescaler"	      , 1 , 1 , prescaler } ,
  { "velprescaler"    , 1 , 1 , velprescaler } ,
  { "openloop"        , 1 , 1 , openloop} ,
  { "setsampletime"   , 1 , 1 , setsampletime } ,
  { "configspeedprofile" , 2 , 2 , configspeedprofile } ,
  { "setpid"          , 4 , 4 , setpid } ,
  { "setpos"          , 1 , 1 , setpos } ,
  { "moveat"          , 1 , 1 , moveat } ,
  { "moveatprofile"   , 1 , 1 , moveatprofile } ,
  { "measure"         , 0 , 0 , measure } ,
  { "status"          , 0 , 0 , status } ,
  { "option"          , 0 , 0 , option } ,
  { "setoption"       , 2 , 2 , setoption } ,
  { "multiplier"      , 0 , 0 , multiplier } ,
  { "setmultiplier"   , 1 , 1 , setmultiplier } ,
  { "statusclear"     , 0 , 0 , statusclear } ,
  { "test"            , 1 , 1 , test } ,
  { "help"            , 0 , 0 ,  help } ,
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
  int rc;
  char * name;

  if ( argc < 2 ) {
    kb_msg("usage: kmot_test <motor_name>\n");
    kb_msg("motor_name: KoreMotorLE:PriMotor1  (0x1F)\n");
    kb_msg("            KoreMotorLE:PriMotor2  (0x20)\n");
    kb_msg("            KoreMotorLE:PriMotor3  (0x21)\n");
    kb_msg("            KoreMotorLE:PriMotor4  (0x22)\n");
    kb_msg("            KoreMotorLE:AltMotor1  (0x23)\n");
    kb_msg("            KoreMotorLE:AltMotor2  (0x24)\n");
    kb_msg("            KoreMotorLE:AltMotor3  (0x25)\n");
    kb_msg("            KoreMotorLE:AltMotor4  (0x26)\n");
    return 0;
  }

  name = argv[1];

  /* Set the libkhepera debug level - Highly recommended for development. */
  kb_set_debug_level(2);

  if ((rc = kb_init( argc , argv )) < 0 )
    return 1;

  signal( SIGINT , ctrlc_handler );

  motor = knet_open( name , KNET_BUS_I2C , 0 , NULL );

  if ( motor ) {
    unsigned int ver , rev;
    unsigned char status , error;
    int min, max;

    kmot_GetFWVersion( motor , &ver );

    printf("Motor %s Firmware v%u.%u\n" , name , 
	KMOT_VERSION(ver) , KMOT_REVISION(ver) );

    while (!quitReq) {
      printf("\n> ");

      if ( fgets( buf , sizeof(buf) , stdin ) != NULL ) {
	buf[strlen(buf)-1] = '\0';
	kb_parse_command( buf , cmds , NULL);
      }
    }

    knet_close( motor );
  } else {
    printf("Cannot open KoreMotor device\r\n");
  }

  return 0;
}


extern void kmot_GetSpeedMultiplier( knet_dev_t * dev ,  unsigned short *mult );

extern void kmot_SetSpeedMultiplier( knet_dev_t * dev , int mode );

