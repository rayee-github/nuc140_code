// pin32 GPB0/RX0 to another board's UART TX
// pin33 GPB1/TX0 to another board's UART RX

#include <stdio.h>
#include "Driver\DrvUART.h"
#include "Driver\DrvGPIO.h"
#include "Driver\DrvSYS.h"
#include "NUC1xx.h"
#include "NUC1xx-LB_002\LCD_Driver.h"

char TEXT3[16] = "                ";
int32_t main()
{
	int  i =0,i2=0;
	uint8_t aa[8];
	int j=1;
	uint8_t a[1];
STR_UART_T sParam;
	UNLOCKREG();
    DrvSYS_Open(48000000);
	LOCKREG();
	
	DrvGPIO_InitFunction(E_FUNC_UART0);
	DrvGPIO_InitFunction(E_FUNC_UART1);
	/* UART Setting */
	sParam.u32BaudRate 		= 115200;
	sParam.u8cDataBits 		= DRVUART_DATABITS_8;
	sParam.u8cStopBits 		= DRVUART_STOPBITS_1;
	sParam.u8cParity 		= DRVUART_PARITY_NONE;
	sParam.u8cRxTriggerLevel= DRVUART_FIFO_1BYTES;
		if(DrvUART_Open(UART_PORT0,&sParam) != E_SUCCESS);
	DrvGPIO_Open(E_GPD, 6, E_IO_OUTPUT); 
	DrvGPIO_ClrBit(E_GPD, 6);
	DrvGPIO_Open(E_GPD, 7, E_IO_INPUT); 
	
	while(1)
	{	
		Initial_pannel();
		clr_all_pannal();
		i=0;
		i2=0;
		j=8388608;
		
		for(int a=0;a<1;a++)
		{
			for(int z=0;z<25;z++)
			{
				DrvGPIO_SetBit(E_GPD, 6);
				DrvSYS_Delay(1);
				if(DrvGPIO_GetBit(E_GPD, 7))
				{
					i+=j;
					j/=2;
				}
				else
				{
					j/=2;
				}
				DrvGPIO_ClrBit(E_GPD, 6);
				DrvSYS_Delay(1);
			}
			DrvSYS_Delay(100);
			i2+=i;
		}
		if(i2>10777215)
		{
			sprintf(TEXT3,"%d",100000-((16777215-i2)/1));
		}
		else
		{
			sprintf(TEXT3,"%d",(i2/1)+100000);
		}
		print_lcd(2,TEXT3);
		DrvUART_Write(UART_PORT0,TEXT3,10);
		DrvSYS_Delay(300000);
	}
}