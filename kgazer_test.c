/*! 
 * \file   kgazer_test.c Stargazer example for Khepera 4 robot
 *
 * \brief 
 *         This module provides  example to interface with the
 *         Stargazer for khepera:
 *					- Setup the landmark
 *					- Display position of the robot.
 *					- Compute correction due to non-parallelism between sensor and ceiling
 *					- Control the robot with the arrows keys. 
 *
 * \author J. Tharin : 2013.07.04
 *
 * \note     Copyright (C) 2013 K-TEAM SA
 * \bug      none discovered.                                         
 * \todo     
 */
 

#include <khepera/khepera.h> 
#include <math.h>


// grid size for displaying x-y position
#define NB_COLUMNS_MAX 80
#define NB_LINES_MAX 50
#define KEYS_TEXT_EXT_LENGTH (158-NB_COLUMNS_MAX) 

// for resizing cm to grid
#define DEFAULT_X_FACTOR (2.0*100.0)
#define DEFAULT_Y_FACTOR (2.0*100.0)
#define MAX_X_FACTOR (2.0*400.0)
#define MAX_Y_FACTOR (2.0*400.0)
#define MIN_X_FACTOR (2.0*20.0)
#define MIN_Y_FACTOR (2.0*20.0)

// x/y scale threshold
#define MEDIUM_SCALE 80
#define BIGGEST_SCALE 240        

// for zooming
static double x_factor = DEFAULT_X_FACTOR;
static double y_factor = DEFAULT_Y_FACTOR;

// zoom change multiplying factor
#define ZOOM_FACTOR 1.25


// define thresholds and values for gotoxya
#define MOVE_ANGLE_DIR_THRESH 10.0
#define END_ANGLE_GOAL_THRESH 5.0
#define END_POS_GOAL_THRESH 4.5
#define FAST_SPEED_POS_THRESH 15.0
#define MOVE_SLOW_SPEED	 100	
#define MOVE_FAST_SPEED	 300
#define ROTATE_LOW_SPEED 40
#define ROTATE_HIGH_SPEED 100

#define ROTATE_ANGLE_TRESH 30.0

// integer round function
#define ROUND(dbl) dbl >= 0.0 ? (int)(dbl + 0.5) : ((dbl - (double)(int)dbl) <= -0.5 ? (int)dbl : (int)(dbl - 0.5))


// motor speed constants for key arrow controls
#define DEFAULT_SPEED 200
#define MAX_SPEED 1500
#define MIN_SPEED 15
#define SPEED_FACTOR 1.25
#define ROTATE_FACT_KEY 0.8

// correction data filename
#define DATA_FILE_NAME "data_corr.csv"


//#define DEBUG 1 // if defined display and use some debug stuff


// khepera4 device
static knet_dev_t * dsPic;

/*--------------------------------------------------------------------*/
/*! initMot initializes then configures the motor control
 * unit.
 *
 * \param hDev device handle
 * \return A value :
 *      - 0 if success
 *      - -1 if any error
 *
 */
int initMot(knet_dev_t *hDev)
{
  if(hDev)
  {
		/* initialize the motors controlers*/
		 
		/* tuned parameters */
		kh4_SetPositionMargin(20 ,hDev ); 				// position control margin
		kh4_ConfigurePID( 10 , 5 , 1,hDev  ); 		// P,I,D
		kh4_SetSpeedProfile(3,0,20,1,400,hDev ); // Acceleration increment ,  Acceleration divider, Minimum speed acc, Minimum speed dec, maximum speed
		
		kh4_SetMode( kh4RegIdle,hDev );  				// Put in idle mode (no control)

	  return 0;
  }
  else
  {
	  printf("initMot error, handle cannot be null\r\n");
	  return -1;
  }
}
													

/*--------------------------------------------------------------------*/
/*! initKH4 initialize various things in the kh4 then
 * sequentialy open the various required handle to the three i2c devices 
 * on the khepera3 using knet_open from the knet.c libkhepera's modules.
 * Finaly, this function initializes then configures the motor control
 * unit.
 *
 * \return A value :
 *      - 0 if success
 *      - <0 if any error
 */
int initKH4( void )
{
  /* This is required */
  kh4_init(0,NULL);
  
  /* open various socket and store the handle in their respective pointers */
 	dsPic  = knet_open( "Khepera4:dsPic" , KNET_BUS_I2C , 0 , NULL );

  if(dsPic!=0)
  {

    return initMot(dsPic);
  }

  return -1;
     
 }    

/*****************************************************************************/
/*!
 * set the Stargazer parameters
 *
 * \param nb_land number of lanmarks
 * \param ref_land reference landmark for map mode
 * \param land_type type of landmark
 * \param land_mode landmark mode (alone or map)
 *
 * \return 	0 : no error
 */
