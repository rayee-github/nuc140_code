//
// Smpl_UART0
//
// Nu-LB-NUC140		  GPB0收資料
// pin32 GPB0/RX0 to another board's UART TX
// pin33 GPB1/TX0 to another board's UART RX

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


char TEXT1[16] = "TX: sending...  ";
char TEXT2[16] = "RX:             ";
char TEXTt1[16] = "                ";
char TEXTt2[16] = "                ";
char TEXTt3[16] = "                ";
char TEXTt4[16] = "                ";
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
		if(bInChar[0]==0xA5)
		{
			DrvUART_Read(UART_PORT0,bInChar,1);	
			if(bInChar[0]==0x5A)
			{
				DrvUART_Read(UART_PORT0,bInChar,1);	
				if(bInChar[0]==0x12)
				{
					DrvUART_Read(UART_PORT0,bInChar,1);	//A1不管  /
					//////////////////////
					DrvUART_Read(UART_PORT0,bInChar,1);
					if(bInChar[0]>=0x80)
					{
						comRbuf[0] = bInChar[0]-0x80;
						comRbuf[6] = 0x2D;
					}
					else 
					{
						comRbuf[0] = bInChar[0];
						comRbuf[6] = 0x20;
					}		   		   
					DrvUART_Read(UART_PORT0,bInChar,1);
					comRbuf[1] = bInChar[0];
					DrvUART_Read(UART_PORT0,bInChar,1);
					if(bInChar[0]>=0x80)
					{
						comRbuf[2] = bInChar[0]-0x80;
						comRbuf[7] = 0x2D;
					}
					else 
					{
						comRbuf[2] = bInChar[0];
						comRbuf[7] = 0x20;
					}		   		   
					DrvUART_Read(UART_PORT0,bInChar,1);
					comRbuf[3] = bInChar[0];
					DrvUART_Read(UART_PORT0,bInChar,1);
					if(bInChar[0]>=0x80)
					{
						comRbuf[4] = bInChar[0]-0x80;
						comRbuf[8] = 0x2D;
					}
					else 
					{
						comRbuf[4] = bInChar[0];
						comRbuf[8] = 0x20;
					}		   
					DrvUART_Read(UART_PORT0,bInChar,1);
					comRbuf[5] = bInChar[0];

					sprintf(TEXTt1,"%c%d  %c%d  %c%d ",comRbuf[6],comRbuf[0]*16*16+comRbuf[1],comRbuf[7],comRbuf[2]*16*16+comRbuf[3],comRbuf[8],comRbuf[4]*16*16+comRbuf[5]);
					print_lcd(0,TEXTt1);
					///////////////////////	  
					DrvUART_Read(UART_PORT0,bInChar,1);
					if(bInChar[0]>=0x80)
					{
						comRbuf[0] = bInChar[0]-0x80;
						comRbuf[4] = 0x2D;
					}
					else 
					{
						comRbuf[0] = bInChar[0];
						comRbuf[4] = 0x20;
					}		   
					DrvUART_Read(UART_PORT0,bInChar,1);
					comRbuf[1] = bInChar[0];
					DrvUART_Read(UART_PORT0,bInChar,1);
					if(bInChar[0]>=0x80)
					{
						comRbuf[2] = bInChar[0]-0x80;
						comRbuf[5] = 0x2D;
					}
					else 
					{
						comRbuf[2] = bInChar[0];
						comRbuf[5] = 0x20;
					}		   
					DrvUART_Read(UART_PORT0,bInChar,1);
					comRbuf[3] = bInChar[0];

					sprintf(TEXTt2,"%c%d  %c%d ",comRbuf[4],comRbuf[0]*16*16+comRbuf[1],comRbuf[5],comRbuf[2]*16*16+comRbuf[3]);
					print_lcd(1,TEXTt2);
					///////////////////////
					DrvUART_Read(UART_PORT0,bInChar,1);
					if(bInChar[0]>=0x80)
						comRbuf[0] = bInChar[0]-0x80;
					else 
						comRbuf[0] = bInChar[0];		   
					DrvUART_Read(UART_PORT0,bInChar,1);
					comRbuf[1] = bInChar[0];

					sprintf(TEXTt3,"%d ",(comRbuf[0]*16*16+comRbuf[1])*10);
					print_lcd(2,TEXTt3);
					
		DrvUART_Write(UART_PORT0,"gfd",3);   
					
				}
				else if(bInChar[0]==0x16)
				{
					DrvUART_Read(UART_PORT0,bInChar,1);	//A2不管
				}
			}
		}








		/*if(comRbytes < 12) // check if Buffer is full
		{
			comRbuf[comRbytes] = bInChar[0];
			comRbytes++;
		}
		else if (comRbytes>=12)
		{
			comRbytes=0;
			sprintf(TEXT2+4,"%s",comRbuf);
			print_lcd(2,TEXT2);
			for(i=4;i<16;i++)
			{
				TEXT2[i]=' ';
			}
			DrvSYS_Delay(5000000);
		}			
		DrvSYS_Delay(5000);		*/			   		
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
	//print_lcd(0,"Smpl_UART0    ");
   	
	/* Set UART Pin */
	DrvGPIO_InitFunction(E_FUNC_UART0);		

	/* UART Setting */
    sParam.u32BaudRate 		= 9600;
    sParam.u8cDataBits 		= DRVUART_DATABITS_8;
    sParam.u8cStopBits 		= DRVUART_STOPBITS_1;
    sParam.u8cParity 		= DRVUART_PARITY_NONE;
    sParam.u8cRxTriggerLevel= DRVUART_FIFO_1BYTES;

	/* Set UART Configuration */
 	if(DrvUART_Open(UART_PORT0,&sParam) != E_SUCCESS);  

	DrvUART_EnableInt(UART_PORT0, DRVUART_RDAINT, UART_INT_HANDLE);  
	
	DrvGPIO_Open(E_GPC, 12, E_IO_OUTPUT); 
	DrvGPIO_SetBit(E_GPC, 12);
	DrvGPIO_Open(E_GPC, 13, E_IO_OUTPUT); 
	DrvGPIO_SetBit(E_GPC, 13);
	
	while(1)
	{


	/*DrvGPIO_ClrBit(E_GPC, 12); // output Low to turn on LED
	DrvGPIO_SetBit(E_GPC, 13);
	DrvSYS_Delay(900000);	   // delay 
	DrvGPIO_SetBit(E_GPC, 12); // output Hi to turn off LED
   	DrvGPIO_ClrBit(E_GPC, 13); 
	DrvSYS_Delay(900000);		*/
								 

 	    //dataout[7] = 0x30 +i;	
	//	aa[j]=DrvGPIO_ClrBit(E_GPC, 12);
	//	aa[j]=DrvGPIO_SetBit(E_GPC, 12);
	//	UART_TX(UART_PORT0,aa,1,j);

	//	 aa[j]=0x3c;
													   
		DrvUART_Write(UART_PORT0,"gfh",3);   
	 //write可以傳字串,和數字
	//	UART_TX(UART_PORT0,aa,1,7);	  //UART_TX只能數字				 
			
			 
			
				/* DrvUART_Read(UART_PORT0,a,1);   
				sprintf(TEXT3+4,"%s",a[0]);
				 print_lcd(1,TEXT3);		*/
				
	/*	i++;
	 
		if (i>=10) i = 0;
		sprintf(TEXT1+14,"%d",i);
		print_lcd(1,TEXT1);	 */
		DrvSYS_Delay(50000);
	}
	//DrvUART_Close(UART_PORT0);
}




































































