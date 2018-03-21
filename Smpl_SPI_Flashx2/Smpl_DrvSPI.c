/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright (c) Nuvoton Technology Corp. All rights reserved.                                             */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/

#include <stdio.h>
#include "NUC1xx.h"
#include "Driver\DrvSYS.h"
#include "Driver\DrvSPI.h"
#include "Driver\DrvUART.h"
#include "Driver\DrvGPIO.h"

#define TEST_NUMBER 1 /* page numbers */
//void SPI0_Callback(uint32_t u32UserData);
//volatile uint32_t SPI0_INT_Flag;

void SpiFlashx2_ReadMidDid(void);
void SpiFlashx2_ChipErase(void);
uint32_t SpiFlash2_ReadStatusReg1(void);
uint32_t SpiFlash2_ReadStatusReg2(void);
void SpiFlashx2_WaitReady(void);
void SpiFlashx2_PageProgram(uint8_t *, uint32_t, uint32_t);
void SpiFlashx2_ReadData(uint8_t *, uint8_t *, uint32_t, uint32_t);

void Initial_pannel(void);
void clr_all_pannal(void);
void print_lcd(unsigned char, char *);

void SysTimerDelay(uint32_t);

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
	     
	uint32_t u32ByteCount, u32FlashAddress, u32PageNumber;
	uint8_t DataBuffer0[256];
	uint8_t DataBuffer1[256];
    
    /* Unlock the protected registers */	
	UNLOCKREG();

	/* Enable the 12MHz oscillator oscillation */
	DrvSYS_SetOscCtrl(E_SYS_XTL12M, 1);

	/* HCLK clock source. 0: external 12MHz; 4:internal 22MHz RC oscillator */
	DrvSYS_SelectHCLKSource(0);

	LOCKREG();

	DrvSYS_SetClockDivider(E_SYS_HCLK_DIV, 0); /* HCLK clock frequency = HCLK clock source / (HCLK_N + 1) */

	Initial_pannel();
	clr_all_pannal();
	
	/* GPIO init for SPI2 */
	DrvGPIO_InitFunction(E_FUNC_SPI2);
	/* GPIO init for SPI2 2-bit mode */
	DrvGPIO_InitFunction(E_FUNC_SPI2_2BIT_MODE);
	/* Configure SPI2 as a master, Type1 waveform, 32-bit transaction */
	DrvSPI_Open(eDRVSPI_PORT2, eDRVSPI_MASTER, eDRVSPI_TYPE1, 32);
	/* MSB first */
	DrvSPI_SetEndian(eDRVSPI_PORT2, eDRVSPI_MSB_FIRST);
	/* Disable the automatic slave select function of SS0. */
	DrvSPI_DisableAutoSS(eDRVSPI_PORT2);
	/* Set the active level of slave select. */
	DrvSPI_SetSlaveSelectActiveLevel(eDRVSPI_PORT2, eDRVSPI_ACTIVE_LOW_FALLING);
	/* Configure SPI2 as 2 bits transfer mode */
	DrvSPI_Set2BitTransferMode(eDRVSPI_PORT2, TRUE);				  
	/* Enable the SPI2 interrupt and install the callback function. */
	//DrvSPI_EnableInt(eDRVSPI_PORT2, SPI2_Callback, 0);
	/* SPI clock rate 1MHz */
	DrvSPI_SetClockFreq(eDRVSPI_PORT2, 1000000, 0);
	
	print_lcd(0,"SPI Sample Code ");
	print_lcd(1,"  for Flash x 2 ");
    print_lcd(3,"Press key 1 ....");
    
	while(1) {
		if (scan_key() == 1) break;
	}	     

    /* read MID & DID */
	SpiFlashx2_ReadMidDid();
	SysTimerDelay(1000000);
	
	/* Erase SPI flash */
	print_lcd(2,"Erase Flash.....");
	print_lcd(3,"                ");
	SpiFlashx2_ChipErase();
	
	/* Wait ready */
	SpiFlashx2_WaitReady();
	
	/* Get the status of SPI flash */
	print_lcd(3,"Erase done !!!  ");
	SysTimerDelay(1000000);
	
	/* source data */
	for(u32ByteCount=0; u32ByteCount<256; u32ByteCount++)
	{
		DataBuffer0[u32ByteCount] = u32ByteCount;
	}
	
	/* Program SPI flash */	
	print_lcd(2,"Program Flash...");
	print_lcd(3,"                ");
	u32FlashAddress = 0;
	for(u32PageNumber=0; u32PageNumber<TEST_NUMBER; u32PageNumber++)
	{
		/* page program */
		SpiFlashx2_PageProgram(DataBuffer0, u32FlashAddress, 256);
		SpiFlashx2_WaitReady();
		u32FlashAddress += 0x100;
	}
	print_lcd(3,"Program done !!!");
	SysTimerDelay(1000000);	
		
	/* clear data buffer */
	for(u32ByteCount=0; u32ByteCount<256; u32ByteCount++)
	{
		DataBuffer0[u32ByteCount] = 0;
		DataBuffer1[u32ByteCount] = 0;
	}
		
	/* read back and compare */
	print_lcd(2,"Verify Flash....");
	print_lcd(3,"                ");
	u32FlashAddress = 0;
	for(u32PageNumber=0; u32PageNumber<TEST_NUMBER; u32PageNumber++)
	{
		SpiFlashx2_ReadData(DataBuffer0, DataBuffer1, u32FlashAddress, 256);
		u32FlashAddress += 0x100;
			
		for(u32ByteCount=0; u32ByteCount<256; u32ByteCount++)
		{
			if(DataBuffer0[u32ByteCount]!=u32ByteCount)
			{
				print_lcd(3,"F0 Verify Error!");
				while(1);
			}
			if((uint8_t) DataBuffer1[u32ByteCount]!=(uint8_t) ~DataBuffer0[u32ByteCount])
			{
				print_lcd(3,"F1 Verify Error!");
				while(1);
			}
		}
			
		/* clear data buffer */
		for(u32ByteCount=0; u32ByteCount<256; u32ByteCount++)
		{
			DataBuffer0[u32ByteCount] = 0;
			DataBuffer1[u32ByteCount] = 0;
		}
	}
	print_lcd(3,"Verify done !!! ");

	DrvSPI_Close(eDRVSPI_PORT2);
	
	return 1;
}

