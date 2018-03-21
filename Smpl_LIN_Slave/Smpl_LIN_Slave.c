/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) 2009 Nuvoton Technology Corp. All rights reserved.                                         */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#include <stdio.h>																											 
#include "NUC1xx.h"
#define BAUDRATE 9600


void LIN_HEADER(uint8_t ID);
void LIN_RESPONSE(void);
uint32_t cCheckSum(uint8_t DataBuffer[], uint32_t Offset);
uint8_t LIN_COUNTER, LIN_RX_DATA[11], LIN_TX_DATA[11];
uint8_t LIN_Address;
			
const uint32_t LIN_PARITY[]={0x80, 0xC1, 0x42, 0x03, 0xC4, 0x85, 0x06, 0x47, 0x08, 0x49, 0xCA, 0x8B, 0x4C, 0x0D, 0x8E, 0xCF
							,0x50, 0x11, 0x92, 0xD3, 0x14, 0x55, 0xD6, 0x97, 0xD8, 0x99, 0x1A, 0x5B, 0x9C, 0xDD, 0x5E, 0x1F
							,0x20, 0x61, 0xE2, 0xA3, 0x64, 0x25, 0xA6, 0xE7, 0xAB, 0xE9, 0x6A, 0x2B, 0xEC, 0xAD, 0x2E, 0x6F
							,0xF0, 0xB1, 0x32, 0x73, 0xB4, 0xF5, 0x76, 0x37, 0x78, 0x39, 0xBA, 0xFB, 0x3C, 0x7D, 0xFE, 0xBF};

void Delay(int count);

