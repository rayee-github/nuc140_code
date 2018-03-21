/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright (c) Nuvoton Technology Corp. All rights reserved.                                             */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include "NUC1xx.h"
#include "VCOM_API.h"


void InitUART()
{
  //Set UART0 Pin 
  outp32(&SYS->GPBMFP, inp32(&SYS->GPBMFP) | 0x3);

  // Select UART Clock Source From 12Mhz
  //SYSCLK->APBCLK.UART0_EN = 1;
  SYSCLK->CLKSEL1.UART_S = 0;
  SYSCLK->CLKDIV.UART_N = 0;

  /* Reset UART 0, move to main()*/    
  //outp32(&SYS->IPRSTC2, 1 << 16);
  //outp32(&SYS->IPRSTC2, 0);
    
  /* Set Rx Trigger Level */
  //UART0->FCR.RFITL = DRVUART_FIFO_1BYTES;  	
  outp32(&UART0->FCR, 0x6);

  /* Set Parity & Data bits & Stop bits */
  outp32(&UART0->LCR, 3); /* 1 stop bit, 8-bit */     		
  outp32(&UART0->BAUD, 0x30000000 | (12000000/115200 - 2));
}


int main(void)
{
  UNLOCKREG();
  /* Enable 12Mhz crystal */
  SYSCLK->PWRCON.XTL12M_EN = 1;
  /* Enable and set PLL clock for 48Mhz */
  outp32(&SYSCLK->PLLCON, 0xC22E);
  /* Waiting for 12M Xtal stalble*/ 
  SysTimerDelay(5000);

  /* USB & UART0 clock enable */
  outp32(&SYSCLK->APBCLK, 0x08010000); 
  /* Reset UART0, USB */
  outp32(&SYS->IPRSTC2, 0x08010000);
  SysTimerDelay(500);
  outp32(&SYS->IPRSTC2, 0x0);

  /* Select PLL clock for HCLK*/
  SYSCLK->CLKSEL0.HCLK_S = 2;
  
  InitUART();

  VCOM_MainProcess();
}

