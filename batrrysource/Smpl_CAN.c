#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "NUC1xx.h"
#include "Driver\DrvGPIO.h"
#include "Driver\DrvSYS.h"
#include "Driver\DrvTIMER.h"
#include "Driver\DrvUART.h"
#include "LCD_Driver.h"
#include "Driver\DrvSPI.h"
#include "Driver\DrvI2C.h"

// E_FUNC_I2C1,SDA1:GPA-10,SCL1:GPA-11
// GPA-10/11 as SDA1/SCL1 pins.
 //uart0 TX:GPB1 RX:GPB0
#define	STE_DATA_READ	DrvGPIO_set(E_IO_INPUT);
#define	STE_DATA_WRITE	DrvGPIO_set(E_IO_OUTPUT);
#define Rd(address)		((address<<1) | 0x01)
#define Wr(address)		((address<<1) & 0xFE)
#define Da(Data)		(Data & 0xFF)

#define D0	(int)20
#define D1	(int)30

#define	ManufacturerAccess		0x00	
#define	RemainingCapacityAlarm	0x01
#define	RemainingTimeAlarm		0x02
#define	BatteryMode				0x03
#define	AtRate					0x04
#define	AtRateTimeToFull		0x05
#define	AtRateTimeToEmpty		0x06
#define	AtRateOK				0x07
#define	Temperature				0x08
#define	Voltage					0x09
#define	Current					0x0a
#define	AverageCurrent			0x0b
#define	MaxError				0x0c
#define	RelativeStateOfCharge	0x0d
#define	AbsoluteStateOfCharge	0x0e
#define	RemainingCapacity		0x0f
#define	FullChargeCapacity		0x10
#define	RunTimeToEmpty			0x11
#define	AverageTimeToEmpty		0x12
#define	AverageTimeToFull		0x13
#define	BatteryStatus			0x16
#define	CycleCount				0x17
#define	DesignCapacity			0x18
#define	DesignVoltage			0x19
#define	SpecificationInfo		0x1a
#define	ManufactureDate			0x1b
#define	SerialNumber			0x1c
#define	ManufacturerName		0x20
#define	DeviceName				0x21
#define	DeviceChemistry			0x22
#define	ManufacturerData		0x23


void SPI1_Callback(uint32_t u32UserData);

int SMBUS_com = 0,send = 1;
uint8_t common = {0x00};
uint32_t datain[6] = {0xFF};
uint32_t dataout[6] = {0xFF};
uint32_t g_Spi1IntFlag;
uint8_t DAT1;
uint8_t DAT2;
uint8_t DATALL;

typedef enum 
{
	L=0,
	H=1
}LEVEL;

void DelayNs(int n)
{
	int i = 0;
	for(i=0;i<n;i++)__NOP();
}

void DrvGPIO_set(E_DRVGPIO_IO mode)
{
    volatile uint32_t u32Reg;
    u32Reg = (uint32_t)&GPIOA->PMD + (4*0x40);
    outpw(u32Reg, inpw(u32Reg) & ~(0x3<<(1*2)));
    if (mode == E_IO_OUTPUT)outpw(u32Reg, inpw(u32Reg) | (0x1<<(1*2)));
}
  
void  writeByte(uint8_t addr ,uint8_t reg , uint8_t data)
{
	uint8_t L,H;
	uint32_t i;
	int count = 0;
    //Open I2C1 and set clock = 50Kbps
	SystemCoreClock = DrvSYS_GetHCLKFreq(); 
	DrvI2C_Open(I2C_PORT1, 50000);
	//send i2c start
    DrvI2C_Ctrl(I2C_PORT1, 1, 0, 1, 1);	 	//set start
	while (I2C1->I2CON.SI == 0);				//poll si flag
   	//send  addr command
	I2C1->I2CDAT = addr<<1;		    	//write  addr   to
    DrvI2C_Ctrl(I2C_PORT1, 0, 0, 1, 1);	   //clr si
    while( I2C1->I2CON.SI == 0 );			   //poll si flag
	//send reg
	I2C1->I2CDAT = reg;					//write  reg   to
	DrvI2C_Ctrl(I2C_PORT1, 0, 0, 1, 1); //clr si and set ack	
	while( I2C1->I2CON.SI == 0 );			//poll si flag
	//send data
	I2C1->I2CDAT = data;					//write data to 
	DrvI2C_Ctrl(I2C_PORT1, 0, 0, 1, 1); //clr si and set ack	
	while( I2C1->I2CON.SI == 0 );			//poll si flag
	//send i2c stop
 	DrvI2C_Ctrl(I2C_PORT1, 0, 1, 1, 0);    //clr si and set stop
	while( I2C1->I2CON.STO){
		count++;
		if(count>10000){
			break;
		}
	}
	for(i=0;i<60;i++);
	DrvI2C_Close(I2C_PORT1);
}
			  
