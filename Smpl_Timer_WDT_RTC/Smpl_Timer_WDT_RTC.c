/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) 2009 Nuvoton Technology Corp. All rights reserved.                                         */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#include <stdio.h>	
#include <string.h>																										 
#include "NUC1xx.h"
#include "LCD_Driver.h"

static uint16_t TimerCounter=0;
static uint8_t Alarm_E=1;
void Delay(int count);



//---------------------------------------------------------------------------------WDT
void InitWDT(void)
{
	UNLOCKREG();		
	/* Step 1. Enable and Select WDT clock source */         
	SYSCLK->CLKSEL1.WDT_S =3;//Select 10Khz for WDT clock source  	
	SYSCLK->APBCLK.WDT_EN =1;//Enable WDT clock source	

	/* Step 2. Select Timeout Interval */
	WDT->WTCR.WTIS=5;//Select level 7			
	/* Step 3. Disable Watchdog Timer Reset function */
	WDT->WTCR.WTRE = 0;
	/* Step 4. Enable WDT interrupt */	
	WDT->WTCR.WTIF =1;//Write 1 to clear for safety		
	WDT->WTCR.WTIE = 1;
	NVIC_EnableIRQ(WDT_IRQn);
	/* Step 5. Enable WDT module */
	//Enable WDT
	WDT->WTCR.WTE = 1;
	 //Clear WDT counter
	WDT->WTCR.WTR = 1;		
	LOCKREG();	 
}
void WDT_IRQHandler(void) 
{ 
	UNLOCKREG(); 
	WDT->WTCR.WTIF =1;
	WDT->WTCR.WTR = 1;
	UNLOCKREG();
	print_lcd(3, "WDT interrupt");
}
//---------------------------------------------------------------------------------RTC
void set_TLR (int32_t a,int32_t b,int32_t c,int32_t d,int32_t e,int32_t f)
{
	outpw(&RTC->TLR, a<<20|b<<16|c<<12|d<<8|e<<4|f)	 ;
}
void set_CLR (int32_t a,int32_t b,int32_t c,int32_t d,int32_t e,int32_t f)
{
	outpw(&RTC->CLR, a<<20|b<<16|c<<12|d<<8|e<<4|f)	 ;
}
void set_TAR(int32_t a,int32_t b,int32_t c,int32_t d,int32_t e,int32_t f)
{
	outpw(&RTC->TAR, a<<20|b<<16|c<<12|d<<8|e<<4|f)	;
}
void set_CAR (int32_t a,int32_t b,int32_t c,int32_t d,int32_t e,int32_t f)
{
	outpw(&RTC->CAR, a<<20|b<<16|c<<12|d<<8|e<<4|f)	;
}
void START_RTC(void)
{

  	while(1)
  	{
		RTC->INIR = 0xa5eb1357;
		if(inpw(&RTC->INIR)==1)
        	break; 
  	}   
  	while(1)
  	{
  		RTC->AER.AER = 0xA965;
    	if(inpw(&RTC->AER)&0x10000)// AER bit
        	break;  
    }
}
void InitRTC(void)
{
	UNLOCKREG();		
	/* Step 1. Enable and Select RTC clock source */     
	SYSCLK->PWRCON.XTL32K_EN = 1;//Enable 32Khz for RTC clock source
	SYSCLK->APBCLK.RTC_EN =1;//Enable RTC clock source	

	/* Step 2. Initiate and unlock RTC module */
	 
	START_RTC();

	/* Step 3. Initiate Time and Calendar  setting */
	RTC->TSSR.HR24_HR12     =1;//Set 24hour mode
	//Set time and calendar for loading register, Calendar„³ 09/1/19, Time„³ 13:20:00
	set_CLR(0,9,0,1,1,9);
	set_TLR(1,3,2,0,0,0);

	/* Step 4. Set alarm interrupt */
	//Set time and calendar for alarm register , Calendar„³ 09/1/19, Time„³ 13:30:00
	set_CAR(0,9,0,1,1,9);
	set_TAR(1,3,2,0,2,0);		
	//Enable alarm interrupt
	RTC->RIER.AIER = 1;
	RTC->RIER.TIER = 1;
	NVIC_EnableIRQ(RTC_IRQn);
}

void RTC_IRQHandler(void)
{
	uint32_t clock;
	char adc_value[15]="Clock:";
	/* tick */
	if(inpw(&RTC->RIIR)&0x2)
 	{
		clock=inpw(&RTC->TLR)&0xFFFFFF;
		sprintf(adc_value+6,"%02x",(clock>>16)&0xFF);
		sprintf(adc_value+9,"%02x",((clock)>>8)&0xFF);
		sprintf(adc_value+12,"%02x",clock&0xFF);
		adc_value[8]=':';
		adc_value[11]=':';
		Show_Word(0,13,' ');
		print_lcd(0, adc_value);
		outpw(&RTC->RIIR,2);
	}
	/* alarm */
	if(inpw(&RTC->RIIR)&0x1) 
	{
		print_lcd(1, "Alarm!!!!");
		GPIOC->DOUT&=0xFF;
		Alarm_E=0;
		outpw(&RTC->RIIR,1);
	}
}
//---------------------------------------------------------------------------------TIMER
void InitTIMER0(void)
{
	/* Step 1. Enable and Select Timer clock source */          
	SYSCLK->CLKSEL1.TMR0_S = 0;	//Select 12Mhz for Timer0 clock source 
    SYSCLK->APBCLK.TMR0_EN =1;	//Enable Timer0 clock source

	/* Step 2. Select Operation mode */	
	TIMER0->TCSR.MODE=1;		//Select periodic mode for operation mode

	/* Step 3. Select Time out period = (Period of timer clock input) * (8-bit Prescale + 1) * (24-bit TCMP)*/
	TIMER0->TCSR.PRESCALE=255;	// Set Prescale [0~255]
	TIMER0->TCMPR = 2765;		// Set TCMPR [0~16777215]
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
	char adc_value[12]="Timer:";
	TimerCounter+=1;
	sprintf(adc_value+6,"%d",TimerCounter);
	print_lcd(2, adc_value);
 	TIMER0->TISR.TIF =1;
}

/*----------------------------------------------------------------------------
  MAIN function
  ----------------------------------------------------------------------------*/
int32_t main (void)
{
	UNLOCKREG();
	SYSCLK->PWRCON.XTL32K_EN = 1;//Enable 32Khz for RTC clock source
	SYSCLK->PWRCON.XTL12M_EN = 1;
	SYSCLK->CLKSEL0.HCLK_S = 0;
	LOCKREG();

	Initial_pannel();  //call initial pannel function
	clr_all_pannal();
	                        
	InitTIMER0();
	InitRTC();
	InitWDT();
		 	
	/* Synch field transmission & Request Identifier Field transmission*/
	while(Alarm_E)
	{
		UNLOCKREG();
		WDT->WTCR.WTR =1;//Write 1 to clear for safety
		LOCKREG();
	}
	while(1)
	{
		__NOP();
	}
}



