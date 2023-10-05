#include <khepera/khepera.h>


int main(int argc, char *argv[]) {
  int rc;

  /* Set the libkhepera debug level - Highly recommended for development. */
  kb_set_debug_level(2);

  printf("Khepera4 lrf battery power on/off program\r\n");
  
    /* Init the korebot library */
  if((rc = kb_init( 0 , NULL )) < 0 )
    return 1;


	if (argc != 2)
	{
		printf("USAGE: %s ON_OFF\n  ON_OFF: 1 for on, 0 for OFF\n",argv[0]);
		return -1;
	}

	if (atoi(argv[1]))
	{
		printf("LRF power on\n\n");
		kb_lrf_Power_On();
	}
	else {
		printf("LRF power off\n\n");
		kb_lrf_Power_Off();
		
	}

 return 0;  
}

