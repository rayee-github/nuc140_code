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
#include "Driver\DrvPDMA.h"
#include "Driver\DrvSPI.h"
#include "LCD_Driver.h"
#include "SpiProg.h"

extern int32_t CDROMHID_MainProcess(void);


/*---------------------------------------------------------------------------------------------------------*/
/*  Main Function									                                           			   */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main (void)
{
    uint32_t u32SpiMidDid;
#ifdef DEBUG
    STR_UART_T sParam;

    /* Set UART Pin */
    DrvGPIO_InitFunction(E_FUNC_UART0);

    /* UART Setting */
    sParam.u32BaudRate 		= 115200;
    sParam.u8cDataBits 		= DRVUART_DATABITS_8;
    sParam.u8cStopBits 		= DRVUART_STOPBITS_1;
    sParam.u8cParity 		= DRVUART_PARITY_NONE;
    sParam.u8cRxTriggerLevel = DRVUART_FIFO_1BYTES;

    /* Select UART Clock Source From 12Mhz*/
    DrvSYS_SelectIPClockSource(E_SYS_UART_CLKSRC, 0);

    /* Set UART Configuration */
    DrvUART_Open(UART_PORT0, &sParam);
#endif
    UNLOCKREG(); 
	/* Enable External Crystal */
	DrvSYS_SetOscCtrl(E_SYS_XTL12M, 1);
    
    /* Wait External Crystal Stable */
    DrvSYS_Delay(1024);
    
    /* Enable PLL */
    DrvSYS_SetPLLMode(0);
	
    /* Wait PLL Stable */
    DrvSYS_Delay(1625);

	/* Switch to PLL clock */
	DrvSYS_SelectHCLKSource(2);    

	LOCKREG();
#ifdef DEBUG
     printf("NUC100 CDROM HID FILE IO\n");
#endif
    Initial_pannel();  //call initial pannel function
    clr_all_pannal();  //clear all pannel
    print_lcd(0, "Initial USB   ");		//printf information

	/* Initial SPI*/
	SpiInit(eDRVSPI_PORT2);

    /* read MID & DID */
	u32SpiMidDid = SpiReadMidDid();
    SysTimerDelay(1000000);

    if ((u32SpiMidDid & 0xFFFF) == 0xEF14)
        print_lcd(3, "MID & DID=0xEF14");
    else
        print_lcd(3, "MID & DID Error!");

    /* Execute CDROMHID process */
	CDROMHID_MainProcess();
}
