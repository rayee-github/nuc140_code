/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) 2009 Nuvoton Technology Corp. All rights reserved.                                         */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include "Driver\DrvFMC.h"
#include "Driver\DrvGPIO.h"
#include "Driver\DrvUART.h"
#include "Driver\DrvSYS.h"
#include "LCD_Driver.h"


#define LDROM_BASE      0x00100000
#define PAGE_SIZE       512
	int32_t  i32Err=0;


extern uint32_t loaderImageBase;
extern uint32_t loaderImageLimit;

void delay_loop(void)
 {
 uint32_t i,j;
 		for(i=0;i<4;i++)
		for(j=0;j<60000;j++);		
		
 
 }
/*---------------------------------------------------------------------------------------------------------*/
/*  Main Function									                                           			   */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main (void)
{
	

	uint32_t u32Data, i, u32ImageSize, j, *pu32Loader;
	UNLOCKREG();
    SYSCLK->PWRCON.XTL12M_EN = 1;

	/*set GPC outpui*/
	for(i=12;i<16;i++)	
	DrvGPIO_Open(E_GPC,i, E_IO_OUTPUT);
	Initial_pannel();  /* call initial pannel function */


	clr_all_pannal();
	
	print_lcd(0, "Boot in AP     ");
	print_lcd(1, "Press SW_INT to");
	print_lcd(2, "Do ISP function");	  
	print_lcd(3, "and Boot in LD ");

	while((GPIOB->PIN & (1 << 15)))
	{
		//Flash led  1 by 1
		for(i=16;i>11;i--)
		{
        DrvGPIO_ClrBit(E_GPC,i);
		delay_loop();
		DrvGPIO_SetBit(E_GPC,i);		
		}
	 }
	 /* Unlock protected registers to write ISP Control Register (ISPCON) */
    UNLOCKREG();
	/* Enable ISP function */
	DrvFMC_EnableISP();

	/* Enable LDROM update */
	DrvFMC_EnableLDUpdate();
    
	/* Page Erase LDROM */
    for(i=0;i<4096;i+=PAGE_SIZE)
        DrvFMC_Erase(LDROM_BASE + i);
	/* Erase Verify */
    i32Err = 0;
	for(i = LDROM_BASE; i < (LDROM_BASE+4096); i += 4) 
    {     
		DrvFMC_Read(i, &u32Data);
        
        if(u32Data != 0xFFFFFFFF)
        { 
            i32Err = 1;
        }         
    }

	
   u32ImageSize = (uint32_t)&loaderImageLimit - (uint32_t)&loaderImageBase;
   
    pu32Loader = (uint32_t *)&loaderImageBase;
    for(i=0;i<u32ImageSize;i+=PAGE_SIZE)
    {
        DrvFMC_Erase(LDROM_BASE + i);    
        for(j=0;j<PAGE_SIZE;j+=4)
        {
            DrvFMC_Write(LDROM_BASE + i + j, pu32Loader[(i + j) / 4]);
        }
    }

    /* Verify loader */
    i32Err = 0;
    for(i=0;i<u32ImageSize;i+=PAGE_SIZE)
    {
        for(j=0;j<PAGE_SIZE;j+=4)
        {
            DrvFMC_Read(LDROM_BASE + i + j, &u32Data);
            if(u32Data != pu32Loader[(i+j)/4])
                i32Err = 1;
            
            if(i + j >= u32ImageSize)
                break;
        }


    }


	print_lcd(0, "Boot in LD     ");
	print_lcd(1, "               ");
	print_lcd(2, "               ");	  
	print_lcd(3, "               ");       
    DrvFMC_BootSelect(E_FMC_LDROM);
    DrvSYS_ResetCPU();
  	while(1);
}




