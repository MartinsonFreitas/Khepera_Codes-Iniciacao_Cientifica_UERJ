/*--------------------------------------------------------------------
 * kmot_test.c - KoreBot Library - KoreMotor Test
 *--------------------------------------------------------------------
 * $Id: kmot_pantilt.c,v 1.9 2005/10/25 12:45:20 pbureau Exp $
 *--------------------------------------------------------------------
 * $Author: pbureau $
 * $Date: 2005/10/25 12:45:20 $
 * $Revision: 1.9 $
 *--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*! 
 * \file   kmot_pantilt.c K-Team pan-tilt test program
 *
 * \brief
 *         Provide a basic test program to control a K-Team pantilt camera
 *         with a KoreMotor using the KoreBot library.
 *
 * \author   Pierre Bureau (K-Team SA)
 * \note     Copyright (C) 2004 K-TEAM SA
 */

#include <signal.h>
#include <khepera/khepera.h>

#define MARGIN     20
#define TGT_MARGIN 100

void InitMotor(knet_dev_t * mot)
{
    kmot_SetMode(mot,0);
    kmot_SetSampleTime(mot,1550);
    kmot_SetMargin(mot,20);
    kmot_SetOptions(mot,0,kMotSWOptWindup|kMotSWOptStopMotorBlk);     
    kmot_ResetError(mot);

    kmot_ConfigurePID(mot,kMotRegSpeed,1500,0,300);
    kmot_ConfigurePID(mot,kMotRegPos,100,30,3);
    kmot_SetSpeedProfile(mot,30,10);

    kmot_SetBlockedTime(mot,5);
    kmot_SetLimits(mot,kMotRegCurrent,0,50);
    kmot_SetLimits(mot,kMotRegPos,-10000,10000);
}

knet_dev_t * PantiltOpen(int motor)
{
  switch(motor)
  {
    case 1 :
      return knet_open( "KoreMotor:PriMotor1", KNET_BUS_ANY, 0 , NULL );
      break;
    case 2 :
      return knet_open( "KoreMotor:PriMotor2", KNET_BUS_ANY, 0 , NULL );
      break;
    case 3 :
      return knet_open( "KoreMotor:PriMotor3", KNET_BUS_ANY, 0 , NULL );
      break;
    case 4 :
      return knet_open( "KoreMotor:PriMotor4", KNET_BUS_ANY, 0 , NULL );
      break;
    default:
      return NULL;
      break;
  }
}

