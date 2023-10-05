/*-------------------------------------------------------------------------------
 * Project: KoreBot Library	
 * $Author: pbureau_local $
 * $Date: 2005/02/02 17:18:12 $
 * $Revision: 1.4 $
 * 
 * 
 * $Header: /home/cvs/libkhepera/src/tests/kmot_ipserver.c,v 1.4 2005/02/02 17:18:12 pbureau_local Exp $
 */

/*--------------------------------------------------------------------*/
/*! 
 * \file   kmot_ipserver.c - koremotor tcp/ip server
 *
 * \brief
 *        Demo program to create a tcp/ip server that controls the KoreMotor.
 *        The server provides network commands for all the KoreMotor functions
 *        to allow a remote client to control the board.
 *
 * \author   Pierre Bureau (K-Team SA)
 * \note     Copyright (C) 2004 K-TEAM SA
 */


#include <khepera/khepera.h>

#define RCVBUFSIZE     32   /* Size of receive buffer */
#define DEFAULTPORT    344
#define NMOTOR         4
#define SENDBUFFERSIZE 64
#define ERRORSTRING    "kmot_error"

knet_dev_t * motorList[NMOTOR];
char sendBuffer[SENDBUFFERSIZE];

/*!
 * Get the motor controler firmare version.
 * 
 * Syntax: "ipFirmware requestId motor\\n"
 * 
 * Return: "ipFirmware requestId version\\n"
 */ 
int kmot_ipFirmware(int argc, char * argv[],void * data)
{
  unsigned int version,motor;
  
  motor = atoi(argv[2]);

  if(motor >= NMOTOR)
  {
    sprintf(sendBuffer,"%s %s %s",argv[0], argv[1],ERRORSTRING);
    ksock_send_answer((int*)data,sendBuffer);
  }
  else
  {
    kmot_GetFWVersion(motorList[motor],&version);

    sprintf(sendBuffer,"%s %s %u.%u",argv[0], argv[1],KMOT_VERSION(version) , KMOT_REVISION(version));

    printf("%s\r\n",sendBuffer);
    ksock_send_answer((int*)data,sendBuffer);
  } 

  return 0;
}

/*!
 * Get the motor controler status.
 * 
 * Syntax: "ipStatus requestId motor\\n"
 *
 * Return: "ipStatus requestId status error\\n"
 */ 
int kmot_ipStatus(int argc, char * argv[],void * data)
{
  unsigned int motor;
  unsigned char status,error;
  
  motor = atoi(argv[2]);

  if(motor >= NMOTOR)
  {
    sprintf(sendBuffer,"%s %s %s",argv[0], argv[1],ERRORSTRING);
    ksock_send_answer((int*)data,sendBuffer);
  }
  else
  {
    kmot_GetStatus( motorList[motor], &status , &error );

    sprintf(sendBuffer,"%s %s %u %u",argv[0], argv[1], status, error);

    printf("%s\r\n",sendBuffer);
    ksock_send_answer((int*)data,sendBuffer);
  } 

  return 0;
}

/*!
 * Configure the motor controller PID
 *
 * Syntax: "ipConfigPID requestId motor regtype Kp Kd Ki\\n"
 *
 * Return: "ipConfigPID requestId\\n"
 */ 
int kmot_ipConfigPID(int argc, char * argv[],void * data)
{
  unsigned int motor;
  
  motor = atoi(argv[2]);

  if(motor >= NMOTOR)
  {
    sprintf(sendBuffer,"%s %s %s",argv[0], argv[1],ERRORSTRING);
    ksock_send_answer((int*)data,sendBuffer);
  }
  else
  {
    kmot_ConfigurePID( motorList[motor], atoi(argv[3]) , atoi(argv[4]), atoi(argv[5]), atoi(argv[6]));

    sprintf(sendBuffer,"%s %s",argv[0], argv[1]);

    printf("%s\r\n",sendBuffer);
    ksock_send_answer((int*)data,sendBuffer);
  } 

  return 0;
}

/*!
 * Select the target point source for the controller
 *
 * Syntax: "ipSetPointSource requestId motor regtype wavetype period amplitude offset\\n"
 *
 * Return: "ipSetPointSource requestId\\n"
 */ 
int kmot_ipSetPointSource(int argc, char * argv[],void * data)
{
  unsigned int motor;
  
  motor = atoi(argv[2]);

  if(motor >= NMOTOR)
  {
    sprintf(sendBuffer,"%s %s %s",argv[0], argv[1],ERRORSTRING);
    ksock_send_answer((int*)data,sendBuffer);
  }
  else
  {
    kmot_SetPointSource( motorList[motor], atoi(argv[3]),atoi(argv[4]),atoi(argv[5]),atoi(argv[6]),atoi(argv[7]));

    sprintf(sendBuffer,"%s %s",argv[0], argv[1]);

    printf("%s\r\n",sendBuffer);
    ksock_send_answer((int*)data,sendBuffer);
  } 

  return 0;
}


