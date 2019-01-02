/*----------------------------------------------------------------------------
 * Name:    Blinky.c
 * Purpose: LED Flasher
 * Note(s): __USE_LCD   - enable Output on LCD, uncomment #define in code to use
 *  				for demo (NOT for analysis purposes)
 *----------------------------------------------------------------------------
 * Copyright (c) 2008-2011 Keil - An ARM Company.
 * Name: Anita Tino
 *----------------------------------------------------------------------------*/

#include <stdio.h>
#include "LPC17xx.H"                       
#include "GLCD.h"
#include "LED.h"
#include "ADC.h"
#include "KBD.h"
#include "type.h"
#include "usb.h"
#include "usbcfg.h"
#include "usbhw.h"
#include "usbcore.h"
#include "usbaudio.h"

#define __FI        1                      /* Font index 16x24               */
#define __USE_LCD   0										/* Uncomment to use the LCD */

//ITM Stimulus Port definitions for printf //////////////////
#define ITM_Port8(n)    (*((volatile unsigned char *)(0xE0000000+4*n)))
#define ITM_Port16(n)   (*((volatile unsigned short*)(0xE0000000+4*n)))
#define ITM_Port32(n)   (*((volatile unsigned long *)(0xE0000000+4*n)))

#define DEMCR           (*((volatile unsigned long *)(0xE000EDFC)))
#define TRCENA          0x01000000

struct __FILE { int handle;  };
FILE __stdout;
FILE __stdin;

int fputc(int ch, FILE *f) {
  if (DEMCR & TRCENA) {
    while (ITM_Port32(0) == 0);
    ITM_Port8(0) = ch;
  }
  return(ch);
}
/////////////////////////////////////////////////////////
extern  void SystemClockUpdate(void);

extern uint32_t SystemFrequency;  
uint8_t  Mute;                                 /* Mute State */
uint32_t Volume;                               /* Volume Level */

#if USB_DMA
uint32_t *InfoBuf = (uint32_t *)(DMA_BUF_ADR);
short *DataBuf = (short *)(DMA_BUF_ADR + 4*P_C);
#else
uint32_t InfoBuf[P_C];
short DataBuf[B_S];                         /* Data Buffer */
#endif

uint16_t  DataOut;                              /* Data Out Index */
uint16_t  DataIn;                               /* Data In Index */

uint8_t   DataRun;                              /* Data Stream Run State */
uint16_t  PotVal;                               /* Potenciometer Value */
uint32_t  VUM;                                  /* VU Meter */
uint32_t  Tick;                                 /* Time Tick */


/*
 * Get Potenciometer Value
 */

void get_potval (void) {
  uint32_t val;

  LPC_ADC->CR |= 0x01000000;              /* Start A/D Conversion */
  do {
    val = LPC_ADC->GDR;                   /* Read A/D Data Register */
  } while ((val & 0x80000000) == 0);      /* Wait for end of A/D Conversion */
  LPC_ADC->CR &= ~0x01000000;             /* Stop A/D Conversion */
  PotVal = ((val >> 8) & 0xF8) +          /* Extract Potenciometer Value */
           ((val >> 7) & 0x08);
}


/*
 * Timer Counter 0 Interrupt Service Routine
 *   executed each 31.25us (32kHz frequency)
 */

