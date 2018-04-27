// pin32 GPB0/RX0
// pin33 GPB1/TX0
// pin19 GPB4/RX1
// GPA12/PWM
// GPA6 /wifi_reset
// GPD
#include <stdio.h>
#include "Driver\DrvUART.h"
#include "Driver\DrvGPIO.h"
#include "Driver\DrvSYS.h"
#include "Driver\DrvADC.h"
#include "NUC1xx.h"
#include "NUC1xx-LB_002\LCD_Driver.h"
#include "LCD_Driver.h"
#include "Timer.h"

uint8_t command[8] = {0x00};
int8_t item_quantity[48] = {0};
void Init_GPIO();
void delay_s(int time);
int s=1050;  
void InitPWM(void)
{
	 /* Step 1. GPIO initial */ 
	SYS->GPAMFP.PWM0_AD13=1;
	SYS->GPAMFP.PWM1_AD14=1;
	SYS->GPAMFP.PWM2_AD15=1;
				
	/* Step 2. Enable and Select PWM clock source*/	
	SYSCLK->APBCLK.PWM01_EN = 1;//Enable PWM clock
	SYSCLK->APBCLK.PWM23_EN = 1;//Enable PWM clock
	SYSCLK->APBCLK.PWM45_EN = 1;//Enable PWM clock
	SYSCLK->CLKSEL1.PWM01_S = 0;//Select 22.1184Mhz for PWM clock source
	SYSCLK->CLKSEL1.PWM23_S = 0;
	PWMA->PPR.CP01=5;//Prescaler 0~255, Setting 0 to stop output clock
	PWMA->PPR.CP23=5;
	PWMA->CSR.CSR0=4;
	PWMA->CSR.CSR1=4;
	PWMA->CSR.CSR2=4;
	//PWM clock = clock source/(Prescaler + 1)/divider
	//clock divider->0:/2, 1:/4, 2:/8, 3:/16, 4:/1
	//Setup clock source divider select register (CSR)
	//PWM frequency = PWMxy_CLK/[(prescale+1)*(clock divider)*(CNR+1)];
	//12MHz/[(5+1)*1*(40000-1+1)]=50Hz=> 20msec
		
	/* Step 3. Select PWM Operation mode */
	//PWM0
	PWMA->PCR.CH0MOD=1;
	PWMA->PCR.CH1MOD=1;
	PWMA->PCR.CH2MOD=1;

	//0:One-shot mode, 1:Auto-load mode
	//CNR and CMR will be auto-cleared after setting CH0MOD form 0 to 1.
	PWMA->CNR0=40000-1;
	PWMA->CNR1=40000-1;
	PWMA->CNR2=40000-1;
	PWMA->CMR0=2850;
	PWMA->CMR1=2850;
	PWMA->CMR2=2850;
	//Duty Cycle = (CMR+1)/(CNR+1)=2001/40000=5%
	//20ms*5%=1ms

	PWMA->PCR.CH0INV=0;	//Inverter->0:off, 1:on
	PWMA->PCR.CH1INV=0;
	PWMA->PCR.CH2INV=0;
	PWMA->PCR.CH0EN=1;	//PWM function->0:Disable, 1:Enable
	PWMA->PCR.CH1EN=1;
	PWMA->PCR.CH2EN=1;
	PWMA->POE.PWM0=1;	//Output to pin->0:Diasble, 1:Enable
	PWMA->POE.PWM1=1;
	PWMA->POE.PWM2=1;
}

/* WIFI UART Callback function   */
void WIFI_UART_INT_HANDLE(void)
{
	uint8_t i,j,x=1;
	uint8_t bInChar[1] = {0xFF};
	uint8_t bInChar2[2] = {0xFF};
	char TEXT3[16] = "                ";

	while(UART0->ISR.RDA_IF==1)
	{
		DrvUART_Read(UART_PORT0,bInChar,1);
		if(bInChar[0]==0xFF)
		{
			DrvUART_Read(UART_PORT0,bInChar2,2);
			sprintf(TEXT3,"%c%d\n",0xff,bInChar2[0]*256+bInChar2[1]);
			DrvUART_Write(UART_PORT0,TEXT3,strlen(TEXT3));  //43690
		}
	}
}
/*16進制轉10進制*/
void hex_to_Decimal(uint8_t arr[6])
{
	int RFID_ID=0;
	int c=1;
	char TEXT3[16] = "                ";

	for(int i=5;i>=0;i--)
	{
		switch(arr[i])
		{
			case 0x31:
				RFID_ID+=1*c;
			break;
			case 0x32:
				RFID_ID+=2*c;
			break;
			case 0x33:
				RFID_ID+=3*c;
			break;
			case 0x34:
				RFID_ID+=4*c;
			break;
			case 0x35:
				RFID_ID+=5*c;
			break;
			case 0x36:
				RFID_ID+=6*c;
			break;
			case 0x37:
				RFID_ID+=7*c;
			break;
			case 0x38:
				RFID_ID+=8*c;
			break;
			case 0x39:
				RFID_ID+=9*c;
			break;
			case 0x41:
				RFID_ID+=10*c;
			break;
			case 0x42:
				RFID_ID+=11*c;
			break;
			case 0x43:
				RFID_ID+=12*c;
			break;
			case 0x44:
				RFID_ID+=13*c;
			break;
			case 0x45:
				RFID_ID+=14*c;
			break;
			case 0x46:
				RFID_ID+=15*c;
			break;
		}
		c*=16;
	}
	sprintf(TEXT3,"%d",RFID_ID);
	DrvUART_Write(UART_PORT0,"AT+CIPSEND=7\r\n",14);
	DrvSYS_Delay(5000);
	DrvUART_Write(UART_PORT0,TEXT3,7);
}

