/*! 
 * \file   kgazer_small_ex.c simple Stargazer example
 *
 * \brief 
 *         This module provides a small example to interface with the
 *         Stargazer for khepera
 *
 * \author J. Tharin : 2013.07.04
 *
 * \note     Copyright (C) 2013 K-TEAM SA
 * \bug      none discovered.                                         
 * \todo     
 */ 

#include <khepera/khepera.h> 

/***** MAIN *************************************/
int main(int argc, char *argv[])
{
	char version[128];
	int ret=0,c=0,i,idnum;
	double angle,x,y,z,xc,yc; // stargazer returned variables and corrected position
	char cmode;	 // current mode of the Stargazer

	/* reset the screen */
  kb_clrscr();
  
  printf("\nKhepera 4 Stargazer small example program\n");
  
  /* Set the libkhepera debug level - Highly recommended for development. */
  kb_set_debug_level(2);

	// initialise the libkhepera
  if((ret = kb_init( argc , argv )) < 0 )
    return -1;

	printf("\nInitialising Stargazer module; please wait!\n");

	// initialise the Stargazer module	
	if ((ret=kb_stargazer_Init())!=0)
  {
  	printf("\nError initialising the Stargazer  (error = %d)!\n",ret); 	
  	kb_stargazer_Close();
  	return -2;
  }
	
	// get Stargazer firmware version
	kb_gazer_get_version(version);
	printf("\nStargazer version is: %s\n",version);
	
	// read parameters values
	printf("\nRead current Stargazer parameters:\n");

	kb_gazer_get_landmark_number(&c);
	printf("  landmark number: %d\n",c);

	kb_gazer_get_ref_id(&c);
	printf("  reference id   : %d\n",c);

	kb_gazer_get_landmark_type(&c);
	printf("  landmark type  : %s (does it match yours?)\n",kb_gazer_landmark_types[c]);
	
	kb_gazer_get_landmark_mode(&c);
	printf("  landmark mode  : %s\n",kb_gazer_landmark_modes[c]);

	kb_gazer_get_height_fix_mode(&c);
	printf("  height fix mode: %s\n\n",kb_gazer_height_fix_modes[c]);
	
  kb_gazer_start_computation(); // start computation of position
    
  for (i=0;i<10; i++) // read 10 data
  {
  	// read sensor values
		ret=kb_stargazer_read_data(&x,&y,&z,&angle,&idnum,&cmode,0);
	
		switch(ret)
		{
			case 0:
					printf("data %d: [cm,deg] x= %+6.1f  y= %+6.1f  angle= %+6.1f height= %+6.1f idnum= %4d mode: %c\n",i,x,y,angle,z,idnum,cmode);
			break;
			case -3:
				fprintf(stderr,"\nERROR: read error: buffer too short, leaving!\n");	
			break;
			case -6:
				printf("The sensor does not see any landmark!\n");
			break;
			case -8:
				fprintf(stderr,"\nERROR: no data received!\n");	
			break;	
			default:
				fprintf(stderr,"\nERROR: read error number %d!\n",ret);
		}
		
		usleep(99000); // wait for next data, should be max 10 fps
	}

	kb_stargazer_Close();
	return 0;
}