/*
void SPI2_Callback(uint32_t u32UserData)
{
	SPI2_INT_Flag = 1;
}
*/

// **************************************
// For W25Q16BV, Manufacturer ID: 0xEF; Device ID: 0x14
// For W26X16, Manufacturer ID: 0xEF; Device ID: 0x14
void SpiFlashx2_ReadMidDid(void)
{
	uint32_t au32SourceData[2];
    uint32_t au32DestinationData[2]; 	

    // configure transaction length as 8 bits
	DrvSPI_SetBitLength(eDRVSPI_PORT2, 8);

	// /CS: active
	DrvSPI_SetSS(eDRVSPI_PORT2, eDRVSPI_SS0);
	
	// send Command: 0x90, Read Manufacturer/Device ID
	au32SourceData[0] = 0x90;
	au32SourceData[1] = 0x90;
	DrvSPI_BurstWrite(eDRVSPI_PORT2, &au32SourceData[0]); 
    	
	// wait
	while (DrvSPI_IsBusy(eDRVSPI_PORT2)) {}
	
	// configure transaction length as 24 bits
	DrvSPI_SetBitLength(eDRVSPI_PORT2, 24);	

	// send 24-bit '0', dummy
	au32SourceData[0] = 0x0;
	au32SourceData[1] = 0x0;
    DrvSPI_BurstWrite(eDRVSPI_PORT2, &au32SourceData[0]); 	

	// wait
	while (DrvSPI_IsBusy(eDRVSPI_PORT2)) {}
    
	// configure transaction length as 16 bits
	DrvSPI_SetBitLength(eDRVSPI_PORT2, 16);	

	// receive
	au32SourceData[0] = 0x0;
	au32SourceData[1] = 0x0;
	DrvSPI_BurstWrite(eDRVSPI_PORT2, &au32SourceData[0]);
	
	// wait
	while (DrvSPI_IsBusy(eDRVSPI_PORT2)) {}
	
	// /CS: de-active
	DrvSPI_ClrSS(eDRVSPI_PORT2, eDRVSPI_SS0); 	

	DrvSPI_DumpRxRegister(eDRVSPI_PORT2, &au32DestinationData[0], 2);

	if ((au32DestinationData[0] & au32DestinationData[1] & 0xffff) == 0xEF14) 
		print_lcd(3,"MID & DID=0xEF14");
	else 
		print_lcd(3,"MID & DID Error!");
	
}