/* RFID UART Callback function   */
void RFID_UART_INT_HANDLE(void)
{
	uint8_t i,j,x=1;
	uint8_t bInChar[1] = {0xFF};
	uint8_t bInChar2[10] = {0xFF};
	while(UART1->ISR.RDA_IF==1)
	{
		DrvUART_Read(UART_PORT1,bInChar,1);
		if(bInChar[0]==0x02)
		{
			DrvUART_Read(UART_PORT1,bInChar2,4);
			DrvUART_Read(UART_PORT1,bInChar2,6);
			hex_to_Decimal(bInChar2);
		}
	}
}

/*Init motor's GPIO   */
void Init_GPIO()
{
	//WIFI
	DrvGPIO_Open(E_GPB, 2, E_IO_OUTPUT); //wifi_reset
	DrvGPIO_SetBit(E_GPB, 2);           //永遠high
	
	//設定馬達PWM初始無電壓
	DrvGPIO_ClrBit(E_GPA, 13);   //right
	DrvGPIO_ClrBit(E_GPA, 14);   //left
}

/* delay 1 secend  */
void delay_s(int time)
{
	int i;
	for(i=0;i<time*3;i++)
	{
		DrvSYS_Delay(330000);
	}
}

int32_t main()
{
	int i =10;
	int g=0,f=0;
	int Distance_ADC,battery;
	uint8_t bInChar[2];
	char adc_value[15]="";
	STR_UART_T sParam,sParam2;
	
	UNLOCKREG();
    DrvSYS_Open(48000000);
		DrvSYS_SetOscCtrl(E_SYS_XTL12M,1);
		DrvSYS_Delay(20000);/* Delay for Xtal stable */
		while(!SYSCLK->CLKSTATUS.XTL12M_STB);
		DrvSYS_SelectHCLKSource(0);
	LOCKREG();
	
	DrvGPIO_ClrBit(E_GPB, 2); //WIFI初始化
	DrvSYS_Delay(10000);
	Init_GPIO();
	//delay_s(5);
	DrvGPIO_InitFunction(E_FUNC_UART0);
	DrvGPIO_InitFunction(E_FUNC_UART1);

	/* UART Setting */
	sParam.u32BaudRate 		= 9600;
	sParam.u8cDataBits 		= DRVUART_DATABITS_8;
	sParam.u8cStopBits 		= DRVUART_STOPBITS_1;
	sParam.u8cParity 		= DRVUART_PARITY_NONE;
	sParam.u8cRxTriggerLevel= DRVUART_FIFO_1BYTES;
	/* UART2 Setting */
	sParam2.u32BaudRate 		= 9600;
	sParam2.u8cDataBits 		= DRVUART_DATABITS_8;
	sParam2.u8cStopBits 		= DRVUART_STOPBITS_1;
	sParam2.u8cParity 		= DRVUART_PARITY_NONE;
	sParam2.u8cRxTriggerLevel= DRVUART_FIFO_1BYTES;
	
	/* Set UART Configuration */
 	if(DrvUART_Open(UART_PORT0,&sParam) != E_SUCCESS);
	if(DrvUART_Open(UART_PORT1,&sParam2) != E_SUCCESS);

	//DrvUART_Write(UART_PORT0,"AT+CIPSTART=\"TCP\",\"192.168.16.254\",8080\r\n",42);
	//delay_s(1);
	
	/* UART interrupt */
	DrvUART_EnableInt(UART_PORT0, DRVUART_RDAINT, WIFI_UART_INT_HANDLE);
	DrvUART_EnableInt(UART_PORT1, DRVUART_RDAINT, RFID_UART_INT_HANDLE);

	InitPWM();
     
	int x=0;	
	while(1)
	{
		if(s>4000)
			x=1;
		else if(s<1000) 
			x=0;
		if(x==0)
			s=s+10;
		else
			s=s-10;
		//1250
		PWMA->CMR0=s;//1050
		DrvGPIO_ClrBit(E_GPA, 8);
		//delay_s(2);
		DrvSYS_Delay(10000);
		PWMA->CMR0=s;//1450;
		DrvGPIO_SetBit(E_GPA, 8);
		//delay_s(2);
		DrvSYS_Delay(10000);
	}
}