void TIMER0_IRQHandler(void) 
{
  long  val;
  uint32_t cnt;

  if (DataRun) {                            /* Data Stream is running */
    val = DataBuf[DataOut];                 /* Get Audio Sample */
    cnt = (DataIn - DataOut) & (B_S - 1);   /* Buffer Data Count */
    if (cnt == (B_S - P_C*P_S)) {           /* Too much Data in Buffer */
      DataOut++;                            /* Skip one Sample */
    }
    if (cnt > (P_C*P_S)) {                  /* Still enough Data in Buffer */
      DataOut++;                            /* Update Data Out Index */
    }
    DataOut &= B_S - 1;                     /* Adjust Buffer Out Index */
    if (val < 0) VUM -= val;                /* Accumulate Neg Value */
    else         VUM += val;                /* Accumulate Pos Value */
    val  *= Volume;                         /* Apply Volume Level */
    val >>= 16;                             /* Adjust Value */
    val  += 0x8000;                         /* Add Bias */
    val  &= 0xFFFF;                         /* Mask Value */
  } else {
    val = 0x8000;                           /* DAC Middle Point */
  }

  if (Mute) {
    val = 0x8000;                           /* DAC Middle Point */
  }

  LPC_DAC->CR = val & 0xFFC0;             /* Set Speaker Output */

  if ((Tick++ & 0x03FF) == 0) {             /* On every 1024th Tick */
    get_potval();                           /* Get Potenciometer Value */
    if (VolCur == 0x8000) {                 /* Check for Minimum Level */
       Volume = 0;                           /* No Sound */
    } else {
      Volume = VolCur * PotVal;             /* Chained Volume Level */
    }
    val = VUM >> 20;                       /* Scale Accumulated Value */
    VUM = 0;                                /* Clear VUM */
    if (val > 7) val = 7;                   /* Limit Value */
  }
	if(get_button() == KBD_RIGHT){
		NVIC_DisableIRQ(TIMER0_IRQn);			
		NVIC_DisableIRQ(USB_IRQn);
		USB_Connect(0);
		USB_Reset();
		USB_Connect(1);
	}
  LPC_TIM0->IR = 1;                         /* Clear Interrupt Flag */
}
/////////////////////////////////////////////////////////
char text[10];

//Use to trace the pot values in Debug
//uint16_t ADC_Dbg;

/* Import external variables from IRQ.c file                                  */
extern uint8_t  clock_ms;
extern unsigned char tower[];
extern unsigned char wham[];
extern unsigned char Turtle[];
extern unsigned char thomas[];
extern unsigned char frogger[];
extern unsigned char lefttruck[];
extern unsigned char righttruck[];
extern unsigned char pacman[];

void delay(int a){
	int i;
	for(i = 0; i < a*1000000; i++){
	}
}

void gallery(){
	
	int piclocation = 0;
	uint32_t value =0x01;
	GLCD_Clear(White);
	while (1) {
	value = get_button();
		if(value == KBD_UP){
			if(piclocation == 0){
				piclocation = 3;
			}else{
				piclocation--;
			}
		}else if(value == KBD_DOWN){
			if(piclocation == 3){
				piclocation = 0 ;
			}else{
				piclocation++;
			}
		}
		if (piclocation == 0){
			
			GLCD_Bitmap (  20,   35, 284,  177, tower);
		}else if(piclocation == 1){
			GLCD_Bitmap (  20,   35, 200,  200, wham);
		}else if (piclocation == 2 ){
			GLCD_Bitmap (20,35,300,200,Turtle);
		}else if (piclocation == 3 ){
			GLCD_Bitmap (20,35,300,200,thomas);
		}
		
		if(value == KBD_RIGHT){
			break;
		}
	}
	GLCD_Clear(White);                         /* Clear graphical LCD display   */
  GLCD_SetBackColor(White);
  GLCD_SetTextColor(Blue);
  GLCD_DisplayString(0, 0, __FI, "    Media Center     ");
  GLCD_DisplayString(1, 0, __FI, "  Adriano Mazzucco   ");
	GLCD_DisplayString(3, 0, __FI, "Gallery   ");
  GLCD_DisplayString(4, 0, __FI, "Audio Player   ");
  GLCD_DisplayString(5, 0, __FI, "Game   ");
	GLCD_DisplayString(6, 0, __FI, "Game   ");
	GLCD_DisplayString(7, 0, __FI, "up/down move");
	GLCD_DisplayString(8, 0, __FI, "right to select");
}