void InitADC(void)
{
	/* Step 1. GPIO initial */ 
	GPIOA->OFFD|=0x00010000; 	//Disable digital input path
	SYS->GPAMFP.ADC0=1; 		//Set ADC function 
//	abc=0;			
	/* Step 2. Enable and Select ADC clock source, and then enable ADC module */          
	SYSCLK->CLKSEL1.ADC_S = 2;	//Select 22Mhz for ADC
	SYSCLK->CLKDIV.ADC_N = 1;	//ADC clock source = 22Mhz/2 =11Mhz;
	SYSCLK->APBCLK.ADC_EN = 1;	//Enable clock source
	ADC->ADCR.ADEN = 1;			//Enable ADC module

	/* Step 3. Select Operation mode */
	ADC->ADCR.DIFFEN = 0;     	//single end input
	ADC->ADCR.ADMD   = 0;     	//single mode
		
	/* Step 4. Select ADC channel */
	ADC->ADCHER.CHEN = 0x01;
	
	/* Step 5. Enable ADC interrupt */
	ADC->ADSR.ADF =1;     		//clear the A/D interrupt flags for safe 
	ADC->ADCR.ADIE = 1;
//	NVIC_EnableIRQ(ADC_IRQn);
	
	/* Step 6. Enable WDT module */
	ADC->ADCR.ADST=1;
}
//--------------------------------------------
void InitPWM(void)
{
 	/* Step 1. GPIO initial */ 
	SYS->GPAMFP.PWM0_AD13=1;
	SYS->GPAMFP.PWM1_AD14=1;
	SYS->GPAMFP.PWM2_AD15=1;
				
	/* Step 2. Enable and Select PWM clock source*/		
	SYSCLK->APBCLK.PWM01_EN = 1;//Enable PWM clock
	SYSCLK->CLKSEL1.PWM01_S = 3;//Select 22.1184Mhz for PWM clock source

	PWMA->PPR.CP01=1;			//Prescaler 0~255, Setting 0 to stop output clock
	PWMA->CSR.CSR0=0;			//clock divider->0:/2, 1:/4, 2:/8, 3:/16, 4:/1
	PWMA->CSR.CSR1=1;			// PWM clock = clock source/(Prescaler + 1)/divider

	SYSCLK->APBCLK.PWM23_EN = 1;//Enable PWM clock
	SYSCLK->CLKSEL1.PWM23_S = 3;//Select 22.1184Mhz for PWM clock source

	PWMA->PPR.CP23=1;			//Prescaler 0~255, Setting 0 to stop output clock
	PWMA->CSR.CSR2=2;			//clock divider->0:/2, 1:/4, 2:/8, 3:/16, 4:/1
 									         
	/* Step 3. Select PWM Operation mode */
	//PWM0
	PWMA->PCR.CH0MOD=1;			//0:One-shot mode, 1:Auto-load mode
								//CNR and CMR will be auto-cleared after setting CH0MOD form 0 to 1.
	PWMA->CNR0=0xFFFF;
	PWMA->CMR0=0xFFFF;

	PWMA->PCR.CH0INV=0;			//Inverter->0:off, 1:on
	PWMA->PCR.CH0EN=1;			//PWM function->0:Disable, 1:Enable
 	PWMA->POE.PWM0=1;			//Output to pin->0:Diasble, 1:Enable

	//PWM1
	PWMA->PCR.CH1MOD=1;			//0:One-shot mode, 1:Auto-load mode
								//CNR and CMR will be auto-cleared after setting CH0MOD form 0 to 1.
	PWMA->CNR1=0xFFFF;
	PWMA->CMR1=0xFFFF;

	PWMA->PCR.CH1INV=0;			//Inverter->0:off, 1:on
	PWMA->PCR.CH1EN=1;			//PWM function->0:Disable, 1:Enable
 	PWMA->POE.PWM1=1;			//Output to pin->0:Diasble, 1:Enable

	//PWM2
	PWMA->PCR.CH2MOD=1;			//0:One-shot mode, 1:Auto-load mode
								//CNR and CMR will be auto-cleared after setting CH0MOD form 0 to 1.
	PWMA->CNR2=0xFFFF;
	PWMA->CMR2=0x7FFF;

	PWMA->PCR.CH2INV=0;			//Inverter->0:off, 1:on
	PWMA->PCR.CH2EN=1;			//PWM function->0:Disable, 1:Enable
 	PWMA->POE.PWM2=1;			//Output to pin->0:Diasble, 1:Enable

	PWMA->PCR.DZEN01=0;			//Dead-Zone->0:Disable, 1:Enable
	PWMA->PCR.DZEN23=0;			//Dead-Zone->0:Disable, 1:Enable
}
//--------------------------------------------
void InitUART0_LIN(void)
{

	/* Step 1. GPIO initial */ 
	SYS->GPBMFP.UART0_RX 	=1;
	SYS->GPBMFP.UART0_TX 	=1;
				
	/* Step 2. Enable and Select UART clock source*/		
	UNLOCKREG();
	SYSCLK->PWRCON.XTL12M_EN = 1;
	LOCKREG();
		
	SYSCLK->APBCLK.UART0_EN = 1;//Enable UART clock
	SYSCLK->CLKSEL1.UART_S = 0;	//Select 12Mhz for UART clock source											           
	SYSCLK->CLKDIV.UART_N = 0;	//UART clock source = 12Mhz;

	SYS->IPRSTC2.UART0_RST = 1;
	SYS->IPRSTC2.UART0_RST = 0;

	/* Step 3. Select Operation mode */
	UART0->FCR.TFR =1;			//Reset Tx FIFO
	UART0->FCR.RFR =1;			//Reset Rx FIFO

	UART0->FCR.RFITL = 0;//Set Rx Trigger Level -1byte FIFO  
	UART0->LCR.PBE	= 0;//Disable parity
	UART0->LCR.WLS	= 3;//8 data bits
	UART0->LCR.NSB	= 0;//Enable 1 Stop bit
		
	/* Step 4. Set BaudRate */
	UART0->BAUD.DIV_X_EN = 1;
	UART0->BAUD.DIV_X_ONE   = 1;
	UART0->BAUD.BRD = (12000000 / BAUDRATE) -2;


	/* Step 5. Set LIN mode */
	UART0->FUNSEL.FUN_SEL=1;// Enable LIN Bus mode 
    UART0->ALTCON.LIN_BKFL=11;// Choose break field length 
							 //Usually set 13 (length=	LINBCNT+2)
	UART0->ALTCON.LIN_RX_EN=1;//Enable LIN Rx mode

	UART0->IER.LIN_RX_BRK_IEN=1;
	UART0->IER.RDA_IEN=1;
	NVIC_EnableIRQ(UART0_IRQn);//Enable UART interrupt
}


void Delay(int count)
{
	while(count--)
	{
//		__NOP;
	 }
}


void UART0_IRQHandler()
{
	LIN_RESPONSE();
	outpw(&UART0->ISR, inpw(&UART0->ISR));
}


/*----------------------------------------------------------------------------
  MAIN function
  ----------------------------------------------------------------------------*/
int32_t main (void)
{
	//Enable 12Mhz and set HCLK->12Mhz		
	UNLOCKREG();
	SYSCLK->PWRCON.XTL12M_EN = 1;
	SYSCLK->CLKSEL0.HCLK_S = 0;
	LOCKREG();

	//Get Lin address
	LIN_Address= (~((GPIOA->PIN & 0xF0)>>4))&0xF;

	
	GPIOB->DOUT=0xFFFB;
	Delay(200);
	GPIOB->DOUT=0xFFFF;
    
	//Enable LIN function                          
	InitUART0_LIN();
	InitPWM();
	InitADC();
							 	
	/* Synch field transmission & Request Identifier Field transmission*/
					 
	while(1)
	{
		while(ADC->ADSR.ADF==0);
		ADC->ADSR.ADF=1;
		PWMA->CMR2=ADC->ADDR[0].RSLT<<4;
		ADC->ADCR.ADST=1;
	}
}