int set_Stargazer_parameters(int *nb_land,int  *ref_land,int  *land_type,int *land_mode )
{
	int c,i,ret;
	char text[256];
	
	
	// read parameters values
	printf("\nRead current Stargazer parameters:\n");

	kb_gazer_get_landmark_number(&c);
	printf("  landmark number: %d\n",c);
	*nb_land=c;

	kb_gazer_get_ref_id(&c);
	printf("  reference id   : %d\n",c);
	*ref_land=c;

	kb_gazer_get_landmark_type(&c);
	printf("  landmark type  : %s\n",kb_gazer_landmark_types[c]);
	*land_type=c;

	kb_gazer_get_landmark_mode(&c);
	printf("  landmark mode  : %s\n",kb_gazer_landmark_modes[c]);
	*land_mode=c;

	kb_gazer_get_height_fix_mode(&c);
	printf("  height fix mode: %s\n",kb_gazer_height_fix_modes[c]);
	
	
	// parameters already set ?
	printf("\nModify Stargazer parameters above (n/y)? ");
	c=getchar();
	getchar(); // read \n
	if ( c != 'y')
	{
		printf("\nSkipping Stargazer parameters modification step.\n");
	}
	else
	{ 
		printf("\nEnter the number of landmarks you use [1-4] (1= alone mode): ");
		c=getchar();
		getchar(); // read \n
		*nb_land=c-'0';	
		if (*nb_land<1 || *nb_land >4)
		{
			printf("ERROR: the numbers of landmark should be in [1..4]; 1 was used as default value!\n");
			*nb_land=1;
		}	
		kb_gazer_set_landmark_number(*nb_land);

		if (*nb_land > 1) // test if > 1 => map mode, ref id needed 
		{
			printf("\nEnter the initial landmark id number: ");
			fgets (text,256,stdin);
			i=strlen(text);
			if (i>0)
			{
				text[strlen(text)-1]='\0'; // remove \n
			}
			*ref_land=atoi(text);	
			if (*ref_land == 0)
			{
				printf("ERROR for the initial landmark id number; 546 was used as default value!\n");
				*ref_land=546;
			}	
			kb_gazer_set_ref_id(*ref_land);
		}
	
		// landmark type setting
		text[0]='\0';
		for (i=0;i<NB_MARK_TYPES;i++)
		{	
			sprintf(text+strlen(text),"%s: %d |",kb_gazer_landmark_types[i],i+1);
		}
	
		printf("\nEnter the landmark type number | %s : ",text);
		c=getchar();
		getchar(); // read \n
		*land_type=c-'0';	
		if (*land_type<1 || *nb_land >6)
		{
			printf("ERROR: the landmark type should be in [1..6]; HLD1S was used as default value!\n");
			*land_type=HLD1S;
		}	else
		{
			(*land_type)--; // asked 1..6 but array index is 0-5
		}
		kb_gazer_set_landmark_type(*land_type); // default HLD1S

		printf("\nWriting parameters values, please wait:\n");
		fflush(stdout);

		// setend to save the parameters needs to be called
		if((ret=kb_gazer_set_end_command())==0)
			printf("  first data written succesfully!\n");
		else 	
			printf("ERROR writting first data (error = %d)!\n",ret);
	
	
		printf("\nWriting other parameters values, please wait:\n");
		fflush(stdout);
		
		
		if (*nb_land>1)
		{
			kb_gazer_set_landmark_mode(MARK_MAP);
		}	
		else
		{	
			kb_gazer_set_landmark_mode(MARK_ALONE);
		}		
	
		kb_gazer_set_height_fix_mode(HEIGHT_FIX_NO); //default No


		// setend to save the parameters needs to be called again, else the first are not saved!
		if((ret=kb_gazer_set_end_command())==0)
			printf("  last data written succesfully!\n");
		else 	
			printf("ERROR  writting last data (error = %d)!\n",ret);


		// reread parameters values
		printf("\nReread parameters:\n");

		kb_gazer_get_landmark_number(&c);
		printf("  landmark number: %d\n",c);
		*nb_land=c;

		kb_gazer_get_ref_id(&c);
		printf("  reference id   : %d\n",c);
		*ref_land=c;

		kb_gazer_get_landmark_type(&c);
		printf("  landmark type  : %s\n",kb_gazer_landmark_types[c]);
		*land_type=c;

		kb_gazer_get_landmark_mode(&c);
		printf("  landmark mode  : %s\n",kb_gazer_landmark_modes[c]);
		*land_mode=c;

		kb_gazer_get_height_fix_mode(&c);
		printf("  height fix mode: %s\n",kb_gazer_height_fix_modes[c]);
	}		
	return 0;
}



/*****************************************************************************/
/*! difference between two angles in degree, modulo 180
 * \param a first angle
 * \param b second angle 
 *
 * \return result 
*/
double diff_angles(double a, double b) {
	double ret;
	
	ret = a-b;
	
	if (ret>180)
		ret-=360;
	else
		if (ret<-180)
			ret+=360;
			
	return ret;

}

/*****************************************************************************/
/*! sum between two angles in degree, modulo 180
 * \param a first angle
 * \param b second angle 
 *
 * \return result 
*/
double sum_angles(double a, double b) {
	double ret;
	
	ret = a+b;
	
	if (ret>180)
		ret-=360;
	else
		if (ret<-180)
			ret+=360;		
	return ret;

}

