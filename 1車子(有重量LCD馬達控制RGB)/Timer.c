#include "NUC1xx.h"
#include <stdio.h>	
#include "LCD_Driver.h"
#include "Timer.h"
#include "Driver\DrvSYS.h"
#include "Driver\DrvGPIO.h"

uint16_t TimerCounter=0;
int pwm=2850;
void InitTIMER0(void)
{
	/* Step 1. Enable and Select Timer clock source */          
	SYSCLK->CLKSEL1.TMR0_S = 0;	//Select 12Mhz for Timer0 clock source 
  SYSCLK->APBCLK.TMR0_EN =1;	//Enable Timer0 clock source

	/* Step 2. Select Operation mode */	
	TIMER0->TCSR.MODE=1;		//Select periodic mode for operation mode

	/* Step 3. Select Time out period = (Period of timer clock input) * (8-bit Prescale + 1) * (24-bit TCMP)*/
	TIMER0->TCSR.PRESCALE=255;	// Set Prescale [0~255]
	TIMER0->TCMPR = 5765;		// Set TCMPR [0~16777215]
								// (1/12000000)*(255+1)*(2765)= 125.01usec or 7999.42Hz

	/* Step 4. Enable interrupt */
	TIMER0->TCSR.IE = 1;
	TIMER0->TISR.TIF = 1;		//Write 1 to clear for safty		
	NVIC_EnableIRQ(TMR0_IRQn);	//Enable Timer0 Interrupt

	/* Step 5. Enable Timer module */
	TIMER0->TCSR.CRST = 1;		//Reset up counter
	TIMER0->TCSR.CEN = 1;		//Enable Timer0

//  	TIMER0->TCSR.TDR_EN=1;		// Enable TDR function
}

void TMR0_IRQHandler(void) // Timer0 interrupt subroutine 
{
	//TimerCounter+=1;
	if(DrvGPIO_GetBit(E_GPD, 1)==0 && DrvGPIO_GetBit(E_GPD, 3)==0)
	{
		pwm=2850;
		PWMA->CMR0=pwm;//2850
		PWMA->CMR1=5;
		PWMA->CMR2=5;
		DrvGPIO_SetBit(E_GPC, 7);
		DrvGPIO_SetBit(E_GPC, 6);
	}
	else if(DrvGPIO_GetBit(E_GPD, 1)==0 && DrvGPIO_GetBit(E_GPD, 3)==1)
	{
		if(pwm<3550)
			pwm+=150;
		PWMA->CMR0=pwm;//3550
		PWMA->CMR1=25000;
		PWMA->CMR2=40000;
		DrvGPIO_ClrBit(E_GPC, 7);
		DrvGPIO_SetBit(E_GPC, 6);
	}
	else if(DrvGPIO_GetBit(E_GPD, 1)==1 && DrvGPIO_GetBit(E_GPD, 3)==0)
	{
		if(pwm>=2150)
			pwm-=150;
		PWMA->CMR0=pwm;//2150
		PWMA->CMR1=40000;
		PWMA->CMR2=25000;
		DrvGPIO_SetBit(E_GPC, 7);
		DrvGPIO_ClrBit(E_GPC, 6);
	}
	else if(DrvGPIO_GetBit(E_GPD, 1)==1 && DrvGPIO_GetBit(E_GPD, 3)==1)
	{
		pwm=2850;
		PWMA->CMR0=pwm;//2850
		PWMA->CMR1=40000;
		PWMA->CMR2=40000;
		DrvGPIO_ClrBit(E_GPC, 7);
		DrvGPIO_ClrBit(E_GPC, 6);
	}
 	TIMER0->TISR.TIF =1;
}