uint8_t Read_BT70791(uint8_t data)
{
	uint8_t value;
	uint32_t i;
	int count = 0;
    //Open I2C1 and set clock = 50Kbps
	SystemCoreClock = DrvSYS_GetHCLKFreq(); 
	DrvI2C_Open(I2C_PORT1, 50000);
	//send i2c start
    DrvI2C_Ctrl(I2C_PORT1, 1, 0, 1, 1);	 	//set start
	while (I2C1->I2CON.SI == 0);				//poll si flag
   	//send writer command
	I2C1->I2CDAT = 0x68<<1;
    DrvI2C_Ctrl(I2C_PORT1, 0, 0, 1, 1);	   //clr si
    while( I2C1->I2CON.SI == 0 );			   //poll si flag
	//send data
	I2C1->I2CDAT = data;					//write data to 
	DrvI2C_Ctrl(I2C_PORT1, 0, 0, 1, 1); //clr si and set ack	
	while( I2C1->I2CON.SI == 0 );			//poll si flag
	//send i2c start
    DrvI2C_Ctrl(I2C_PORT1, 1, 0, 1, 1);	 	//set start
	while (I2C1->I2CON.SI == 0);				//poll si flag
   	//send writer command
	I2C1->I2CDAT = (0x68<<1)+1;
    DrvI2C_Ctrl(I2C_PORT1, 0, 0, 1, 1);	   //clr si
    while( I2C1->I2CON.SI == 0 );			   //poll si flag
	//resive data
	I2C1->I2CDAT = 0XFF;
	DrvI2C_Ctrl(I2C_PORT1, 0, 0, 1, 1);    //clr si	
	while( I2C1->I2CON.SI == 0 );			   //poll si flag
	value= I2C1->I2CDAT;
	//send i2c stop
 	DrvI2C_Ctrl(I2C_PORT1, 0, 1, 1, 0);    //clr si and set stop
	while( I2C1->I2CON.STO){
		count++;
		if(count>10000){
			break;
		}
	}
	for(i=0;i<60;i++);
	DrvI2C_Close(I2C_PORT1);
	return value; 
}
 volatile uint8_t comRbuf[16] ={0x30};
volatile uint16_t comRbytes = 0;
volatile uint16_t comRhead 	= 0;
volatile uint16_t comRtail 	= 0;

char TEXT1[16] = "TX: sending...  ";
char TEXT2[16] = "RX:             ";

/*---------------------------------------------------------------------------------------------------------*/
/* UART Callback function                                                                           	   */
/*---------------------------------------------------------------------------------------------------------*/
void UART_INT_HANDLE(void)
{
	uint8_t i;
	uint8_t bInChar[1] = {0xFF};

	while(UART0->ISR.RDA_IF==1) 
	{
		DrvUART_Read(UART_PORT0,bInChar,1);	
		if(comRbytes < 8) // check if Buffer is full
		{
			comRbuf[comRbytes] = bInChar[0];
			comRbytes++;
		}
		else if (comRbytes==8)
		{
			comRbytes=0;
			sprintf(TEXT2+4,"%s",comRbuf);
			print_lcd(2,TEXT2);
		}			
	}
}
int main (void)
{
	uint8_t  i =0;
	STR_UART_T sParam;

	uint8_t	read	= 0;
	UNLOCKREG();
    DrvSYS_Open(48000000);
    DrvSYS_SetOscCtrl(E_SYS_XTL12M,1);
    DrvSYS_Delay(20000);/* Delay for Xtal stable */
    while(!SYSCLK->CLKSTATUS.XTL12M_STB);
    DrvSYS_SelectHCLKSource(0);
		LOCKREG();
			Initial_pannel();
	clr_all_pannal();
	print_lcd(0,"Smpl_UART0 456   ");
		DrvGPIO_InitFunction(E_FUNC_I2C1);
	
	
	DrvGPIO_InitFunction(E_FUNC_UART0);		

	/* UART Setting */
    sParam.u32BaudRate 		= 9600;
    sParam.u8cDataBits 		= DRVUART_DATABITS_8;
    sParam.u8cStopBits 		= DRVUART_STOPBITS_1;
    sParam.u8cParity 		= DRVUART_PARITY_NONE;
    sParam.u8cRxTriggerLevel= DRVUART_FIFO_1BYTES;

	/* Set UART Configuration */
 	if(DrvUART_Open(UART_PORT0,&sParam) != E_SUCCESS);  

	DrvUART_EnableInt(UART_PORT0, DRVUART_RDAINT, UART_INT_HANDLE);  
	
	while(1)
	{
			
			Initial_pannel();
	        clr_all_pannal();

		 /*...........§ì¨ú¹q¶q..........*/ 
		 read  = Read_BT70791(0x75);
		DrvSYS_Delay(10000);	

			print_lcd(0,"VoltageValue=");
		 	sprintf(TEXT2,"%d mV",read);
			print_lcd(1,TEXT2);
			DrvSYS_Delay(100000); 	
				
	}
}