void audio(){
	volatile uint32_t pclkdiv, pclk;
	uint32_t value =0x01;
	GLCD_Clear(White);                         
  GLCD_SetBackColor(White);
  GLCD_SetTextColor(Blue);
  GLCD_DisplayString(0, 0, __FI, "    Audio Player     ");
  GLCD_DisplayString(3, 0, __FI, "Use potentiometer to ");
	GLCD_DisplayString(4, 0, __FI, "Adjust Volume");
	GLCD_DisplayString(6, 0, __FI, "Right to Return");

  /* SystemClockUpdate() updates the SystemFrequency variable */
  SystemClockUpdate();

  LPC_PINCON->PINSEL1 &=~((0x03<<18)|(0x03<<20));  
  /* P0.25, A0.0, function 01, P0.26 AOUT, function 10 */
  LPC_PINCON->PINSEL1 |= ((0x01<<18)|(0x02<<20));

  /* Enable CLOCK into ADC controller */
  LPC_SC->PCONP |= (1 << 12);

  LPC_ADC->CR = 0x00200E04;		/* ADC: 10-bit AIN2 @ 4MHz */
  LPC_DAC->CR = 0x00008000;		/* DAC Output set to Middle Point */

  /* By default, the PCLKSELx value is zero, thus, the PCLK for
  all the peripherals is 1/4 of the SystemFrequency. */
  /* Bit 2~3 is for TIMER0 */
  pclkdiv = (LPC_SC->PCLKSEL0 >> 2) & 0x03;
  switch ( pclkdiv )
  {
	case 0x00:
	default:
	  pclk = SystemFrequency/4;
	break;
	case 0x01:
	  pclk = SystemFrequency;
	break; 
	case 0x02:
	  pclk = SystemFrequency/2;
	break; 
	case 0x03:
	  pclk = SystemFrequency/8;
	break;
  }

  LPC_TIM0->MR0 = pclk/DATA_FREQ - 1;	/* TC0 Match Value 0 */
  LPC_TIM0->MCR = 3;					/* TCO Interrupt and Reset on MR0 */
  LPC_TIM0->TCR = 1;					/* TC0 Enable */
  NVIC_EnableIRQ(TIMER0_IRQn);

  USB_Init();				/* USB Initialization */
  USB_Connect(TRUE);		/* USB Connect */
  /********* The main Function is an endless loop ***********/ 

  while(value != KBD_RIGHT ){
		value = get_button();		 
	}
	
	GLCD_Clear(White);                        
	GLCD_SetBackColor(White);
	GLCD_SetTextColor(Blue);
	GLCD_DisplayString(0, 0, __FI, "    Media Center     ");
	GLCD_DisplayString(1, 0, __FI, "  Adriano Mazzucco   ");
	GLCD_DisplayString(3, 0, __FI, "Gallery   ");
	GLCD_DisplayString(4, 0, __FI, "Audio Player   ");
	GLCD_DisplayString(5, 0, __FI, "Game   ");
	GLCD_DisplayString(6, 0, __FI, "Game   ");
	GLCD_DisplayString(7, 0, __FI, "up/down move");
	GLCD_DisplayString(8, 0, __FI, "right to select");
}

