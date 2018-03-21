#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "NUC1xx.h"
#include "Driver\DrvGPIO.h"
#include "Driver\DrvSYS.h"
#include "Driver\DrvTIMER.h"
#include "Driver\DrvUART.h"
#include "LCD_Driver.h"
#include "Driver\DrvI2C.h"
#include "Driver\DrvADC.h"
#include "Driver\DrvUSB.h"
#include "HIDSys.h"
#include "HID_API.h"
// pin32 GPB0/RX0 to another board's UART TX
// pin33 GPB1/TX0 to another board's UART RX
// E_FUNC_I2C1,SDA1:GPA-10,SCL1:GPA-11
// GPA-10/11 as SDA1/SCL1 pins.							   ADC A7
 //uart0 TX:GPB1 RX:GPB0
#define	STE_DATA_READ	DrvGPIO_set(E_IO_INPUT);
#define	STE_DATA_WRITE	DrvGPIO_set(E_IO_OUTPUT);
#define Rd(address)		((address<<1) | 0x01)
#define Wr(address)		((address<<1) & 0xFE)
#define Da(Data)		(Data & 0xFF)

#define D0	(int)20
#define D1	(int)30


int SMBUS_com = 0,send = 1;
uint8_t common = {0x00};
uint32_t datain[6] = {0xFF};
uint32_t dataout[6] = {0xFF};
uint32_t g_Spi1IntFlag;
uint8_t DAT1;
uint8_t DAT2;
uint8_t DATALL;

void DrvGPIO_set(E_DRVGPIO_IO mode)
{
    volatile uint32_t u32Reg;
    u32Reg = (uint32_t)&GPIOA->PMD + (4*0x40);
    outpw(u32Reg, inpw(u32Reg) & ~(0x3<<(1*2)));
    if (mode == E_IO_OUTPUT)outpw(u32Reg, inpw(u32Reg) | (0x1<<(1*2)));
}

volatile uint8_t comRbuf[16] ={0x30};
volatile uint16_t comRbytes = 0;
volatile uint16_t comRhead 	= 0;
volatile uint16_t comRtail 	= 0;

char TEXT1[16] = "TX: sending...  ";
char TEXT2[16] = "RX:             ";


void ADC_initialize (void)
{
 DrvADC_Open (ADC_SINGLE_END,  //Single家Α
     ADC_SINGLE_CYCLE_OP, //?㏄戳?磞家Α
     0xf3,     //?竚璶???ADC0,1,4,5,6,7 /1111 0011				  f3
     EXTERNAL_12MHZ,     //ㄏノ场12M?????方
     0x9          //ADCClock = 12M/(9+1)
    );
}



static int32_t ADC_average (int32_t *ADC_value)  //staticボ?セゅン?场ノ
{
 int32_t value[9] = {0};
 uint8_t i,j,k;
 int32_t value_max;
 for (i=0; i<9; i++)
 {
  value[i] = *ADC_value++;                    //??璶??
 }
 
 //玙獁猭?逼
 for (j=0; j<9; j++)                    //璶ゑ?9Ω
 {
  for (k=0; k<9-j; k++)      //材Ω?程程蔼竚
  {           //碞ぃノゑ?程ê?じ
   if(k != 8)        //ňゎk?8?value[8]蒓value[9]ゑ???禬??璖?
   {
    if (value[k] > value[k+1])      //┮–Ωゑ?ЧΩぶゑ?Ω
    {
     value_max = value[k];   //??玂癬?
     value[k] = value[k+1];   //???じ
     value[k+1] = value_max;   //???蔼じ
    }
   }
  }
 }
 return (value[4]);
}

static int32_t get_ADC_value(uint8_t ADC_channel_number)
{
 uint8_t i;
    int32_t ADC_calibration[9]={0};                                   //ADC―い??誹???
 for (i=0; i<9; i++)                      //?9Ω
 {
  DrvADC_StartConvert();                    //??ADC??
  while(!DrvADC_IsConversionDone())                 //单ADC??ЧΘ
  {
  }
  ADC_calibration[i] = DrvADC_GetConversionData(ADC_channel_number); //???ADC硄笵
  DrvADC_StopConvert();                                           //氨ゎ??
 }
 return (ADC_average(ADC_calibration));                 //?―い?
}

void InitHCLK12M(void)
{
UNLOCKREG();
//External 4~24 MHz High Speed Crystal Enable (writeprotectionbit)
SYSCLK->PWRCON.XTL12M_EN = 1;
//HCLK clock source select (write-protection bits)
//000 = Clock source from external 12 MHz
SYSCLK->CLKSEL0.HCLK_S = 0;
LOCKREG();
}


