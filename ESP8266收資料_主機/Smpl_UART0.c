//
// Smpl_UART0				   收資料 主機
//
// Nu-LB-NUC140
// pin32 GPB0/RX0 to another board's UART TX
// pin33 GPB1/TX0 to another board's UART RX

#include <stdio.h>
#include "Driver\DrvUART.h"
#include "Driver\DrvGPIO.h"
#include "Driver\DrvSYS.h"
#include "NUC1xx.h"
#include "NUC1xx-LB_002\LCD_Driver.h"

volatile uint8_t comRbuf[15] ={0x30};
volatile uint16_t comRbytes = 0;
volatile uint16_t comRhead 	= 0;
volatile uint16_t comRtail 	= 0;


char TEXT1[16] = "TX: sending...  ";
char TEXT2[16] = "RX:             ";

/*---------------------------------------------------------------------------------------------------------*/
/* UART Callback function                                                                           	   */
/*---------------------------------------------------------------------------------------------------------*/
void UART_INT_HANDLE(void)
{
	uint8_t i;
	uint8_t bInChar[1] = {0xFF};

	//while(UART0->ISR.RDA_IF==1) 
	{

		DrvUART_Read(UART_PORT0,bInChar,1);	   
		if(comRbytes < 15) 
		{
			comRbuf[comRbytes] = bInChar[0];
			comRbytes++;
				
		}
		else if (comRbytes==15)
		{
			comRbytes=0;
			sprintf(TEXT2,"%s",comRbuf);
			print_lcd(2,TEXT2);
		}		
		else if (comRbytes>15)
		{
		print_lcd(0,"j");
			comRbytes=0;
			sprintf(TEXT2,"%s",comRbuf);
			print_lcd(2,TEXT2);
		}		
	}
}

void delay_time(int time)		     // 1 secend
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
	char a[100];
	uint8_t bInChar[10] = {0xFF};
	uint8_t bInChar2[100] = {0xFF};
	STR_UART_T sParam;

	UNLOCKREG();
    DrvSYS_Open(48000000);
	LOCKREG();
	Initial_pannel();
	clr_all_pannal();
   	
	/* Set UART Pin */
	DrvGPIO_InitFunction(E_FUNC_UART0);		

	/* UART Setting */
    sParam.u32BaudRate 		= 115200;
    sParam.u8cDataBits 		= DRVUART_DATABITS_8;
    sParam.u8cStopBits 		= DRVUART_STOPBITS_1;
    sParam.u8cParity 		= DRVUART_PARITY_NONE;
    sParam.u8cRxTriggerLevel= DRVUART_FIFO_1BYTES;

	/* Set UART Configuration */
 	if(DrvUART_Open(UART_PORT0,&sParam) != E_SUCCESS);  
	
	//DrvUART_EnableInt(UART_PORT0, DRVUART_RDAINT, UART_INT_HANDLE);  
	//DrvUART_Write(UART_PORT0,"AT+CWJAP=\"1234\",\"11111111\"\r\n",27); 

	delay_time(5);
	print_lcd(0,"WIFI OK");

	DrvUART_Write(UART_PORT0,"AT+CIPMUX=1\r\n",13);
	delay_time(1);
	
	DrvUART_Write(UART_PORT0,"AT+CIPSERVER=1,8087\r\n",21);
	delay_time(1);

	while(1)
	{
		//DrvUART_EnableInt(UART_PORT0, DRVUART_RDAINT, UART_INT_HANDLE);         
		DrvUART_Read(UART_PORT0,bInChar,1);	
		if(bInChar[0]==0x3A)           //3A=冒號
		{
			DrvUART_Read(UART_PORT0,bInChar,2);
			sprintf(TEXT2+4,"%s",bInChar);
			print_lcd(1,TEXT2);
			DrvSYS_Delay(30000);
		}
	}
	//DrvUART_Close(UART_PORT0);
}




































