void frogger_game(){
	int frog_x = 200;
	int frog_y = 150;
	int left_truck_y = 250;
	int right_truck_y = -100;
	int counter1, counter2 = 0;
	uint32_t value =0x01;
	GLCD_Clear(White);
	GLCD_Bitmap (frog_y,frog_x,40,40,frogger);
	while (1) {
	GLCD_SetTextColor(Blue);
	GLCD_DisplayString(0, 0, __FI, "---------WIN---------");
	left_truck_y = left_truck_y - 10;
	right_truck_y = right_truck_y + 10;
	GLCD_Bitmap (left_truck_y,50,93,30,lefttruck);
	GLCD_Bitmap (left_truck_y,170,93,30,lefttruck);
	GLCD_Bitmap (right_truck_y,110,93,30,righttruck);
	GLCD_SetTextColor(White);
	for (counter1 = 0; counter1 < 20; counter1++){
		for (counter2 =0;counter2 < 30; counter2++){
			GLCD_PutPixel(left_truck_y + 93 + counter1,50 + counter2);
			GLCD_PutPixel(right_truck_y  - counter1,110 + counter2);
			GLCD_PutPixel(left_truck_y + 93 + counter1,170 + counter2);

		}
	}
	if (left_truck_y == -100){
		left_truck_y = 250;
	}
	if (right_truck_y == 250){
		right_truck_y = -100 ;
	}
	value = get_button();
		if(value == KBD_UP){
					GLCD_SetTextColor(White);
			for (counter1 = 0; counter1 < 40; counter1++){
				for (counter2 =0;counter2 < 40; counter2++){
					GLCD_PutPixel(frog_y + counter1,frog_x + counter2);
				}
			}
			frog_x = frog_x - 30;
			GLCD_Bitmap (frog_y,frog_x,40,40,frogger);
		}else if(value == KBD_DOWN){
					GLCD_SetTextColor(White);
			for (counter1 = 0; counter1 < 40; counter1++){
				for (counter2 =0;counter2 < 40; counter2++){
					GLCD_PutPixel(frog_y + counter1,frog_x + counter2);
				}
			}
			frog_x = frog_x + 30;
			GLCD_Bitmap (frog_y,frog_x,40,40,frogger);

		}else if(value == KBD_LEFT){
					GLCD_SetTextColor(White);
			for (counter1 = 0; counter1 < 40; counter1++){
				for (counter2 =0;counter2 < 40; counter2++){
					GLCD_PutPixel(frog_y + counter1,frog_x + counter2);
				}
			}
			frog_y = frog_y - 30;
			GLCD_Bitmap (frog_y,frog_x,40,40,frogger);

		}else if(value == KBD_RIGHT){
					GLCD_SetTextColor(White);
			for (counter1 = 0; counter1 < 40; counter1++){
				for (counter2 =0;counter2 < 40; counter2++){
					GLCD_PutPixel(frog_y + counter1,frog_x + counter2);
				}
			}
			frog_y = frog_y + 30;
					GLCD_Bitmap (frog_y,frog_x,40,40,frogger);

		}
		if((frog_x == 170 || frog_x == 50)&&(frog_y > left_truck_y && frog_y < left_truck_y+93)){
				GLCD_Clear(Black);
				GLCD_SetBackColor(Black);
				GLCD_SetTextColor(Red);
				GLCD_DisplayString(5, 0, __FI, "	YOU DIED");
				delay(20);
				GLCD_Clear(White);                        
				frog_x = 200;
				frog_y = 150;
				GLCD_Bitmap (frog_y,frog_x,40,40,frogger);

		}
				if((frog_x == 110)&&(frog_y > right_truck_y && frog_y < right_truck_y+93)){
				GLCD_Clear(Black);
				GLCD_SetBackColor(Black);
				GLCD_SetTextColor(Red);
				GLCD_DisplayString(5, 0, __FI, "	YOU DIED");
				delay(20);
				GLCD_Clear(White);                        
				frog_x = 200;
				frog_y = 150;
				GLCD_Bitmap (frog_y,frog_x,40,40,frogger);

		}

		if(frog_x == 20){
			break;
		}
	}
	delay(2);
	GLCD_Clear(Green);
	GLCD_SetBackColor(Green);
	GLCD_SetTextColor(Red);
	GLCD_DisplayString(4, 0, __FI, "Congradulations");
  GLCD_DisplayString(5, 0, __FI, "You beat Frogger!");
	delay(15);
	GLCD_Clear(White);                        
  GLCD_SetBackColor(White);
  GLCD_SetTextColor(Blue);
  GLCD_DisplayString(0, 0, __FI, "    Media Center     ");
  GLCD_DisplayString(1, 0, __FI, "  Adriano Mazzucco   ");
	GLCD_DisplayString(3, 0, __FI, "Gallery   ");
  GLCD_DisplayString(4, 0, __FI, "Audio Player   ");
  GLCD_DisplayString(5, 0, __FI, "Game   ");
	GLCD_DisplayString(6, 0, __FI, "Game   ");
	GLCD_DisplayString(7, 0, __FI, "up/down move");
	GLCD_DisplayString(8, 0, __FI, "right to select");
	
}

