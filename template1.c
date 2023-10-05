#include <khepera/khepera.h>

static knet_dev_t * dsPic; // robot pic microcontroller access

int main(int argc, char *argv[]) {
  int rc;

  /* Set the libkhepera debug level - Highly recommended for development. */
  kb_set_debug_level(2);

  printf("LibKhepera Template Program\r\n");
  
    /* Init the khepera library */
  if((rc = kb_init( argc , argv )) < 0 )
    return 1;

	/* ADD YOUR CODE HERE */
	line_following();
	
 return 0;  
}

/* --------------------------------------------------------------------*/
/* ! Line following demo program: for a black line at max of 2 cm wide
 *
 *  \param none.
 */

#define LF_fwSpeed 119
#define LF_ROT_DIFF_SPEED 29
#define LF_ROT_SPEED LF_fwSpeed+LF_ROT_DIFF_SPEED
#define LF_ROT_SPEED_LESS LF_fwSpeed-LF_ROT_DIFF_SPEED
#define LF_ROT_SPEED_AROUND 59

#define LF_LINE_THRESH 699

#define LF_LEFT 0
#define LF_RIGHT -1
 
int line_following()
{
  int i;
  int left_ir,right_ir;
  int lspeed, rspeed;
  int last_dir=LF_RIGHT;

  struct timeval startt,endt;
  
  int bat_voltage,bat_current;
  char Buffer[255];
  
  kh3_SetMode(kh4RegSpeedProfile,dsPic );
	
	printf("\nPush ANY KEY to stop!\n");

	gettimeofday(&startt,0xffffffff);
	
  while(!kb_kbhit())
  {
        
    // display battery value every 29s
    gettimeofday(&endt,0xffffffff);
		if ((timeval_diff(NULL,&endt,&startt)>29999999))
		{
				kh3_battery_status(Buffer,dsPic);
	 			printf("\nBattery: Voltage = %4.3f [V]  Current=%5.0f [mA]  Remaining capacity=%3d %%\n", 
		        (Buffer[9] | Buffer[11]<<8)*0.00976,(short)(Buffer[4] | Buffer[5]<<8)*0.07813,Buffer[3]);
				gettimeofday(&startt,0xffffffff);
		}	

		// get ir sensor
		kh3_proximity_ir(Buffer, dsPic);
				 
		left_ir=(Buffer[8*2] | Buffer[9*2+1]<<8);
		right_ir=(Buffer[9*2] | Buffer[10*2+1]<<8);

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
		
		kh3_set_speed(lspeed ,rspeed ,dsPic );
			
    usleep(19999); 
  }
  
  kh3_set_speed(0,0,dsPic ); // stop robot
  kh3_SetMode( kh4RegIdle,dsPic ); // set motors to idle
  
}

 // For a black line wider than the 1 front sensors (black line at least 2.7cm wide)
