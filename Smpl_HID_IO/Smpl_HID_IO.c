/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) 2009 Nuvoton Technology Corp. All rights reserved.                                         */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include "Driver\DrvGPIO.h"
#include "Driver\DrvSYS.h"
#include "Driver\DrvUART.h"
#include "Driver\DrvUSB.h"
#include "LCD_Driver.h"

#define DEBUG 0

extern int32_t HID_MainProcess(void);

void Delay(uint32_t delayCnt)
{
    while(delayCnt--)
    {
        __NOP();
        __NOP();
    }
}

/*---------------------------------------------------------------------------------------------------------*/
/*  Main Function									                                           			   */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main (void)
{
//	extern uint32_t SystemFrequency;

#ifdef DEBUG
	STR_UART_T sParam;

	/* Set UART Pin */
	DrvGPIO_InitFunction(E_FUNC_UART0);
	
	/* UART Setting */
    sParam.u32BaudRate 		= 115200;
    sParam.u8cDataBits 		= DRVUART_DATABITS_8;
    sParam.u8cStopBits 		= DRVUART_STOPBITS_1;
    sParam.u8cParity 		= DRVUART_PARITY_NONE;
    sParam.u8cRxTriggerLevel= DRVUART_FIFO_1BYTES;

 	/* Select UART Clock Source From 12Mhz*/
	DrvSYS_SelectIPClockSource(E_SYS_UART_CLKSRC,0); 

	/* Set UART Configuration */
	DrvUART_Open(UART_PORT0,&sParam);
#endif

    UNLOCKREG();

    SYSCLK->PWRCON.XTL12M_EN = 1;

#ifdef DEBUG
     printf("CPU has been hold. Press any SW30 to continue...\n");
#endif
    HID_Init();
    /* Enable PLL */
    DrvSYS_SetPLLMode(0);
	Delay(1000);

	/* Switch to PLL clock */
	DrvSYS_SelectHCLKSource(2);    
    Delay(100);

    /* Update system core clock */
    SystemCoreClockUpdate();

#ifdef DEBUG    
    printf("NUC100 USB HID\n");
#endif

    Initial_pannel();  //call initial pannel function
    clr_all_pannal();  //clear all pannel
    print_lcd(0, "Initial USB   ");		//printf information
	
    /* Execute HID process */
	HID_MainProcesss();

}




