/* \file khepera4.c 

 *
 * \brief 
 *         This is the big application example for the Khepera4	 
 *         
 *        
 * \author   Julien Tharin (K-Team SA)                               
 *
 * \note     Copyright (C) 2015 K-TEAM SA
 * \bug      none discovered.                                         
 * \todo     nothing.


*/
#include <khepera/khepera.h>
#include <signal.h>

//#define DEBUG 1

static knet_dev_t * dsPic; // robot pic microcontroller access

int maxsp,accinc,accdiv,minspacc, minspdec; // for speed profile

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

/*!
 * Compute time difference
 *

 * \param difference difference between the two times, in structure timeval type
 * \param end_time end time
 * \param start_time start time  
 *
 * \return difference between the two times in [us]
 *
 */
long long
timeval_diff(struct timeval *difference,
             struct timeval *end_time,
             struct timeval *start_time
            )
{
  struct timeval temp_diff;

  if(difference==NULL)
  {
    difference=&temp_diff;
  }

  difference->tv_sec =end_time->tv_sec -start_time->tv_sec ;
  difference->tv_usec=end_time->tv_usec-start_time->tv_usec;

  /* Using while instead of if below makes the code slightly more robust. */

  while(difference->tv_usec<0)
  {
    difference->tv_usec+=1000000;
    difference->tv_sec -=1;
  }

  return 1000000LL*difference->tv_sec+
                   difference->tv_usec;

} /* timeval_diff() */

/*--------------------------------------------------------------------*/
/*! Braintenberg demo program
 *
 *  \param none.
 */

#define BR_IRGAIN 5
#define fwSpeed 200//150
#define BMIN_SPEED 10

#define RotSpeedL 120
#define RotSpeedR -120

#define MAX_DIST 500
#define MIN_DIST 80 // 70

#define IMOBIL_SENS 250

#define IMOBIL_POS 30.0/KH4_PULSE_TO_MM
 
int braitenbergAvoidance()
{
  int Connections_B[8] = { -2, -3, -4, -9,  4,  3,  2, 1 }; // weight of every 8 sensor for the right motor 
  int Connections_A[8] = { 2,  3,  4, -7, -4, -3, -2, 1}; // weight of every 8 sensor for the left motor 

  int i, buflen, sensval;


  int lspeed, rspeed;
  int tabsens[8];
  int left_speed, right_speed;
  unsigned int immobility = 0;
  unsigned int prevpos_left, pos_left, prevpos_right,  pos_right;
  int sensors[8];
  struct timeval startt,endt;
  int bat_voltage,bat_current,chrg_current;
  char Buffer[256];
  
  accinc=10;//3;
  accdiv=0;
  minspacc=20;
  minspdec=1;
  maxsp=400;
  // configure acceleration slope
  kh4_SetSpeedProfile(accinc,accdiv,minspacc, minspdec,maxsp,dsPic ); // Acceleration increment ,  Acceleration divider, Minimum speed acc, Minimum speed dec, maximum speed
  
  kh4_SetMode(kh4RegSpeedProfile,dsPic );
	
	printf("\nPush ANY KEY to stop!\n");

	gettimeofday(&startt,0x0);
	
  while(!kb_kbhit())
  {
    lspeed = 0; rspeed = 0;
    
    // display battery value every 30s
    gettimeofday(&endt,0x0);
		if ((timeval_diff(NULL,&endt,&startt)>30000000))
		{
				kh4_battery_status(Buffer,dsPic);
	 			printf("\nBattery: Voltage= %3.1f [V]  Average current= %4.0f [mA]  Remaining capacity= %3d %%\n", 
		        (Buffer[10] | Buffer[11]<<8)*0.00976,(short)(Buffer[6] | Buffer[7]<<8)*0.07813,Buffer[3]);
				gettimeofday(&startt,0x0);
		}	


		// get ir sensor
		kh4_proximity_ir(Buffer, dsPic);
				 
		//limit the sensor values, don't use ground sensors
		for (i = 0; i < 8; i++)	
		{
			sensval = (Buffer[i*2] | Buffer[i*2+1]<<8);
			if(sensval > MAX_DIST)
				tabsens[i] = MAX_DIST;
			else if (sensval < MIN_DIST)
				tabsens[i] = 0;
			else
				tabsens[i] = sensval-MIN_DIST; //)>>1;
			#ifdef DEBUG
			printf("%4d ",sensval);	
			#endif
		}

		

		for (i = 0; i < 8; i++)
		{
		  lspeed += Connections_A[i] * tabsens[i];
		  rspeed += Connections_B[i] * tabsens[i];				
		}

		left_speed = ((lspeed / BR_IRGAIN) + fwSpeed);
		right_speed = ((rspeed / BR_IRGAIN) + fwSpeed);

		if(left_speed >= 0 && left_speed < BMIN_SPEED)
		  left_speed = BMIN_SPEED;
		if(left_speed < 0 && left_speed > -BMIN_SPEED)
		  left_speed = -BMIN_SPEED;
		if(right_speed >= 0 && right_speed < BMIN_SPEED)
		  right_speed = BMIN_SPEED;
		if(right_speed < 0 && right_speed > -BMIN_SPEED)
		  right_speed = -BMIN_SPEED;


		kh4_set_speed(left_speed ,right_speed ,dsPic );
		
		#ifdef DEBUG
		printf("  sl: %4d  sr: %4d\n",left_speed ,right_speed);
		#endif
		
		/* Get the new position of the wheel to compare with previous values */
		kh4_get_position(&pos_left, &pos_right,dsPic);

		if((pos_left < (prevpos_left + IMOBIL_POS)) && (pos_left > (prevpos_left -IMOBIL_POS)) && (pos_right < (prevpos_right + IMOBIL_POS)) && (pos_right > (prevpos_right -IMOBIL_POS)))
		{
			
		  if(++immobility > 5)		  {
		  	#ifdef DEBUG
		  		 printf("immobility: %d\n",immobility);
		  	#endif	 
		     left_speed = RotSpeedL;
		     right_speed = RotSpeedR;

				 kh4_set_speed(left_speed ,right_speed ,dsPic );

				 do{
						usleep(1000);
						kh4_proximity_ir(Buffer, dsPic);
					}
					while ( ((Buffer[3*2] | Buffer[3*2+1]<<8) >IMOBIL_SENS)); // front sensor

		     immobility = 0;
		     prevpos_left = pos_left;
		     prevpos_right = pos_right;
		  }
		}
		else
		{

		   immobility = 0;
		   prevpos_left = pos_left;
		   prevpos_right = pos_right;
		}

	//printf("lspd = %d rspd = %d\r\n", left_speed, right_speed); 			
		
    usleep(20000); 
  }
  
  kh4_set_speed(0,0,dsPic ); // stop robot
  kh4_SetMode( kh4RegIdle,dsPic ); // set motors to idle
  accinc=3;//3;
  accdiv=0;
  minspacc=20;
  minspdec=1;
  maxsp=400;
  // configure acceleration slope
  kh4_SetSpeedProfile(accinc,accdiv,minspacc, minspdec,maxsp,dsPic ); // Acceleration increment ,  Acceleration divider, Minimum speed acc, Minimum speed dec, maximum speed
}

/*--------------------------------------------------------------------*/
/*! Line following demo program: for a black line at max of 2 cm wide
 *
 *  \param none.
 */

 
