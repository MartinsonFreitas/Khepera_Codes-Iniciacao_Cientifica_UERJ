/*! 
 * \file   kgripper_test.c  Khepera3 Gripper test application              
 *
 * \brief 
 *         This is an application that demonstrates the various Gripper commands. 
 *         
 *        
 * \author   Frederic Lambercy (K-Team SA)                               
 *
 * \note     Copyright (C) 2010 K-TEAM SA
 * \bug      none discovered.                                         
 * \todo     nothing.
 */

#include <khepera/khepera.h>


static int quitReq = 0;



/*! handle to the various Gripper devices (knet socket, i2c mode)
 */
static knet_dev_t * Arm;
static knet_dev_t * Gripper;



												

/*--------------------------------------------------------------------*/
/*! initGripper open the various required handle to the both i2c devices 
 * on the Gripper using knet_open from the knet.c libkhepera's modules.
 *
 * \return A value :
 *      - 1 if success
 *      - 0 if any error
 */
int initGripper( void )
{

  kgripper_init();
  /* open various socket and store the handle in their respective pointers */
  Arm = knet_open( "Kgripper:Arm" , KNET_BUS_I2C , 0 , NULL );
  Gripper  = knet_open( "Kgripper:Gripper" , KNET_BUS_I2C , 0 , NULL );

  if(Arm!=0)
  {
    if(Gripper!=0)
    {
      return 0;
    }
    else
      return -1;
  }

  return -2;

} 



/*--------------------------------------------------------------------*/
/*! revisionOS retrieves the Gripper os version using kb_gripper.c library.
 */
int revisionOS( int argc, char * argv[], void * data)
{
  unsigned char version, revision ;

  
  version = kgripper_Arm_Get_Version( Arm );
  revision = version & 0x0F;
  version = ((version & 0xF0) >> 4);
  printf("KheperaIII Gripper, ARM Firmware v%X-%2x" , version, revision);
  version = kgripper_Gripper_Get_Version( Gripper );
  revision = version & 0x0F;
  version = ((version & 0xF0) >> 4);
  printf(", Gripper Firmware v%X-%2x" , version, revision);


}

/*--------------------------------------------------------------------*/
/*! status return all the value of the Gripper using kb_gripper.c library.
 */
int status( int argc, char * argv[], void * data)
{
  unsigned char Data8, PosH, PosL;
  signed char DataMin8;
  unsigned short Data16, Min_Position, Max_Position,i;
  float Voltage;
   
  for(i = 0; i< atoi(argv[1]); i++)
  {
  printf("%c",0x0C);
  printf("ARM Status\n");
  Data16 = kgripper_Arm_Get_Position( Arm );
  printf(" Position    %4u\n",Data16);
  DataMin8 =  kgripper_Arm_Get_Speed( Arm );
  printf(" Speed       %4d\n",DataMin8);
  Data16 = kgripper_Arm_Get_Current(Arm);
  printf(" Current     %4u\n",Data16);
  Data8 = kgripper_Arm_OnTarget(Arm);
  printf(" On Target   %4u\n",Data8);
  Data16 = kgripper_Arm_Get_Voltage(Arm);
  Voltage = (float)Data16 / 102.3;
  printf(" Voltage     %1.2f\n",Voltage);
  Data8 = kgripper_Arm_Get_Capacity(Arm);
  printf(" Capacity    %3u%%\n",Data8);
  kgripper_Arm_Get_Limits(Arm, &Min_Position ,  &Max_Position );
  printf(" Limits      %4u to %4u\n", Min_Position, Max_Position);
  Data16 = kgripper_Arm_Get_Order(Arm);
  printf(" Order       %4u\n",Data16);
  Data8 = kgripper_Arm_Get_Max_Speed(Arm);
  printf(" Max Speed   %4u\n\n",Data8);

  printf("Gripper Status\n");
  Data8 = kgripper_Gripper_Get_Position( Gripper );
  printf(" Position    %4u\n",Data8);
  DataMin8 =  kgripper_Gripper_Get_Speed( Gripper );
  printf(" Speed       %4d\n",DataMin8);
  Data16 = kgripper_Gripper_Get_Current(Gripper);
  printf(" Current     %4u\n",Data16);
  Data8 = kgripper_Gripper_OnTarget(Gripper);
  printf(" On Target   %4u\n",Data8);
  Data8 = kgripper_Gripper_Get_Limits(Gripper);
  printf(" Limits      %4u\n", Data8);
  Data16 = kgripper_Gripper_Get_Order(Gripper);
  printf(" Order       %4u\n",Data16);
  Data16 = kgripper_Gripper_Get_Torque(Gripper);
  printf(" Max Torque  %4u\n\n",Data16);
  usleep(80000);

  
  }


}

