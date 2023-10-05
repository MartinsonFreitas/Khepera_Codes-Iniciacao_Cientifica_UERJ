/* \file kh4_small_ex.c 

 *
 * \brief 
 *         This is the small application example for the Khepera4	 
 *         
 *        
 * \author   Julien Tharin (K-Team SA)                               
 *
 * \note     Copyright (C) 2013 K-TEAM SA
 * \bug      none discovered.                                         
 * \todo     nothing.

 * compile with command (don't forget to source the env.sh of your development folder!):
 		arm-angstrom-linux-gnueabi-gcc kh4_small_ex.c -o kh4_small_ex -I $INCPATH -L $LIBPATH -lkhepera 


*/
#include <khepera/khepera.h>
#include <signal.h>


static knet_dev_t * dsPic; // robot pic microcontroller access



static int quitReq = 0; // quit variable for loop

/*--------------------------------------------------------------------*/
/*!
 * Make sure the program terminate properly on a ctrl-c
 */
static void ctrlc_handler( int sig ) 
{
  quitReq = 1;
  
  kh4_set_speed(0 ,0 ,dsPic); // stop robot
  kh4_SetMode( kh4RegIdle,dsPic );
  
  kh4_SetRGBLeds(0,0,0,0,0,0,0,0,0,dsPic); // clear rgb leds because consumes energy
  
  kb_change_term_mode(0); // revert to original terminal if called
  
  exit(0);
}


/*--------------------------------------------------------------------*/
/*!
 * Main
 */
