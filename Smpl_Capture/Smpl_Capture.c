/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) 2009 Nuvoton Technology Corp. All rights reserved.                                         */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#include <stdio.h>																											 
#include "NUC1xx.h"
#include "LCD_Driver.h"
#define	PWM_CNR	0xFFFF
uint16_t	CaptureCounter=0;
uint32_t	CaptureValue[2];

void InitCapture(void)
{
 	/* Step 1. GPIO initial */ 
	SYS->GPAMFP.PWM0_AD13=1;			
	/* Step 2. Enable and Select PWM clock source*/		
	SYSCLK->APBCLK.PWM01_EN = 1;//Enable PWM clock
	SYSCLK->CLKSEL1.PWM01_S = 0;//Select 12Mhz for PWM clock source

	PWMA->PPR.CP01=11;			//Prescaler 0~255, Setting 0 to stop output clock
	PWMA->CSR.CSR0=4;			//clock divider->0:/2, 1:/4, 2:/8, 3:/16, 4:/1
									         
	/* Step 3. Select PWM Operation mode */
	PWMA->PCR.CH0MOD=1;			//0:One-shot mode, 1:Auto-load mode
								//CNR and CMR will be auto-cleared after setting CH0MOD form 0 to 1.
	PWMA->CNR0=PWM_CNR;			//Set Reload register
	PWMA->CAPENR=1;				//Enable Capture function pin
	PWMA->CCR0.CAPCH0EN=1;		//Enable Capture function

	/* Step 4. Set PWM Interrupt */
	PWMA->CCR0.CRL_IE0=1;		//Enable Capture rising edge interrupt
	PWMA->CCR0.CFL_IE0=1;		//Enable Capture falling edge interrupt
	PWMA->PIER.PWMIE0=1;		//Enable PWM interrupt for down-counter equal zero.
 	NVIC_EnableIRQ(PWMA_IRQn);  //enable PWM inturrupt

	/* Step 5. Enable PWM down counter*/
	PWMA->PCR.CH0EN=1;			//Enable PWM down counter
}
//--------------------------------------------
void InitPWM1(void)
{
 	/* Step 1. GPIO initial */ 
	SYS->GPAMFP.PWM1_AD14=1;
				
	/* Step 2. Enable and Select PWM clock source*/		
	SYSCLK->APBCLK.PWM01_EN = 1;//Enable PWM clock
	SYSCLK->CLKSEL1.PWM01_S = 0;//Select 12Mhz for PWM clock source

	PWMA->PPR.CP01=11;			//Prescaler 0~255, Setting 0 to stop output clock
	PWMA->CSR.CSR1=3;			// PWM clock = clock source/(Prescaler + 1)/divider
								//clock divider->0:/2, 1:/4, 2:/8, 3:/16, 4:/1
				         
	/* Step 3. Select PWM Operation mode */
	//PWM0
	PWMA->PCR.CH1MOD=1;			//0:One-shot mode, 1:Auto-load mode
								//CNR and CMR will be auto-cleared after setting CH0MOD form 0 to 1.
	PWMA->CNR1=0xFFFF;
	PWMA->CMR1=0x3FFF;

	PWMA->PCR.CH1INV=0;			//Inverter->0:off, 1:on
	PWMA->PCR.CH1EN=1;			//PWM function->0:Disable, 1:Enable
 	PWMA->POE.PWM1=1;			//Output to pin->0:Diasble, 1:Enable
}

void InitPWM2(void)
{
 	/* Step 1. GPIO initial */ 
	SYS->GPAMFP.PWM2_AD15=1;
				
	/* Step 2. Enable and Select PWM clock source*/		
	SYSCLK->APBCLK.PWM23_EN = 1;//Enable PWM clock
	SYSCLK->CLKSEL1.PWM23_S = 0;//Select 12Mhz for PWM clock source

	PWMA->PPR.CP23=1;			//Prescaler 0~255, Setting 0 to stop output clock
	PWMA->CSR.CSR2=4;			// PWM clock = clock source/(Prescaler + 1)/divider
				         
	/* Step 3. Select PWM Operation mode */
	//PWM0
	PWMA->PCR.CH2MOD=1;			//0:One-shot mode, 1:Auto-load mode
								//CNR and CMR will be auto-cleared after setting CH0MOD form 0 to 1.
	PWMA->CNR2=0x1FFF;
	PWMA->CMR2=0x0FFF;

	PWMA->PCR.CH2INV=0;			//Inverter->0:off, 1:on
	PWMA->PCR.CH2EN=1;			//PWM function->0:Disable, 1:Enable
 	PWMA->POE.PWM2=1;			//Output to pin->0:Diasble, 1:Enable
}



void PWMA_IRQHandler(void) // PWM interrupt subroutine 
{
	if(PWMA->PIIR.PWMIF0)
	{
		CaptureCounter++;//Delay (PWM_CNR+1) usec
		if(CaptureCounter==0)
		{
			//Overflow
		}
		PWMA->PIIR.PWMIF0=1;
	}
	if(PWMA->CCR0.CAPIF0)
	{
		if(PWMA->CCR0.CFLRI0)//Calculate High Level
		{
			CaptureValue[0]=CaptureCounter*(PWM_CNR+1)+(PWM_CNR-PWMA->CFLR0);//usec
			CaptureCounter=0;
			PWMA->CCR0.CFLRI0=0;
		}
		if(PWMA->CCR0.CRLRI0)//Calculate Low Level
		{
			CaptureValue[1]=CaptureCounter*(PWM_CNR+1)+(PWM_CNR-PWMA->CRLR0);//usec
			CaptureCounter=0;
			PWMA->CCR0.CRLRI0=0;	
		}
		PWMA->CCR0.CAPIF0=1;
	}
}



void Delay(int count)
{
	while(count--)
	{
		//__NOP;
	 }
}



/*----------------------------------------------------------------------------
  MAIN function
  ----------------------------------------------------------------------------*/
int32_t main (void)
{
	char adc_value[18];
	UNLOCKREG();
	SYSCLK->PWRCON.XTL12M_EN = 1;
	SYSCLK->CLKSEL0.HCLK_S = 0;
	Delay(200);
	SYSCLK->CLKSEL0.HCLK_S = 4;
	LOCKREG();

	Initial_pannel();  //call initial pannel function
	clr_all_pannal();

	InitPWM1();
	InitPWM2();
	InitCapture();
	print_lcd(0, "Capture demo");
	print_lcd(1, ">>GPA12");
	 
	while(1)
	{

		sprintf(adc_value,"High:           ");
		print_lcd(2, adc_value);
		if(CaptureValue[0]>=1000000)
		{
		   	sprintf(adc_value+5,"%dsec",CaptureValue[0]/1000000);
		}else if(CaptureValue[0]>=1000)
		{
		   	sprintf(adc_value+5,"%dmsec",CaptureValue[0]/1000);
		}else			  
			sprintf(adc_value+5,"%dusec",CaptureValue[0]);
		print_lcd(2, adc_value);


		sprintf(adc_value,"Low:           ");
		print_lcd(3, adc_value);		
		if(CaptureValue[1]>=1000000)
		{
		   	sprintf(adc_value+4,"%dsec",CaptureValue[1]/1000000);
		}else if(CaptureValue[1]>=1000)
		{
		   	sprintf(adc_value+5,"%dmsec",CaptureValue[1]/1000);
		}else			  
			sprintf(adc_value+5,"%dusec",CaptureValue[1]);		
		print_lcd(3, adc_value);

	}				 

}


