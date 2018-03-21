// pin32 GPB0/RX0 to another board's UART TX
// pin33 GPB1/TX0 to another board's UART RX
//PA7 pin78  ~ PA0 pin71

#include <stdio.h>																											 
#include "NUC1xx.h"
#include "DrvUART.h"
#include "LCD_Driver.h"
#include "DrvGPIO.h"
#include "DrvADC.h"
#define BAUDRATE 9600

void Delay(int count)
{
	while(count--)
	{
//		__NOP;
	 }
}

void ADC_initialize (void)
{
	DrvADC_Open(ADC_SINGLE_END,ADC_SINGLE_CYCLE_OP,0xE0,EXTERNAL_12MHZ,0x9);   //0xE0 adc7,6,5
	Delay (100); 
}

static int32_t ADC_average (int32_t *ADC_value) 
{
	int32_t value[9] = {0};
	uint8_t i,j,k;
	int32_t value_max;
	for (i=0; i<9; i++)
	{
		value[i] = *ADC_value++;
	}
	for (j=0; j<9; j++)
	{
		for (k=0; k<9-j; k++) 
		{
			if(k != 8) 
			{
				if (value[k] > value[k+1]) 
				{
					value_max = value[k];
					value[k] = value[k+1]; 
					value[k+1] = value_max;
				}
			}
		}
	}
	return (value[4]);
}

static int32_t get_ADC_value(uint8_t ADC_channel_number)
{
	uint8_t i;
	int32_t ADC_calibration[9]={0}; 
	for (i=0; i<9; i++)
	{
		DrvADC_StartConvert(); 
		while(!DrvADC_IsConversionDone()) 
		{
		}
		ADC_calibration[i] = DrvADC_GetConversionData(ADC_channel_number); 
		DrvADC_StopConvert(); 
	}
	return (ADC_average(ADC_calibration)); 
}

/*----------------------------------------------------------------------------
  MAIN function
  ----------------------------------------------------------------------------*/
int32_t main (void)
{
	//Enable 12Mhz and set HCLK->12Mhz
	char adc_value[15]="            ";	
	
	int32_t adc1,adc2,adc3;
	STR_UART_T sParam;

	UNLOCKREG();
	DrvSYS_Open(48000000);
	SYSCLK->PWRCON.XTL12M_EN = 1;
	SYSCLK->CLKSEL0.HCLK_S = 0;
	LOCKREG();

	DrvGPIO_InitFunction(E_FUNC_UART0);		

	/* UART Setting */
    sParam.u32BaudRate 		= 9600;
    sParam.u8cDataBits 		= DRVUART_DATABITS_8;
    sParam.u8cStopBits 		= DRVUART_STOPBITS_1;
    sParam.u8cParity 		= DRVUART_PARITY_NONE;
    sParam.u8cRxTriggerLevel= DRVUART_FIFO_1BYTES;

	/* Set UART Configuration */
 	if(DrvUART_Open(UART_PORT0,&sParam) != E_SUCCESS);  

	ADC_initialize();

	Initial_pannel();  //call initial pannel function
	clr_all_pannal();

	DrvGPIO_Open(E_GPC, 8, E_IO_OUTPUT); 						 	
	/* Synch field transmission & Request Identifier Field transmission*/
					 
	while(1)
	{
		adc1=(get_ADC_value(7)-2900)*8;                            //10K¼Ú©i    1000¤W¤U
		Delay(3000);
		adc2=(get_ADC_value(6)-2900)*8;
		Delay(3000);
		adc3=(get_ADC_value(5)-2900)*8;
		Delay(3000);
		if(adc1<1200)
		{
			DrvUART_Write(UART_PORT0,"A",1); 
		}	
		else if(adc2<1200)
		{
			DrvUART_Write(UART_PORT0,"B",1); 
		}
		else if(adc3<1200)
		{
			DrvUART_Write(UART_PORT0,"C",1); 
		}		
		else
		{
			DrvUART_Write(UART_PORT0,"X",1);
		}
		Delay(330000);
	}
}