int main() 
{ 

  double fpos,dval;
  int lpos,rpos;
  char Buffer[100],bar[12][64],revision,version;
  int i,n,val,type_of_test=0,sl,sr,pl,pr,mean;
  short index, value,sensors[12],usvalues[5];
  char c;
  long motspeed;
  char line[80],l[9];
  int kp,ki,kd;
  int pmarg,maxsp,accinc,accdiv,minspacc, minspdec; // SetSpeedProfile
  
  
  printf("\nKhepera 4 small example program\n");
  
  // initiate libkhepera and robot access
  if ( kh4_init(0,NULL)!=0)
  {
  	printf("\nERROR: could not initiate the libkhepera!\n\n");
  	return -1;
  }	

  /* open robot socket and store the handle in its pointer */
  dsPic  = knet_open( "Khepera4:dsPic" , KNET_BUS_I2C , 0 , NULL );

	if ( dsPic==NULL)
  {
  	printf("\nERROR: could not initiate communication with Kh4 dsPic\n\n");
  	return -2;
  }	

  /* ------  initialize the motors controlers --------------------------------*/
   
  /* tuned parameters */
  pmarg=20;
  kh4_SetPositionMargin(pmarg,dsPic ); 				// position control margin
  kp=10;
  ki=5;
  kd=1;
  kh4_ConfigurePID( kp , ki , kd,dsPic  ); 		// configure P,I,D
  
  accinc=3;
  accdiv=0;
  minspacc=20;
  minspdec=1;
  maxsp=400;
  kh4_SetSpeedProfile(accinc,accdiv,minspacc, minspdec,maxsp,dsPic ); // Acceleration increment ,  Acceleration divider, Minimum speed acc, Minimum speed dec, maximum speed
  
	kh4_SetMode( kh4RegIdle,dsPic );  				// Put in idle mode (no control)


  // get revision
  if(kh4_revision(Buffer, dsPic)==0){
   	version=(Buffer[0]>>4) +'A';
  	revision=Buffer[0] & 0x0F; 
    printf("\r\nVersion = %c, Revision = %u\r\n",version,revision);        
  }
  

  signal( SIGINT , ctrlc_handler ); // set signal for catching ctrl-c
  
  
  //  ------  battery example --------------------------------------------------
	kh4_battery_status(Buffer,dsPic);
	printf("\nBattery:\n  status (DS2781)   :  0x%x\n",Buffer[0]);
	printf("  remaining capacity:  %4.0f mAh\n",(Buffer[1] | Buffer[2]<<8)*1.6);
	printf("  remaining capacity:   %3d %%\n",Buffer[3]);
	printf("  current           : %5.0f mA\n",(short)(Buffer[4] | Buffer[5]<<8)*0.07813);
	printf("  average current   : %5.0f mA\n",(short)(Buffer[6] | Buffer[7]<<8)*0.07813);
	printf("  temperature       :  %3.1f C \n",(short)(Buffer[8] | Buffer[9]<<8)*0.003906);
	printf("  voltage           :  %4.0f mV \n",(Buffer[10] | Buffer[11]<<8)*9.76);
	printf("  charger           :  %s\n",kh4_battery_charge(dsPic)?"plugged":"unplugged");
  
		
	//  ------  rgb led example --------------------------------------------------
  printf("\nRGB leds : front...\n");
  
	for (i=0;i<0x20;i++) // 0x20  => 50%
	{
		usleep(10000);
		kh4_SetRGBLeds(i,0,0,0,0,0,0,0,0,dsPic); // R
	}
	for (i=0;i<0x20;i++)
	{
		kh4_SetRGBLeds(0,i,0,0,0,0,0,0,0,dsPic); // G
		usleep(10000);
	}
	for (i=0;i<0x20;i++)
	{
		kh4_SetRGBLeds(0,0,i,0,0,0,0,0,0,dsPic); // B
		usleep(10000);
	}


	printf("\nRGB leds : right...\n");
	for (i=0;i<0x20;i++)
	{
		usleep(10000);
		kh4_SetRGBLeds(0,0,0,i,0,0,0,0,0,dsPic); // R
	}
	for (i=0;i<0x20;i++)
	{
		kh4_SetRGBLeds(0,0,0,0,i,0,0,0,0,dsPic); // G
		usleep(10000);
	}
	for (i=0;i<0x20;i++)
	{
		kh4_SetRGBLeds(0,0,0,0,0,i,0,0,0,dsPic); // B
		usleep(10000);
	}



	printf("\nRGB leds : back...\n");
	for (i=0;i<0x20;i++)
	{
		usleep(10000);
		kh4_SetRGBLeds(0,0,0,0,0,0,i,0,0,dsPic); // R
	}
	for (i=0;i<0x20;i++)
	{
		kh4_SetRGBLeds(0,0,0,0,0,0,0,i,0,dsPic); // G
		usleep(10000);
	}
	for (i=0;i<0x20;i++)
	{
		kh4_SetRGBLeds(0,0,0,0,0,0,0,0,i,dsPic); // B
		usleep(10000);
	}
		

  kh4_SetRGBLeds(0,0,0,0,0,0,0,0,0,dsPic); // stop LEDS


	// ------ motors examples ----------------------------------------------------

	kh4_get_position(&lpos,&rpos,dsPic); // read  encoder position

	printf("\nPosition factor: pulse to mm: %lf\nSpeed factor: speed to mm/s: %lf\n",KH4_PULSE_TO_MM,KH4_SPEED_TO_MM_S);

	fpos = 10.0*10.0/KH4_PULSE_TO_MM; // compute number of pulse from mm
	printf("\nMoving forward %.1f cm in  %.1f pulses with position control\nencoders: left %ld  (final expected %ld) | right %ld (final expected %ld)\n",10.0,fpos,lpos,lpos+(long)fpos,rpos,rpos+(long)fpos);
	 
	// tell motor controllers to move K4 forward, in position control
	kh4_SetMode( kh4RegPosition,dsPic );
	kh4_set_position(lpos+(long)fpos,rpos+(long)fpos,dsPic);
		

	printf("\n wait 5s \n");
	sleep(5); // Wait 5 seconds
	kh4_get_position(&lpos,&rpos,dsPic);
	printf("\n encoders: left %ld | right %ld\n",lpos,rpos);


	// Tell to the motor controller to move the Khepera 4 backward, in speed profile control
	
	motspeed= (long)(-25.0/KH4_SPEED_TO_MM_S); // convert speed from mm/s to pulse/10ms
	kh4_SetMode( kh4RegSpeedProfile,dsPic );
	kh4_set_speed(motspeed ,motspeed ,dsPic);
	

	printf("\nMoving backward %.1f cm at %.1f mm/s (pulse speed %ld) with speed profile control\n",10.0,-25.0,motspeed);


	sleep(4); 	// Wait 5 seconds

	// Tell to the motor controller to stop the Khepera 4
	kh4_set_speed(0 ,0,dsPic);

	
	sleep(1);	 // Wait 1 seconds
	kh4_get_position(&lpos,&rpos,dsPic);
	printf("\n encoders: left %ld | right %ld\n",lpos,rpos);

	
	motspeed= (long)(40.0/KH4_SPEED_TO_MM_S);
	kh4_SetMode(kh4RegSpeed,dsPic );
	kh4_set_speed(motspeed ,-motspeed ,dsPic);
	printf("\nRotating 5s at %.1f mm/s (pulse speed %ld) with speed only\n",40.0,motspeed);
	
	sleep(5); // Wait 5 seconds

 

	kh4_get_position(&lpos,&rpos,dsPic);
	printf("\nread encoders: left %ld | right %ld\n",lpos,rpos);
	
	// Open loop
	motspeed= KH4_MAX_OPENLOOP_SPEED_PWM_100/4;  // 25%
	kh4_SetMode(kh4RegSOpenLoop,dsPic );
	kh4_set_speed(-motspeed ,motspeed ,dsPic);
	printf("\nRotating (control pwm: %ld %%) with openloop during 3s\n",motspeed*100/KH4_MAX_OPENLOOP_SPEED_PWM_100);
	
	sleep(3); // Wait 5 seconds


	
	kh4_ResetEncoders(dsPic);
	sleep(1);
	
	kh4_SetMode( kh4RegIdle,dsPic );
	
	// Read encoders
	while(kb_kbhit()==0)
	{
		kb_clrscr();
		printf("\nRead encoders (motor in idle mode)\nPlease move manualy the robot\n");	
		kh4_get_position(&lpos,&rpos,dsPic);
		printf("encoders: left %5ld | right %5ld)\n",lpos,rpos);
		printf("\nPush any key to continue program\n");
		usleep(200000);
	}

	tcflush(0, TCIFLUSH);  // flush input key
	


	
  // ------- IR proximity sensors example --------------------------------------
	#define IR_BAR_LEN 15 	// display bar length for IR sensor
	while(!kb_kbhit())
	{	
		kb_clrscr();		

		printf("IR proximity sensors:\n");

		// get ir proximity sensor
		kh4_proximity_ir(Buffer, dsPic);

		for (i=0;i<12;i++)
		{
			sensors[i]=(Buffer[i*2] | Buffer[i*2+1]<<8);
							                       
			n=(int)(sensors[i]*IR_BAR_LEN/1024.0);
							                      
			if (n==0)
				sprintf(bar[i],"|\33[%dC>|",IR_BAR_LEN-1);
			else
				if (n>=IR_BAR_LEN-1)
					sprintf(bar[i],"|>\33[%dC|",IR_BAR_LEN-1);
				else
				 sprintf(bar[i],"|\33[%dC>\33[%dC|",IR_BAR_LEN-1-n,n);
							                      
		 }                                
		 printf("\n                    near               far\
		 \nback left      : %4u  %s\nleft           : %4u  %s\
		 \nfront left     : %4u  %s\nfront          : %4u  %s\
		 \nfront right    : %4u  %s\nright          : %4u  %s\
		 \nback right     : %4u  %s\nback           : %4u  %s\
		 \nground left    : %4u  %s\ngnd front left : %4u  %s\
		 \ngnd front right: %4u  %s\nground right   : %4u  %s\n",
			 sensors[0],bar[0],  sensors[1],bar[1],
			 sensors[2],bar[2],  sensors[3],bar[3],
			 sensors[4],bar[4],  sensors[5],bar[5],
			 sensors[6],bar[6],  sensors[7],bar[7],
			 sensors[8], bar[8], sensors[9], bar[9],
			 sensors[10], bar[10],sensors[11], bar[11]
			 );

		printf("\nPush any key to continue program\n");
		usleep(200000); // wait 200ms

	} 
  
  tcflush(0, TCIFLUSH);  // flush input key

	
  // ------- gyroscope sensor example ------------------------------------------
	motspeed= (int)(KH4_WHEELS_DISTANCE*M_PI/KH4_SPEED_TO_MM_S);  // rotate 2rev/s
	kh4_SetMode(kh4RegSpeed,dsPic );
	kh4_set_speed(-motspeed ,motspeed ,dsPic);
	


	
	while(kb_kbhit()==0)
	{
	  kb_clrscr();	
		kh4_measure_gyro((char *)Buffer, dsPic);
		
		printf("The robot rotates\ngyro sensor [deg/s]: new data                         old data\n");
		printf("gyro X: ");
		for (i=0;i<10;i++)
		{
		  dval=((short)((Buffer[i*2] | Buffer[i*2+1]<<8)))*KH4_GYRO_DEG_S; // convert to [deg/s]
		  printf("%6.1f ",dval);                               	                                                                   
		}   
		printf("\ngyro Y: ");
		for (i=10;i<20;i++)
		{
		  dval=((short)((Buffer[i*2] | Buffer[i*2+1]<<8)))*KH4_GYRO_DEG_S;
		  printf("%6.1f ",dval);                              	                                                                   
		} 
		printf("\ngyro Z: ");
		for (i=20;i<30;i++)
		{
		  dval=((short)((Buffer[i*2] | Buffer[i*2+1]<<8)))*KH4_GYRO_DEG_S;
		  printf("%6.1f ",dval);                              	                                                                   
		}     
		printf("\n\nPush any key to continue program\n");

		usleep(200000);
	}

	tcflush(0, TCIFLUSH);  // flush input key

	kh4_set_speed(0 ,0 ,dsPic); // stop robot

	// ------- acceleration sensor example ---------------------------------------
  while(kb_kbhit()==0)
	{
		kb_clrscr();	
		printf("Take the robot in hand and rotate it.\n");
	
		// get accel sensor
		kh4_measure_acc((char *)Buffer, dsPic);
	
		printf("\nAcceleration sensor [g]:  new data                 old data\nacc  X: ");
		for (i=0;i<10;i++)
		{
			dval=((short)(Buffer[i*2] | Buffer[i*2+1]<<8)>>4)/1000.0; // convert to [g]
			printf("%5.2f ",dval);                            	                                                                   
		}   
		printf("\nacc  Y: ");
		for (i=10;i<20;i++)
		{
			dval=((short)(Buffer[i*2] | Buffer[i*2+1]<<8)>>4)/1000.0;
			printf("%5.2f ",dval);                               	                                                                   
		} 
		printf("\nacc  Z: ");
		for (i=20;i<30;i++)
		{
			dval=((short)(Buffer[i*2] | Buffer[i*2+1]<<8)>>4)/1000.0;
			printf("%5.2f ",dval);                             	                                                                   
		} 

		 printf("\n\nPush any key to continue program\n");
		 
		 usleep(250000);
	 
	}
	tcflush(0, TCIFLUSH); // flush input 


 // ------- ultrasonic sensors example -----------------------------------------
	#define US_BAR_LEN 24 	// display bar length for US sensor
	#define MAX_US_DISTANCE 250.0 // max distance US
	
	// convert US value to text comment
	#define US_VAL(val) ((val)==KH4_US_DISABLED_SENSOR ? "Not activated" : ((val)==KH4_US_NO_OBJECT_IN_RANGE ? "No object in range" : ((val)==KH4_US_OBJECT_NEAR ? "Object at less than 25cm" : "Object in range 25..250cm")))
	

	
	while(!kb_kbhit())
	{	
		kb_clrscr();

		// get and print us sensors
		kh4_measure_us(Buffer,dsPic);
 		for (i=0;i<5;i++)
 		{
 			usvalues[i] = (short)(Buffer[i*2] | Buffer[i*2+1]<<8);   
 			
 			
 			if((usvalues[i] == KH4_US_DISABLED_SENSOR) || (usvalues[i] == KH4_US_NO_OBJECT_IN_RANGE))
 			{ // out of range or disabled
				sprintf(bar[i],"|\33[%dC|",US_BAR_LEN);	
			}  else
			{
				// in range or less than 25cm
				n=(int)(usvalues[i]*US_BAR_LEN/MAX_US_DISTANCE);
												                
				if (n==0)
					sprintf(bar[i],"|>\33[%dC|",US_BAR_LEN-1);
				else
					if (n>=US_BAR_LEN-1)
						sprintf(bar[i],"|\33[%dC>|",US_BAR_LEN-1);
					else
					 sprintf(bar[i],"|\33[%dC>\33[%dC|",n,US_BAR_LEN-1-n); 
			}                                 
 		}
 		
 		
		printf("\nUS sensors : distance [cm]\
		        \n                     50  100  150  200\
		        \n                0|    .    :    .    :    |%.0f\nleft 90   : %4d %s  %s\nleft 45   : %4d %s  %s\
		\nfront     : %4d %s  %s\nright 45  : %4d %s  %s\nright 90  : %4d %s  %s\n",MAX_US_DISTANCE,
			 usvalues[0],bar[0],US_VAL(usvalues[0]),usvalues[1],bar[1],US_VAL(usvalues[1]),usvalues[2],bar[2],US_VAL(usvalues[2]),usvalues[3],bar[3],US_VAL(usvalues[3]),usvalues[4],bar[4],US_VAL(usvalues[4])); 
		printf("\nPush any key to end program\n");
		usleep(200000); // wait 200ms

	}
  tcflush(0, TCIFLUSH);  // flush input key
	
	// good at the end of the program:
  kh4_set_speed(0 ,0 ,dsPic); // stop robot
  kh4_SetRGBLeds(0,0,0,0,0,0,0,0,0,dsPic); // clear rgb leds because consumes energy
  
  return 0;
}
