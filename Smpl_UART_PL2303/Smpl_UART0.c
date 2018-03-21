// pin32 GPB0/RX0 to another board's UART TX
// pin33 GPB1/TX0 to another board's UART RX
// E_FUNC_I2C1,SDA1:GPA-10,SCL1:GPA-11
//PA10 pin10    PA11 pin9
//PA7 pin78  ~ PA0 pin71
//uart1   tx 20   rx 19

#include <stdio.h>
#include "Driver\DrvUART.h"
#include "Driver\DrvGPIO.h"
#include "Driver\DrvSYS.h"
#include "NUC1xx.h"
#include "NUC1xx-LB_002\LCD_Driver.h"

volatile uint8_t comRbuf[16] ={0x30};
volatile uint16_t comRbytes = 0;
volatile uint16_t comRhead 	= 0;
volatile uint16_t comRtail 	= 0;

/*---------------------------------------------------------------------------------------------------------*/
/* UART Callback function                                                                           	   */
/*---------------------------------------------------------------------------------------------------------*/
void UART_INT_HANDLE(void)
{
	uint8_t i;
	uint8_t bInChar[1] = {0xFF};

	while(UART0->ISR.RDA_IF==1) 
	{
		DrvUART_Read(UART_PORT0,bInChar,1);
		DrvUART_Write(UART_PORT1,bInChar1,1);	
	}
}

void delay_s(int time)		     // 1 secend
{
	int i=0;
	for(i=0;i<time*3;i++)
	{
		DrvSYS_Delay(330000);
	}
}

int32_t main()
{
	uint8_t  i =0;
	uint8_t  dataout[8] = "UARTTest";
	uint8_t aa[8];
	uint8_t j=0;
	uint8_t a[1];
	
	STR_UART_T sParam;

	UNLOCKREG();
    DrvSYS_Open(48000000);
	LOCKREG();
	Initial_pannel();
	clr_all_pannal();
   	
	/* Set UART Pin */
	DrvGPIO_Open(E_GPC, 12, E_IO_OUTPUT); 
	DrvGPIO_InitFunction(E_FUNC_UART0);
	DrvGPIO_InitFunction(E_FUNC_UART1);
	/* UART Setting */
    sParam.u32BaudRate 		= 115200;
    sParam.u8cDataBits 		= DRVUART_DATABITS_8;
    sParam.u8cStopBits 		= DRVUART_STOPBITS_1;
    sParam.u8cParity 		= DRVUART_PARITY_NONE;
    sParam.u8cRxTriggerLevel= DRVUART_FIFO_1BYTES;

	/* Set UART Configuration */
 	if(DrvUART_Open(UART_PORT0,&sParam) != E_SUCCESS);  
	if(DrvUART_Open(UART_PORT1,&sParam) != E_SUCCESS);  

	DrvUART_EnableInt(UART_PORT0, DRVUART_RDAINT, UART_INT_HANDLE);  
	delay_s(5);
	DrvUART_Write(UART_PORT0,"AT+CIPMUX=1\r\n",13);
	delay_s(1);
	DrvUART_Write(UART_PORT0,"AT+CIPSERVER=1,8087\r\n",21);
	delay_s(1);
	DrvGPIO_ClrBit(E_GPC, 12);	
	while(1);
}
