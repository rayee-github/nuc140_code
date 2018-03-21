/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) 2009 Nuvoton Technology Corp. All rights reserved.                                         */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include "DrvGPIO.h"
#include "DrvSYS.h"
#include "DrvUART.h"
#include "DrvUSB.h"

extern int32_t UAC_MainProcess(void);

void Delay(uint32_t delayCnt)
{
    while(delayCnt--)
    {
        __NOP();
        __NOP();
    }
}

void Disable_Buzzer(void)
{
 	DrvGPIO_Open(E_GPB,11, E_IO_OUTPUT);	
	DrvGPIO_ClrBit(E_GPB,11);	
}
/*---------------------------------------------------------------------------------------------------------*/
/* Main function                                                                                           */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main (void)
{
	//extern uint32_t SystemFrequency;
    /* Unlock the locked registers */
    UNLOCKREG();

	/* Enable 12M OSC */
	DrvSYS_SetOscCtrl(E_SYS_XTL12M, 1);

    /* Enable PLL */
    DrvSYS_SetPLLMode(0);
	Delay(1000);
	DrvSYS_SelectHCLKSource(2);    
    Delay(100);

    /* The PLL must be 48MHz x N times when using USB */
    //SystemFrequency = 48000000;
    
    UAC_MainProcess();

}