void uint16_ascii(uint16_t value,char text[])
{ uint8_t d1,d0;
int8_t ia=5;
for (ia=4; ia>0; ia--)
{
d1=value/10; //value=0-65535
d0=value-d1*10;
text[ia]=d0+'0'; //get [4],[3],[2],[1]
value=d1; //next value
}
text[0]=value+'0'; //get [0]
text[5]=0; //string delimiter, /0
}

void show_string(unsigned char x, unsigned char y, char *str)
{
int i=y;
do{
Show_Word(x,i,*str++); //display a character at (x,i)
i++; // next character
if(i>15) break; // max 16 character
} while(*str!='\0'); //
}




int main (void)
{
	
	char adc_value[15]="ADC Value:";
	int32_t temperature_ADC_value;           
uint16_t ADC_result;
//Initial 12M and set HCLK=12MHz
InitHCLK12M();
// initial LCD pannel function
Initial_pannel(); //(LCD_Driver.c)
// clear LCD pannel
clr_all_pannal(); //(LCD_Driver.c)
//Initial ADC
ADC_initialize();													  
// show title on row 0
print_lcd(0, adc_value);//(LCD_Driver.c					

					   
#ifdef DEBUG
    STR_UART_T sParam;
#endif
	
    UNLOCKREG();

		//outpw(&SYS->GPF_MFP, inpw(&SYS->GPF_MFP) | (0x3<<0));
	
    /* Enable External 12M Xtal */
    DrvSYS_SetOscCtrl(E_SYS_XTL12M, ENABLE);
    
    /* Waiting for 12M Xtal stable */
    while (DrvSYS_GetChipClockSourceStatus(E_SYS_XTL12M) != 1);
   
#ifdef DEBUG
    
    /* UART Setting */
    sParam.u32BaudRate      = 砾9600;
    sParam.u8cDataBits      = DRVUART_DATABITS_8;
    sParam.u8cStopBits      = DRVUART_STOPBITS_1;
    sParam.u8cParity        = DRVUART_PARITY_NONE;
    sParam.u8cRxTriggerLevel= DRVUART_FIFO_1BYTES;

    /* Select UART Clock Source From 12MHz */
    DrvSYS_SelectIPClockSource(E_SYS_UART_CLKSRC, 0); 

    if (DEBUG_PORT == 0)
    {
        /* Set UART0 Configuration */
        DrvUART_Open(UART_PORT0, &sParam);

        /* Set UART0 Pin */
        DrvGPIO_InitFunction(E_FUNC_UART0);
    }
    else if (DEBUG_PORT == 1)
    {
        /* Set UART1 Configuration */
        DrvUART_Open(UART_PORT1, &sParam);

        /* Set UART1 Pin */
        DrvGPIO_InitFunction(E_FUNC_UART1);    
    }

#endif

	/* PLL_Out = 240MHz(0x023A), 192MHz(0x0A6E), 96MHz(0x8A6E), 48MHz(0xC22E) */
	DrvSYS_SetPLLContent(0x023a);  

	/* Set HCLK and USB Divider from Clock Source */
	SYSCLK->CLKDIV.HCLK_N=3; // HCLK=PLL_out/(HCLK_N+1) = 60MHz
	SYSCLK->CLKDIV.USB_N=4;  // USB_CLK=PLL_out/(USB_N+1) = 48MHz  
    
    /* Enable PLL */
    DrvSYS_SetPLLMode(0);
    
    /* Waiting for PLL stable */
    while (DrvSYS_GetChipClockSourceStatus(E_SYS_PLL) != 1);
    
    /* Switch to PLL clock */
    DrvSYS_SelectHCLKSource(2);

	/* Update System Core Clock */
	SystemCoreClockUpdate();

#ifdef DEBUG    
    printf("NUC122 USB HID\n");
#endif
      show_string(0,10,"dsf");
    /* Execute HID process */
  //  HID_MainProcess();
while(1)
{

   Initial_pannel();  //call initial pannel function
	clr_all_pannal();
ADC_result = get_ADC_value(1); 
// convert 16 bits number to ASCII char
//uint16_ascii(ADC_result,adc_value);
sprintf(adc_value,"%d", ADC_result)	;
	print_lcd(2, adc_value);
DrvSYS_Delay(5000); 
}	 
								 						
}


