/*! 
 * \file   klrf_test.c simple led range finder test executable             
 *
 * \brief 
 *         This module provides useful basic examples to interface with the
 *         LedRangeFinder library for khepera
 *
 * \author Arnaud Maye (K-Team SA), J. Tharin : 2011.11.07 modified for using Hokuyo URG-04LX-UG01 LRF
 *
 * \note     Copyright (C) 2011 K-TEAM SA
 * \bug      none discovered.                                         
 * \todo     
 */


#include <math.h>
#include <signal.h>
#include <limits.h>
#include <khepera/khepera.h>

static int quitReq = 0;  // for exiting the program
static int lrfHandle = -1; // serial port handle for lrf


static FILE *outfile; 	// output filename for measurement saving

static char filename[256];

// grid size for displaying x-y position
#define NB_COLUMNS_MAX 80.0
#define NB_LINES_MAX 24.0

// for resizing mm to grid
#define DEFAULT_X_FACTOR (2.0*2000.0)
#define DEFAULT_Y_FACTOR (2.0*2000.0)
#define MAX_X_FACTOR (2.0*5600.0)
#define MAX_Y_FACTOR (2.0*5600.0)
#define MIN_X_FACTOR (2.0*500.0)
#define MIN_Y_FACTOR (2.0*500.0)
        
#define LRF_DEVICE "/dev/ttyACM0" // device where the LRF is connected: here USB port

// for zooming
static float x_factor = DEFAULT_X_FACTOR;
static float y_factor = DEFAULT_Y_FACTOR;

#define ZOOM_FACTOR 1.25

/*--------------------------------------------------------------------*/
/*!
 * Make sure the program terminate properly on a ctrl-c
 *
 * \param sig signal
 */
static void ctrlc_handler( int sig ) 
{
  quitReq = 1;
  // close the lrf device
	kb_lrf_Close(lrfHandle);
	lrfHandle=-1;
	
	if (outfile)
	{
		fclose(outfile);
  	printf("\nMeasure file is %s\n",filename);
  }
	
	
  exit(1);
}



/*--------------------------------------------------------------------*/
/*!
 * Set laser beam on
 */
int laseron( int argc, char * argv[], void * data)
{
  if (lrfHandle<0)
  { 
		printf("Error with lrfmeasure: lrf not initialised; run lrfinit first!\n");	
		return -1;
   }	
  kb_lrf_Laser_On(lrfHandle);   
	printf("lrf laser beam is now on...\r\n");
 
}

/*--------------------------------------------------------------------*/
/*!
 * Set laser beam off
 */
int laseroff( int argc, char * argv[], void * data)
{
	if (lrfHandle<0)
	{ 
	 printf("Error with lrfmeasure: lrf not initialised; run lrfinit first!\n");	
	 return -1;
	}	

	kb_lrf_Laser_Off(lrfHandle);
	printf("lrf laser beam is now off...\r\n"); 

   
}


/*--------------------------------------------------------------------*/
/*!
 * Set power supply on
 */
int poweron( int argc, char * argv[], void * data)
{
	 kb_lrf_Power_On();  
 
}

/*--------------------------------------------------------------------*/
/*!
 * Set power supply off, close the laser communication
 */
int poweroff( int argc, char * argv[], void * data)
{

	if (lrfHandle>=0)
	{
		kb_lrf_Laser_Off(lrfHandle);
		kb_lrf_Close(lrfHandle);
		lrfHandle=-1;
	}
	
	kb_lrf_Power_Off();
	    
}


/*--------------------------------------------------------------------*/
/*!
 * Set header of output file for saving lrf data 
 */
void lrfheader( )
{
  printf("lrf_header : preparing the measure header..\n");
  if(outfile)
  {
     printf("lrf_header : putting the measurement  header as file header..\n");
     fprintf(outfile, "LED RANGE FINDER MEASUREMENT\ncount\tindex\tangle\tdist\tx\ty\n\n");
  }
  else
     printf("lrf_header : error file not been set, please use the setfile command..\n");
}    

/*--------------------------------------------------------------------*/
/*!
 * Open output file for saving lrf data
 */   
