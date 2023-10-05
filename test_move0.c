#include <khepera/khepera.h>
#include <signal.h>



static knet_dev_t * dsPic; // robot pic microcontroller access
int maxsp,accinc,accdiv,minspacc, minspdec; // for speed profile

#define fwSpeed 200 //150
#define MAX_DIST 500
#define MIN_DIST 80 // 70

int main()
{
	  printf("\nKhepera 4 - testkh4\n");

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

		/* ADD YOUR CODE HERE */

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
		 
		pmarg=20;
  		kh4_SetPositionMargin(pmarg,dsPic ); 
    		
		
		
		kh4_ConfigurePID( 10 , 5 , 1,dsPic  );// configure P,I,D

		accinc=3;//3;
		accdiv=0;
		minspacc=20;
		minspdec=1;
		maxsp=400;

		kh4_SetSpeedProfile(accinc,accdiv,minspacc, minspdec,maxsp,dsPic );
		// Acceleration increment ,  Acceleration divider, Minimum speed acc, 			Minimum speed dec, maximum speed
		
		//




		
		kh4_SetMode(kh4RegIdle, dsPic);

    		// Defina a velocidade desejada
    		int left_speed = 300;
    		int right_speed = 300;
    		kh4_set_speed(left_speed, right_speed, dsPic);


    		// get revision
		
  		if(kh4_revision(Buffer, dsPic)==0){
   			version=(Buffer[0]>>4) +'A';
  			revision=Buffer[0] & 0x0F; 
    		printf("\r\nVersion = %c, Revision = %u\r\n",version,revision);        
  		}
  		
		

		kh4_get_position(&lpos,&rpos,dsPic); // read  encoder position

		fpos = 10.0*10.0/KH4_PULSE_TO_MM;

		kh4_SetMode(kh4RegIdle, dsPic);
		kh4_set_position(lpos+(long)fpos,rpos+(long)fpos,dsPic);
		
		

    		// Pare o robô
    		kh4_set_speed(0, 0, dsPic);

    		return 0;
}
		
		
		
		//sleep(1);
		//kh4_set_speed(speed ,speed,dsPic ); // robot forward
		//if (buffer == 30)
		//{
		//kh4_set_speed(0 ,0 ,dsPic); // stop robot
		//}

		//return 0;

//}