/*********************************************************************************************
										LIN subrountine													  
 *********************************************************************************************/
void LIN_SLAVE_ACTION(void)
{
	if (LIN_RX_DATA[1]==LIN_PARITY[0x20])
 	{
			PWMA->CMR0=LIN_RX_DATA[2]<<8;
			PWMA->CNR0=LIN_RX_DATA[3]<<8;
			PWMA->CMR1=LIN_RX_DATA[4]<<8;
			PWMA->CNR1=LIN_RX_DATA[5]<<8;
	}
}
//--------------------------------------------------------------
void LIN_SLAVE_TX(void)
{
	if (LIN_RX_DATA[1]==LIN_PARITY[(LIN_Address+0x10)])
	{
			LIN_TX_DATA[0]=0x55;
			LIN_TX_DATA[1]=LIN_RX_DATA[1];
			LIN_TX_DATA[2]=(PWMA->CMR2)>>8;
			LIN_TX_DATA[3]=PWMA->CMR2;
			LIN_TX_DATA[4]=cCheckSum(LIN_TX_DATA, 1);

 			UART0->DATA=LIN_TX_DATA[2];
			UART0->DATA=LIN_TX_DATA[3]; 	
			UART0->DATA=LIN_TX_DATA[4];
	}
}
//--------------------------------------------------------------
uint32_t cCheckSum(uint8_t DataBuffer[], uint32_t Offset)
{
 	uint32_t	i,j,CheckSum;

	j=DataBuffer[Offset]&0x30;
	if(j==0x00)
		j=Offset+3;
	else if (j==0x10)
		j=Offset+3;
	else if (j==0x20)
		j=Offset+5;
	else if (j==0x30)
		j=Offset+9;
	else
		j=0;

	for(i=Offset,CheckSum=0;i<j;i++)
	{
	  CheckSum+=DataBuffer[i];
	  if (CheckSum>=256)
	  	CheckSum-=255;
	}
	return (255-CheckSum);	
}
//--------------------------------------------------------------
void LIN_HEADER(uint8_t ID)
{
    LIN_TX_DATA[0]=0x55;
	LIN_TX_DATA[1]=LIN_PARITY[ID];
	UART0->DATA=LIN_TX_DATA[0];	//Sync	
	UART0->DATA=LIN_TX_DATA[1];	//Parity + length + ID
	UART0->ALTCON.LIN_TX_EN=1;	//Send break + synch + ID
}
//--------------------------------------------------------------

void LIN_RESPONSE(void)
{
	if(UART0->ISR.LIN_RX_BREAK_IF)
	{
		UART0->FCR.RFR =1;			//Reset Rx FIFO
		LIN_COUNTER=0;
		LIN_RX_DATA[0]=0;
	}else if(UART0->ALTCON.LIN_RX_EN&UART0->ISR.RDA_IF&(LIN_COUNTER<12))
	{
		LIN_RX_DATA[LIN_COUNTER]=UART0->DATA;
		LIN_COUNTER++;
		if(LIN_RX_DATA[0]==0x55)
		{
			if(LIN_COUNTER==2)
			{
				LIN_SLAVE_TX();

			}else if(LIN_COUNTER==5&((LIN_RX_DATA[1]&0x30)==0x00))	//2 bytes
			{
				if(cCheckSum(LIN_RX_DATA, 1)==LIN_RX_DATA[4])
					LIN_SLAVE_ACTION();
			}else if(LIN_COUNTER==5&((LIN_RX_DATA[1]&0x30)==0x10))	//2 bytes
			{
				if(cCheckSum(LIN_RX_DATA, 1)==LIN_RX_DATA[4]);
					LIN_SLAVE_ACTION();
			}else if(LIN_COUNTER==7&((LIN_RX_DATA[1]&0x30)==0x20))	//4 bytes
			{
				if(cCheckSum(LIN_RX_DATA, 1)==LIN_RX_DATA[6]);
					LIN_SLAVE_ACTION();
			}else if(LIN_COUNTER==11&((LIN_RX_DATA[1]&0x30)==0x30)) //8 bytes
			{
				if(cCheckSum(LIN_RX_DATA, 1)==LIN_RX_DATA[10])
					LIN_SLAVE_ACTION();
	 		}
		}
	}
}