// **************************************
void SpiFlashx2_ChipErase(void)
{
	
	uint32_t au32SourceData[2];

	// configure transaction length as 8 bits
	DrvSPI_SetBitLength(eDRVSPI_PORT2, 8); 	

	// /CS: active
	DrvSPI_SetSS(eDRVSPI_PORT2, eDRVSPI_SS0); 	

	// send Command: 0x06, Write enable
	au32SourceData[0] = 0x06;
	au32SourceData[1] = 0x06;
	DrvSPI_BurstWrite(eDRVSPI_PORT2, &au32SourceData[0]); 	

	// wait
	while (DrvSPI_IsBusy(eDRVSPI_PORT2)) {}

	// /CS: de-active
	DrvSPI_ClrSS(eDRVSPI_PORT2, eDRVSPI_SS0); 

	// /CS: active
	DrvSPI_SetSS(eDRVSPI_PORT2, eDRVSPI_SS0);	

	// send Command: 0xC7, Chip Erase
	au32SourceData[0] = 0xc7;
	au32SourceData[1] = 0xc7;
	DrvSPI_BurstWrite(eDRVSPI_PORT2, &au32SourceData[0]); 	

	// wait
	while (DrvSPI_IsBusy(eDRVSPI_PORT2)) {}

	// /CS: de-active
	DrvSPI_ClrSS(eDRVSPI_PORT2, eDRVSPI_SS0); 
}

// **************************************
uint32_t SpiFlash2_ReadStatusReg1(void)
{
    uint32_t au32SourceData[2];
    uint32_t au32DestinationData[2]; 	

	// configure transaction length as 16 bits
	DrvSPI_SetBitLength(eDRVSPI_PORT2, 16);
		
	// /CS: active
	DrvSPI_SetSS(eDRVSPI_PORT2, eDRVSPI_SS0);
	
	// send Command: 0x05, Read status register 1
	au32SourceData[0] = 0x0500;
	au32SourceData[1] = 0x0500;
	DrvSPI_BurstWrite(eDRVSPI_PORT2, &au32SourceData[0]); 
	
	// wait
	while (DrvSPI_IsBusy(eDRVSPI_PORT2)) {}
		
	// /CS: de-active
	DrvSPI_ClrSS(eDRVSPI_PORT2, eDRVSPI_SS0);

	// dump Rx register
	DrvSPI_DumpRxRegister(eDRVSPI_PORT2, &au32DestinationData[0], 2);

    return ((au32DestinationData[0] | au32DestinationData[1]) & 0xFF);

}


// **************************************
uint32_t SpiFlash2_ReadStatusReg2(void)
{
	uint32_t au32SourceData[2];
    uint32_t au32DestinationData[2]; 	
	
	// configure transaction length as 16 bits
	DrvSPI_SetBitLength(eDRVSPI_PORT2, 16);
	
	// /CS: active
	DrvSPI_SetSS(eDRVSPI_PORT2, eDRVSPI_SS0);
	
	// send Command: 0x35, Read status register 2
	au32SourceData[0] = 0x3500;
	au32SourceData[1] = 0x3500;
	DrvSPI_BurstWrite(eDRVSPI_PORT2, &au32SourceData[0]); 
	
	// wait
	while (DrvSPI_IsBusy(eDRVSPI_PORT2)) {}
	
	// /CS: de-active
	DrvSPI_ClrSS(eDRVSPI_PORT2, eDRVSPI_SS0);

    // dump Rx register
	DrvSPI_DumpRxRegister(eDRVSPI_PORT2, &au32DestinationData[0], 2);

	return ((au32DestinationData[0] | au32DestinationData[1]) & 0xFF);
}


// **************************************
void SpiFlashx2_WaitReady(void)
{
	uint32_t ReturnValue;
	
	do{
		ReturnValue = SpiFlash2_ReadStatusReg1();
		ReturnValue = ReturnValue & 1;
	}while(ReturnValue!=0); // check the BUSY bit

}

