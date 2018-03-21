/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) 2009 Nuvoton Technology Corp. All rights reserved.                                         */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include "NUC1xx.h"
#include "Driver\DrvGPIO.h"
#include "Driver\DrvUART.h"
#include "Driver\DrvSYS.h"

void EINT1Callback(void)
{
   printf("Interrupt\n");

}
void Delay(uint32_t counter)
{
	while(counter--);
}  

int main (void)
{
	STR_UART_T sParam;
	/* SYSCLK =>12Mhz*/
	UNLOCKREG();
	/* Enable 12M Crystal */
	DrvSYS_SetOscCtrl(E_SYS_XTL12M, 1);
	/* Switch to PLL clock */
	DrvSYS_SelectHCLKSource(0);
	LOCKREG();

	/* Set UART0 Pin */
	DrvGPIO_InitFunction(E_FUNC_UART0);
	
	/* UART Setting */
    sParam.u32BaudRate 		= 115200;
    sParam.u8cDataBits 		= DRVUART_DATABITS_8;
    sParam.u8cStopBits 		= DRVUART_STOPBITS_1;
    sParam.u8cParity 		= DRVUART_PARITY_NONE;
    sParam.u8cRxTriggerLevel= DRVUART_FIFO_1BYTES;

	/* Select UART Clock Source From 12Mhz*/
	DrvSYS_SelectIPClockSource(E_SYS_UART_CLKSRC,0); 

	/* Set UART0 Configuration */
	DrvUART_Open(UART_PORT0, &sParam);
 
/*---------------------------------------------------------------------------------------------------------*/
/* GPIO Interrupt Test                                                                                     */
/*---------------------------------------------------------------------------------------------------------*/    
    
    /* Configure general GPIO interrupt */
    DrvGPIO_Open(E_GPB, 15, E_IO_INPUT);

    /* Configure external interrupt */
    DrvGPIO_EnableEINT1(E_IO_BOTH_EDGE, E_MODE_EDGE, EINT1Callback);

    /* Waiting for interrupts */
    while(1)
	{
		printf("Deep Sleep\n");
		while(UART0->FSR.TE_FLAG == 0);
    GPIOA->DOUT|=0x7000;
		UNLOCKREG();						  
		SCB->SCR = 4;
		SYSCLK->PWRCON.PD_WU_INT_EN= 0;	
		SYSCLK->PWRCON.PD_WAIT_CPU = 1;
	  	SYSCLK->PWRCON.PWR_DOWN_EN = 1;
	 	LOCKREG();
	
		__WFI();
    
       while((GPIOB->PIN &= 0x8000)==0)
	   {
			GPIOA->DOUT&=0x8FFF;
			Delay(30000);
			GPIOA->DOUT|=0x7000;
			Delay(30000);
	   }
 	}
}