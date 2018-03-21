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
#define DEBUG 1


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

int main (void)
{
	STR_UART_T sParam;
	//Initial 12M and set HCLK=12MHz
	InitHCLK12M();

	UNLOCKREG();
	/* Enable External 12M Xtal */
	DrvSYS_SetOscCtrl(E_SYS_XTL12M, ENABLE);

	/* Waiting for 12M Xtal stable */
	while (DrvSYS_GetChipClockSourceStatus(E_SYS_XTL12M) != 1);
#ifdef DEBUG

	/* UART Setting */
	sParam.u32BaudRate      = 115200;
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
	//printf("NUC122 USB HID\n");
#endif
	delay_time(5);
	DrvUART_Write(UART_PORT0,"AT+CIPMUX=1\r\n",13);
	delay_time(1);
	DrvUART_Write(UART_PORT0,"AT+CIPSERVER=1,8087\r\n",21);
	/* Execute HID process */

	HID_MainProcess();

}


