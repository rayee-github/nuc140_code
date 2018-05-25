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
int count=0;

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
		}
		else if(bInChar[0]==0x31)
		{
			TIMER0->TCSR.IE = 1;
		}
		else if(bInChar[0]==0x32)
		{
			TIMER0->TCSR.IE = 0;
		}
	}
}

void EINT0Callback(void)
{
	count++;
}

/*Init motor's GPIO   */
void Init_GPIO()
{
	DrvGPIO_Open(E_GPC, 0, E_IO_OUTPUT); 
	DrvGPIO_SetBit(E_GPC, 0);          //1
	DrvGPIO_Open(E_GPC, 1, E_IO_OUTPUT);
	DrvGPIO_SetBit(E_GPC, 1);          //1
	
	DrvGPIO_Open(E_GPC, 2, E_IO_OUTPUT); 
	DrvGPIO_ClrBit(E_GPC, 2);          
	DrvGPIO_Open(E_GPC, 3, E_IO_OUTPUT);
	DrvGPIO_ClrBit(E_GPC, 3);
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
	STR_UART_T sParam;
	uint8_t bInChar[20];
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
		count=0;
		DrvGPIO_ClrBit(E_GPC, 2); //00
		DrvGPIO_ClrBit(E_GPC, 3);
		DrvGPIO_EnableEINT0(E_IO_RISING, E_MODE_EDGE, EINT0Callback);//E_GPB.14
		DrvSYS_Delay(10000);
		sprintf(bInChar,"R:%d \r\n",count);
		DrvUART_Write(UART_PORT0,bInChar,strlen(bInChar));
		DrvGPIO_DisableEINT0();
		
		count=0;
		DrvGPIO_SetBit(E_GPC, 2); //11
		DrvGPIO_SetBit(E_GPC, 3); 
		DrvGPIO_EnableEINT0(E_IO_RISING, E_MODE_EDGE, EINT0Callback);//E_GPB.14
		DrvSYS_Delay(10000);
		sprintf(bInChar,"G:%d \r\n",count);
		DrvUART_Write(UART_PORT0,bInChar,strlen(bInChar));
		DrvGPIO_DisableEINT0();
		
		count=0;
		DrvGPIO_ClrBit(E_GPC, 2); //01
		DrvGPIO_SetBit(E_GPC, 3);
		DrvGPIO_EnableEINT0(E_IO_RISING, E_MODE_EDGE, EINT0Callback);//E_GPB.14
		DrvSYS_Delay(10000);
		sprintf(bInChar,"B:%d \r\n\r\n",count);
		DrvUART_Write(UART_PORT0,bInChar,strlen(bInChar));
		DrvGPIO_DisableEINT0();
		
		count=0;
		DrvGPIO_SetBit(E_GPC, 2); //10
		DrvGPIO_ClrBit(E_GPC, 3);
		DrvSYS_Delay(10000);
	}
}