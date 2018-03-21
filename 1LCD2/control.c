// pin32 GPB0/RX0
// pin33 GPB1/TX0
// GPC,GPD,GPE
#include <stdio.h>
#include "Driver\DrvUART.h"
#include "Driver\DrvGPIO.h"
#include "Driver\DrvSYS.h"
#include "Driver\DrvADC.h"
#include "Driver\DrvI2C.h"
#include "NUC1xx.h"
#include "NUC1xx-LB_002\LCD_Driver.h"
#include "Timer.h"

int LCDAddr = 0x27;
int BLEN = 1;
int fd = 0x27;

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
	/*I2C1->I2CDAT = reg;					//write  reg   to
	DrvI2C_Ctrl(I2C_PORT1, 0, 0, 1, 1); //clr si and set ack	
	while( I2C1->I2CON.SI == 0 );			//poll si flag
	*/
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

void write_word(int data){
	int temp = data;
	if ( BLEN == 1 )
		temp |= 0x08;
	else
		temp &= 0xF7;
	writeByte(fd,LCDAddr, temp);
}

void send_command(int comm){
	int buf;
	// Send bit7-4 firstly
	buf = comm & 0xF0;
	buf |= 0x04;			// RS = 0, RW = 0, EN = 1
	write_word(buf);
	DrvSYS_Delay(2000);	
	buf &= 0xFB;			// Make EN = 0
	write_word(buf);

	// Send bit3-0 secondly
	buf = (comm & 0x0F) << 4;
	buf |= 0x04;			// RS = 0, RW = 0, EN = 1
	write_word(buf);
	DrvSYS_Delay(2000);
	buf &= 0xFB;			// Make EN = 0
	write_word(buf);
}

void send_data(int data){
	int buf;
	// Send bit7-4 firstly
	buf = data & 0xF0;
	buf |= 0x05;			// RS = 1, RW = 0, EN = 1
	write_word(buf);
	DrvSYS_Delay(2000);
	buf &= 0xFB;			// Make EN = 0
	write_word(buf);

	// Send bit3-0 secondly
	buf = (data & 0x0F) << 4;
	buf |= 0x05;			// RS = 1, RW = 0, EN = 1
	write_word(buf);
	DrvSYS_Delay(2000);
	buf &= 0xFB;			// Make EN = 0
	write_word(buf);
}

void init_LCD(){
	send_command(0x33);	// Must initialize to 8-line mode at first
	DrvSYS_Delay(5000);
	send_command(0x32);	// Then initialize to 4-line mode
	DrvSYS_Delay(5000);
	send_command(0x28);	// 2 Lines & 5*7 dots
	DrvSYS_Delay(5000);
	send_command(0x0C);	// Enable display without cursor
	DrvSYS_Delay(5000);
	send_command(0x01);	// Clear Screen
	writeByte(fd,LCDAddr, 0x08);
}

void clear(){
	send_command(0x01);	//clear Screen
}

void write(int x, int y, char data[]){
	int addr, i;
	int tmp;
	if (x < 0)  x = 0;
	if (x > 15) x = 15;
	if (y < 0)  y = 0;
	if (y > 1)  y = 1;

	// Move cursor
	addr = 0x80 + 0x40 * y + x;
	send_command(addr);
	
	tmp = strlen(data);
	for (i = 0; i < tmp; i++){
		send_data(data[i]);
	}
}


int main(){
	
	UNLOCKREG();
  DrvSYS_Open(48000000);
	DrvSYS_SetOscCtrl(E_SYS_XTL12M,1);
	DrvSYS_Delay(20000);/* Delay for Xtal stable */
	while(!SYSCLK->CLKSTATUS.XTL12M_STB);
	DrvSYS_SelectHCLKSource(0);
	LOCKREG();
	
	DrvGPIO_InitFunction(E_FUNC_I2C1);

	//fd = wiringPiI2CSetup(LCDAddr);
	DrvSYS_Delay(200000);
	init_LCD();
	write(0, 0, "Greetings!");
	write(1, 1, "From SunFounder");
	DrvSYS_Delay(200000);
	clear();
	while(1);
}