/*
#define LF_fwSpeed 120
#define LF_ROT_DIFF_SPEED 30
#define LF_ROT_SPEED LF_fwSpeed+LF_ROT_DIFF_SPEED
#define LF_ROT_SPEED_LESS LF_fwSpeed-LF_ROT_DIFF_SPEED
#define LF_ROT_SPEED_AROUND 60

#define LF_LINE_THRESH 700

#define LF_LEFT 1
#define LF_RIGHT 0
 
int line_following()
{
  int i;
  int left_ir,right_ir;
  int lspeed, rspeed;
  int last_dir=LF_RIGHT;

  struct timeval startt,endt;
  
  int bat_voltage,bat_current;
  char Buffer[256];
  
  kh4_SetMode(kh4RegSpeedProfile,dsPic );
	
	printf("\nPush ANY KEY to stop!\n");

	gettimeofday(&startt,0x0);
	
  while(!kb_kbhit())
  {
        
    // display battery value every 30s
    gettimeofday(&endt,0x0);
		if ((timeval_diff(NULL,&endt,&startt)>30000000))
		{
				kh4_battery_status(Buffer,dsPic);
	 			printf("\nBattery: Voltage = %5.3f [V]  Current=%5.0f [mA]  Remaining capacity=%3d %%\n", 
		        (Buffer[10] | Buffer[11]<<8)*0.00976,(short)(Buffer[4] | Buffer[5]<<8)*0.07813,Buffer[3]);
				gettimeofday(&startt,0x0);
		}	


	

		// get ir sensor
		kh4_proximity_ir(Buffer, dsPic);
				 
		left_ir=(Buffer[9*2] | Buffer[9*2+1]<<8);
		right_ir=(Buffer[10*2] | Buffer[10*2+1]<<8);

		if (right_ir < LF_LINE_THRESH) {
			// right on black: turn right
			lspeed=LF_ROT_SPEED;
			rspeed=LF_ROT_SPEED_LESS;
			#ifdef DEBUG
			printf("turn right\n");
			#endif
			last_dir=LF_RIGHT;
		} else 
		if (left_ir < LF_LINE_THRESH) {
			// left more on black: turn left
			lspeed=LF_ROT_SPEED_LESS;
			rspeed=LF_ROT_SPEED;
			#ifdef DEBUG
			printf("turn left\n");
			#endif
			last_dir=LF_LEFT;
		} else 
		{
			// no more on black: turn around, using last direction
			
			if (last_dir==LF_LEFT)
			{
				lspeed=-LF_ROT_SPEED_AROUND;
				rspeed=LF_ROT_SPEED_AROUND;
			} else
			{
				lspeed=LF_ROT_SPEED_AROUND;
				rspeed=-LF_ROT_SPEED_AROUND;
			} 
			#ifdef DEBUG
			printf("rotate around himself\n");
			#endif
		}

		
		kh4_set_speed(lspeed ,rspeed ,dsPic );
		
		
    usleep(20000); 
  }
  
  kh4_set_speed(0,0,dsPic ); // stop robot
  kh4_SetMode( kh4RegIdle,dsPic ); // set motors to idle
  
}
*/
 // For a black line wider than the 2 front sensors (black line at least 2.7cm wide)

#define LF_fwSpeed 90
#define LF_ROT_DIFF_SPEED 50
#define LF_ROT_SPEED 70

#define LF_LINE_THRESH 800
#define LF_ROT_SPEED_AROUND 60
#define LF_LEFT 1
#define LF_RIGHT 0 
 
int line_following()
{
  int i;
  int left_ir,right_ir;
  int lspeed, rspeed;
  int last_dir=LF_RIGHT;

  struct timeval startt,endt;
  
  int bat_voltage,bat_current;
  char Buffer[256];
  
  kh4_SetMode(kh4RegSpeedProfile,dsPic );
	
	printf("\nPush ANY KEY to stop!\n");

	gettimeofday(&startt,0x0);
	
  while(!kb_kbhit())
  {
    lspeed = 0; rspeed = 0;
    
    // display battery value every 30s
    gettimeofday(&endt,0x0);
		if ((timeval_diff(NULL,&endt,&startt)>30000000))
		{
				kh4_battery_status(Buffer,dsPic);
	 			printf("\nBattery: Voltage = %5.3f [V]  Current=%5.0f [mA]  Remaining capacity=%3d %%\n", 
		        (Buffer[10] | Buffer[11]<<8)*0.00976,(short)(Buffer[4] | Buffer[5]<<8)*0.07813,Buffer[3]);
				gettimeofday(&startt,0x0);
		}	


	

		// get ir sensor
		kh4_proximity_ir(Buffer, dsPic);
				 
		left_ir=(Buffer[9*2] | Buffer[9*2+1]<<8);
		right_ir=(Buffer[10*2] | Buffer[10*2+1]<<8);

		if ((left_ir < LF_LINE_THRESH) && (right_ir < LF_LINE_THRESH))
		{
			// both on black: go straightaway
			lspeed=LF_fwSpeed;
		  rspeed=LF_fwSpeed;
		} else
		if (left_ir < LF_LINE_THRESH) {
			// left  on black: turn left
			lspeed=LF_fwSpeed-LF_ROT_DIFF_SPEED;
			rspeed=LF_fwSpeed+LF_ROT_DIFF_SPEED;
			last_dir=LF_LEFT;
		} else 
		if (right_ir < LF_LINE_THRESH) {
			// right  on black: turn right
			lspeed=LF_fwSpeed+LF_ROT_DIFF_SPEED;
			rspeed=LF_fwSpeed-LF_ROT_DIFF_SPEED;
			last_dir=LF_RIGHT;
		} else 
		{
			// both no more on black: stop
			//lspeed=0;
			//rspeed=0;
			
						if (last_dir==LF_LEFT)
			{
				lspeed=-LF_ROT_SPEED_AROUND;
				rspeed=LF_ROT_SPEED_AROUND;
			} else
			{
				lspeed=LF_ROT_SPEED_AROUND;
				rspeed=-LF_ROT_SPEED_AROUND;
			} 
		}

		
		kh4_set_speed(lspeed ,rspeed ,dsPic );
		
		
    usleep(20000); 
  }
  
  kh4_set_speed(0,0,dsPic ); // stop robot
  kh4_SetMode( kh4RegIdle,dsPic ); // set motors to idle
  
}

/*--------------------------------------------------------------------*/
/*!
 * record a sound an play again
 */
