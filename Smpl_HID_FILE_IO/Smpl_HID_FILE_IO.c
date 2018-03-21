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
#include "Driver\DrvPDMA.h"
#include "Driver\DrvSPI.h"
#include "LCD_Driver.h"
#include "HIDSys.h"
#include "SPI_FLASH.h"

extern int32_t HID_MainProcess(void);

void Delay(uint32_t delayCnt)
{
    while (delayCnt--)
    {
        __NOP();
        __NOP();
    }
}

/*---------------------------------------------------------------------------------------------------------*/
/*  Main Function									                                           			   */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main(void)
{
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
    SYSCLK->PWRCON.XTL12M_EN = 1;

    /* Enable PLL */
    SYSCLK->PLLCON.OE = 0;
    DrvSYS_SetPLLMode(0);
    Delay(1000);

    /* Switch to PLL clock */
    DrvSYS_SelectHCLKSource(2);
    Delay(100);

    /* The PLL must be 48MHz x N times when using USB */
    // SystemFrequency = 48000000;
	LOCKREG();
#ifdef DEBUG
     printf("NUC100 USB HID FILE IO\n");
#endif
    Initial_pannel();  //call initial pannel function
    clr_all_pannal();  //clear all pannel
    print_lcd(0, "Initial USB   ");		//printf information

    /* ---------------------------------------------- */
    /* Configure SPI2 as a master to access SPI-Flash */
    /* ---------------------------------------------- */
    /*SPI0 test*/
    DrvGPIO_InitFunction(E_FUNC_SPI2);

    /* Configure SPI2 as a master, Type1 waveform, 32-bit transaction */
    DrvSPI_Open(eDRVSPI_PORT2, eDRVSPI_MASTER, eDRVSPI_TYPE1, 32);
    /* MSB first */
    DrvSPI_SetEndian(eDRVSPI_PORT2, eDRVSPI_MSB_FIRST);
    /* Disable the automatic slave select function of SS0. */
    DrvSPI_DisableAutoSS(eDRVSPI_PORT2);
    /* Set the active level of slave select. */
    DrvSPI_SetSlaveSelectActiveLevel(eDRVSPI_PORT2, eDRVSPI_ACTIVE_LOW_FALLING);
    /* Enable the SPI0 interrupt and install the callback function. */
    //DrvSPI_EnableInt(eDRVSPI_PORT2, SPI2_Callback, 0);
    /* SPI clock rate 12MHz */
    DrvSPI_SetClockFreq(eDRVSPI_PORT2, 12000000, 0);

	/* ---------------------------------------------- */
    /* read MID & DID */
    SpiFlash_ReadMidDid();
    SysTimerDelay(1000000);
    /* Erase SPI flash */
    print_lcd(2, "Erase Flash.....");
    print_lcd(3, "                ");
    SpiFlash_ChipErase();

    /* Wait ready */
    SpiFlash_WaitReady();

    /* Get the status of SPI flash */
    print_lcd(3, "Erase done !!!  ");
    SysTimerDelay(1000000);

    /* Execute HID process */
    HID_MainProcess();

}




