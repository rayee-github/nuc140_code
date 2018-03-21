/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Sample Code : Smpl_7seg_Keypad                                                                          */
/*               input  : 3x3 Keypad   																       */
/*               output : 7-segment display			                 							  	       */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#include <stdio.h>																											 
#include "NUC1xx.h"
#include "Driver\DrvSYS.h"
#include "Driver\DrvGPIO.h"
#include "Seven_Segment.h"
#include "scankey.h"
#define BAUDRATE 9600

//=============================================================
// set GPIOx for 7-seg.
//=============================================================
void seven_segment_open(void)
{
	//Initial GPIOE [7:0] to output mode for 7-seg. 
	DrvGPIO_Open(E_GPE, 0, E_IO_OUTPUT);	
	DrvGPIO_Open(E_GPE, 1, E_IO_OUTPUT);	
	DrvGPIO_Open(E_GPE, 2, E_IO_OUTPUT);	
	DrvGPIO_Open(E_GPE, 3, E_IO_OUTPUT);	
	DrvGPIO_Open(E_GPE, 4, E_IO_OUTPUT);	
	DrvGPIO_Open(E_GPE, 5, E_IO_OUTPUT);	
	DrvGPIO_Open(E_GPE, 6, E_IO_OUTPUT);	
	DrvGPIO_Open(E_GPE, 7, E_IO_OUTPUT);	

	//Initial GPIOC [7:4] to output mode for nth 7-seg.
	DrvGPIO_Open(E_GPC, 4, E_IO_OUTPUT);	
	DrvGPIO_Open(E_GPC, 5, E_IO_OUTPUT);	
	DrvGPIO_Open(E_GPC, 6, E_IO_OUTPUT);	
	DrvGPIO_Open(E_GPC, 7, E_IO_OUTPUT);	
}


//=============================================================
// MAIN function
//=============================================================
int32_t main (void)
{
	#define SEG_N0   0x82 
	#define SEG_N1   0xEE 
	#define SEG_N2   0x07 
	#define SEG_N3   0x46 
	#define SEG_N4   0x6A  
	#define SEG_N5   0x52 
	#define SEG_N6   0x12 
	#define SEG_N7   0xE6 
	#define SEG_N8   0x02 
	#define SEG_N9   0x62 
	
	int8_t number;
//	unsigned char SEG_BUF[10]={SEG_N0, SEG_N1, SEG_N2, SEG_N3, SEG_N4, SEG_N5, SEG_N6, SEG_N7, SEG_N8, SEG_N9};

//=============================================================
// Initial 7-seg.
//=============================================================
	//Initial GPIOC [7:4] to output mode
	seven_segment_open();//set GPIOC_PMD[7:4] 

	close_seven_segment();//tuen off 7-seg. GPIOC[7:4] 
  //GPIOC->DOUT &= ~(0xF<<4);

//=============================================================
// Initial keypad
//=============================================================
	//Initial GPIOA [5:0]=11,QUASI mode
	OpenKeyPad(); //: (ScanKey.h)
	//GPIOA->PMD.PMD0 = (0x3);	
	//GPIOA->PMD.PMD1 = (0x3);	
	//GPIOA->PMD.PMD2 = (0x3);	
	//GPIOA->PMD.PMD3 = (0x3);	
	//GPIOA->PMD.PMD4 = (0x3);	
	//GPIOA->PMD.PMD5 = (0x3);	
	 

//=============================================================
// main loop
//=============================================================
	while(1)
	{
	  number = Scankey(); 
	
		show_seven_segment(0,number);
		//GPIOE->DOUT &= ~(0xFF);				 //clear 7 seg., GPIOE bit0-7=0000000
		//GPIOE->DOUT |= SEG_BUF[number];//show 7 seg.,GPIOE bit0-7=
		//GPIOC->DOUT |= (1<<(0+4)); 		 //display 1st 7 seg.,GPIOC bit 7=1
		
	}

}