/*****************************************************************************/
/*!
 * make one increment of moving robot to x,y,angle; must be called perodically
 * very basic algorithm: rotate to be in direction to the goal xy, move straight to goal position and rotate to goal angle
 *
 * \param curr_x current x position [cm]
 * \param curr_y current a position [cm]
 * \param curr_a current angle position [degree]
 * \param goal_x goal x position [cm]
 * \param goal_y goal a position [cm]
 * \param goal_a goal angle position [degree]
 * \param dist2goal return computed distance to goal	
 * \param angle2goal return computed angle to goal (or to direction to goal when moving)	 
 * \return 	0 : to destination
 *					1 : first rotation
 *					2 : translation
 *					3 : final rotation
 */
int goto_xya(double curr_x,double curr_y, double curr_a, double goal_x,double goal_y,double goal_a, double *dist2goal, double *angle2goal)
{
	double dx,dy,da,d;
	int speed;

	d=sqrt((curr_y-goal_y)*(curr_y-goal_y)+(curr_x-goal_x)*(curr_x-goal_x));
	*dist2goal=d;

	// check if at goal x,y
	if ( d <END_POS_GOAL_THRESH)
	{
		// check if at goal angle
		
		da=diff_angles(goal_a,curr_a);
		*angle2goal=da;
		
		if (fabs(da) <END_ANGLE_GOAL_THRESH)
		{
			// stop: at x,y,a
			kh4_set_speed(0 ,0 ,dsPic); // stop robot
			return 0;
		}
		else
		{
			if (fabs(da)>ROTATE_ANGLE_TRESH)
			{
				speed=ROTATE_HIGH_SPEED;
			}
			else
			{
				speed=ROTATE_LOW_SPEED;
			}
		
			// rotate to angle
			if (da>0)
			{
				// left
				kh4_SetMode(kh4RegSpeed,dsPic );
				kh4_set_speed(-speed,speed ,dsPic);
			}	else
			{
				// right
				kh4_SetMode(kh4RegSpeed,dsPic );
				kh4_set_speed(speed,-speed ,dsPic);
			}
			return 3;
		}
	}	

	
	// compute angle
	if ((curr_x-goal_x) == 0)
	{
		// parallel to y axis
		if (curr_y>goal_y)
			da = 180;
		else
			da = 0;
	}
	else
	if ((curr_y-goal_y) == 0)
	{
		// parallel to x axis
		if (curr_x>goal_x)
			da = 90;
		else
			da = -90;
	}
	{
		// compute angle
		da=atan((goal_y-curr_y)/(goal_x-curr_x))*180.0/M_PI;
		
		// adapt it to origin
		if (curr_x>goal_x)
		{
			da= sum_angles(90,da);
		}
		else
		{
			da= sum_angles(270,da);
		}
		
	}
	
	// difference between goal angle and direction to goal
	da = diff_angles(da,curr_a);
	*angle2goal=da;
	// must rotate
	if (fabs(da) > MOVE_ANGLE_DIR_THRESH)
	{
		if (fabs(da)>ROTATE_ANGLE_TRESH)
		{
			speed=ROTATE_HIGH_SPEED;
		}
		else
		{
			speed=ROTATE_LOW_SPEED;
		}
	
		// rotate
		if (da>0)
		{
			// left
			kh4_SetMode(kh4RegSpeed,dsPic );
			kh4_set_speed(-speed,speed ,dsPic);
		}	else
		{
			// right
			kh4_SetMode(kh4RegSpeed,dsPic );
			kh4_set_speed(speed,-speed ,dsPic);
		}
		return 1;
	}
	else
	{
		// 	move straight
		
		if (d > FAST_SPEED_POS_THRESH)
		{
			kh4_SetMode(kh4RegSpeed,dsPic );
			kh4_set_speed(MOVE_FAST_SPEED,MOVE_FAST_SPEED ,dsPic);
		}
		else
		{
			kh4_SetMode(kh4RegSpeed,dsPic );
			kh4_set_speed(MOVE_SLOW_SPEED,MOVE_SLOW_SPEED ,dsPic);
		}
		return 2;		
	}
	
	return -1;	
	
}

