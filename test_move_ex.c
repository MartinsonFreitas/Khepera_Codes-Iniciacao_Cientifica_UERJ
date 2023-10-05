#include <khepera/khepera.h>
#include <signal.h>


static knet_dev_t * dsPic; // robot pic microcontroller access



static int quitReq = 0; // quit variable for loop

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
  
  
  printf("\nKhepera 4 test move ex\n");
  
  // initiate libkhepera and robot access
  if ( kh4_init(0,NULL)!=0)
  {
  	printf("\nERROR: could not initiate the libkhepera!\n\n");
  	return -1;
  }	

  /* open robot socket and store the handle in its pointer */
  dsPic  = knet_open( "Khepera4:dsPic" , KNET_BUS_I2C , 0 , NULL );

if ( dsPic==NULL)  {
  	printf("\nERROR: could not initiate communication with Kh4 dsPic\n\n");
  	return -2;
  	}	



/* ------  inicializa os controles dos motores --------------------------------*/
   
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
  

 // signal( SIGINT , ctrlc_handler ); // set signal for catching ctrl-c
  


//    Talvez tenha que definir a velocidade antes 



 // -------------------------- motores ------------------------------ //
	// ============================== INICIO =================================== //

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
	
	// ============================== FIM =================================== //

tcflush(0, TCIFLUSH);  // flush input key


kh4_set_speed(0 ,0 ,dsPic); // stop robot

//kh4_SetRGBLeds(0,0,0,0,0,0,0,0,0,dsPic); // clear rgb leds because consumes energy
  
return 0;

}



