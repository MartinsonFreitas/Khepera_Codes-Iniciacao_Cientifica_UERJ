#include <khepera/khepera.h>



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

 		// Inicialização do robô e configuração da porta serial
    		//kh4_init(0, NULL);
    		//kh4_SetMode(kh4RegIdle, dsPic);
		
		kh4_ConfigurePID( 10 , 5 , 1,dsPic  );// configure P,I,D

		accinc=3;//3;
		accdiv=0;
		minspacc=20;
		minspdec=1;
		maxsp=400;

		kh4_SetSpeedProfile(accinc,accdiv,minspacc, minspdec,maxsp,dsPic );
		struct timeval startt,endt;
		kh4_SetMode(kh4RegIdle, dsPic);

    		// Defina a velocidade desejada
    		int left_speed = 300;
    		int right_speed = 300;
    		kh4_set_speed(left_speed, right_speed, dsPic);

    		// Tempo robô se move (segundos)
    		int move_duration = 5; 

    		// tempo inicial
    		time_t start_time = time(NULL);

    		// Faz o robô se mover pelo tempo do move_duration
    		while (difftime(time(NULL), start_time) < move_duration)
    		{
        		//
    		}

    		// Pare o robô
    		kh4_set_speed(0, 0, dsPic);

    		// Libere recursos e finalize
    		
    		//kh4_close();

    		return 0;
}
		
		// Parte do cod que eu estava montando vvvv

		//speed=fwSpeed;
		
		//sleep(1);
		//kh4_set_speed(speed ,speed,dsPic ); // robot forward
		//if (buffer == 30)
		//{
		//kh4_set_speed(0 ,0 ,dsPic); // stop robot
		//}

		//return 0;

//}