// **************************************
void SpiFlashx2_PageProgram(uint8_t *DataBuffer, uint32_t StartAddress, uint32_t ByteCount)
{
	uint32_t au32SourceData[2];
	uint32_t Counter;

    // configure transaction length as 8 bits
	DrvSPI_SetBitLength(eDRVSPI_PORT2, 8);
	
	// /CS: active
	DrvSPI_SetSS(eDRVSPI_PORT2, eDRVSPI_SS0);
	
	// send Command: 0x06, Write enable
	au32SourceData[0] = 0x06;
	au32SourceData[1] = 0x06;
	DrvSPI_BurstWrite(eDRVSPI_PORT2, &au32SourceData[0]); 
	
	// wait
	while (DrvSPI_IsBusy(eDRVSPI_PORT2)) {}
	
	// /CS: de-active
	DrvSPI_ClrSS(eDRVSPI_PORT2, eDRVSPI_SS0);
	
	// /CS: active
	DrvSPI_SetSS(eDRVSPI_PORT2, eDRVSPI_SS0);
	
	// send Command: 0x02, Page program
	au32SourceData[0] = 0x02;
	au32SourceData[1] = 0x02;
	DrvSPI_BurstWrite(eDRVSPI_PORT2, &au32SourceData[0]); 
	
	// wait
	while (DrvSPI_IsBusy(eDRVSPI_PORT2)) {}
	
	// configure transaction length as 24 bits
	DrvSPI_SetBitLength(eDRVSPI_PORT2, 24);	
	
	// send 24-bit start address
	au32SourceData[0] = StartAddress;
	au32SourceData[1] = StartAddress;
	DrvSPI_BurstWrite(eDRVSPI_PORT2, &au32SourceData[0]);
		
	// wait
	while (DrvSPI_IsBusy(eDRVSPI_PORT2)) {}
	
	// configure transaction length as 8 bits
	DrvSPI_SetBitLength(eDRVSPI_PORT2, 8); 
	
	for(Counter=0; Counter<ByteCount; Counter++)
	{
	    // send	data to program
		au32SourceData[0] = DataBuffer[Counter];
		au32SourceData[1] = ~au32SourceData[0];
	    DrvSPI_BurstWrite(eDRVSPI_PORT2, &au32SourceData[0]);
		
		// wait
		while (DrvSPI_IsBusy(eDRVSPI_PORT2)) {}
	}
	
	// /CS: de-active
	DrvSPI_ClrSS(eDRVSPI_PORT2, eDRVSPI_SS0);
	
}


// **************************************
void SpiFlashx2_ReadData(uint8_t *DataBuffer0, uint8_t *DataBuffer1, uint32_t StartAddress, uint32_t ByteCount)
{
	uint32_t au32SourceData[2];
    uint32_t au32DestinationData[2]; 	
   	uint32_t Counter;

	// configure transaction length as 8 bits
	DrvSPI_SetBitLength(eDRVSPI_PORT2, 8);	

	// /CS: active
	DrvSPI_SetSS(eDRVSPI_PORT2, eDRVSPI_SS0);

	// send Command: 0x03, Read data
	au32SourceData[0] = 0x03;
	au32SourceData[1] = 0x03;
	DrvSPI_BurstWrite(eDRVSPI_PORT2, &au32SourceData[0]); 	

	// wait
	while (DrvSPI_IsBusy(eDRVSPI_PORT2)) {}
	
	// configure transaction length as 24 bits
	DrvSPI_SetBitLength(eDRVSPI_PORT2, 24);	

	// send 24-bit start address
	au32SourceData[0] = StartAddress;
	au32SourceData[1] = StartAddress;
	DrvSPI_BurstWrite(eDRVSPI_PORT2, &au32SourceData[0]); 	

	// wait
	while (DrvSPI_IsBusy(eDRVSPI_PORT2)) {}
	
	// configure transaction length as 8 bits
	DrvSPI_SetBitLength(eDRVSPI_PORT2, 8); 	

	for(Counter=0; Counter<ByteCount; Counter++)
	{
		// receive
		au32SourceData[0] = 0x0;
		au32SourceData[1] = 0x0;
		DrvSPI_BurstWrite(eDRVSPI_PORT2, &au32SourceData[0]);
		
		// wait
		while (DrvSPI_IsBusy(eDRVSPI_PORT2)) {}
		
		// dump Rx register	
		DrvSPI_DumpRxRegister(eDRVSPI_PORT2, &au32DestinationData[0], 2);
		DataBuffer0[Counter] = (uint8_t) au32DestinationData[0];
		DataBuffer1[Counter] = (uint8_t) au32DestinationData[1];
		
	}
	
	// /CS: de-active
	DrvSPI_ClrSS(eDRVSPI_PORT2, eDRVSPI_SS0); 	

}

