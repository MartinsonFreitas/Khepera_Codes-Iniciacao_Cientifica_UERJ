/*! 
 * \file   klrf_small_ex.c simple led range finder small example
 *
 * \brief 
 *         This module provides small example to interface with the
 *         LedRangeFinder library for khepera
 *
 * \author J. Tharin : 2011.11.07
 *
 * \note     Copyright (C) 2011 K-TEAM SA
 * \bug      none discovered.                                         
 * \todo     
 */



#include <math.h>
#include <khepera/khepera.h>

// device where the LRF is connected: here USB port        
#define LRF_DEVICE "/dev/ttyACM0" 


/*--------------------------------------------------------------------*/
/*! Main program to process the command line. 
 *
 */

int main( int argc , char * argv[] )
{
  int rc,i;
  int LRF_DeviceHandle; // serial port handle for lrf
  
  float angle,x,y;

  /* reset the screen */
  kb_clrscr();
 
	printf("Led Range Finder Small Example Program  (C) K-Team S.A\r\n");
  
  /* Set the libkhepera debug level - Highly recommended for development. */
  kb_set_debug_level(2);

	// initialise the libkhepera
  if((rc = kb_init( argc , argv )) < 0 )
  {
  	printf("\nERROR: port %s could not initialise libkhepera!\n");
    return -1;
	}

	kb_lrf_Power_On(); // activate the power supply battery module

	// initialise LRF device
	if ((LRF_DeviceHandle = kb_lrf_Init(LRF_DEVICE))<0)
	{
		printf("\nERROR: port %s could not initialise LRF!\n");
		return -2;
	} 

	// get distances
	if (kb_lrf_GetDistances(LRF_DeviceHandle)<0)
	{
	 	printf("\nERROR: port %s could not initialise LRF!\n");
  	kb_lrf_Close(LRF_DeviceHandle);
  	return -3;
	}
	


	printf("index dist[mm] angle[deg] x[mm] y[mm]\n");
				
	// process distances:
	// You have the distances radii from the center of the robot in [mm],
	// starting at -30 deg and rotating to counterclockwise direction
	// inside kb_lrf_DistanceData array. Values < 20 are errors.
	// You can get the distances average with function kb_lrf_GetDistances(lrfHandle,average).	
	for (i=0;i<LRF_DATA_NB;i++)
	{
		angle= (i-LRF_DATA_NB/2+1024/4)*360.0/1024.0; // angle of each data
		
		// convert from polar to cartesian
		x=kb_lrf_DistanceData[i]*cos(angle*M_PI/180.0); // direction: right side of robot
		y=kb_lrf_DistanceData[i]*sin(angle*M_PI/180.0); // direction: front of robot	
		
		printf("%3d\t%4ld\t%+6.1f\t%+7.1f\t%+7.1f\n",i,kb_lrf_DistanceData[i],angle,x,y);
		
	}

 
	// close the lrf device
	kb_lrf_Close(LRF_DeviceHandle);

  return 0;
  
}