/*--------------------------------------------------------------------*/
/*! movearm function set a new position order for the Arm of the Gripper using kb_gripper.c library.
 */
int movearm( int argc, char * argv[], void * data)
{
  printf("Move Arm gripper to position %4u\n",atoi(argv[1]));
  kgripper_Arm_Set_Order( Arm,  atoi(argv[1]));


}

/*--------------------------------------------------------------------*/
/*! movegrip function set a new position order for the Finger of the Gripper using kb_gripper.c library.
 */
int movegrip( int argc, char * argv[], void * data)
{
  printf("Move Finger gripper to position %4u\n",atoi(argv[1]));
  kgripper_Gripper_Set_Order( Gripper,  atoi(argv[1]));


}


/*--------------------------------------------------------------------*/
/*! get function will move the gripper to get an object (=1) or release it (= 0) using kb_gripper.c library.
 */
int get( int argc, char * argv[], void * data)
{
   unsigned short Min_Position, Max_Position, Data16;

  if(atoi(argv[1]) == 0)
  {
    printf("Release an object\n");
    kgripper_Arm_Get_Limits(Arm, &Min_Position ,  &Max_Position );
    kgripper_Arm_Set_Order( Arm, Max_Position-5);  					// Move the arm to the ground
    usleep(10000);									// Wait at least that a control loop is made
    while(kgripper_Arm_OnTarget(Arm) == 0);						// Wait that the arm is on position
    kgripper_Gripper_Set_Order( Gripper,  kgripper_Gripper_Get_Limits(Gripper));	// Open the gripper to its maximal limit
    usleep(10000);									// Wait at least that a control loop is made
    while(kgripper_Gripper_OnTarget(Gripper) == 0);					// Wait that the gripper is on position
    kgripper_Arm_Set_Order( Arm, Min_Position); 					// Move the arm up the Robot    
    usleep(10000);									// Wait at least that a control loop is made
    while(kgripper_Arm_OnTarget(Arm) == 0);						// Wait that the arm is on position
    kgripper_Gripper_Set_Order( Gripper, 10);						// Close the gripper
    usleep(10000);									// Wait at least that a control loop is made
    while(kgripper_Gripper_OnTarget(Gripper) == 0);					// Wait that the gripper is on position
  }
  else
  {
    printf("Get an object\n");
    kgripper_Gripper_Set_Order( Gripper,  kgripper_Gripper_Get_Limits(Gripper));	// Open the gripper to its maximal limit
    usleep(10000);									// Wait at least that a control loop is made
    while(kgripper_Gripper_OnTarget(Gripper) == 0);					// Wait that the gripper is on position
    kgripper_Arm_Get_Limits(Arm, &Min_Position ,  &Max_Position );
    kgripper_Arm_Set_Order( Arm, Max_Position); 					// Move the arm to the ground
    usleep(10000);									// Wait at least that a control loop is made
    while(kgripper_Arm_OnTarget(Arm) == 0);						// Wait that the arm is on position
    while(kgripper_Gripper_Object_Detected(Gripper) == 0);				// Wait an object
    kgripper_Gripper_Set_Torque( Gripper, 250);					 	// Set the maximal torque
    kgripper_Gripper_Set_Order( Gripper, 0);						// Close the gripper
    usleep(100000);									// Wait at least that a control loop is made
    while(kgripper_Gripper_Get_Current( Gripper) < 250);				// Wait that the object is grip
    kgripper_Arm_Set_Order( Arm, Min_Position);						// Move the arm up the Robot    
    usleep(10000);									// Wait at least that a control loop is made
    while(kgripper_Arm_OnTarget(Arm) == 0);						// Wait that the arm is on position
    Data16 = kgripper_Gripper_Get_Resistivity( Gripper);
    printf(" Object resistivity  %u\n", Data16);
  
  }


}
/*--------------------------------------------------------------------*/
/*! grip function will move the gripper n times (0 = infiny) using kb_gripper.c library.
 */