int test_sound()
{
#define SAMPLING_FREQ 22050 // sampling frequency [Hz]

#define SAMPLE_SIZE 16 // bits of the sample

#define NB_CHANNELS 2 // 2 for stereo

#define LENGTH 5 // [s] duration of the sound

#define ENDIAN 0 //endianness: 0 little, 1 BIG

#define SIGNED 1 // sign : 0 unsigned, 1 signed

#define NB_SAMPLES (SAMPLING_FREQ*NB_CHANNELS*LENGTH*SAMPLE_SIZE/8)


	char FILENAME[] = "sound16.wav";
	char FILENAME_AMP [] = "sound16_amp.wav";
	char FILENAME_INV [] = "sound16_inv.wav";
	char sound[NB_SAMPLES];
	
	// for the sound to be loaded
	char *sound_buff=NULL; // memory will be allocated by the load function
	int data_size;
	short channels; 
	short bits_per_sample;
	int sample_rate;
	short data;	
	char line[80];
	
	int err,i;
	
	
  printf("\r\nSound example program\n(C) K-Team S.A.\r\n\r\n");

	// initialise sound
	if (kb_sound_init()<0)
	{
    fprintf(stderr,"Error: Unable to initialize the sound!\r\n");
    return -2;
  }

  // mute Ultrasounds
  kh4_activate_us(0,dsPic);

	// set the volume of the speakers	to 0 for record: needs to be off during recording otherwise
	// loop noise appears! 
	set_speakers_volume(0,0); 
	
	// set the volume of the microphones (in %)
 	set_microphones_volume(80,80);
	
	
	// configure the sound: (sampling_frequency,sample_size = nb of bits,sign: 0 = unsigned,endian : 0 = LSB,channels)
	if ((err=kb_sound_configure(SAMPLING_FREQ,SAMPLE_SIZE,SIGNED,ENDIAN,NB_CHANNELS))<0)
	{
		fprintf(stderr,"Error: Unable to configure sound: error number %d!\r\n",err);
		kb_sound_release();
    return -1;
	}
	
	
	printf("\n Record %d s of sound; press ENTER to start\n",LENGTH);
	// wait and save choice
	fgets(line,80,stdin);
	
	// set "hardware" speaker switch off
	mute_speaker(1);
	switch_speakers_ON_OFF(0);
		
	printf(" Start recording:\n");
	record_buffer(sound,NB_SAMPLES);


	// set the volume of the microphones (in %)
 	set_microphones_volume(0,0);
 	// set "hardware" speaker switch on
	switch_speakers_ON_OFF(1);
	mute_speaker(0);
	
		// set the volume of the speakers	(in %)
	set_speakers_volume(80,80); 
	

	
		
  usleep(1000000); // waits 1s
	printf("\n Play recorded sound\n");
	play_buffer(sound,NB_SAMPLES);

	wait_end_of_play(); 

	
 	printf(" saving data into file :%s\n",FILENAME);
 	save_wav_file(FILENAME,sound,NB_SAMPLES,NB_CHANNELS,SAMPLE_SIZE,SAMPLING_FREQ);
 

	// reload sound
	if ((err=load_wav_file(FILENAME,&sound_buff,&data_size,&channels,&bits_per_sample,&sample_rate))<0)
	{
		fprintf(stderr,"Error: could not open wav file %s, error number %d!\r\n",FILENAME,err);
			// free the memory for the loaded sound 
		free(sound_buff);

		switch_speakers_ON_OFF(0);
		mute_speaker(1);

		kb_sound_release();
		
		// unmute Ultrasounds
  	kh4_activate_us(31,dsPic);
		
		
		return -1;
		
	}

	printf(" amplify sound\n");

	// amplify sound
	for (i=0;i<data_size;)
	{
	 
	 if (bits_per_sample == 16)
	 {
		 data = (sound_buff[i] | sound_buff[i+1]<<8);
		 data*=1.5; // amp factor
		 
		 sound_buff[i]= 0xFF & data;
		 sound_buff[i+1]= data>>8;
		 
		 i+=2;
		 
	 } else
	 {
			sound_buff[i++]*=1.5; // amp factor	 
	 }
	}

	printf(" and play again\n");

	// play the amplified sound
	play_buffer(sound_buff,data_size);
	wait_end_of_play();

	printf("\n Saving amplified data into file :%s\n",FILENAME_AMP);
	save_wav_file(FILENAME_AMP,sound_buff,NB_SAMPLES,NB_CHANNELS,SAMPLE_SIZE,SAMPLING_FREQ);


	// realloc if 
	if (NB_SAMPLES!=data_size)
	{
     sound_buff = (char*) realloc (sound_buff, NB_SAMPLES * sizeof(char)); //set aside sound buffer space
        
    if (sound_buff == NULL)
    {
      free(sound_buff);

			switch_speakers_ON_OFF(0);
			mute_speaker(1);

			kb_sound_release();
		
			// unmute Ultrasounds
			kh4_activate_us(31,dsPic);
    	return -5;
    }	
  }     	

	printf(" invert sound\n");

	for (i=0;i<NB_SAMPLES;)
	{
		if (SAMPLE_SIZE == 16)
		{
			if (NB_CHANNELS == 2) // stereo
			{	
				sound_buff[i]= sound[NB_SAMPLES-4-i];
				sound_buff[i+1]= sound[NB_SAMPLES-3-i];
				sound_buff[i+2]= sound[NB_SAMPLES-i-2];
				sound_buff[i+3]= sound[NB_SAMPLES-i-1];
				i+=4;
			}
			else
			{
				sound_buff[i]= sound[NB_SAMPLES-2-i];
				sound_buff[i+1]= sound[NB_SAMPLES-1-i];
				i+=2;
			}

			
		 
	 } else
	 {
	 		if (NB_CHANNELS == 2) // stereo
	 		{	
				sound_buff[i]=sound[NB_SAMPLES-2-i];
				sound_buff[i+1]=sound[NB_SAMPLES-1-i]; 
				i+=2;
			} else
			{
				sound_buff[i]=sound[NB_SAMPLES-1-i]; 
				i++;
			}	
			
			
			
	 }
	}
	
	
	printf(" and play again\n");
	
	// play the sound again
	play_buffer(sound_buff,NB_SAMPLES);
	wait_end_of_play();
	
	printf("\n Saving inverted data into file :%s\n",FILENAME_INV);
	save_wav_file(FILENAME_INV,sound_buff,NB_SAMPLES,NB_CHANNELS,SAMPLE_SIZE,SAMPLING_FREQ);

	// free the memory for the loaded sound 
	free(sound_buff);
	
	printf("\nEnd of sound example\n");
	
	switch_speakers_ON_OFF(0);
	mute_speaker(1);
	
  kb_sound_release();
  
	// unmute Ultrasounds
	kh4_activate_us(31,dsPic);
  
	return 0;
}

/*--------------------------------------------------------------------*/

// #define for driver mode
#define BIG_SPEED_FACTOR 25
#define SPEED_FACTOR 1
#define MAX_SPEED 1500
#define MIN_SPEED 15
#define DEFAULT_SPEED 200
#define ROTATE_HIGH_SPEED_FACT 0.5
#define ROTATE_LOW_SPEED_FACT 0.75
#define ROT_SPEED_HIGH_TRESH 300
#define STOP_TIME 100000 // us

#define SIGN(x) ((x)>0?1:((x)<0?-1:0))  // sign or zero


/*!
 * Drive the robot with the keyboard
 *
 * \param none
 *
 * \return an error code:
 *         - <0  standard error code. See errno.h
 *         - >=0 on success
 *
 */