int setfile( int argc, char * argv[], void * data)
{
    
    if (outfile)
    {
    	fclose(outfile);
    }
    
    outfile = fopen(argv[1], "w");
    
    strcpy(filename,argv[1]);
    
    if(outfile)
    {
       printf("File %s has been opened with success!\r\n",filename);
       
    }
    else
       printf("unable to open the %s file\r\n", filename);       
       
}



/*--------------------------------------------------------------------*/
/*!
 * Initialise lrf communication
 */
int lrfinit( int argc, char * argv[], void * data)
{
   lrfHandle = kb_lrf_Init(LRF_DEVICE);       
       
}

/*--------------------------------------------------------------------*/
/*!
 * Get measures from the lrf
 */
int lrfmeasure( int argc, char * argv[], void * data)
{
   int i, j,k,ret,stop=0;
   int max,average=1;
   float x,y,angle;
   int line,column,ll=0,lc=0;
   char key;
   
   if (lrfHandle<0)
   { 
   	 printf("Error with lrfmeasure: lrf not initialised; run lrfinit first!\n");	
   	 return -1;
   }	
   
   
   max = atoi(argv[1]);
   
   if (argc == 3)
   {
   	average=atoi(argv[2]);
   }
   
   // set to "infinite"
   if (max ==0)
   	max=INT_MAX-1;
   	
   	
   
   lrfheader();
   
   printf("lrfmeasure : going to proceed with %d measure...\r\n", max); 
   
   
   kb_change_term_mode(1); // change terminal mode for kbhit and getchar to return immediately
   
   for (k=0; (k < max) && (stop==0); k++)
   {
      //printf("lrfmeasure : measure %d \n", k+1); 
         
      kb_clrscr();
      
      
      // print scale X scale
      kb_move_cursor(1,NB_LINES_MAX);
      
      if (x_factor<3000)
      	printf(" scale x -> : 20 cm = |%*c",(int)floor(NB_COLUMNS_MAX*200.0/x_factor),'|');	
			else
				printf(" scale x -> : 1 meter = |%*c",(int)floor(NB_COLUMNS_MAX*1000.0/x_factor),'|');
			
			// print scale Y scale
			if (y_factor<3000)
				kb_move_cursor(NB_COLUMNS_MAX,NB_LINES_MAX-(int)floor(NB_LINES_MAX*200.0/y_factor));
			else
				kb_move_cursor(NB_COLUMNS_MAX,NB_LINES_MAX-(int)floor(NB_LINES_MAX*1000.0/y_factor));	
			printf("-");
			kb_move_cursor(NB_COLUMNS_MAX-22,NB_LINES_MAX);
			
			if (y_factor<3000)
				printf("scale y ^ : 20 cm   = -");
			else	
				printf("scale y ^ : 1 meter = -");
			
      
      
      if (average >1)
        ret=kb_lrf_GetDistances_Averaged(lrfHandle,average);
      else  
        ret=kb_lrf_GetDistances(lrfHandle);
      
			if (ret>=0)
			{
			
				kb_erase_line(1);
				kb_erase_line(2);
				kb_move_cursor(1,1);
				// display information in 2 lines
				printf("measure index:%4d  R =robot * =obstacle  timestamp=%12ld[ms] zoom: %3.1fx\n  KEYS: (k)=zoom in (l)=zoom out (a)verage=%d (Any other key):stop",k+1,kb_lrf_Get_Timestamp(),x_factor/DEFAULT_X_FACTOR,average);

				// display robot		
				kb_move_cursor(NB_COLUMNS_MAX/2-1,2.0*NB_LINES_MAX/3.0+2+1);
				printf("(R)");

				for (i=0; i< LRF_DATA_NB; i++)
				{


					//printf ("data %6ld mm\n", kb_lrf_DistanceData[i]);

					angle= (i-(LRF_DATA_NB/2.0-1024/4.0))*360.0/1024.0; 

					// convert from polar to cartesian	
					x=kb_lrf_DistanceData[i]*cos(angle*M_PI/180.0);
					y=kb_lrf_DistanceData[i]*sin(angle*M_PI/180.0);
					

					// convert from x,y [mm] to screen coordinates					
					line = (int)ceil(-y/y_factor*NB_LINES_MAX+2.0*NB_LINES_MAX/3.0+2+1);
					column=(int)ceil(x/x_factor*NB_COLUMNS_MAX+NB_COLUMNS_MAX/2+1); 

					
					//printf("\033[1;31;40mR\033[0;37;40m"); // print R in red

					// display position (skip 4000 < distances < 20 mm)
					if ((line >=3 && line<NB_LINES_MAX ) && (column >=1 && column <NB_COLUMNS_MAX) && (kb_lrf_DistanceData[i]>=20) && (kb_lrf_DistanceData[i]<=4000)) 
					{
						kb_move_cursor(column,line);
						printf("*");
					}

					

				
					if(outfile)
					{
						fprintf(outfile,"%d\t%d\t%6.1f\t%ld\t%7.1f\t%7.1f\n",k,i,angle,kb_lrf_DistanceData[i],x,y);
					}
				} // for (i=0, j = 0; i< LRF_DATA_NB; i++)





      } // if (ret>=0)
			else
			{
				printf("Error: could not get measure %d!\n",k+1);
			}
      
      usleep(100000); // wait 100 ms
      
      
		  // process any key
			if (kb_kbhit())
			{
				key=getchar();

				switch(key)
				{
				 	case 'a': // average
				 		if (average<5)
				 			average++;
				 		else
				 			average=1;	
					break;
					
					case 'k': // zoom in
				 		if (x_factor>MIN_X_FACTOR)
				 		{
				 			x_factor/=ZOOM_FACTOR;
				 		  y_factor/=ZOOM_FACTOR;
				 		}				 			
				 		else
				 		{
				 		  x_factor=MIN_X_FACTOR;
				 		  y_factor=MIN_Y_FACTOR;
				 		}
					break;
					
					case 'l': // zoom out
				 		if (x_factor<MAX_X_FACTOR)
				 		{
				 			x_factor*=ZOOM_FACTOR;
				 		  y_factor*=ZOOM_FACTOR;
				 		}
				 		else
				 		{
				 		  x_factor=MAX_X_FACTOR;
				 		  y_factor=MAX_Y_FACTOR;
				 		};	
					break;
					 
				 	default:
						printf("\r\n Another key pushed, quitting...\r\n");
						stop=1;
					break;
				}
      } // if (kbhit())
   } // for (k=0; k < max; k++)
   kb_clrscr();

	 kb_change_term_mode(0);

   laseroff(0, NULL, NULL);
   
   
}      
   
