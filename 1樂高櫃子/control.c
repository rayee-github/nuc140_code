// pin32 GPB0/RX0
// pin33 GPB1/TX0
#include <stdio.h>
#include "Driver\DrvUART.h"
#include "Driver\DrvGPIO.h"
#include "Driver\DrvSYS.h"
#include "Driver\DrvADC.h"
#include "NUC1xx.h"
#include "NUC1xx-LB_002\LCD_Driver.h"
#include "Timer.h"

uint8_t command[8] = {0x00};
int8_t item_quantity[48] = {0};
void Init_GPIO();
void delay_s(int time);
void LED_control(int *command,int a);
void row_column(int row,int column);

/* UART Callback function   */
void UART_INT_HANDLE(void)
{
	uint8_t bInChar[1] = {0xFF};
	uint8_t count[1] = {0xFF};
	int command[10]={0};
	
	while(UART0->ISR.RDA_IF==1)
	{
		DrvUART_Read(UART_PORT0,bInChar,1);
		if(bInChar[0]==0xAA)
		{
			DrvUART_Read(UART_PORT0,count,1);
			for(int i=0;i<count[0];i++)
			{
				DrvUART_Read(UART_PORT0,bInChar,1);
				command[i]=bInChar[0];
			}
			LED_control(command,count[0]);
		}
	}
}

void LED_control(int *command,int c)
{
	int row,column;
	for(int i=0;i<c;i++)
	{
		row=command[i]/16;
		column=command[i]%16;
		row_column(row,column);
	}
}

void row_column(int row,int column)
{
	uint8_t bInChar2[10] = {0xFF};
	
	DrvGPIO_ClrBit(E_GPD,row);
	DrvGPIO_SetBit(E_GPE,column);
	delay_s(1);
	sprintf(bInChar2,"r=%d c=%d\r\n",row,column);
	DrvUART_Write(UART_PORT0,bInChar2,strlen(bInChar2));
	
	for(int i=0;i<10;i++)        //row initial
		DrvGPIO_SetBit(E_GPD,i);
	for(int i=0;i<6;i++)         //column initial
		DrvGPIO_ClrBit(E_GPE,i);
	
	//while(DrvGPIO_GetBit(E_GPE,10));
}

/*Init motor's GPIO   */
void Init_GPIO()
{
	for(int i=0;i<10;i++)
	{
		DrvGPIO_Open(E_GPD, i, E_IO_OUTPUT); 
		DrvGPIO_SetBit(E_GPD,i);
	}
	for(int i=0;i<6;i++)
	{
		DrvGPIO_Open(E_GPE, i, E_IO_OUTPUT); 
		DrvGPIO_ClrBit(E_GPE,i);
	}
	
	DrvGPIO_Open(E_GPE, 10, E_IO_INPUT); //button
}

/* delay 1 secend  */
void delay_s(int time)
{
	int i;
	for(i=0;i<time*3;i++)
	{
		DrvSYS_Delay(330000);
	}
}

int32_t main()
{
	STR_UART_T sParam;
	
	UNLOCKREG();
	  DrvSYS_Open(48000000);
	LOCKREG();
	
	Init_GPIO();
	DrvGPIO_InitFunction(E_FUNC_UART0);
	
	/* UART Setting */
	sParam.u32BaudRate 		= 115200;
	sParam.u8cDataBits 		= DRVUART_DATABITS_8;
	sParam.u8cStopBits 		= DRVUART_STOPBITS_1;
	sParam.u8cParity 		= DRVUART_PARITY_NONE;
	sParam.u8cRxTriggerLevel= DRVUART_FIFO_1BYTES;
	
	/* Set UART Configuration */
 	if(DrvUART_Open(UART_PORT0,&sParam) != E_SUCCESS);
	DrvUART_EnableInt(UART_PORT0, DRVUART_RDAINT, UART_INT_HANDLE);
	
	while(1)
	{
		DrvGPIO_SetBit(E_GPE,15);
		delay_s(2);
		DrvGPIO_ClrBit(E_GPE,15);
		delay_s(2);
	}
}