int drive_robot()
{
	int out=0,speed=DEFAULT_SPEED,vsl,vsr,anymove=0;
	char c;
	struct timeval startt,endt;

	
	kb_clrscr(); // erase screen
	
	printf("Drive the robot with the keyboard:\n  's' for stop\n  arrows (UP, DOWN, LEFT , RIGHT) for direction\n  PAGE UP/DOWN for changing speed  by small increments\n  Home/End for changing speed by big increments\n  'q' for going back to main menu\n");
	
	
	printf("\ndefault parameters:\n  robot speed %d  (%5.1f mm/s)  (min %d, max %d)\n\n",DEFAULT_SPEED,DEFAULT_SPEED*KH4_SPEED_TO_MM_S,MIN_SPEED,MAX_SPEED);
	
	kb_change_term_mode(1); // change terminal mode for kbhit and getchar to return immediately
	

	kh4_SetMode(kh4RegSpeed,dsPic );
	
	gettimeofday(&startt,0x0);
	
	// loop until 'q' is pushed
	while(!out)
	{
		if(kb_kbhit())
		{
			c=getchar();


			// get special keys
			if (c== 27  ) 
			{
			
			 if (c=getchar()==91) // escape with [
			 {
				 c = getchar(); 
			 
				 switch(c)
				 {
					case 65: // UP arrow = forward
							 kh4_set_speed(speed ,speed,dsPic );
							anymove=1;						
					break;
					case 66: // DOWN arrow = backward			
							 kh4_set_speed(-speed ,-speed,dsPic  );
							anymove=1;
					break;

					case 68: // LEFT arrow = left
							if (speed > ROT_SPEED_HIGH_TRESH) // at high speed, rotate too fast
								 kh4_set_speed(-speed*ROTATE_HIGH_SPEED_FACT ,speed*ROTATE_HIGH_SPEED_FACT ,dsPic );
							else
								 kh4_set_speed(-speed*ROTATE_LOW_SPEED_FACT ,speed*ROTATE_LOW_SPEED_FACT ,dsPic );
							anymove=1;	
					break;

					case 67: // RIGHT arrow = right
							if (speed > ROT_SPEED_HIGH_TRESH) // at high speed, rotate too fast
								 kh4_set_speed(speed*ROTATE_HIGH_SPEED_FACT ,-speed*ROTATE_HIGH_SPEED_FACT ,dsPic );
							else
								 kh4_set_speed(speed*ROTATE_LOW_SPEED_FACT ,-speed*ROTATE_LOW_SPEED_FACT ,dsPic );
							anymove=1;	
					break;

					case 53: // PAGE UP  = speed up
						speed+=SPEED_FACTOR;
				 		if (speed>MAX_SPEED)
				 		{
							speed=MAX_SPEED;
				 		};
				 		c = getchar(); // get last character
				 		
				 		 kh4_get_speed(&vsl,&vsr,dsPic );
				 		 kh4_set_speed(SIGN(vsl)*speed ,SIGN(vsr)*speed ,dsPic ); // set new speed, keeping direction with sign
				 		printf("\033[1`\033[Krobot speed: %d (%5.1f mm/s)",speed,speed*KH4_SPEED_TO_MM_S); // move cursor to first column, erase line and print info
				 		fflush(stdout); // make the display refresh
				 		anymove=1;
					break;

					case 54: // PAGE DOWN = speed down
						speed-=SPEED_FACTOR;
				 		if (speed<MIN_SPEED)
				 		{
							speed=MIN_SPEED;
				 		};
				 		c = getchar(); // get last character
				 		
				 		kh4_get_speed(&vsl,&vsr,dsPic );
				 		kh4_set_speed(SIGN(vsl)*speed ,SIGN(vsr)*speed,dsPic  ); // set new speed, keeping direction with sign
				 		printf("\033[1`\033[Krobot speed: %d (%5.1f mm/s)",speed,speed*KH4_SPEED_TO_MM_S); // move cursor to first column, erase line and print info
				 		fflush(stdout); // make the display refresh
				 		anymove=1;
					break;
			

					default:
					break;
					} // switch(c)
				} // escape with [
				else
				{ // other special key code
					
					 c = getchar(); 
					 
					switch(c){
				
						case 72: // Home  = speed up
							speed+=BIG_SPEED_FACTOR;
					 		if (speed>MAX_SPEED)
					 		{
								speed=MAX_SPEED;
					 		};
					 		//c = getchar(); // get last character
					 		
					 		 kh4_get_speed(&vsl,&vsr,dsPic );
					 		 kh4_set_speed(SIGN(vsl)*speed ,SIGN(vsr)*speed ,dsPic ); // set new speed, keeping direction with sign
					 		printf("\033[1`\033[Krobot speed: %d (%5.1f mm/s)",speed,speed*KH4_SPEED_TO_MM_S); // move cursor to first column, erase line and print info
					 		fflush(stdout); // make the display refresh
					 		anymove=1;
						break;

						case 70: // End = speed down
							speed-=BIG_SPEED_FACTOR;
					 		if (speed<MIN_SPEED)
					 		{
								speed=MIN_SPEED;
					 		};
					 		//c = getchar(); // get last character
					 		
					 		kh4_get_speed(&vsl,&vsr,dsPic );
					 		kh4_set_speed(SIGN(vsl)*speed ,SIGN(vsr)*speed,dsPic  ); // set new speed, keeping direction with sign
					 		printf("\033[1`\033[Krobot speed: %d (%5.1f mm/s)",speed,speed*KH4_SPEED_TO_MM_S); // move cursor to first column, erase line and print info
					 		fflush(stdout); // make the display refresh
					 		anymove=1;
						break;
						
						default:
						break	;	
						
					}  
			
				} // ether special key code
							
				
			} // if (c== '\027')	 
			else 
			{
				switch(c)
				{
				 	case 'q': // quit to main menu
				 		out=1;
				   	break;
					case 's': // stop motor
						 kh4_set_speed(0,0,dsPic);
					break;
				   
				 	default:
				   break;
				}
		  }
		  
		  gettimeofday(&startt,0x0);
		} else
		{
		
			gettimeofday(&endt,0x0);;
			// stop when no key is pushed after some time
			
			if (anymove &&  (timeval_diff(NULL,&endt,&startt)>STOP_TIME))
			{
				 kh4_set_speed(0 ,0,dsPic );
				anymove=0;
			}	
				
		}
		

		usleep(10000); // wait some ms
	} // while

	kb_change_term_mode(0); // switch to normal key input mode	
	kh4_set_speed(0,0,dsPic );	 // stop robot
	kh4_SetMode(kh4RegIdle,dsPic );
	return 0;
}

/*--------------------------------------------------------------------*/
/*!
 *  gpio and pwm example
 */
int test_gpio_and_pwm()
{
	int i,j, out=0;
	char key;
	
	int duties[2]= {100,100};
	int frequencies[2]={100,100};
	
	while(!kb_kbhit() && !out)
	{
	
	
		printf("\n--- GPIO test --- PUSH any key to stop\n\n");
		printf("all GPIO diodes should switch ON - OFF, each separately\n");
	
	
		for (i=GPIO_FIRST;i<=GPIO_LAST;i++)
		{
			printf(" GPIO %d\n",i);
			// Set gpio i to IO mode
			kb_gpio_function(i,0);

			// set direction to output and zero => activate diodes
			kb_gpio_dir_val(i,0,0);	
			usleep(200000);
			kb_gpio_set(i); // set to 1 , desactivate diodes
		
			if (kb_kbhit())
			return -2;	
		
		}
		
		
		usleep(500000);
		
		   
		printf("\n--- PWM test --- PUSH any key to stop\n\n");

	
		for (i=0;i<2;i++)
		{

			printf(" Test of PWM %d\n",i);

			kb_pwm_period(i,frequencies[i]); // currently not implemented!
			kb_pwm_duty(i,duties[i]);

			kb_pwm_activate(i);
	
		
			for (j=100;j>=0;j=j-5) {
				kb_pwm_duty(i,j);
				usleep(50000);
				if (kb_kbhit()) {
					kb_pwm_duty(i,100);
					kb_pwm_desactivate(i);
					return -3;
				}
			}


			

			printf("  LED of PWM %d should have flashed\n",i);

			kb_pwm_desactivate(i);

		}
 
 }
 
 key=getchar();
 printf("\nGPIO and PWN test finished\n");
  
 return 0; 
	
}

/*--------------------------------------------------------------------*/
/*!
 *  i2c test: needs KoreMotorLE connected
 */
int i2c_test() {
	knet_dev_t * motor;
	
	char name[]="KoreMotorLE:PriMotor1";
	
	motor = knet_open( name , KNET_BUS_I2C , 0 , NULL );

  if ( motor ) {
    unsigned int ver , rev;

    kmot_GetFWVersion( motor , &ver );
		
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

    printf("Motor %s Firmware v%u.%u\n" , name , 
	KMOT_VERSION(ver) , KMOT_REVISION(ver) );
	
		knet_close( motor );
	} else
	{
		printf("\nERROR: cannot communicate with KoreMotorLE\n\n!");
		return -1;
	}
	return 0;
}


/*--------------------------------------------------------------------*/
/*!
 *  camera example
 */