/*--------------------------------------------------------------------*/
/*!
 * Get measures from the lrf, averaged
 */
int lrfmeasureaverage( int argc, char * argv[], void * data)
{
	lrfmeasure( argc, argv, data);
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
  { "bye"             , 0 , 0 , quit } ,
  { "setfile"         , 1 , 1 , setfile },
  { "lrfinit"        , 0 , 0 , lrfinit },
  { "laseron"           , 0 , 0 , laseron },
  { "laseroff"          , 0 , 0 , laseroff },
  { "poweron"           , 0 , 0 , poweron },
  { "poweroff"          , 0 , 0 , poweroff },
  { "lrfmeasure"      , 1 , 2 , lrfmeasure }, /* first argument is number of scans, second (optional) is averaging */
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
  int rc;


  /* reset the screen */
  kb_clrscr();
  
  /* Set the libkhepera debug level - Highly recommended for development. */
  kb_set_debug_level(2);

  if((rc = kb_init( argc , argv )) < 0 )
    return 1;

  printf("Led Range Finder Test Program  (C) K-Team S.A\r\n");

	// set the ctrl-c handler
	signal( SIGINT , ctrlc_handler );

  
   /* parse commands */
  while (!quitReq) {
    printf("\n> ");

    if ( fgets( buf , sizeof(buf) , stdin ) != NULL ) {
      buf[strlen(buf)-1] = '\0';
      kb_parse_command( buf , cmds , NULL);
    }
  }

  
  // close the lrf device
	kb_lrf_Close(lrfHandle);
	
	if (outfile)
	{
		fclose(outfile);
  	printf("\nMeasure file is %s\n",filename);
  }
	
	return 0;
}