int main( int argc , char * argv[] )
{
  unsigned int ver;
  int rc;
  char * name;
  int32_t position,speed,current;
  knet_dev_t *motor0, *motor1;
  int32_t minpos0,maxpos0,minpos1,maxpos1;
  int32_t tgtmin0,tgtmax0,tgtmax1,tgtmin1;
  unsigned char status0,erreur0,status1,erreur1;
  int32_t pos0,pos1;
  unsigned counter = 10, addr0, addr1;
  
  /*! \todo check args should be rewriten using libpopt */
  if(argc < 3)
  {
    //printf("Usage: pantilt [-a] motor0 motor1 [nb cycle]\r\n");
    printf("Usage: pantilt motor0 motor1 [nb cycle]\r\n");
    //printf("\t-a to use alternate address (unsuported)\r\n");
    printf("\tmotor number is 1,2,3 or 4\r\n");
    printf("\tnb cycle is 10 by default\r\n");
    return 0;
  }

  if(argc > 3)
    counter = atoi(argv[3]);
  else
    counter = 10;

  /* Set the libkhepera debug level - Highly recommended for development. */
  kb_set_debug_level(2);

  if((rc = kb_init( argc , argv )) < 0 )
    return 1;

  printf("K-Team Pantilt Test Program\r\n");

  /* Open the motor devices */
  motor0 = PantiltOpen(atoi(argv[1]));
  if(!motor0)
  {
    printf("Cannot open motor %d\r\n",atoi(argv[1]));
    return 1;
  }

  motor1 = PantiltOpen(atoi(argv[2]));
  if(!motor1)
  {
    printf("Cannot open motor %d\r\n",atoi(argv[2]));
    return 1;
  }

  /* read controller software version */
  kmot_GetFWVersion( motor1, &ver );
  printf("Motor 1 Firmware v%u.%u\n" , KMOT_VERSION(ver) , KMOT_REVISION(ver));

  /* Intialize motor controller */
  InitMotor(motor0);
  InitMotor(motor1);
  kmot_SearchLimits(motor0, 5, 3, &minpos0, &maxpos0,100000);
  tgtmin0 = minpos0 + TGT_MARGIN;
  tgtmax0 = maxpos0 - TGT_MARGIN;
  printf("motor0: min:%ld max:%ld\n\r",minpos0, maxpos0);

  kmot_SearchLimits(motor1, 5, 3, &minpos1, &maxpos1,100000);
  tgtmin1 = minpos1 + TGT_MARGIN;
  tgtmax1 = maxpos1 - TGT_MARGIN;
  printf("motor1: min:%ld max:%ld\n\r",minpos1, maxpos1);

  printf("%d: set pos: %ld,%ld\n\r",counter,tgtmin0,tgtmin1);
#if 0
  kmot_SetPoint(motor0, kMotRegPosProfile, tgtmin0);
  kmot_SetPoint(motor1, kMotRegPosProfile, tgtmin1);
#else
  kmot_SetPoint(motor0, kMotRegPos, tgtmin0);
  kmot_SetPoint(motor1, kMotRegPos, tgtmin1);
#endif
  
  while(counter)
  {
    usleep(100000);
    printf("%d: set pos: %ld,%ld\n\r",counter,tgtmin0,tgtmin1);
#if 0
    kmot_SetPoint(motor0, kMotRegPosProfile, tgtmin0);
    kmot_SetPoint(motor1, kMotRegPosProfile, tgtmin1);
#else
    kmot_SetPoint(motor0, kMotRegPos, tgtmin0);
    kmot_SetPoint(motor1, kMotRegPos, tgtmin1);
#endif

#if 0
    kmot_GetStatus(motor0,&status0,&erreur0);
    kmot_GetStatus(motor1,&status1,&erreur1);
    while( ! ((status0 & 0x8) && (status1 & 0x8)))
    {
      kmot_GetStatus(motor0,&status0,&erreur0);
      kmot_GetStatus(motor1,&status1,&erreur1);
    }
#else
    pos0 = kmot_GetMeasure(motor0,kMotMesPos);
    pos1 = kmot_GetMeasure(motor1,kMotMesPos);
    while( abs(pos0 - tgtmin0) > MARGIN || abs(pos1 - tgtmin1) > MARGIN)
    {
      pos0 = kmot_GetMeasure(motor0,kMotMesPos);
      pos1 = kmot_GetMeasure(motor1,kMotMesPos);
    }
#endif
    kmot_SetMode(motor0,2);
    kmot_SetMode(motor1,2);

    usleep(100000);
    printf("%d: set pos: %ld,%ld\n\r",counter, tgtmax0,tgtmax1);
#if 0
    kmot_SetPoint(motor0, kMotRegPosProfile, tgtmax0);
    kmot_SetPoint(motor1, kMotRegPosProfile, tgtmax1);
#else
    kmot_SetPoint(motor0, kMotRegPos, tgtmax0);
    kmot_SetPoint(motor1, kMotRegPos, tgtmax1);
#endif

#if 0
    kmot_GetStatus(motor0,&status0,&erreur0);
    kmot_GetStatus(motor1,&status1,&erreur1);
    while( ! ((status0 & 0x8) && (status1 & 0x8)))
    {
      kmot_GetStatus(motor0,&status0,&erreur0);
      kmot_GetStatus(motor1,&status1,&erreur1);
    }
#else
    pos0 = kmot_GetMeasure(motor0,kMotMesPos);
    pos1 = kmot_GetMeasure(motor1,kMotMesPos);
    while( abs(pos0 - tgtmax0) > MARGIN || abs(pos1 - tgtmax1) > MARGIN)
    {
      pos0 = kmot_GetMeasure(motor0,kMotMesPos);
      pos1 = kmot_GetMeasure(motor1,kMotMesPos);
    }
#endif

    kmot_SetMode(motor0,2);
    kmot_SetMode(motor1,2);

    counter--;
  }

}