int grip( int argc, char * argv[], void * data)
{
   unsigned short Data16, loop, n, Grip_Lim;

  n = atoi(argv[1]);
  if(n == 0)
  {
   loop = n+1;
   printf("Start demo mode. Type CTRL+C to stop\n", n);
  }
  else
  {
   loop = n;
   printf("Start %5u demo move\n", n);
  }
  Grip_Lim = kgripper_Gripper_Get_Limits(Gripper);
  do
  {
    kgripper_Gripper_Set_Order( Gripper, 0);						// Close the gripper
    usleep(100000);									// Wait at least that a control loop is made
    while(kgripper_Gripper_Get_Current( Gripper) < kgripper_Gripper_Get_Torque( Gripper ));// Wait that the object is grip
    kgripper_Gripper_Set_Order( Gripper,Grip_Lim - 20 );				// Open the gripper to its maximal limit
    usleep(10000);									// Wait at least that a control loop is made
    while(kgripper_Gripper_OnTarget(Gripper) == 0);					// Wait that the gripper is on position
 
    if(n != 0)
      loop--;
  
  }while(loop > 0);


}
/*--------------------------------------------------------------------*/
/*! demo function will move the gripper n times (0 = infiny) using kb_gripper.c library.
 */
int demo( int argc, char * argv[], void * data)
{
   unsigned short Min_Position, Max_Position, Data16, loop, n, Grip_Lim;

  n = atoi(argv[1]);
  if(n == 0)
  {
   loop = n+1;
   printf("Start demo mode. Type CTRL+C to stop\n", n);
  }
  else
  {
   loop = n;
   printf("Start %5u demo move\n", n);
  }
  kgripper_Gripper_Set_Torque( Gripper, 300);					 	// Set the maximal torque
  kgripper_Arm_Get_Limits(Arm, &Min_Position ,  &Max_Position );
  Grip_Lim = kgripper_Gripper_Get_Limits(Gripper);
  do
  {

    kgripper_Arm_Set_Order( Arm, Max_Position);  					// Move the arm to the ground
    kgripper_Gripper_Set_Order( Gripper,Grip_Lim - 20 );					// Open the gripper to its maximal limit
    usleep(10000);									// Wait at least that a control loop is made
    while(kgripper_Gripper_OnTarget(Gripper) == 0);					// Wait that the gripper is on position
    kgripper_Gripper_Set_Order( Gripper, 0);						// Close the gripper
    usleep(100000);									// Wait at least that a control loop is made
    while(kgripper_Gripper_Get_Current( Gripper) < 300);				// Wait that the object is grip
    while(kgripper_Arm_OnTarget(Arm) == 0);						// Wait that the arm is on position
    kgripper_Arm_Set_Order( Arm, 600);		 					// Move the arm up the Robot    
    kgripper_Gripper_Set_Order( Gripper,Grip_Lim - 20 );  					// Open the gripper to its maximal limit
    usleep(10000);									// Wait at least that a control loop is made
    while(kgripper_Gripper_OnTarget(Gripper) == 0);					// Wait that the gripper is on position
    while(kgripper_Arm_OnTarget(Arm) == 0);						// Wait that the arm is on position
    kgripper_Arm_Set_Order( Arm, Min_Position);						// Move the arm up the Robot    
    usleep(10000);									// Wait at least that a control loop is made
    while(kgripper_Arm_OnTarget(Arm) == 0);						// Wait that the arm is on position
    kgripper_Gripper_Set_Order( Gripper, 10);						// Close the gripper
    usleep(10000);									// Wait at least that a control loop is made
    while(kgripper_Gripper_OnTarget(Gripper) == 0);					// Wait that the gripper is on position
 
    if(n != 0)
      loop--;
  
  }while(loop > 0);


}

/*--------------------------------------------------------------------*/
/*! sensor return all the value of the sensors in the Gripper finger using kb_gripper.c library.
 */
