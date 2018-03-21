/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) 2009 Nuvoton Technology Corp. All rights reserved.                                         */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include "Driver\DrvGPIO.h"
#include "Driver\DrvI2C.h"
#include "Driver\DrvSYS.h"
#include "Driver\DrvUART.h"
#include "NAU7802.h"
#include "LCD_Driver.h"
/*---------------------------------------------------------------------------------------------------------*/
/* Global variables                                                                                        */
/*---------------------------------------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------------------------------------*/
/*  Main Function									                                           			   */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main (void)
{
	uint32_t ADCresult;
	char adc_value[15];

	Initial_pannel();  //call initial pannel function
	clr_all_pannal();
	
	print_lcd(0, "Welcome! Nuvoton");	  
	print_lcd(1, "NAU7802 24-bits ");
	print_lcd(2, "ADC value:      ");	  


	InitNAU7802();
	while(1)
	{
		ADCresult=GetADCResult_NAU7802();
		sprintf(adc_value,"%8d",ADCresult);
		print_lcd(3, adc_value);
	}
}




