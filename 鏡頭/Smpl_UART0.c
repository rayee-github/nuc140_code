//
// Smpl_UART0
//
// Nu-LB-NUC140		  GPB0收資料
// pin32 GPB0/RX0 to another board's UART TX
// pin33 GPB1/TX0 to another board's UART RX
//b5 TX1 
//b4 RX1

#include <stdio.h>
#include <string.h>
#include "Driver\DrvUART.h"
#include "Driver\DrvGPIO.h"
#include "Driver\DrvSYS.h"
#include "NUC1xx.h"
#include "NUC1xx-LB_002\LCD_Driver.h"

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
	}
}

int32_t main()
{
	unsigned char command[] = { 0xAA, 0x0D, 0x00, 0x00, 0x00, 0x00 };
  uint8_t rx[6];
	uint8_t pkt[128];
	uint8_t p[1];
	int total = 0;
	int flag=0;
	char c[1];
	 
	STR_UART_T sParam;
	STR_UART_T sParam2;

	UNLOCKREG();
    DrvSYS_Open(48000000);
	LOCKREG();
   	
	/* Set UART Pin */
	DrvGPIO_InitFunction(E_FUNC_UART0);		
	DrvGPIO_InitFunction(E_FUNC_UART1);		

	/* UART Setting */
    sParam.u32BaudRate 		= 115200;             //鏡頭
    sParam.u8cDataBits 		= DRVUART_DATABITS_8;
    sParam.u8cStopBits 		= DRVUART_STOPBITS_1;
    sParam.u8cParity 		= DRVUART_PARITY_NONE;
    sParam.u8cRxTriggerLevel= DRVUART_FIFO_1BYTES;
		
		sParam2.u32BaudRate 		= 115200;           //wifi
    sParam2.u8cDataBits 		= DRVUART_DATABITS_8;
    sParam2.u8cStopBits 		= DRVUART_STOPBITS_1;
    sParam2.u8cParity 		= DRVUART_PARITY_NONE;
    sParam2.u8cRxTriggerLevel= DRVUART_FIFO_1BYTES;

	/* Set UART Configuration */
 	if(DrvUART_Open(UART_PORT0,&sParam) != E_SUCCESS);  
	if(DrvUART_Open(UART_PORT1,&sParam2) != E_SUCCESS);  

	//DrvUART_EnableInt(UART_PORT0, DRVUART_RDAINT, UART_INT_HANDLE);  

	//喚醒模組
	for (int i = 0; i < 30; i++)
  {
		DrvUART_Write(UART_PORT0,command,6);
		DrvSYS_Delay(100000);
		DrvUART_Read(UART_PORT0,rx,6);	
  }
	while(1)
	{
		DrvUART_Read(UART_PORT1,c,1);	
		if(c[0]=='A')
		{
				//初始化
				command[1] = 0x01;
				command[2] = 0x00;
				command[3] = 0x07;
				command[4] = 0x01;
				command[5] = 0x07;
				DrvUART_Write(UART_PORT0,command,6);      //{'AA','01','00','07','01','07'}
				DrvSYS_Delay(100000);
				DrvUART_Read(UART_PORT0,rx,6);	
				//設定128byte
				command[1] = 0x06;
				command[2] = 0x08;
				command[3] = 0x80;
				command[4] = 0x00;
				command[5] = 0x00;
				DrvUART_Write(UART_PORT0,command,6);      //{'AA','06','08','80','00','00'}
				DrvSYS_Delay(100000);
				DrvUART_Read(UART_PORT0,rx,6);	
				//壓縮圖像
				command[1] = 0x05;
				command[2] = 0x00;
				command[3] = 0x00;
				command[4] = 0x00;
				DrvUART_Write(UART_PORT0,command,6);      //{'AA','05','00','00','00','00'}
				DrvSYS_Delay(100000);
				DrvUART_Read(UART_PORT0,rx,6);	
				//獲取圖像
				command[1] = 0x04;
				command[2] = 0x01;
				DrvUART_Write(UART_PORT0,command,6);      //{'AA','04','01','00','00','00'}
				DrvSYS_Delay(100000);	
				DrvUART_Read(UART_PORT0,rx,6);	
				//DrvUART_Read(UART_PORT0,rx,6);	
				total = (rx[3]) | (rx[4] << 8);
				total=6000;
				//收圖
				unsigned int pktCnt = total / (128 - 6); 
				command[1] = 0x0E;
				command[2] = 0x00;
				command[3] = 0x00;
				command[4] = 0x00;
				command[5] = 0x00;
				for (unsigned int i = 0; i < pktCnt; i++)
				{
					uint8_t s[128];
					command[4] = i & 0xff;
					command[5] = (i >> 8) & 0xff;
					DrvUART_Write(UART_PORT0,command,6);
					if(flag==0)
					{
						DrvUART_Read(UART_PORT0,rx,6);	
						total = (rx[3]) | (rx[4] << 8) |(rx[5] << 16);
						pktCnt = total / (128 - 6);
						char totalc[4];
						sprintf(totalc,"A%d",pktCnt);
						DrvUART_Write(UART_PORT1,totalc,4);
						flag=1;
					}
					for(int j=0;j<128;j++)
					{
						DrvUART_Read(UART_PORT0,p,1);
						if(p[0]==0x00)
							pkt[j]=0x00;
						else
							pkt[j]=p[0];
					}
					for(int j=0;j<122;j++)
					{
						sprintf(s+j,"%c",pkt[j+4]);
					}
					DrvUART_Write(UART_PORT1,s,122);
					sprintf(s,"");
				}
				command[4] = 0xf0;
				command[5] = 0xf0;
				DrvUART_Write(UART_PORT0,command,6);
				DrvSYS_Delay(300000);
				flag=0;
		}
	}
}