void pacman_game(){
	int counter1, counter2, counter3 = 0;
	int frog_x = 160;
	int frog_y = 90;
	int left_truck_y = 250;
	int right_truck_y = -100;
	uint32_t value =0x01;

	GLCD_Clear(White);                        
  GLCD_SetTextColor(Blue);
	for (counter1 = 0; counter1 < 190; counter1++){
		for (counter2 =0;counter2 < 200; counter2++){
		
			if(counter2 <= 10){
				GLCD_PutPixel(0 + counter1,220 + counter2);
				GLCD_PutPixel(0 + counter1,10 + counter2);
				
				if(counter1<= 30){
					GLCD_PutPixel(10 + counter1,220 - 90 + counter2);
					GLCD_PutPixel(10 + counter1,220 - 110 + counter2);
					GLCD_PutPixel(10 + counter1,220 - 130 + counter2);
					GLCD_PutPixel(10 + counter1,220 - 150 + counter2);
					GLCD_PutPixel(150 + counter1,220 - 90 + counter2);
					GLCD_PutPixel(150 + counter1,220 - 110 + counter2);
					GLCD_PutPixel(150 + counter1,220 - 130 + counter2);
					GLCD_PutPixel(150 + counter1,220 - 150 + counter2);
					GLCD_PutPixel(110 + counter1,220 - 150 + counter2);
					GLCD_PutPixel(50 + counter1,220 - 150 + counter2);
					GLCD_PutPixel(20 + counter1,220 - 30 + counter2);
					GLCD_PutPixel(50 + counter1,220 - 30 + counter2);
					GLCD_PutPixel(110 + counter1,220 - 30 + counter2);
					GLCD_PutPixel(140 + counter1,220 - 30 + counter2);
					
					GLCD_PutPixel(70 + counter1,220 - 50 + counter2);
					GLCD_PutPixel(90 + counter1,220 - 50 + counter2);
					GLCD_PutPixel(70 + counter1,220 - 90 + counter2);
					GLCD_PutPixel(90 + counter1,220 - 90 + counter2);
					GLCD_PutPixel(70 + counter1,220 - 110 + counter2);
					GLCD_PutPixel(90 + counter1,220 - 110 + counter2);
					
					GLCD_PutPixel(50 + counter1,220 - 70 + counter2);
					GLCD_PutPixel(110 + counter1,220 - 70 + counter2);
					
					GLCD_PutPixel(90 + counter2,220 - 50 + counter1);
					GLCD_PutPixel(90 + counter2,220 - 90 + counter1);
					GLCD_PutPixel(90 + counter2,220 - 200 + counter1);
					GLCD_PutPixel(90 + counter2,220 - 170 + counter1);
					
					GLCD_PutPixel(30 + counter2,220 - 70 + counter1);
					GLCD_PutPixel(150 + counter2,220 - 70 + counter1);
					GLCD_PutPixel(50 + counter2,220 - 50 + counter1);
					GLCD_PutPixel(130 + counter2,220 - 50 + counter1);
					
					GLCD_PutPixel(30 + counter2,220 - 110 + counter1);
					GLCD_PutPixel(50 + counter2,220 - 110 + counter1);
					GLCD_PutPixel(130 + counter2,220 - 110 + counter1);
					GLCD_PutPixel(150 + counter2,220 - 110 + counter1);
					
					GLCD_PutPixel(30 + counter2,220 - 110 -40 + counter1);
					GLCD_PutPixel(50 + counter2,220 - 110 -40 + counter1);
					GLCD_PutPixel(130 + counter2,220 - 110 -40 + counter1);
					GLCD_PutPixel(150 + counter2,220 - 110 -40 + counter1);
					
					GLCD_PutPixel(50 + counter2,220 - 180 + counter1);
					GLCD_PutPixel(130 + counter2,220 - 180 + counter1);
					
					GLCD_PutPixel(70 + counter2,220 - 130 + counter1);
					GLCD_PutPixel(110 + counter2,220 - 130 + counter1);

				}
				
				if(counter1<= 20){
					GLCD_PutPixel(0 + counter1,220 - 50 + counter2);
					GLCD_PutPixel(170 + counter1,220 - 50 + counter2);
					GLCD_PutPixel(20 + counter1,220 - 170 + counter2);
					GLCD_PutPixel(20 + counter1,220 - 180 + counter2);
					GLCD_PutPixel(150 + counter1,220 - 170 + counter2);
					GLCD_PutPixel(150 + counter1,220 - 180 + counter2);
					GLCD_PutPixel(70 + counter2,220 -  180+ counter1);
					GLCD_PutPixel(100 + counter1,220 -  180 + counter1);

					GLCD_PutPixel(70 + counter1,220 - 130 + counter2);
					GLCD_PutPixel(100 + counter1,220 - 130 + counter2);
					
					GLCD_PutPixel(20 + counter1,220 -  70 + counter1);
					GLCD_PutPixel(160 + counter1,220 -  70 + counter1);

				}

			}
			
			if(counter1 <= 10){
				if(!(counter2 > 110 && counter2 < 120)){
				GLCD_PutPixel(0 + counter1, 220 - counter2);
				GLCD_PutPixel(180 + counter1, 220 - counter2);
				}
			}
		}
	}
	
//	for (counter1 = 0; counter1 < 10; counter1++){
//		for (counter2 =0;counter2 < 200; counter2++){
//			GLCD_PutPixel(0 + counter1,220 - counter1);
//		}
//	}
	GLCD_Bitmap (frog_y,frog_x,10,10,pacman);

	while(1){
		value = get_button();
		if(value == KBD_UP){
					GLCD_SetTextColor(White);
			for (counter1 = 0; counter1 < 10; counter1++){
				for (counter2 =0;counter2 < 10; counter2++){
					GLCD_PutPixel(frog_y + counter1,frog_x + counter2);
				}
			}
			frog_x = frog_x - 10;
			GLCD_Bitmap (frog_y,frog_x,10,10,pacman);
		}else if(value == KBD_DOWN){
					GLCD_SetTextColor(White);
			for (counter1 = 0; counter1 < 10; counter1++){
				for (counter2 =0;counter2 < 10; counter2++){
					GLCD_PutPixel(frog_y + counter1,frog_x + counter2);
				}
			}
			frog_x = frog_x + 10;
			GLCD_Bitmap (frog_y,frog_x,10,10,pacman);

		}else if(value == KBD_LEFT){
			GLCD_SetTextColor(White);
			if (frog_y == 40 && (frog_x == 220-60 || frog_x == 220-50||frog_x==220-70||frog_x==90||frog_x==100||frog_x==110||frog_x==130||frog_x==140||frog_x==150||frog_x==180||frog_x==170)){
				
			}else{
			for (counter1 = 0; counter1 < 10; counter1++){
				for (counter2 =0;counter2 < 10; counter2++){
					GLCD_PutPixel(frog_y + counter1,frog_x + counter2);
				}
			}
			frog_y = frog_y - 10;
			GLCD_Bitmap (frog_y,frog_x,10,10,pacman);
			}
		}else if(value == KBD_RIGHT){
					GLCD_SetTextColor(White);
			for (counter1 = 0; counter1 < 10; counter1++){
				for (counter2 =0;counter2 < 10; counter2++){
					GLCD_PutPixel(frog_y + counter1,frog_x + counter2);
				}
			}
			frog_y = frog_y + 10;
					GLCD_Bitmap (frog_y,frog_x,10,10,pacman);
		}
	}
}
/*----------------------------------------------------------------------------
  Main Program
 *----------------------------------------------------------------------------*/