int sensor( int argc, char * argv[], void * data)
{
  unsigned char Data8;
  unsigned short Data16, Dist_IR_Left, Dist_IR_Right,i;


  for(i = 0; i< atoi(argv[1]); i++)
  {
  printf("%c",0x0C);  
  printf("Finger Sensors value\n");
  kgripper_Gripper_Get_Distance_Sensors( Gripper , &Dist_IR_Left , &Dist_IR_Right );
  printf(" Distance            Left = %4u  Right = %4u\n",Dist_IR_Left , Dist_IR_Right);
  kgripper_Gripper_Get_Ambiant_IR_Light( Gripper , &Dist_IR_Left , &Dist_IR_Right );
  printf(" Ambiant light       Left = %4u  Right = %4u\n",Dist_IR_Left , Dist_IR_Right);
  Data8 = kgripper_Gripper_Object_Detected(Gripper);
  printf(" Object detection    %u\n", Data8);
  Data16 = kgripper_Gripper_Get_Resistivity( Gripper);
  printf(" Object resistivity  %u\n", Data16);
  usleep(80000);

  
  }


}


/*--------------------------------------------------------------------*/
/*! setTorque define the maximal Torque use by the finger to grip an object using kb_gripper.c library.
 */
int setTorque( int argc, char * argv[], void * data)
{
  printf("Maximal Torque set at %4u\n",atoi(argv[1]));
  kgripper_Gripper_Set_Torque( Gripper,  atoi(argv[1]));

}

/*--------------------------------------------------------------------*/
/*! setMaxSpeed define the maximal speed of the arm gripper using kb_gripper.c library.
 */
int setMaxSpeed( int argc, char * argv[], void * data)
{
  printf("Maximal speed set at %2u\n",atoi(argv[1]));
  kgripper_Arm_Set_Max_Speed( Arm,  atoi(argv[1]));

}

/*--------------------------------------------------------------------*/
/*! searchLimit start the mechanical limit search using kb_gripper.c library.
 */
int searchLimit( int argc, char * argv[], void * data)
{
 unsigned short Min_Position, Max_Position;
 unsigned char Data8;
  printf("Search the maximal mechanical limits\n\r");
  kgripper_Arm_Set_Search_Limit( Arm, 1);
  kgripper_GripperSet_Search_Limit( Gripper, 1);
  while(kgripper_Arm_Get_Search_Limit(Arm) || kgripper_Gripper_Get_Search_Limit(Gripper))
  {
    sleep(1);
  }
  kgripper_Arm_Get_Limits(Arm, &Min_Position ,  &Max_Position );
  Data8 = kgripper_Gripper_Get_Limits(Gripper);
  printf("New mechanical Limits: Arm     = %4u to %4u\n", Min_Position, Max_Position);  
  printf("                       Gripper = %4u\n",Data8);

}

/*--------------------------------------------------------------------*/
/*! Quit the program.
*/
int quit( int argc , char * argv[] , void * data)
{
  quitReq = 1;
}


int help( int argc , char * argv[] , void * data);
/*--------------------------------------------------------------------*/
/*! The command table contains:
 * command name : min number of args : max number of args : the function to call
 */
static kb_command_t cmds[] = {
 { "quit"            , 0 , 0 , quit } ,
 { "exit"            , 0 , 0 , quit } ,
 { "status"	     , 1 , 1 , status },
 { "getrev"	     , 0 , 0 , revisionOS },
 { "movearm"         , 1 , 1 , movearm } ,
 { "movegrip"        , 1 , 1 , movegrip } ,
 { "sensor"          , 1 , 1 , sensor } ,
 { "maxspeed"        , 1 , 1 , setMaxSpeed } ,
 { "get"             , 1 , 1 , get } ,
 { "demo"            , 1 , 1 , demo } ,
 { "grip"            , 1 , 1 , grip } ,
 { "torque"          , 1 , 1 , setTorque } ,
 { "search"	     , 0 , 0 , searchLimit },
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
/*! Main
*/
int main( int arc, char *argv[])
{
  char i;

	char buf[64];

  printf("Khepera3 Gripper test program (C) K-Team S.A\r\n");

  if(!initGripper())
  {
    printf("Init oke...\r\n");

    while (!quitReq) 
    {


      printf("\n> ");

      if ( fgets( buf , sizeof(buf) , stdin ) != NULL ) 
      {
				buf[strlen(buf)-1] = '\0';
				kb_parse_command( buf , cmds , NULL);
      }
    }

    printf("Exiting...\r\n");
	}
	else
	  printf("Fatal error, unable to initialize\r\n");

}


