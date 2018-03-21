/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright (c) Nuvoton Technology Corp. All rights reserved.                                             */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/

#include <stdio.h>
#include "NUC1xx.h"
#include "Driver\DrvSYS.h"
#include "Driver\DrvGPIO.h"
#include "LCD_Driver.h"
#include "EEPROM_24LC64.h"
#include "Driver\DrvI2C.h"
void delay_loop(void)
 {
 uint32_t i,j;
	for(i=0;i<3;i++)	
	{
		for(j=0;j<60000;j++);
    }
 
 }

void delay(void)
{
int j;
   for(j=0; j<1000; j++);
}

uint8_t scan_key(void)
{
uint8_t act[4]={0x3b, 0x3d, 0x3e};    
uint8_t i,temp,pin;

for(i=0;i<3;i++)
{
       temp=act[i];
	   for(pin=0;pin<6;pin++)
	    {
	  	if((temp&0x01)==0x01)
        DrvGPIO_SetBit(E_GPA,pin);
		else
		DrvGPIO_ClrBit(E_GPA,pin);		  
		temp=temp>>1; 	  
	    }							    
	    delay();
		if(DrvGPIO_GetBit(E_GPA,3)==0) 
		return (i+1);
}

for(i=0;i<3;i++)
{
       temp=act[i];
	   for(pin=0;pin<6;pin++)
	    {
	  	if((temp&0x01)==0x01)
        DrvGPIO_SetBit(E_GPA,pin);
		else
		DrvGPIO_ClrBit(E_GPA,pin);		  
		temp=temp>>1; 	  
	    }							    
	    delay();
		if(DrvGPIO_GetBit(E_GPA,4)==0) 
		return (i+4);
}

for(i=0;i<3;i++)
{
       temp=act[i];
	   for(pin=0;pin<6;pin++)
	    {
	  	if((temp&0x01)==0x01)
        DrvGPIO_SetBit(E_GPA,pin);
		else
		DrvGPIO_ClrBit(E_GPA,pin);		  
		temp=temp>>1; 	  
	    }							    
	    delay();
		if(DrvGPIO_GetBit(E_GPA,5)==0) 
		return (i+7);
}

 return 0;

}