/*!
 * Get a measure from a motor controller
 *
 * Syntax: "ipMeasure requestId motor regtype\\n"
 *
 * Return: "ipMeasure requestId measure\\n"
 */ 
int kmot_ipMeasure(int argc, char * argv[],void * data)
{
  unsigned int motor;
  long int measure;
  
  motor = atoi(argv[2]);

  if(motor >= NMOTOR)
  {
    sprintf(sendBuffer,"%s %s %s",argv[0], argv[1],ERRORSTRING);
    ksock_send_answer((int*)data,sendBuffer);
  }
  else
  {
    measure = kmot_GetMeasure( motorList[motor], atoi(argv[3]));

    sprintf(sendBuffer,"%s %s %ld",argv[0], argv[1],measure);

    printf("%s\r\n",sendBuffer);
    ksock_send_answer((int*)data,sendBuffer);
  } 

  return 0;
}

/*!
 * Reset errors from a motor controller
 *
 * Syntax: "ipResetError requestId motor\\n"
 *
 * Return: "ipResetError requestId\\n"
 */ 
int kmot_ipResetError(int argc, char * argv[],void * data)
{
  unsigned int motor;
  
  motor = atoi(argv[2]);

  if(motor >= NMOTOR)
  {
    sprintf(sendBuffer,"%s %s %s",argv[0], argv[1],ERRORSTRING);
    ksock_send_answer((int*)data,sendBuffer);
  }
  else
  {
    kmot_ResetError( motorList[motor] );

    sprintf(sendBuffer,"%s %s",argv[0], argv[1]);

    printf("%s\r\n",sendBuffer);
    ksock_send_answer((int*)data,sendBuffer);
  } 

  return 0;
}

/*!
 * Set target speed for the given motor
 *
 * Syntax: "ipSetSpeed requestId motor speed\\n"
 *
 * Return: "ipSetSpeed requestId\\n"
 */ 
int kmot_ipSetSpeed(int argc, char * argv[],void * data)
{
  unsigned int motor;
  
  motor = atoi(argv[2]);

  if(motor >= NMOTOR)
  {
    sprintf(sendBuffer,"%s %s %s",argv[0], argv[1],ERRORSTRING);
    ksock_send_answer((int*)data,sendBuffer);
  }
  else
  {
    kmot_SetPoint( motorList[motor] , kMotRegSpeed , atoi(argv[3]));

    sprintf(sendBuffer,"%s %s",argv[0], argv[1]);

    printf("%s\r\n",sendBuffer);
    ksock_send_answer((int*)data,sendBuffer);
  } 

  return 0;
}

/*!
 * Set target position for the given motor
 *
 * Syntax: "ipSetPos requestId motor position\\n"
 *
 * Return: "ipSetPos requestId\\n"
 */ 
int kmot_ipSetPos(int argc, char * argv[],void * data)
{
  unsigned int motor;
  
  motor = atoi(argv[2]);

  if(motor >= NMOTOR)
  {
    sprintf(sendBuffer,"%s %s %s",argv[0], argv[1],ERRORSTRING);
    ksock_send_answer((int*)data,sendBuffer);
  }
  else
  {
    kmot_SetPoint( motorList[motor] , kMotRegPos, atoi(argv[3]));

    sprintf(sendBuffer,"%s %s",argv[0], argv[1]);

    printf("%s\r\n",sendBuffer);
    ksock_send_answer((int*)data,sendBuffer);
  } 

  return 0;
}



/*!
 * Set default initialization for the given controller
 *
 * Syntax: "ipInit requestId motor\\n"
 *
 * Return: "ipInit requestId\\n"
 */ 
int kmot_ipInitMotor(int argc, char * argv[],void * data)
{
  unsigned int nmotor;
  knet_dev_t * motor;
  
  nmotor = atoi(argv[2]);

  if(nmotor >= NMOTOR)
  {
	  sprintf(sendBuffer,"%s %s %s",argv[0], argv[1],ERRORSTRING);
	  ksock_send_answer((int*)data,sendBuffer);
  }
  else
  {
	  motor = motorList[nmotor];

	  kmot_SetMode( motor , kMotModeIdle );
	  kmot_SetSampleTime( motor , 1550 );
	  kmot_SetMargin( motor , 6 );
	  kmot_SetOptions( motor , 0x0 , kMotSWOptWindup | kMotSWOptStopMotorBlk );

	  kmot_ResetError( motor );
	  kmot_SetBlockedTime( motor , 10 );
	  kmot_SetLimits( motor , kMotRegCurrent , 0 , 500 );
	  kmot_SetLimits( motor , kMotRegPos , -10000 , 10000 );

	  /* PID  */
	  kmot_ConfigurePID( motor , kMotRegSpeed , 1500 , 0 , 300 );
	  kmot_ConfigurePID(motor,kMotRegPos,70,50,10);

	  kmot_SetSpeedProfile(motor,30,10);

	  sprintf(sendBuffer,"%s %s",argv[0], argv[1]);

	  printf("%s\r\n",sendBuffer);
	  ksock_send_answer((int*)data,sendBuffer);
  }
  
	  return 0;
}

