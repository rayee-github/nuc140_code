// C11-pin58 C12-pin57
// Smpl_GPIO_LED4 : GPC12 ~ 15 to control on-board LEDs
//                  low-active otuput to control Red LEDs
//c12 pin57   c13 pin56        left hand  button c8   c0 c1 c2 
#include <stdio.h>
#include "NUC1xx.h"
#include "Driver\DrvGPIO.h"
#include "Driver\DrvUART.h"
#include "Driver\DrvSYS.h"

// Initial GPIO pins (GPC 12,13,14,15) to Output mode  
void Init_LED()
{
	// initialize GPIO pins
	DrvGPIO_Open(E_GPA, 8, E_IO_OUTPUT); 
	DrvGPIO_Open(E_GPA, 9, E_IO_OUTPUT); 
	DrvGPIO_SetBit(E_GPA, 8); // GPC13 pin output Hi to turn off LED  1   P.11
	DrvGPIO_ClrBit(E_GPA, 9); // GPC12 pin output Hi to turn off LED  0   P.12
	DrvGPIO_Open(E_GPA, 10, E_IO_OUTPUT); 
	DrvGPIO_Open(E_GPA, 11, E_IO_OUTPUT); 
	DrvGPIO_SetBit(E_GPA, 10); // GPC13 pin output Hi to turn off LED  1   P.10
	DrvGPIO_ClrBit(E_GPA, 11); // GPC12 pin output Hi to turn off LED  0   P.9
	DrvGPIO_Open(E_GPE, 13, E_IO_OUTPUT); 
	DrvGPIO_Open(E_GPE, 14, E_IO_OUTPUT); 
	DrvGPIO_Open(E_GPE, 15, E_IO_OUTPUT); 
	DrvGPIO_SetBit(E_GPE, 13); // GPC13 pin output Hi to turn off LED  1   P.3
	DrvGPIO_ClrBit(E_GPE, 14); // GPC13 pin output Hi to turn off LED  0   P.2
	DrvGPIO_SetBit(E_GPE, 15); // GPC12 pin output Hi to turn off LED  1   P.1
	
	
	DrvGPIO_Open(E_GPC, 8, E_IO_INPUT); // GPC12 pin set to output mode
	DrvGPIO_Open(E_GPC, 0, E_IO_OUTPUT); // GPC13 pin set to output mode
	DrvGPIO_Open(E_GPC, 1, E_IO_OUTPUT); // GPC14 pin set to input mode
	DrvGPIO_Open(E_GPC, 2, E_IO_OUTPUT); // GPC14 pin set to input mode
	DrvGPIO_Open(E_GPC, 12, E_IO_INPUT); // GPC14 pin set to input mode
	// set GPIO pins to output Low
	DrvGPIO_SetBit(E_GPC, 0); // GPC12 pin output Hi to turn off LED
	DrvGPIO_SetBit(E_GPC, 1); // GPC13 pin output Hi to turn off LED
	//DrvGPIO_ClrBit(E_GPC, 8); // GPC12 pin output Hi to turn off LED
	DrvGPIO_ClrBit(E_GPC, 2); // GPC13 pin output Hi to turn off LED
}

int main (void)
{
	UNLOCKREG();			    // unlock register for programming
  DrvSYS_Open(48000000);// set System Clock to run at 48MHz 
	                      // 12MHz crystal input, PLL output 48MHz
	LOCKREG();				    // lock register from programming

  Init_LED();  // Initialize LEDs (four on-board LEDs below LCD panel)
	while(1){
	if(DrvGPIO_GetBit(E_GPC, 8)==1)
	{
		DrvGPIO_ClrBit(E_GPC, 0); // GPC13 pin output Hi to turn off LED
	}
	else if(DrvGPIO_GetBit(E_GPC, 8)==0)
	{
		DrvGPIO_SetBit(E_GPC, 0); // GPC13 pin output Hi to turn off LED
	}
}
	/*while(1)
	{
		if(DrvGPIO_GetBit(E_GPC, 14)==1)
			DrvGPIO_SetBit(E_GPC, 12);
		else if(DrvGPIO_GetBit(E_GPC, 14)==0)
			DrvGPIO_ClrBit(E_GPC, 12);
		DrvSYS_Delay(10000);	
	}
 DrvGPIO_ClrBit(E_GPC, 13);
	DrvGPIO_SetBit(E_GPC, 12);*/
	/*while (1)	   // forever loop to keep flashing four LEDs one at a time
	{
		DrvGPIO_ClrBit(E_GPC, 12); // output Low to turn on LED
	  DrvSYS_Delay(300000);	     // delay 
	  DrvGPIO_SetBit(E_GPC, 12); // output Hi to turn off LED
	  DrvSYS_Delay(300000);	     // delay
	  DrvGPIO_ClrBit(E_GPC, 13); // output Low to turn on LED
	  DrvSYS_Delay(300000);	     // delay 
	  DrvGPIO_SetBit(E_GPC, 13); // output Hi to turn off LED
	  DrvSYS_Delay(300000);	     // delay
	  DrvGPIO_ClrBit(E_GPC, 14); // output Low to turn on LED
	  DrvSYS_Delay(300000);	     // delay 
	  DrvGPIO_SetBit(E_GPC, 14); // output Hi to turn off LED
	  DrvSYS_Delay(300000);	     // delay
	  DrvGPIO_ClrBit(E_GPC, 15); // output Low to turn on LED
	  DrvSYS_Delay(300000);	     // delay 
	  DrvGPIO_SetBit(E_GPC, 15); // output Hi to turn off LED
	  DrvSYS_Delay(300000);	     // delay
	}*/
}