/*****************************************************************************/
/*! compute time difference
 * \param *difference returned time difference in * struct timeva
 * \param *end_time finish time
 * \param *start_time start time
 *
 * \return time difference in [us] 
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

}

/***** MAIN *************************************/
int main(int argc, char *argv[])
{
	
	int ret=0,c=0,retxya;
	int kh3ver,kh3rev; // robot version and revision
	char version[128];
  char text[256];
  
	// goal variables
  int gstop=0,gcxo,gcyo;
  int gcx=NB_COLUMNS_MAX/2+1; // x goal at 0 in column coordinates system
  int gcy=NB_LINES_MAX/2+2;   // y goal at 0 in line coordinates system  
  double dist2goal,angle2goal; // distance/angle to goal 	
	double goal_x=0,goal_y=0,goal_a=0; // position control goal
  
	double angle,x,y,z,xc,yc; // stargazer returned variables and corrected position
	
	
	int nb_land, ref_land, land_type,i,land_mode; // landmark settings variable
	
	
	int idnum,line,column,ll=1,lc=1,lp=1; // x-y ASCII display variables

	// modes
  int stop=0, anyarrow=0, trace =0 , mapm=0,gxya=0; // current modes of the program
  char cmode;	 // current mode of the Stargazer
	int save_data=0;  // 1: save data in file, 0: dont' save
	int correction=0; // 1: use position correction due to non parallel, 0: don't use


	int speed=DEFAULT_SPEED; // speed for moving the robot

	double center_x,center_y,a_axis,b_axis,angle_rot,stddevx,stddevy; // fitted ellipse  parameters

	FILE *data_file=NULL; // data file pointer for saving data
	
	#ifdef DEBUG
	struct timeval earlier;
  struct timeval later;
  struct timeval interval;
  double ti;
  #endif

	/* reset the screen */
  kb_clrscr();
  
  
  printf("\nKhepera 4 Stargazer demo program\n");
  
  
  // init khepera4 robot (libkhepera included)
	if ((ret=initKH4())!=0)
	{
		printf("\nERROR: cannot initialize the Khepera 4 (error = %d)!\n",ret);
		return -2;
	}

	// print kh4 version
  // get revision
  if(kh4_revision(text, dsPic)==0){
    printf("\r\nVersion = %c, Revision = %u\r\n",(text[0]>>4) +'A',text[0] & 0x0F);        
  }
	
	// print kh3 battery remaining
	if(kh4_battery_status((char *)text,dsPic)>0){
		printf("  remaining capacity: %3d %%\n",text[3]);
	}
	
	printf("\nInitialising Stargazer module; please wait!\n");
	fflush( stdout );


	// init Stargazer module	
	if ((ret=kb_stargazer_Init())!=0)
	{
		printf("\nError initialising the Stargazer  (error = %d)!\n",ret);
	
		kb_stargazer_Close();
		return -3;
  }
	
	// get Stargazer firmware version
	kb_gazer_get_version(version);
	printf("\nStargazer version is: %s\n",version);
	
	// set Stargazer parameters
	set_Stargazer_parameters(&nb_land, &ref_land, &land_type,&land_mode);
	
  
 	printf("\nWould you like to do the position calibration (n/y)? ");
	fflush( stdout );
	c = getchar(); // wait return key
  
  tcflush(0,TCIFLUSH); // empty key buffer	

  kb_gazer_start_computation(); // start computation of position
  
  if (c == 'y')
  {
	
	 // calibration
		if ((ret=kb_gazer_calibration(dsPic,&center_x,&center_y,&angle_rot,&a_axis,&b_axis,&stddevx,&stddevy))<0)
		{
			
			if (ret==-9)
			{	
				printf("\nERROR during calibration: no landmark found!\n");
			}
			else
			if (ret==-4)
			{
						printf("ERROR during calibration: max standard deviation (%.1f cm) reached:\n  ellipse parameters: centre: (%4.1f,%4.1f)  angle: %4.1f  major axe: %4.1f  minor axe: %4.1f  stddevx = %4.2f  stddevy = %4.2f\n",CALIB_STDEV_MAX ,center_x,center_y,angle_rot,a_axis,b_axis,stddevx,stddevy);
			}	
			else
			{
				printf("\nERROR during calibration (error : %d)!\n",ret);
			}
			kb_stargazer_Close();
	
			return -4;
		} 
		else
		{
			correction=1;
		
			// calibration ok
			printf("\n  calibration OK: standard deviation x= %4.2f  y= %4.2f\n",stddevx,stddevy); 
		}
	} else
	{
		correction=0;
	}
  
 	printf("\nExtend your terminal console size to have at least %d columns and %d lines\nthen push any key to start displaying position (use arrows to control the robot)!\n",NB_COLUMNS_MAX+KEYS_TEXT_EXT_LENGTH,NB_LINES_MAX);

  kb_clrscr();
	
	kb_change_term_mode(1); // change terminal mode for kbhit and getchar to return immediately
	
	// wait for key pushed
	while(!kb_kbhit())
	{
		usleep(100);
	}
	
	// empty stdin
	while(kb_kbhit())
	{
		c = getchar(); // wait return key
	}
	
	#ifdef DEBUG
	gettimeofday(&earlier,NULL);
	#endif
	
	// **** MAIN LOOP: continue until stop is 1 *******************
	while(!stop)
	{
		
		anyarrow=0;
		
		// test keyboard and process keys
		if(kb_kbhit())
		{
					
			c = getchar();
				
		  // get special keys
			if (c== 27 && (getchar()==91))
			{
			 c = getchar();
			 switch(c)
			 {
				case 65: // UP arrow = forward
					anyarrow=1;
					kh4_SetMode(kh4RegSpeed,dsPic );
					kh4_set_speed(speed,speed, dsPic);
				break;

				case 66: // DOWN arrow = backward
					anyarrow=1;
					kh4_SetMode(kh4RegSpeed,dsPic );
					kh4_set_speed(-speed,-speed, dsPic);
				break;
				case 68: // LEFT arrow = left
					anyarrow=1;
					kh4_SetMode(kh4RegSpeed,dsPic );
					kh4_set_speed(-speed*ROTATE_FACT_KEY,speed*ROTATE_FACT_KEY, dsPic);
				break;

				case 67: // RIGHT arrow = right
					anyarrow=1;
					kh4_SetMode(kh4RegSpeed,dsPic );
					kh4_set_speed(speed*ROTATE_FACT_KEY,-speed*ROTATE_FACT_KEY, dsPic);
				break;

				case 53: // PAGE UP  = speed up
					speed*=SPEED_FACTOR;
			 		if (speed>MAX_SPEED)
			 		{
						speed=MAX_SPEED;
			 		};
  		 		c = getchar(); // get last character	
				break;

				case 54: // PAGE DOWN = speed down
					speed/=SPEED_FACTOR;
			 		if (speed<MIN_SPEED)
			 		{
						speed=MIN_SPEED;
			 		};
  		 		c = getchar(); // get last character
				break;
				

				default:
					printf("\r\n Another key pushed, quitting...\r\n");
					stop=1;
				break;
				} // switch(c)
				
				
			 } // if (c== '\027')	 
			else 
			{
				// get normal keys
				switch (c)
				{
					
					case '+': // zoom in
				 		
				 		x_factor/=ZOOM_FACTOR;
				 		y_factor/=ZOOM_FACTOR;	 		
				 		if (x_factor<MIN_X_FACTOR)
				 		{
				 		  x_factor=MIN_X_FACTOR;
				 		  y_factor=MIN_Y_FACTOR;
				 		}
					break;
					
					case '-': // zoom out
				 			x_factor*=ZOOM_FACTOR;
				 		  y_factor*=ZOOM_FACTOR;
				 		if (x_factor>MAX_X_FACTOR)
				 		{
				 		  x_factor=MAX_X_FACTOR;
				 		  y_factor=MAX_Y_FACTOR;
				 		};	
					break;
					
					case 'c':  // redo position calibration
						if (gxya == 0) // not in goto xya mode
						{
							kb_clrscr();
							printf("\nPush any key to start calibration: the robot will rotate very slowly one revolution!\n");
							fflush( stdout );
							tcflush(0,TCIFLUSH); // empty key buffer	
							while(!kb_kbhit())
							{
								usleep(100);
							}
							// calibration
							if ((ret=kb_gazer_calibration(dsPic,&center_x,&center_y,&angle_rot,&a_axis,&b_axis,&stddevx,&stddevy))<0)
							{
								if (ret==-4)
								{
									printf("ERROR during calibration: max standard deviation reached (%.1f cm):\n  ellipse parameters: centre: (%4.1f,%4.1f)  angle: %4.1f  major axe: %4.1f  minor axe: %4.1f  stddevx = %4.2f  stddevy = %4.2f\n",CALIB_STDEV_MAX,center_x,center_y,angle_rot,a_axis,b_axis,stddevx,stddevy);
								}	else
								{
									printf("\nERROR during calibration (error : %d)\n",ret);
								}					
							} else
							{
								// calibration ok
								printf("\n  calibration OK: standard deviation x= %4.2f  y= %4.2f\n",stddevx,stddevy);  

								correction=1; // activate position correction
								
							}
							printf("\nPush any key to continue.\n");	
							tcflush(0,TCIFLUSH); // empty key buffer	
							while(!kb_kbhit())
							{
								usleep(100);
							}
							kb_clrscr();
						}
					break;
					
					case 'g': // goto position angle
						if (gxya==0) // not in goto xya mode
						{	

							gxya=1;
							
							kb_erase_line(2);
							printf("Move with arrows and push RETURN to set goal (another key: cancel)");
							
							gstop=0;
							
				
							
							goal_x=(gcx-(NB_COLUMNS_MAX/2+1))*x_factor/NB_COLUMNS_MAX;
							goal_y=-(gcy-(NB_LINES_MAX/2+2))*y_factor/NB_LINES_MAX;
							kb_erase_line(3);
							printf("goal: x=%6.1f y=%6.1f angle=%6.1f",goal_x,goal_y,goal_a);
							
							
							//display current robot position
							if ((line >2 && line<NB_LINES_MAX ) && (column >=1 && column <=NB_COLUMNS_MAX)) 
							{
								kb_move_cursor(column,line);
	
								if ((angle > 45) && (angle <= 135))
									printf("<");
								else
									if ((angle > 135) && (angle <= 225))
										printf("V");
									else
									if ((angle > 225) && (angle <= 315))
										printf(">");
									else
										printf("A");						
							}
							
							gcxo=gcx;
							gcyo=gcy;
							kb_move_cursor(gcx,gcy);
							printf("G");
							fflush( stdout );
							anyarrow=0;
							while(!gstop)
							{
		
								anyarrow=0;
		
								// test keyboard and process keys
								if(kb_kbhit())
								{
					
									c = getchar();
				
									// get special keys
									if (c== 27 && (getchar()==91))
									{
										c = getchar();
										switch(c)
										{
										case 65: // UP arrow = forward
											gcy--;
											if (gcy <4)
												gcy = NB_LINES_MAX-1; 						
											anyarrow=1;	 
										break;

										case 66: // DOWN arrow = backward
											gcy++;
											if (gcy >NB_LINES_MAX-1)
												gcy = 4; 
											anyarrow=1;	 
										break;

										case 68: // LEFT arrow = left
											gcx--;
											if (gcx <1)
												gcx = NB_COLUMNS_MAX-1;
											anyarrow=1;	  
										break;

										case 67: // RIGHT arrow = right
											gcx++;
											if (gcx >NB_COLUMNS_MAX-1)
												gcx = 1;												
											anyarrow=1;	 
										break;
										}
									} else
									{
										// other keys
										if (c== '\n' || c== '\r')
										{
											gstop=1;
										}
										else
										{
											// any other key : cancel set goal
											gstop=1;
											gxya=0;
										
											kb_clrscr();			
											kb_erase_line(3);
											printf("Cancelled set goal!");
											fflush( stdout );
											
										}
									} 
									
									if (anyarrow)
									{
										// remove older goal
										kb_move_cursor(gcxo,gcyo);
										printf(" ");					
										gcxo=gcx;
										gcyo=gcy;
										
										// display origin			
										kb_move_cursor(NB_COLUMNS_MAX/2,NB_LINES_MAX/2+2); // 0,0
										printf("0,0");
										
										//display current robot position
										if ((line >2 && line<NB_LINES_MAX ) && (column >=1 && column <=NB_COLUMNS_MAX)) 
										{
											kb_move_cursor(column,line);
				
											if ((angle > 45) && (angle <= 135))
												printf("<");
											else
												if ((angle > 135) && (angle <= 225))
													printf("V");
												else
												if ((angle > 225) && (angle <= 315))
													printf(">");
												else
													printf("A");						
										}
										
										// display goal
										kb_move_cursor(gcx,gcy);
										printf("G");
																				
										goal_x=(gcx-(NB_COLUMNS_MAX/2+1))*x_factor/NB_COLUMNS_MAX;
										goal_y=-(gcy-(NB_LINES_MAX/2+2))*y_factor/NB_LINES_MAX;
										kb_erase_line(3);
										printf("goal: x=%6.1f y=%6.1f angle=%6.1f",goal_x,goal_y,goal_a);
										
										fflush( stdout );
									}
									
								}	// if special key
							} // while
							
							
							if (gxya)// not cancelled ?
							{
								goal_x=(gcx-(NB_COLUMNS_MAX/2+1))*x_factor/NB_COLUMNS_MAX;
								goal_y=-(gcy-(NB_LINES_MAX/2+2))*y_factor/NB_LINES_MAX;
								goal_a=0;
								kb_clrscr();
							
								kb_erase_line(3);
								printf("Moving to goal: x=%6.1f y=%6.1f angle=%6.1f",goal_x,goal_y,goal_a);
							}
							
							
						}	else
						{

							// stop robot
							kh4_set_speed(0,0,dsPic);
							kb_erase_line(3);
							printf("Stopped goto manually");
							gxya=0;
						}
												
					break;
					
					case 'k':  // use position correction
						correction=!correction;
					break;
					case 'm':	// map mode
						if ((mapm==0) && (gxya==0)) // not already in map mode and not in goto xya mode
						{
						
							if (land_mode == MARK_ALONE)
							{
								kb_erase_line(3);
								printf("ERROR: could not enter in map mode because configured in ALONE mode!");
								break;
							}
							
							ret=kb_gazer_start_map_mode();
							
							if (ret<0)
							{
								kb_erase_line(3);
								printf("ERROR: could not enter in map mode (error: %d )!",ret);
								if (ref_land != idnum) 
								 printf(" Verify that the robot is under reference landmark: %d ; current landmark is %d!",ref_land,idnum);
								break;
							} else
							{
								kb_erase_line(3);
								printf("MAP MODE: Move towards other nearest landmark and stop for about 2s near halfway between the two landmarks,\nthen move to the next landmark proceed the same way until all the landmarks are detected");
							}
							
							mapm=1;
							
						}
					break;
					
					case 'p': // set parameters
						if (gxya==0) // not in goto xya mode
						{
							kb_clrscr();
							kb_gazer_wait_stop_computation();
							kb_change_term_mode(0);
							set_Stargazer_parameters(&nb_land, &ref_land, &land_type,&land_mode);
							kb_change_term_mode(1);
							kb_gazer_start_computation(); // start computation of position
							printf("\nPush any key to continue.\n");
							fflush( stdout );
							tcflush(0,TCIFLUSH); // empty key buffer	
							while(!kb_kbhit())
							{
								usleep(100);
							}
							kb_clrscr();
						}
					break;
					
					
					case 'q': // quit
						printf("\r\n q key pushed, quitting...\r\n");	
						stop=1;
					break;
					case 's': // position save data into file
						if (save_data)
						{
							if (data_file)
							{
								fclose(data_file);
								data_file=NULL;
							}	
							save_data=0;
						} else
						{
							save_data=1;
							data_file=fopen(DATA_FILE_NAME,"w");
							if (data_file)
							{
								fprintf(data_file,"center x\t%.5f\tcenter y\t%.5f\tangle\t%.5f\thalf major axis\t%.5f\thalf minor axis\t%.5f\t\n",center_x,center_y,angle_rot,a_axis,b_axis);
								fprintf(data_file,"x\ty\txc\tyc\tangle\n");
							}	
						}
						
					break;
					case 't': // add trace of robot
						if (trace)
						{
							kb_clrscr();
							trace=0;													
						}
						else
						{
							trace=1;
						}
						
					break;
					
					
					 
				 	default:

					break;
				}
			}
			
			tcflush(0,TCIFLUSH); // empty key buffer	
		} // if(kb_kbhit())
		
		if (!anyarrow && !gxya)
		{
			/* Tell the motor controller to stop the Khepera I4  */
  		kh4_set_speed(0 ,0 ,dsPic); // stop robot
		}	
		
		// read sensor values
		ret=kb_stargazer_read_data(&x,&y,&z,&angle,&idnum,&cmode,0);
		
		// bufferoverun
		if (ret==-3)
		{
			fprintf(stderr,"\nERROR: read error: buffer too short, leaving!\n");
			kb_change_term_mode(0);
			kb_stargazer_Close();
			/* Tell to the motor controller to stop the Khepera 4  */
  		kh4_set_speed(0 ,0 ,dsPic); // stop robot
			return -5;
		}


	
		// data ok
		if (ret==0)
		{	
		
		  if (correction)
		  {
			// apply position correction:
			
				xc=a_axis*cos((angle+ANGLE_CORRECTION-angle_rot)*M_PI/180.0)
				*cos(angle_rot*M_PI/180.0)-b_axis*sin((angle+ANGLE_CORRECTION-angle_rot)*M_PI/180.0)*sin(angle_rot*M_PI/180.0);
	
				yc=a_axis*cos((angle+ANGLE_CORRECTION-angle_rot)*M_PI/180.0)
				*sin(angle_rot*M_PI/180.0)+b_axis*sin((angle+ANGLE_CORRECTION-angle_rot)*M_PI/180.0)*cos(angle_rot*M_PI/180.0);	
	
				if (save_data)
				{
					if (data_file)
						{
							fprintf(data_file,"%.5f\t%.5f\t%.5f\t%.5f\t%.5f\n",x,y,x-xc,y-yc,angle);
						}
				}
				x-=xc;
				y-=yc;			
			}

			line = (int)ROUND(-y/y_factor*NB_LINES_MAX+NB_LINES_MAX/2+2);
	
			column=(int)ROUND(x/x_factor*NB_COLUMNS_MAX+NB_COLUMNS_MAX/2+1); 
			
			
			// display origin			
			kb_move_cursor(NB_COLUMNS_MAX/2,NB_LINES_MAX/2+2); // 0,0
			printf("0,0");

			// display robot
			if ((line >2 && line<NB_LINES_MAX ) && (column >=1 && column <=NB_COLUMNS_MAX)) 
			{
				kb_move_cursor(column,line);
				
				if ((angle > 45) && (angle <= 135))
					printf("<");
				else
					if ((angle > 135) && (angle <= 225))
						printf("V");
					else
					if ((angle > 225) && (angle <= 315))
						printf(">");
					else
						printf("A");		
				
			}
			
			
			// print X scale
      kb_erase_line(NB_LINES_MAX);
            
      if (x_factor<MEDIUM_SCALE)
      	printf("scale: x 5cm =  >%*c",(int)floor(NB_COLUMNS_MAX*5.0/x_factor),'<');
      else		
      if (x_factor<BIGGEST_SCALE)
      	printf("scale: x 20cm = >%*c",(int)floor(NB_COLUMNS_MAX*20.0/x_factor),'<');	
			else
				printf("scale: x 1m =   >%*c",(int)floor(NB_COLUMNS_MAX*100.0/x_factor),'<');
			
			
			// remove y scale
			kb_move_cursor(NB_COLUMNS_MAX,lp);
			printf(" ");
			
			// print new Y scale
			if (y_factor<MEDIUM_SCALE)
			{
				lp=NB_LINES_MAX-(int)floor(NB_LINES_MAX*5.0/y_factor);
			}	
			else
			if (y_factor<BIGGEST_SCALE)
			{
				lp=NB_LINES_MAX-(int)floor(NB_LINES_MAX*20.0/y_factor);
			}	
			else
			{
				lp=NB_LINES_MAX-(int)floor(NB_LINES_MAX*100.0/y_factor);									
			}
			kb_move_cursor(NB_COLUMNS_MAX,lp);
			printf("-");
			
			kb_move_cursor(NB_COLUMNS_MAX-9,NB_LINES_MAX);
			
			if (y_factor<MEDIUM_SCALE)		
				printf("y 5cm  = -");
			else	
			if (y_factor<BIGGEST_SCALE)
				printf("y 20cm = -");
			else	
				printf(" y  1m = -");
			
			
			if (gxya) // in goto xya mode ?
			{
				retxya=goto_xya(x,y,angle,goal_x,goal_y,goal_a,&dist2goal,&angle2goal);
				if (retxya==0) // end of goto
				{
					gxya=0;
					kb_erase_line(3);
					printf("Goal reached: x=%6.1f y=%6.1f angle=%6.1f (error remaining: distance= %6.1f  angle= %6.1f) !",goal_x,goal_y,goal_a,dist2goal,angle2goal);
					
				}	else
				{	// display goal
					
					gcx=(int)ROUND(goal_x/x_factor*NB_COLUMNS_MAX+NB_COLUMNS_MAX/2+1); 
					gcy = (int)ROUND(-goal_y/y_factor*NB_LINES_MAX+NB_LINES_MAX/2+2);
					kb_move_cursor(gcx,gcy);
					printf("G");
					kb_erase_line(3);
					printf("Moving to goal: x=%6.1f y=%6.1f angle=%6.1f (remaining: distance= %6.1f  angle= %6.1f)",goal_x,goal_y,goal_a,dist2goal,angle2goal);
				}
			}	
			
			// display information
			kb_erase_line(1);
			sprintf(text,"[cm,deg] x= %+6.1f  y= %+6.1f  angle= %5.1f height= %5.1f idnum= %d | zoom: %3.1fx speed[mm/s]: %4.1f (%5d) mode: %c\n",x,y,angle,z,idnum,DEFAULT_X_FACTOR/x_factor,speed*KH4_SPEED_TO_MM_S,speed,cmode);
			printf("%s",text);
			
			// save current line and column
			ll=line;
			lc=column;

		}
		else {
			// data not ok!
			kb_erase_line(1);
			
			//dead zone
			if (ret==-6)
			{
			  printf("The sensor does not see any landmark!\n");
			}  
			else
			if ((ret==1) && (mapm==1))
			{		
				kb_erase_line(4);
				kb_erase_line(3);
				printf("End of map mode; update parameters detected!");
				mapm=0;
			}
			else
			if (ret>1)
			{ // MAP IP detected
				kb_erase_line(3);
				kb_erase_line(4);
				printf("MAPID number %d detected!",ret);
			}
			else
			if (ret==-8)
			{			  
			  printf("No data received: the sensor may not see any landmark!");
			  printf("    Robot speed for arrows control [mm/s]: %4.1f (%5d)",speed*KH4_SPEED_TO_MM_S,speed);
			}  
			else
			{
				printf("Another error happened (error: %d)!",ret);
			  printf("    Robot speed for arrows control [mm/s]: %4.1f (%5d)",speed*KH4_SPEED_TO_MM_S,speed);
			  
			}
		} // data ok 	
		
		kb_erase_line(2);
		printf("KEYS: (q)=quit (arrows)=move (+/-)=zoom (PG UP/DOWN)=speed (s)=save(%s) (k)=apply corr(%s) (t)=trace(%s) (m)=build map(%s) (p)=param (c)=calib g=goto(%s)",save_data?"ON ":"OFF",correction?"ON ":"OFF",trace?"ON ":"OFF",mapm?"ON ":"OFF",gxya?(retxya==1?"ON1":(retxya==2?"ON2":"ON3")):"OFF");
		
		fflush( stdout );
		usleep(99000);	// if more than 10 fps cannot get enough stargazer data
		//usleep(300000);
		
		// erase robot
		if ( (ll >2 && ll<NB_LINES_MAX ) && (lc >=1 && lc <=NB_COLUMNS_MAX)) 
		{
		  
			kb_move_cursor(lc,ll);
			if (trace)
				printf(".");
			else
				printf(" ");
		}
		
		
		#ifdef DEBUG
		gettimeofday(&later,NULL);
		ti=timeval_diff(NULL,&later,&earlier)/1000000.0;
		kb_erase_line(4);
		
		printf("frame time [s]: %4.3f  (fps= %4.1f)",ti,1.0/ti);
		earlier.tv_sec=later.tv_sec;
		earlier.tv_usec=later.tv_usec;
		
		#endif
		
	} // while !stop
	
	if (data_file)
		fclose(data_file);
	
	/* Tell to the motor controller to stop the Khepera 4  */
  kh4_set_speed(0 ,0 ,dsPic); // stop robot
  
	kb_stargazer_Close();
	
	kb_change_term_mode(0);
	
	kb_clrscr();

	return 0;

}

