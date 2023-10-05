#include <khepera/khepera.h>

int maxsp,accinc,accdiv,minspacc, minspdec; // for speed profile
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
    go();

 return 0;  
}


int go()
{ 
  accinc=10;//3;
  accdiv=0;
  minspacc=20;
  minspdec=1;
  maxsp=400;
  // configure acceleration slope
  kh4_SetSpeedProfile(accinc,accdiv,minspacc, minspdec,maxsp,dsPic ); 
  // Acceleration increment ,  Acceleration divider, Minimum speed acc, 
  // Minimum speed dec, maximum speed
  
  kh4_SetMode(kh4RegSpeedProfile,dsPic );
	
	printf("\nPush ANY KEY to stop!\n");
	
  // configure acceleration slope
  kh4_SetSpeedProfile(0,0,0,0,0,dsPic ); // Stop robot
}