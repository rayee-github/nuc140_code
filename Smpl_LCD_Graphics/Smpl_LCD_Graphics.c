//
// Smpl_LCD_Graphics
// render 2D graphics on LCD panel                                          
// Rectangle, Circle, Line, Triangle
//                                                 
#include <stdio.h>	
#include <string.h>																										 
#include "NUC1xx.h"
#include "LCD_Driver.h"
#include "2D_Graphic_Driver.h"

unsigned char DisplayBuf [8*128];

void Delay(int count);

//---------------------------------------------------------------------------------TIMER
void InitTIMER0(void)
{
	/* Step 1. Enable and Select Timer clock source */          
	SYSCLK->CLKSEL1.TMR0_S = 1;	//Select 32.768KHz for Timer0 clock source 
    SYSCLK->APBCLK.TMR0_EN =1;	//Enable Timer0 clock source

	/* Step 2. Select Operation mode */	
	TIMER0->TCSR.MODE=1;		//Select periodic mode for operation mode

	/* Step 3. Select Time out period = (Period of timer clock input) * (8-bit Prescale + 1) * (24-bit TCMP)*/
	TIMER0->TCSR.PRESCALE=255;	// Set Prescale [0~255]
	TIMER0->TCMPR = 4;		// Set TCMPR [0~16777215]
								// (1/32768)*(255+1)*(4)= 32Hz

	/* Step 4. Enable interrupt */
	TIMER0->TCSR.IE = 1;
	TIMER0->TISR.TIF = 1;		//Write 1 to clear for safty		
	NVIC_EnableIRQ(TMR0_IRQn);	//Enable Timer0 Interrupt

	/* Step 5. Enable Timer module */
	TIMER0->TCSR.CRST = 1;		//Reset up counter
//	TIMER0->TCSR.CEN = 1;		//Enable Timer0
	TIMER0->TCSR.CEN = 0;		//Disable Timer0
//  	TIMER0->TCSR.TDR_EN=1;		// Enable TDR function
}

void TMR0_IRQHandler(void) // Timer0 interrupt subroutine 
{
    draw_LCD(DisplayBuf);
 	TIMER0->TISR.TIF =1; 	   
}

void DelayTime(uint32_t count)
{
	while(count>0) count--;
}

// Draw every pixel of LCD (64x128)
void draw_LCD(unsigned char *buffer)
{
	int X_max = 64;
	int Y_max =128;
	int x=0;
	int y=0;
	for (y=0; y< Y_max; y++) 
	{
    		for (x=0; x< (X_max/8); x++) 
		{
			SetPACA(x,(129-y)); 	// set pixel location
			WriteData(buffer[y*8+x]);    //  draw (write buffer data into LCD)
		}
	}
}

/*----------------------------------------------------------------------------
  MAIN function
  ----------------------------------------------------------------------------*/
int32_t main (void)
{
	int color = 1;
    int x =0;
	int y =0;
	for (y=0;y<128; y++) {
		for (x=0;x<64;x++) {
			draw_pixel(x,y,0);
			}
		}
	UNLOCKREG();
	SYSCLK->PWRCON.XTL32K_EN = 1;//Enable 32Khz for RTC clock source
	SYSCLK->PWRCON.XTL12M_EN = 1;
	SYSCLK->CLKSEL0.HCLK_S = 0;
	LOCKREG();

	Initial_pannel();
	clr_all_pannal();
	                        
	InitTIMER0();

	color = 1;

	// draw rectangle 
	RectangleDraw(0,0,63,127, color);

	// draw box
	RectangleFill(10,10,20,20, color);

	// draw circle
	CircleBresenham(30,22,20,color);

	CircleMidpoint(30,63,20,color);

	CircleOptimized(30,105,20,color);

	LineBresenham(0,20,50,60,color);

	LineOptimized(20,30,40,100,color);

	// draw triangle
	Triangle(30,90,60,30,50,60,color);

	// render to LCD
	draw_LCD(DisplayBuf);
}



