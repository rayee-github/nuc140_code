/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright (c) Nuvoton Technology Corp. All rights reserved.                                             */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include "Driver\DrvGPIO.h"
#include "Driver\DrvSYS.h"
#include "Driver\DrvUART.h"
#include "Driver\DrvUSB.h"
#include "LCD_Driver.h"
#include "Seven_Segment.h"
#include "ScanKey.h"

#define DEBUG 0

extern int32_t VCOM_MainProcess(void);

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
#endif

    UNLOCKREG();
	/* Enable 12M Crystal */
	DrvSYS_SetOscCtrl(E_SYS_XTL12M, 1);
    
    /* Enable PLL */
    DrvSYS_SetPLLMode(0);
	Delay(1000);

	/* Switch to PLL clock */
	DrvSYS_SelectHCLKSource(2);    

    Delay(100);

    /* The PLL must be 48MHz when using USB */

#ifdef DEBUG
     printf("NUC100 Virtual Com.\n");
#endif

    Initial_pannel();
    clr_all_pannal();
    print_lcd(0, "Initial VCOM  ");

	/* Initial Key Pad */
	OpenKeyPad();

	close_seven_segment();
		
    /* Execute VCOM process */
	VCOM_MainProcess();
}