int camera_example()
{

#define WAIT_TIME 60000 // [us] => max 16.6 fps


#define DISPLAY_DIVIDE_FACTOR 8 // divider for display image in ASCII
	int SELECTED_COLOR[] = {90,160,80}; // color detection R, G, B
	int DETECTION_THRESHOLD = 25; // color detection threshold

	int G_DETECTION_THRESHOLD = 10; // green detection threshold

	int ret,x,y,i,quitr=0;
	double sum,sumc;
	
	unsigned int dWidth=192;//192;	// image width (max 752)
	unsigned int dHeight=144;//144; // image height  (max 480)
	
	unsigned char* buffer=NULL; // pointer for image buffer
	unsigned char* output_buff=NULL; // pointer for image buffer

	char *array=NULL,key;
	
	int display_image=1;  // display image
	
	int default_detect=1; // green only detect

	
	printf("\r\nCamera example program (C) K-Team S.A.\r\n\r\n");
	
		
  // camera initialisation
	if ((ret=kb_camera_init(&dWidth, &dHeight))<0)
	{
		fprintf(stderr,"camera init error %d\r\n",ret);
		free(buffer);
		return -1;
	} else
	{
		switch(ret) {
			case 1:
				printf("width adjusted to %d\r\n",dWidth);
				break;
			case 2:
				printf("height adjusted to %d\r\n",dHeight);
				break;
			case 3:
				printf("width adjusted to %d and height adjusted to %d !\r\n",dWidth,dHeight);
				break;
			default:
				break;
		}
	}
		
	// allocating memory for image
	buffer=malloc(dWidth*dHeight*3*sizeof(char));

	if (buffer==NULL)
	{
		fprintf(stderr,"could alloc image buffer!\r\n");
		free(buffer);
		return -2;
	}

	/****    taking image  ********************************************************/
	if ((ret=take_one_image(buffer))<0)
	{
		fprintf(stderr,"take image error %d\r\n",ret);
		free(buffer);
		kb_camera_release();
		return -3;
	}


	// saving image
	if ((ret=save_buffer_to_jpg("original.jpg",70,buffer))<0)
	{
		fprintf(stderr,"save image error %d\r\n",ret);
		free(buffer);
		kb_camera_release();
		return -4;
	}
	
	
  /****  transform into greyscale  **********************************************/
  
	// into greyscale
	printf("\n  converting into greyscale\n");
	into_greyscale(buffer);
	
	// saving image
	if ((ret=save_buffer_to_jpg("greyscale.jpg",70,buffer))<0)
	{
		fprintf(stderr,"save image error %d\r\n",ret);
		free(buffer);
		kb_camera_release();
		return -4;
	}
	
 /****   apply Sobel filter for detecting edges  ******************************/
	
	printf("\n  applying Sobel filter for detecting edges...\n");
	
	// allocate memory for output
	output_buff=malloc(dWidth*dHeight*3*sizeof(char));
	
	#define FILTER_SIZE 3
	
	int filterx[FILTER_SIZE*FILTER_SIZE]={-1,0,1,-2,0,2,-1,0,1}; // {[0][0],[0][1],[0][2]],[1][0],[1][1],[1][2]],...}
	int filtery[FILTER_SIZE*FILTER_SIZE]={1,2,1,0,0,0,-1,-2,-1};
	
	apply_filter(buffer,output_buff,filterx,filtery,FILTER_SIZE,FILTER_SIZE);
	
	// saving image
	if ((ret=save_buffer_to_jpg("edges.jpg",70,output_buff))<0)
	{
		fprintf(stderr,"save image error %d\r\n",ret);
		free(buffer);
		kb_camera_release();
		return -4;
	}
	
	free(output_buff);
	
	
	/****   capture and process directly mutli frames ***************/

		
	if(kb_captureStart()<0)
  {
		free(buffer);
		kb_camera_release();
		fprintf(stderr,"ERROR: capture start error in mutli frames!\r\n");
    return -3;
  }

  usleep(100000); // wait for 100ms initialisation

  // allocate memory for output
	output_buff=malloc(dWidth*dHeight*3*sizeof(char));
	
	array=malloc(dWidth/DISPLAY_DIVIDE_FACTOR*dHeight/DISPLAY_DIVIDE_FACTOR*sizeof(char));
  memset(&array[0], 0,dWidth/DISPLAY_DIVIDE_FACTOR*dHeight/DISPLAY_DIVIDE_FACTOR);

  kb_change_term_mode(1); // change terminal mode for kbhit and getchar to return immediately

	
  while (!quitr)
  {		

  
	  if ((ret=kb_frameRead(buffer))<0)
  	{
  		fprintf(stderr,"ERROR: frame capture error %d!\r\n",ret);
  	} else
  	{
  	  
  	  
  	  // find positions of pixels having a similar color		  
			 
			for (y=0; y<dHeight;y++)
			{
				for (x=0; x<dWidth;x++)
				{
					i=3*(x+y*dWidth); // compute array index
					
					ret=0;
								
					if (default_detect)
					{
					
						// easy green detection	 
						if ( (buffer[i+1]> buffer[i]*(100+G_DETECTION_THRESHOLD)/100.0) && (buffer[i+1]> buffer[i+2]*(100+G_DETECTION_THRESHOLD)/100.0) )	
							ret=1;
					}
					
					else
					{ // detect any SELECTED_COLOR[] color  +/- %DETECTION_THRESHOLD 
						// if in R range
						
						sum=buffer[i]+buffer[i+1]+buffer[i+2];
						if (sum<=0)
							sum=1;
							
						sumc=SELECTED_COLOR[0]+SELECTED_COLOR[1]+SELECTED_COLOR[2];
						if (sumc<=0)
							sumc=1;	
						
						if ( ((buffer[i]/sum>(100-DETECTION_THRESHOLD)*SELECTED_COLOR[0]/100.0/sumc) &&  (buffer[i]/sum<(100+DETECTION_THRESHOLD)*SELECTED_COLOR[0]/100.0/sumc))
					   // and in G range
					  && ((buffer[i+1]/sum>(100-DETECTION_THRESHOLD)*SELECTED_COLOR[1]/100.0/sumc) &&  (buffer[i+1]/sum<(100+DETECTION_THRESHOLD)*SELECTED_COLOR[1]/100.0/sumc))
						// and in B range
						 && ((buffer[i+2]/sum>(100-DETECTION_THRESHOLD)*SELECTED_COLOR[2]/100.0/sumc) &&  (buffer[i+2]/sum<(100+DETECTION_THRESHOLD)*SELECTED_COLOR[2]/100.0/sumc)) )
						{
						 	ret=1;
						}
					} 
						 
	 
					if (ret) 
					{
						output_buff[i]=255;  	 // R
						output_buff[i+1]=buffer[i+1]; // G
						output_buff[i+2]=buffer[i+2]; // B 
						
						if ((x %DISPLAY_DIVIDE_FACTOR == 0) && (y %DISPLAY_DIVIDE_FACTOR ==0)) // print only at every DISPLAY_DIVIDE_FACTORth column and row
							array[(x+y*dWidth/DISPLAY_DIVIDE_FACTOR)/DISPLAY_DIVIDE_FACTOR]='*'; // something	
					} else
					{
						output_buff[i]=buffer[i];  	 // R
						output_buff[i+1]=buffer[i+1]; // G
						output_buff[i+2]=buffer[i+2]; // B 
						if ((x %DISPLAY_DIVIDE_FACTOR == 0 ) && (y %DISPLAY_DIVIDE_FACTOR ==0)) // print only at every DISPLAY_DIVIDE_FACTORth column and row
							array[(x+y*dWidth/DISPLAY_DIVIDE_FACTOR)/DISPLAY_DIVIDE_FACTOR]=' '; // nothing
					}
					
				} // for x			
			} // for y
	
			kb_clrscr(); //clear screen  
	  
			printf("Multi frames example\n");
			printf("\nkeys:");
			printf(" anykey to stop\n");
			printf("      i      for toggling display of ascii image\n");
			printf("      d      for toggling default (green) and custom detection :current: %s\n            (custom parameters: Red:%3d Green:%3d Blue:%3d Threshold:%3d%%)\n",default_detect?"DEFAULT":"CUSTOM",SELECTED_COLOR[0],SELECTED_COLOR[1],SELECTED_COLOR[2],default_detect?G_DETECTION_THRESHOLD:DETECTION_THRESHOLD);
			printf("      r/t    change red detection\n");
			printf("      g/h    change green detection\n");
			printf("      b/n    change blue detection\n");
			printf("      c/v    change detection threshold\n");
 	  
  	  printf("\n move the object in front of the camera (default: green STABILO BOSS (R))...\n\n");
		
			// print processed image
  	  if(display_image)
		  {
		  
				printf("--------------------------\n");
				for (y=0;y<dHeight/DISPLAY_DIVIDE_FACTOR;y++)
				{
					putc('|',stdout);
					fwrite(array+(y*dWidth/DISPLAY_DIVIDE_FACTOR),1,dWidth/DISPLAY_DIVIDE_FACTOR,stdout);
					putc('|',stdout);
					putc('\n',stdout);
					
					
					
				}
				printf("--------------------------\n");
			}
  	  
  	} // if framebuffer
		

		usleep(WAIT_TIME); 

		if(kb_kbhit())
		{
			key=getchar();

		  switch(key)
		  {
		   	case 'i': // toggle display image
		   		display_image=display_image?0:1;
		     	break;
		    case 'd': // toggle detection type
		   		default_detect=!default_detect;
		     	break;
		    case 'r': // change +r
					 SELECTED_COLOR[0]++;
					 if (SELECTED_COLOR[0] >255)
					 	SELECTED_COLOR[0]=0;
		     	break;
		    case 't': // change -r
					 SELECTED_COLOR[0]--;
					 if (SELECTED_COLOR[0] <0)
					 	SELECTED_COLOR[0]=255;
		     	break;
		    case 'g': // change +g
		    		SELECTED_COLOR[1]++;
					 if (SELECTED_COLOR[1] >255)
					 	SELECTED_COLOR[1]=0;				
		    break;
		    case 'h': // change -g
		    		SELECTED_COLOR[1]--;
					 if (SELECTED_COLOR[1]<0)
					 	SELECTED_COLOR[1]=255;				
		    break;
		    case 'b': // change +b
		    		SELECTED_COLOR[2]++;
					 if (SELECTED_COLOR[2] >255)
					 	SELECTED_COLOR[2]=0;				
		    break;
		     case 'n': // change -b
		    		SELECTED_COLOR[2]--;
					 if (SELECTED_COLOR[2] <0)
					 	SELECTED_COLOR[2]=255;				
		    break;		
		    case 'c': // change +t
		    	if (default_detect) {
		    			  G_DETECTION_THRESHOLD++;
						 if (G_DETECTION_THRESHOLD >100)
						 	G_DETECTION_THRESHOLD=0;
		    		} else {
		    			DETECTION_THRESHOLD++;
						 if (DETECTION_THRESHOLD >100)
						 	DETECTION_THRESHOLD=0;
		    		}				
		    break;	 
		    case 'v': // change -t
		    	if (default_detect) {
		    		G_DETECTION_THRESHOLD--;
					 if (G_DETECTION_THRESHOLD <0)
					 	G_DETECTION_THRESHOLD=100;
		    	} else {
		    		DETECTION_THRESHOLD--;
					 if (DETECTION_THRESHOLD <0)
					 	DETECTION_THRESHOLD=100;	
					 	}			
		    break;		   
		   	default:
		      printf("\r\n Another key pushed, quitting...\r\n");
		      quitr=1;
		      break;
		  }
		}

	} // while

	kb_change_term_mode(0); // switch to normal key input mode

  free(array); // free output display array

	// saving image with selected color
	if ((ret=save_buffer_to_jpg("selected_color.jpg",100,output_buff))<0)
	{
		fprintf(stderr,"save image error %d\r\n",ret);
		free(buffer);
		free(output_buff);
		kb_camera_release();
		return -4;
	}
	
	free(output_buff);

  if (kb_captureStop()<0)
  {
    fprintf(stderr,"ERROR: capture stop error in mutli frames!\r\n");
  }
	
	// releasing camera
	kb_camera_release();
	
	// free image memory
	free(buffer);
	
	printf("\nImages taken successfully:\n\n  original saved under name ""original.jpg""\n  selected color under name  ""selected_color.jpg""\n  Greyscale under name ""greyscale.jpg""\n  Edge detection with Sobel filter under name with ""edges.jpg""\n\n");
	
	return 0;
}

