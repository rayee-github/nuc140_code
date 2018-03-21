// pin32 GPB0/RX0
// pin33 GPB1/TX0
// GPC,GPD,GPE
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
void action_GPIO();
void Init_GPIO();
void delay_s(int time);

/* UART Callback function   */
void UART_INT_HANDLE(void)
{
	extern uint16_t TimerCounter;
	uint8_t i,j,x=1;
	uint8_t bInChar[1] = {0xFF};
	
	while(UART0->ISR.RDA_IF==1)
	{
		DrvUART_Read(UART_PORT0,bInChar,1);
		if(bInChar[0]==0xAA)
		{
			DrvUART_Read(UART_PORT0,command,8);
			for(i=0;i<8;i++)
			{
				for(j=0;j<8;j++)
				{
					if((command[i]&0x01)==0x01)
					{
						item_quantity[i] += x;
					}
					x=x*2;
					command[i]=command[i]>>1;
				}
				x=1;
			}
			action_GPIO();
		}
		else if(bInChar[0]==0x31)
		{
			TimerCounter=0;
			TIMER0->TCSR.IE = 1;
		}
		else if(bInChar[0]==0x32)
		{
			TIMER0->TCSR.IE = 0;
		}
	}
}

/*Init motor's GPIO   */
void Init_GPIO()
{
	int i;
	DrvGPIO_Open(E_GPA, 6, E_IO_OUTPUT); //wifi_reset
	DrvGPIO_SetBit(E_GPA, 6);           //¥Ã»·high
	for(i=0;i<16;i++)//GPC set0
	{
		DrvGPIO_Open(E_GPC, i, E_IO_OUTPUT);
		DrvGPIO_SetBit(E_GPC, i);
	}
	/*for(i=0;i<16;i++)//GPD set0
	{
		DrvGPIO_Open(E_GPD, i, E_IO_OUTPUT); 
		DrvGPIO_ClrBit(E_GPD, i);
	}
	for(i=0;i<16;i++)//GPE set0
	{
		DrvGPIO_Open(E_GPE, i, E_IO_OUTPUT);
		DrvGPIO_ClrBit(E_GPE, i);
	}*/
}

/*action motor's GPIO   */
void action_GPIO()
{
	int i,j;
	for(i=0;i<48;i++)
	{
		if(i>=0 && i<8) //16
		{
			for(j=0;j<item_quantity[i];j++)
			{
				DrvGPIO_ClrBit(E_GPC, i);
				DrvSYS_Delay(200000);
				DrvGPIO_SetBit(E_GPC, i);
				DrvSYS_Delay(200000);
			}
			delay_s(1);
		}
		/*else if(i>=16 && i<32)
		{
			if(motor_action[i]==0x01)
			{
				DrvGPIO_SetBit(E_GPD, i-16);
			}
			else
			{
				DrvGPIO_ClrBit(E_GPD, i-16);
			}
			motor_action[i]=0x00;
		}
		else if(i>=32 && i<48)
		{
			if(motor_action[i]==0x01)
			{
				DrvGPIO_SetBit(E_GPE, i-32);
			}
			else
			{
				DrvGPIO_ClrBit(E_GPE, i-32);
			}
			motor_action[i]=0x00;
		}*/
	}
	for(i=0;i<48;i++)
	{
		item_quantity[i]=0;
	}
	DrvUART_Write(UART_PORT0,"AT+CIPSEND=4\r\n",14); 
	DrvSYS_Delay(5000);
	DrvUART_Write(UART_PORT0,"1_OK",4);
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
	uint8_t i =0;
	uint8_t char1[8]={0xAA,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
	STR_UART_T sParam;
	
	UNLOCKREG();
	  DrvSYS_Open(48000000);
	LOCKREG();
	
	DrvGPIO_ClrBit(E_GPA, 6); //wifiªì©l¤Æ
	DrvSYS_Delay(10000);
	Init_GPIO();
	delay_s(5);
	DrvGPIO_InitFunction(E_FUNC_UART0);
	
	Initial_pannel();  //call initial pannel function
	clr_all_pannal();
	
	/* UART Setting */
	sParam.u32BaudRate 		= 115200;
	sParam.u8cDataBits 		= DRVUART_DATABITS_8;
	sParam.u8cStopBits 		= DRVUART_STOPBITS_1;
	sParam.u8cParity 		= DRVUART_PARITY_NONE;
	sParam.u8cRxTriggerLevel= DRVUART_FIFO_1BYTES;
	
	/* Set UART Configuration */
 	if(DrvUART_Open(UART_PORT0,&sParam) != E_SUCCESS);
	DrvUART_Write(UART_PORT0,"AT+CIPSTART=\"TCP\",\"192.168.16.254\",8080\r\n",42); 
	delay_s(1);
	DrvUART_EnableInt(UART_PORT0, DRVUART_RDAINT, UART_INT_HANDLE);
	InitTIMER0();
	while(1);
}