int main(void)
{
	  uint32_t i2cdata=0,i;
	  unsigned char temp;
	  char addr[16]="Address:";
	  char Write[16]="Write:";
	  char read[16]="Read:";
	/* Unlock the protected registers */	
	UNLOCKREG();
   	/* Enable the 12MHz oscillator oscillation */
	DrvSYS_SetOscCtrl(E_SYS_XTL12M, 1);
 
     /* Waiting for 12M Xtal stalble */
    SysTimerDelay(5000);
 
	/* HCLK clock source. 0: external 12MHz; 4:internal 22MHz RC oscillator */
	DrvSYS_SelectHCLKSource(0);		
    /*lock the protected registers */
	LOCKREG();				

	DrvSYS_SetClockDivider(E_SYS_HCLK_DIV, 0); /* HCLK clock frequency = HCLK clock source / (HCLK_N + 1) */

	Initial_pannel();  //call initial pannel function
	clr_all_pannal();
	
	print_lcd(0, "I2C with 24LC65");
	print_lcd(1, "test read and  ");
	print_lcd(2, "write function ");	  
	print_lcd(3, "press key1-key9");
	//initial key board
	for(i=0;i<6;i++)		
	DrvGPIO_Open(E_GPA, i, E_IO_QUASI);


	while(1)
	{
	
	   temp=scan_key();
		if(temp==1)
		{
			print_lcd(0,"Key1 had pressed ");
		    print_lcd(1,"               ");
		    print_lcd(2,"               ");
			print_lcd(3,"               ");
			DrvGPIO_InitFunction(E_FUNC_I2C1);  	  	  
		 	Write_24LC64(0x00000000+temp,temp+11);
			i2cdata= Read_24LC64(0x00000000+temp);
		    sprintf(addr+8,"%x",temp);
			sprintf(Write+6,"%x",temp+11);
			sprintf(read+5,"%x",i2cdata);
			print_lcd(1,addr);
			print_lcd(2,Write);
		    print_lcd(3,read);	
		}
		if(temp==2)
		{
			print_lcd(0,"Key2 had pressed ");
		    print_lcd(1,"               ");
		    print_lcd(2,"               ");
			print_lcd(3,"               ");
			DrvGPIO_InitFunction(E_FUNC_I2C1);  	  	  
		 	Write_24LC64(0x00000000+temp,temp+22);
			i2cdata= Read_24LC64(0x00000000+temp);
		    sprintf(addr+8,"%x",temp);
			sprintf(Write+6,"%x",temp+22);
			sprintf(read+5,"%x",i2cdata);
			print_lcd(1,addr);
			print_lcd(2,Write);
		    print_lcd(3,read);
		}
		if(temp==3)
		{
			print_lcd(0,"Key3 had pressed ");
		    print_lcd(1,"               ");
		    print_lcd(2,"               ");
			DrvGPIO_InitFunction(E_FUNC_I2C1);  	  	  
		 	Write_24LC64(0x00000000+temp,temp+33);
			i2cdata= Read_24LC64(0x00000000+temp);
		    sprintf(addr+8,"%x",temp);
			sprintf(Write+6,"%x",temp+33);
			sprintf(read+5,"%x",i2cdata);
			print_lcd(1,addr);
			print_lcd(2,Write);
		    print_lcd(3,read);
		}
		if(temp==4)
		{
			print_lcd(0,"Key4 had pressed ");
			print_lcd(1,"               ");
		    print_lcd(2,"               ");
			DrvGPIO_InitFunction(E_FUNC_I2C1);  	  	  
		 	Write_24LC64(0x00000000+temp,temp+44);
			i2cdata= Read_24LC64(0x00000000+temp);
		    sprintf(addr+8,"%x",temp);
			sprintf(Write+6,"%x",temp+44);
			sprintf(read+5,"%x",i2cdata);
			print_lcd(1,addr);
			print_lcd(2,Write);
		    print_lcd(3,read);
		}
		if(temp==5)
		{
			print_lcd(0,"Key5 had pressed ");
		    print_lcd(1,"               ");
		    print_lcd(2,"               ");	
			DrvGPIO_InitFunction(E_FUNC_I2C1);  	  	  
		 	Write_24LC64(0x00000000+temp,temp+55);
			i2cdata= Read_24LC64(0x00000000+temp);
		    sprintf(addr+8,"%x",temp);
			sprintf(Write+6,"%x",temp+55);
			sprintf(read+5,"%x",i2cdata);
			print_lcd(1,addr);
			print_lcd(2,Write);
		    print_lcd(3,read);
		}
		if(temp==6)
		{
			print_lcd(0,"Key6 had pressed ");
		    print_lcd(1,"               ");
		    print_lcd(2,"               ");
			DrvGPIO_InitFunction(E_FUNC_I2C1);  	  	  
		 	Write_24LC64(0x00000000+temp,temp+66);
			i2cdata= Read_24LC64(0x00000000+temp);
		    sprintf(addr+8,"%x",temp);
			sprintf(Write+6,"%x",temp+66);
			sprintf(read+5,"%x",i2cdata);
			print_lcd(1,addr);
			print_lcd(2,Write);
		    print_lcd(3,read);
	    }
		if(temp==7)
		{
			print_lcd(0,"Key7 had pressed ");
		    print_lcd(1,"               ");
		    print_lcd(2,"               ");
			DrvGPIO_InitFunction(E_FUNC_I2C1);  	  	  
		 	Write_24LC64(0x00000000+temp,temp+77);
			i2cdata= Read_24LC64(0x00000000+temp);
		    sprintf(addr+8,"%x",temp);
			sprintf(Write+6,"%x",temp+77);
			sprintf(read+5,"%x",i2cdata);
			print_lcd(1,addr);
			print_lcd(2,Write);
		    print_lcd(3,read);
		}
		if(temp==8)
		{
			print_lcd(0,"Key8 had pressed");
		    print_lcd(1,"               ");
		    print_lcd(2,"               ");
			DrvGPIO_InitFunction(E_FUNC_I2C1);  	  	  
		 	Write_24LC64(0x00000000+temp,temp+88);
			i2cdata= Read_24LC64(0x00000000+temp);
		    sprintf(addr+8,"%x",temp);
			sprintf(Write+6,"%x",temp+88);
			sprintf(read+5,"%x",i2cdata);
			print_lcd(1,addr);
			print_lcd(2,Write);
		    print_lcd(3,read);
		}
		if(temp==9)
		{
			print_lcd(0,"Key9 had pressed");
		    print_lcd(1,"               ");
		    print_lcd(2,"               ");
			DrvGPIO_InitFunction(E_FUNC_I2C1);  	  	  
		 	Write_24LC64(0x00000000+temp,temp+99);
			i2cdata= Read_24LC64(0x00000000+temp);
		    sprintf(addr+8,"%x",temp);
			sprintf(Write+6,"%x",temp+99);
			sprintf(read+5,"%x",i2cdata);
			print_lcd(1,addr);
			print_lcd(2,Write);
		    print_lcd(3,read);
	    }
	}


	  		
}


