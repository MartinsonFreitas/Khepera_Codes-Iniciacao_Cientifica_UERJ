#include <khepera/khepera.h>
#include <signal.h>
#include <sys/time.h>

#define  MOT_SetPointLL		0x2F

knet_dev_t * AltMotorOpen(int motor)
{
  switch(motor)
  {
    case 1 :
      return knet_open( "KoreMotorLE:AltMotor1", KNET_BUS_ANY, 0 , NULL );
      break;
    case 2 :
      return knet_open( "KoreMotorLE:AltMotor2", KNET_BUS_ANY, 0 , NULL );
      break;
    case 3 :
      return knet_open( "KoreMotorLE:AltMotor3", KNET_BUS_ANY, 0 , NULL );
      break;
    case 4 :
      return knet_open( "KoreMotorLE:AltMotor4", KNET_BUS_ANY, 0 , NULL );
      break;
    default:
      return NULL;
      break;
  }
}
knet_dev_t * MotorOpen(int motor)
{
  switch(motor)
  {
    case 1 :
      return knet_open( "KoreMotorLE:PriMotor1", KNET_BUS_ANY, 0 , NULL );
      break;
    case 2 :
      return knet_open( "KoreMotorLE:PriMotor2", KNET_BUS_ANY, 0 , NULL );
      break;
    case 3 :
      return knet_open( "KoreMotorLE:PriMotor3", KNET_BUS_ANY, 0 , NULL );
      break;
    case 4 :
      return knet_open( "KoreMotorLE:PriMotor4", KNET_BUS_ANY, 0 , NULL );
      break;
    default:
      return NULL;
      break;
  }
}

#define timercpy(a,b) \
 (a)->tv_sec = (b)->tv_sec; \
 (a)->tv_usec = (b)->tv_usec; 

int main(int argc, char *argv[]) {
    
  int ver,rc;
  int32_t position,speed,current;
  knet_dev_t * motor;
  unsigned char val,status,erreur;

  struct timeval clock, clocksav, clockdif;
  struct timezone timez;
  char   result;

  if(argc < 2)
  {
    printf("motor number needed\n\r");
    exit(0);
  }

  /* Set the libkhepera debug level - Highly recommended for development. */
  kb_set_debug_level(2);

  if((rc = kb_init( argc , argv )) < 0 )
    return 1;

  printf("K-Team Monitor Program\r\n");

  motor = MotorOpen(atoi(argv[1]));
  if(!motor)
  {
    printf("Cannot open motor %d\r\n",atoi(argv[1]));
    return 1;
  }

  /* read controller software version */
  kmot_GetFWVersion( motor, &ver );
  printf("Motor Firmware v%u.%u\n" , KMOT_VERSION(ver) , KMOT_REVISION(ver));


  /* Intialize motor controller */
  //kmot_ResetError(&motor);
  //kmot_SetPosition(&motor,0);
  kmot_SetOptions(motor,0,kMotSWOptWindup|kMotSWOptStopMotorBlk);     

  gettimeofday(&clocksav,&timez);

  while(1)
  {
    do{
      gettimeofday(&clock,&timez);
      timersub(&clock,&clocksav,&clockdif);
    }
    
    while(clockdif.tv_usec < 5000);

    timercpy(&clocksav,&clock);

    /* Read the motor speed  continuously */
    speed = kmot_GetMeasure(motor, kMotMesSpeed);
    printf("spd: %7ld ",speed);
    position = kmot_GetMeasure(motor, kMotMesPos);
    printf("pos: %7ld ",position);
    current = kmot_GetMeasure(motor, kMotMesCurrent);
    printf("cur: %7ld ",current);
    //printf("set: %7ld ",kmot_I2cRead32(motor,MOT_SetPointLL));
    kmot_GetStatus(motor,&status,&erreur);
    printf("err: 0x%x stat: 0x%x\r\n",erreur,status);
    //i2c_read8(&i2c,0x61,0x20,&val);
    //printf("sens 0x%x\r\n",val);
    //printf("mode: %x\r\n",kmot_I2cRead(&motor,0x28));
  }
}