/*--------------------------------------------------------------------*/
/*!
 * Main
 */
int main(int argc , char * argv[]) 
{ 
 
#define IR_BAR_LEN 15 	// display bar length for IR sensor
#define US_BAR_LEN 23 	// display bar length for US sensor
#define ACGY_BAR_LEN 30 // display bar length for Accel/gyro sensor
#define MAX_US_DISTANCE 250.0 // max distance US
#define MAX_G 2 		// max acceleration in g

// convert US value to text comment
#define US_VAL(val) ((val)==KH4_US_DISABLED_SENSOR ? "Not activated" : ((val)==KH4_US_NO_OBJECT_IN_RANGE ? "No object in range" : ((val)==KH4_US_OBJECT_NEAR ? "Object at less than 25cm" : "Object in range 25..250cm")))

  double fpos,dval,dmean;
  long lpos,rpos;
  char Buffer[100],bar[12][64],revision,version;
  int i,n,type_of_test=0,sl,sr,pl,pr;
  short index, value,sensors[12],usvalues[5];
  char c;
  long motspeed;
  char line[80],l[9];
  int kp,ki,kd;
  int pmarg;
  
  // initiate libkhepera and robot access
  if ( kh4_init(argc ,argv)!=0)
  {
  	printf("\nERROR: could not initiate the libkhepera!\n\n");
  	return -1;
  }	

  /* open robot socket and store the handle in their respective pointers */
  dsPic  = knet_open( "Khepera4:dsPic" , KNET_BUS_I2C , 0 , NULL );

	if ( dsPic==NULL)
  {
  	printf("\nERROR: could not initiate communication with Kh4 dsPic\n\n");
  	return -2;
  }	

  /* initialize the motors controlers*/
   
  /* tuned parameters */
  pmarg=20;
  kh4_SetPositionMargin(pmarg,dsPic ); 				// position control margin
  kp=10;
  ki=5;
  kd=1;
  kh4_ConfigurePID( kp , ki , kd,dsPic  ); 		// configure P,I,D
  
  accinc=3;//3;
  accdiv=0;
  minspacc=20;
  minspdec=1;
  maxsp=400;
  // configure acceleration slope
  kh4_SetSpeedProfile(accinc,accdiv,minspacc, minspdec,maxsp,dsPic ); // Acceleration increment ,  Acceleration divider, Minimum speed acc, Minimum speed dec, maximum speed
  
	kh4_SetMode( kh4RegIdle,dsPic );  				// Put in idle mode (no control)

  // get revision
  if(kh4_revision(Buffer, dsPic)==0){
   	version=(Buffer[0]>>4) +'A';
  	revision=Buffer[0] & 0x0F; 
    printf("\r\nVersion = %c, Revision = %u\r\n",version,revision);        
  }
  

  signal( SIGINT , ctrlc_handler ); // set signal for catching ctrl-c
  
  /* For ever loop until ctrl-c key */
  while(quitReq==0)
  {
  	
  	// print menu
		printf("\nChoose an option: enter (letter) then [its argument(s)], then push ENTER:\n\n");
		
		printf("  (a)mbiant ir sensors\n");
		printf("  (ag) accel and gyro sensors\n");
		printf("  (b)attery status\n");
		printf("  (br)aitenberg obstacle avoidance mode\n");
		printf("  (c)amera demo\n");
		printf("  (d)rive the robot with keyboard\n");
		printf("  (i)2c test with KoreMotorLE board\n");
		printf("  (g)et motors speed and position\n");
		printf("  (gp) test pwm and gpios\n");
		printf("  (l)eds [ lr lg lb rr rg rb br bg bb] (max 63)\n");
		printf("  (li)ne following mode\n");
		printf("  (ms) motor speed [left] [right] speed in [pulse/%dms]  (1 pulse/%dms = %.3f mm/s)\n",KH4_TIME_BTWN,KH4_TIME_BTWN,KH4_SPEED_TO_MM_S);
		printf("  (msp) motor speed profile [left] [right] speed in [pulse/%dms]  (1 pulse/%dms = %.3f mm/s)\n",KH4_TIME_BTWN,KH4_TIME_BTWN,KH4_SPEED_TO_MM_S);
		printf("  (mso) motor speed open loop [left] [right] speed ( %d = 100 %%)\n",KH4_MAX_OPENLOOP_SPEED_PWM_100);
		printf("  (mp) motor position [left] [right] absolute position in [pulses]  (1 pulse = %.6f mm)\n",KH4_PULSE_TO_MM);
		printf("  (p)roximity ir sensors\n");	
		printf("  (pi)d [p i d]  current: %d %d %d\n",kp,ki,kd);
		printf("  (pm) position margin [pm]  current: %d\n",pmarg);
		printf("  (s)top motor\n");
		printf("  (sp) speed profile [Acc_inc  Acc_div Min_speed_acc  Min_speed_dec Max_speed]  current: %d %d %d %d %d\n",accinc,accdiv,minspacc, minspdec,maxsp); 
		printf("  (so)und demo\n");	
		printf("  (st)atus led (red on = 1, green on = 16, both on = 0, both off = 17)\n");		
		printf("  (re)set encoders\n");
		printf("  (rm) reset the microcontroller\n");
		printf("  (q)uit program\n");
		printf("  (u)ltrasonic sensors\n");
		printf("  (ua) ultrasonic sensors activate (1:left, 2:front left, 4:front, 8:front right, 16:right, all:31)\n");

		
		printf("\noption: "); 
		
		// wait and save choice
		fgets(line,80,stdin);
		
		
		// applay selected choice
		switch(line[0])
		{
			case 'a': // accel and gyro or ambiant ir sensors
				if ((strlen(line)>1 && (line[1]=='g')))
				{		
					while(!kb_kbhit())
					{	
						kb_clrscr();
						// get gyro sensor
						printf("\ngyro sensor [deg/s]\n       new data                                                      old data    average\ngyro X: ");
						kh4_measure_gyro(Buffer, dsPic);
						dmean=0;
						for (i=0;i<10;i++)
						{
							dval=((short)((Buffer[i*2] | Buffer[i*2+1]<<8)))*KH4_GYRO_DEG_S;
							printf("%6.1f ",dval);
							dmean+=dval;                                                       
						}   
						
						printf(" %6.1f",dmean/10.0);

						printf("\ngyro Y: ");
						dmean=0;
						for (i=10;i<20;i++)
						{

							dval=((short)((Buffer[i*2] | Buffer[i*2+1]<<8)))*KH4_GYRO_DEG_S;
							printf("%6.1f ",dval);
							dmean+=dval;                               	                                                                   
						} 
						printf(" %6.1f",dmean/10.0);
							
						printf("\ngyro Z: ");
						dmean=0;
						for (i=20;i<30;i++)
						{
							dval=((short)((Buffer[i*2] | Buffer[i*2+1]<<8)))*KH4_GYRO_DEG_S;
							printf("%6.1f ",dval);
							dmean+=dval;                               	                                                                   
						}
						printf(" %6.1f",dmean/10.0);     
						printf("\n");



						// get accel sensor
						kh4_measure_acc((char *)Buffer, dsPic);

						printf("\nAcceleration sensor [g]\n       new data                                            old data  average  [g]: -2     -1      0      1      2\nacc  X: ");
						dmean=0;
						for (i=0;i<10;i++)
						{
							dval=((short)(Buffer[i*2] | Buffer[i*2+1]<<8)>>4)/1000.0;
							printf("%5.2f ",dval);
							dmean+=dval;                                                                                       
						}
						
						dval=dmean/10.0;
						
						printf(" %5.2f",dval);   
						
						// compute bar index
						n = (int)abs(dval*ACGY_BAR_LEN/MAX_G /2.0); 
						// fill up bar
						if (dval < 0)
						{
							for (i=0;i<ACGY_BAR_LEN/2-n;i++)
								bar[0][i]=' ';
						 	for (i=ACGY_BAR_LEN/2-n;i<ACGY_BAR_LEN/2;i++)
								bar[0][i]='-';
							bar[0][ACGY_BAR_LEN/2]='0';								
							for (i=ACGY_BAR_LEN/2+1;i<ACGY_BAR_LEN;i++)
								bar[0][i]=' ';  
						}
						else
						{
							for (i=0;i<ACGY_BAR_LEN/2;i++)
								bar[0][i]=' '; 
								
					    bar[0][ACGY_BAR_LEN/2]='0';	
					
							for (i=ACGY_BAR_LEN/2+1;i<ACGY_BAR_LEN/2+n+1;i++)
							  bar[0][i]='+';
				 
							for (i=ACGY_BAR_LEN/2+n+1;i<ACGY_BAR_LEN;i++)
								bar[0][i]=' ';
						}
								 
				 	  bar[0][ACGY_BAR_LEN/2]='0';
					
						bar[0][ACGY_BAR_LEN]='\0';
					
						printf("        |%s|",bar[0]);
							
						printf("\nacc  Y: ");
						dmean=0;
						for (i=10;i<20;i++)
						{
							dval=((short)(Buffer[i*2] | Buffer[i*2+1]<<8)>>4)/1000.0;
							printf("%5.2f ",dval);
							dmean+=dval;
						}
						
						dval=dmean/10.0;
						printf(" %5.2f",dval);
						
												// compute bar index
						n = (int)abs(dval*ACGY_BAR_LEN/MAX_G /2.0); 
						// fill up bar
						if (dval < 0)
						{
							for (i=0;i<ACGY_BAR_LEN/2-n;i++)
								bar[0][i]=' ';
						 	for (i=ACGY_BAR_LEN/2-n;i<ACGY_BAR_LEN/2;i++)
								bar[0][i]='-';
							bar[0][ACGY_BAR_LEN/2]='0';
								
							for (i=ACGY_BAR_LEN/2+1;i<ACGY_BAR_LEN;i++)
								bar[0][i]=' ';  
						}
						else

						{
							for (i=0;i<ACGY_BAR_LEN/2;i++)
								bar[0][i]=' '; 
								

					    bar[0][ACGY_BAR_LEN/2]='0';	
					
							for (i=ACGY_BAR_LEN/2+1;i<ACGY_BAR_LEN/2+n+1;i++)
							  bar[0][i]='+';
				 
							for (i=ACGY_BAR_LEN/2+n+1;i<ACGY_BAR_LEN;i++)

								bar[0][i]=' ';
						}
								 
				 	  bar[0][ACGY_BAR_LEN/2]='0';
					
						bar[0][ACGY_BAR_LEN]='\0';
					
						printf("        |%s|",bar[0]);
						
						printf("\nacc  Z: ");
						dmean=0;
						for (i=20;i<30;i++)
						{
							dval=((short)(Buffer[i*2] | Buffer[i*2+1]<<8)>>4)/1000.0;
							printf("%5.2f ",dval);
							dmean+=dval;
						}
						
						dval=dmean/10.0;
						printf(" %5.2f",dval);
						
						// compute bar index
						n = (int)abs(dval*ACGY_BAR_LEN/MAX_G/2.0); 
						// fill up bar
						if (dval < 0)
						{
							for (i=0;i<ACGY_BAR_LEN/2-n;i++)
								bar[0][i]=' ';
						 	for (i=ACGY_BAR_LEN/2-n;i<ACGY_BAR_LEN/2;i++)
								bar[0][i]='-';
							bar[0][ACGY_BAR_LEN/2]='0';
								
							for (i=ACGY_BAR_LEN/2+1;i<ACGY_BAR_LEN;i++)
								bar[0][i]=' ';  
						}
						else
						{
							for (i=0;i<ACGY_BAR_LEN/2;i++)
								bar[0][i]=' '; 
								
					    bar[0][ACGY_BAR_LEN/2]='0';	
					
							for (i=ACGY_BAR_LEN/2+1;i<ACGY_BAR_LEN/2+n+1;i++)
							  bar[0][i]='+';
				 
							for (i=ACGY_BAR_LEN/2+n+1;i<ACGY_BAR_LEN;i++)
								bar[0][i]=' ';
						}
								 
				 	  bar[0][ACGY_BAR_LEN/2]='0';
					
						bar[0][ACGY_BAR_LEN]='\0';
					
						printf("        |%s|",bar[0]);
						
						
						printf("\n\nPush any key to stop\n");
						usleep(200000); // wait 200ms
					}	
				}
				else
				{
					while(!kb_kbhit())
					{	
						kb_clrscr();	
						// get ir sensor
						kh4_ambiant_ir(Buffer, dsPic);
				
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
						 printf("\n                   bright                dark\
						 \nback left      : %4.4u  %s\nleft           : %4.4u  %s\
						 \nfront left     : %4.4u  %s\nfront          : %4.4u  %s\
						 \nfront right    : %4.4u  %s\nright          : %4.4u  %s\
						 \nback right     : %4.4u  %s\nback           : %4.4u  %s\
						 \nground left    : %4.4u  %s\ngnd front left : %4.4u  %s\
						 \ngnd front right: %4.4u  %s\nground right   : %4.4u  %s\n",
							 sensors[0],bar[0],  sensors[1],bar[1],
							 sensors[2],bar[2],  sensors[3],bar[3],
							 sensors[4],bar[4],  sensors[5],bar[5],
							 sensors[6],bar[6],  sensors[7],bar[7],
							 sensors[8], bar[8], sensors[9], bar[9],
							 sensors[10], bar[10],sensors[11], bar[11]
							 );
		
						printf("\nPush any key to stop\n");
						usleep(200000); // wait 200ms
					}
				}
				tcflush(0, TCIFLUSH);
				
			break;
			case 'b': // braitenberg or battery status
				if ((strlen(line)>2 && (line[1]=='r'))) {
					// braitenberg
					braitenbergAvoidance();
				}
				else
				{ // battery status
					while(!kb_kbhit())
					{	
						kb_clrscr();	
						kh4_battery_status(Buffer,dsPic);
						printf("Battery:\n  status (DS2781)   :  0x%x\n",Buffer[0]);
						printf("  remaining capacity:  %4.0f mAh\n",(Buffer[1] | Buffer[2]<<8)*1.6);
						printf("  remaining capacity:   %3d %%\n",Buffer[3]);
						printf("  current           :  %4.0f mA\n",(short)(Buffer[4] | Buffer[5]<<8)*0.07813);
						printf("  average current   :  %4.0f mA\n",(short)(Buffer[6] | Buffer[7]<<8)*0.07813);
						printf("  temperature       :  %3.1f C \n",(short)(Buffer[8] | Buffer[9]<<8)*0.003906);
						printf("  voltage           :  %4.0f mV \n",(Buffer[10] | Buffer[11]<<8)*9.76);
						printf("  charger           :  %s\n",kh4_battery_charge(dsPic)?"plugged":"unplugged");
						printf("\nPush any key to stop\n");
						usleep(200000); // wait 200ms
					}
				}
				tcflush(0, TCIFLUSH);
			break;			
			case 'c': // camera
				camera_example();
			break;
		
			case 'd': // drive mode
				drive_robot();
			break;
		
			case 'g': // test gpio and pwm  or get motor speed and position		
				if ((strlen(line)>2 && (line[1]=='p'))) {	
					test_gpio_and_pwm();
				}
				else
				{ // get motor speed and position		
					while(!kb_kbhit())
					{	
						kb_clrscr();		
						kh4_get_speed(&sl,&sr,dsPic);
						kh4_get_position(&pl,&pr,dsPic);
						printf("motors speed [mm/s (pulse/)]: left: %7.1f  (%5d)  | right: %7.1f (%5d)\n",sl*KH4_SPEED_TO_MM_S,sl,sr*KH4_SPEED_TO_MM_S,sr);
						printf("motors position [mm (pulse)]: left: %7.1f (%7d) | right: %7.1f (%7d)\n",pl*KH4_PULSE_TO_MM,pl,pr*KH4_PULSE_TO_MM,pr);
					
						printf("\nPush any key to stop\n");
						usleep(200000); // wait 200ms
					
					}
					tcflush(0, TCIFLUSH); // flush input	
				}
			break;
			
			
			case 'i': // i2c tests
				i2c_test();
			break;
			
			case 'l': // line following or leds
				if ((strlen(line)>2 && (line[1]=='i'))) {
					line_following();
				}
				else {
				// leds
					if (EOF==sscanf(line,"%*c %d %d %d %d %d %d %d %d %d",&l[0],&l[1],&l[2],&l[3],&l[4],&l[5],&l[6],&l[7],&l[8]))
						printf("\n*** ERROR ***: the led commands must be 9 integers\n");
					else	
					{	
						kh4_SetRGBLeds(l[0],l[1],l[2],l[3],l[4],l[5],l[6],l[7],l[8], dsPic);
						printf("rgb %d %d %d %d %d %d %d %d %d\n",l[0],l[1],l[2],l[3],l[4],l[5],l[6],l[7],l[8]);
					}	
				}
			break;		
			case 'm': // set motors speed profile or speed or position
				if ((strlen(line)>2 && (line[1]=='s') && (line[2]=='p')))
				{ // speed profile
					sl=sr=0;
					if (EOF==sscanf(line,"%*c %*c %*c %d %d",&sl,&sr))
						printf("\n*** ERROR ***: the motors speeds must be integers\n");
					else
						printf("\nspeeds to set: %d %d with speed profile\n",sl,sr);	
						kh4_SetMode(kh4RegSpeedProfile,dsPic );
						kh4_set_speed(sl,sr, dsPic);
				}	else
				if ((strlen(line)>2 && (line[1]=='s') && (line[2]=='o')))
				{ // speed openloop
					sl=sr=0;
					if (EOF==sscanf(line,"%*c %*c %*c %d %d",&sl,&sr))
						printf("\n*** ERROR ***: the motors speeds must be integers\n");
					else
						printf("\nspeeds to set: %d %d with openloop\n",sl,sr);	
						kh4_SetMode(kh4RegSOpenLoop,dsPic );
						kh4_set_speed(sl,sr, dsPic);
				}	else
				if ((strlen(line)>2 && (line[1]=='s')))
				{ // speed
					sl=sr=0;
					if (EOF==sscanf(line,"%*c %*c %d %d",&sl,&sr))
						printf("\n*** ERROR ***: the motors speeds must be integers\n");
					else
						printf("\nspeeds to set: %d %d with speed\n",sl,sr);	
						kh4_SetMode(kh4RegSpeed,dsPic );
						kh4_set_speed(sl,sr, dsPic);
				}	else
				if ((strlen(line)>1 && (line[1]=='p')))
				{ // position
					sl=sr=0;
					if (EOF==sscanf(line,"%*c %*c %d %d",&sl,&sr))
						printf("\n*** ERROR ***: the motors positions must be integers\n");
					else
						printf("\nposition to set: %d %d with position regulation\n",sl,sr);	
						kh4_SetMode(kh4RegPosition,dsPic );
						kh4_set_position(sl,sr, dsPic);
				}
			break;
			case 'p':  //pid,position margin, proximity sensors
				if ((strlen(line)>2 && (line[1]=='i')))
				{ // pid
					if (EOF!=sscanf(line,"%*c%*c %d %d %d",&kp,&ki,&kd))
					{
						kh4_ConfigurePID(kp,ki,kd,dsPic);
					}
				}
				else
				if ((strlen(line)>2 && (line[1]=='m')))
				{ // position margin
					if (EOF!=sscanf(line,"%*c%*c %d",&pmarg))
					{
						kh4_SetPositionMargin(pmarg,dsPic);
					}
				}
				else
				{ // proximity sensors
					while(!kb_kbhit())
					{	
						kb_clrscr();		
		
						// get ir sensor
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
		
						printf("\nPush any key to stop\n");
						usleep(200000); // wait 200ms

					}
				}
				tcflush(0, TCIFLUSH); // flush input	
			break;	
			case 'q': // quit
				quitReq=1;
				break;
			case 'r': // reset  encoders the microcontroller
				if ((strlen(line)>1 && (line[1]=='e')))
				{ //or encoders
					kh4_ResetEncoders(dsPic);
				}
				else
					if ((strlen(line)>1 && (line[1]=='m')))
					{// reset the microcontroller 
						kh4_SetStatusLeds(KH4_ST_LED_RED_ON, dsPic);
						usleep(10000); // wait 10ms
						kh4_reset(dsPic);
						printf("\nWait for microcontroller to reset...\n");
						sleep(3); // wait for Koala to restart
					}	
			break;
			case 's': // status leds, speed profile or stop motor 
				if ((strlen(line)>2 && (line[1]=='t')))
				{
					if (EOF!=sscanf(line,"%*c%*c %d",&n))
					{
						// status leds
						kh4_SetStatusLeds(n,dsPic);				
					}
				}
				else					
				if ((strlen(line)>2 && (line[1]=='p')))
				{ // speed profile
					if (EOF!=sscanf(line,"%*c%*c %d %d %d %d %d",&accinc,&accdiv,&minspacc, &minspdec,&maxsp))
					{
						kh4_SetSpeedProfile(accinc,accdiv,minspacc, minspdec,maxsp,dsPic );	
					}
				}
				else
				if ((strlen(line)>2 && (line[1]=='o')))
				{
					test_sound();
				}
				else
				{
					//stop motor
					kh4_set_speed(0 ,0 ,dsPic); // stop robot
					kh4_SetMode( kh4RegIdle,dsPic ); // set motors to idle
				}		
			break;
			case 'u': // us sensors
				if ((strlen(line)>2 && (line[1]=='a')))
				{ // activate us
					if (EOF!=sscanf(line,"%*c%*c %d",&c))
					{
						kh4_activate_us(c,dsPic);				
					}
				}
				else
				{ // display us
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
					tcflush(0, TCIFLUSH); // flush input
				}	
			break;
			default: printf("\n*** ERROR ***: option '%c' (0x%02x) is undefined!\n\n",line[0],line[0]);		
		}

  } // while
  
	
  kh4_set_speed(0 ,0 ,dsPic); // stop robot
  kh4_SetMode( kh4RegIdle,dsPic ); // set motors to idle
  kh4_SetRGBLeds(0,0,0,0,0,0,0,0,0,dsPic); // clear rgb leds because consumes energy
  
	return 0;
}