/****************************************************************/
int main( int argc , char * argv[] )
{
  ksock_t server;
  int rc;
  unsigned char buf[16];
  unsigned char a,b,c,d;
  int clntSocket;
  char echoBuffer[RCVBUFSIZE];        /* Buffer for echo string */
  int recvMsgSize;                    /* Size of received message */
  int port;                           /* The server port to open */
  
  pthread_t sensor_task;
  pthread_t sound_task;

  if(argc > 1)
    port = atoi(argv[1]);
  else
    port = DEFAULTPORT;

  /* Various initialization */
  /* Set the libkhepera debug level - Highly recommended for development. */
  kb_set_debug_level( 2 );

  /* Global libkhepera init */
  if((rc = kb_init( argc , argv )) < 0 )
    return 1;
  
  /* Korebot socket module init */
  if((rc = ksock_init('\n')) < 0 )
    return 1;

  /* Open the motor devices */
  motorList[0] = knet_open( "KoreMotor:PriMotor1", KNET_BUS_ANY, 0 , NULL );
  if(!motorList[0])
  {
    printf("Cannot open motor 0\r\n");
    return 1;
  }
  motorList[1] = knet_open( "KoreMotor:PriMotor2", KNET_BUS_ANY, 0 , NULL );
  if(!motorList[1])
  {
    printf("Cannot open motor 1\r\n");
    return 1;
  }
  motorList[2] = knet_open( "KoreMotor:PriMotor3", KNET_BUS_ANY, 0 , NULL );
  if(!motorList[2])
  {
    printf("Cannot open motor 2\r\n");
    return 1;
  }
  motorList[3] = knet_open( "KoreMotor:PriMotor4", KNET_BUS_ANY, 0 , NULL );
  if(!motorList[3])
  {
    printf("Cannot open motor 3\r\n");
    return 1;
  }

  
  /* KoreMotor Comm test */
  kmot_GetFWVersion( motorList[0], &rc);
  printf("Motor 1 Firmware v%u.%u\n" , KMOT_VERSION(rc) , KMOT_REVISION(rc));

  /* Network setup - Create a ksock server */
  ksock_server_open(&server, port);
  //printf("Server IP address is %s on port %d\r\n",inet_ntoa(server.serv_addr.sin_addr),port);
  
  /* Create the list of network commands for KoreMotor */
  ksock_add_command("ipFirmware",2,2,kmot_ipFirmware);
  ksock_add_command("ipInit",2,2,kmot_ipInitMotor);
  ksock_add_command("ipMeasure",3,3,kmot_ipMeasure);
  ksock_add_command("ipSetSpeed",3,3,kmot_ipSetSpeed);
  ksock_add_command("ipSetPos",3,3,kmot_ipSetPos);
  ksock_add_command("ipStatus",2,2,kmot_ipStatus);
  ksock_add_command("ipConfigPID",6,6,kmot_ipConfigPID);
  ksock_add_command("ipSetPointSource",7,7,kmot_ipSetPointSource);
  ksock_add_command("ipResError",2,2,kmot_ipResetError);
  list_command();

  /* Network connection handling */
  clntSocket = ksock_next_connection(&server);
  for(;;)
  {
    rc = ksock_get_command(clntSocket, echoBuffer, RCVBUFSIZE);

    if(rc>0)
    {
      ksock_exec_command_pending(clntSocket,echoBuffer);
#if 0
      /* Echo message back to client */
      if (send(clntSocket, echoBuffer, rc, 0) != rc)
	DieWithError("send() failed");
#endif
    } 
    else 
      switch(rc)
      {
	case 0 :
	  break;
	case -3 :
	  printf("Client disconnected\r\n");
	  clntSocket = ksock_next_connection(&server);
	  break;
	case -1 :
	  /* Incomplete command received, waiting for completion */
	  break;
	default :
	  printf("Socket error: %d\r\n",rc);
	  clntSocket = ksock_next_connection(&server);
	  break;
      }
  }

  close(clntSocket);    /* Close client socket */

  return 0;
}