int main (void) {
	int location = 0;
	
  uint32_t value =0x01;
	KBD_Init();
  LED_Init();	/* LED Initialization	*/

#ifdef __USE_LCD
  GLCD_Init();                               /* Initialize graphical LCD (if enabled */

  GLCD_Clear(White);                         /* Clear graphical LCD display   */
  GLCD_SetBackColor(White);
  GLCD_SetTextColor(Blue);
  GLCD_DisplayString(0, 0, __FI, "    Media Center     ");
  GLCD_DisplayString(1, 0, __FI, "  Adriano Mazzucco   ");
	GLCD_DisplayString(3, 0, __FI, "Gallery   ");
  GLCD_DisplayString(4, 0, __FI, "Audio Player   ");
  GLCD_DisplayString(5, 0, __FI, "Game   ");
	GLCD_DisplayString(6, 0, __FI, "Game   ");
	GLCD_DisplayString(7, 0, __FI, "up/down move");
	GLCD_DisplayString(8, 0, __FI, "right to select");


#endif


  while (1) {
		/* Loop forever                  */
		
		value = get_button();
		if(value == KBD_UP){
			if(location == 0){
				location = 3;
			}else{
				location--;
			}
		}else if(value == KBD_DOWN){
			if(location == 3){
				location = 0 ;
			}else{
				location++;
			}
		}//else if(value == KBD_LEFT){
		//	LED_Out(0x04);
		//	sprintf(text, "Left ");
		//}else if(value == KBD_RIGHT){
		//	LED_Out(0x08);
		//	sprintf(text, "Right");
		//}
			
		delay(2);
			if(location == 0){
					GLCD_SetBackColor(Blue);
					GLCD_SetTextColor(White);
					GLCD_DisplayString(3, 0, __FI, "Gallery   ");
					GLCD_SetBackColor(White);
					GLCD_SetTextColor(Blue);	
					GLCD_DisplayString(5, 0, __FI, "Game   ");
					GLCD_SetBackColor(White);
					GLCD_SetTextColor(Blue);
					GLCD_DisplayString(4, 0, __FI, "Audio Player   ");
					GLCD_DisplayString(6, 0, __FI, "Game   ");
			}else if(location == 1){
					GLCD_SetBackColor(Blue);
					GLCD_SetTextColor(White);
					GLCD_DisplayString(4, 0, __FI, "Audio Player   ");
					GLCD_SetBackColor(White);
					GLCD_SetTextColor(Blue);
					GLCD_DisplayString(3, 0, __FI, "Gallery   ");
					GLCD_SetBackColor(White);
					GLCD_SetTextColor(Blue);	
					GLCD_DisplayString(5, 0, __FI, "Game   ");
					GLCD_DisplayString(6, 0, __FI, "Game   ");
			}else if(location == 2){
					GLCD_SetBackColor(Blue);
					GLCD_SetTextColor(White);	
					GLCD_DisplayString(5, 0, __FI, "Game   ");
					GLCD_SetBackColor(White);
					GLCD_SetTextColor(Blue);
					GLCD_DisplayString(3, 0, __FI, "Gallery   ");
					GLCD_SetBackColor(White);
					GLCD_SetTextColor(Blue);
					GLCD_DisplayString(4, 0, __FI, "Audio Player   ");
					GLCD_DisplayString(6, 0, __FI, "Game   ");
			}else if(location == 3){
					GLCD_SetBackColor(Blue);
					GLCD_SetTextColor(White);	
					GLCD_DisplayString(6, 0, __FI, "Game   ");
					GLCD_SetBackColor(White);
					GLCD_SetTextColor(Blue);
					GLCD_DisplayString(3, 0, __FI, "Gallery   ");
					GLCD_SetBackColor(White);
					GLCD_SetTextColor(Blue);
					GLCD_DisplayString(4, 0, __FI, "Audio Player   ");
					GLCD_DisplayString(5, 0, __FI, "Game   ");
			}
			if(value == KBD_RIGHT){
				if(location == 0){
					gallery();
				}else if(location == 1){
					audio();
				}else if(location == 2){
					frogger_game();
				}else if(location == 3){
					pacman_game();
				}
			}
  }
}