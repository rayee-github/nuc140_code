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
#include "Driver\DrvPDMA.h"

#define TEST_NUMBER 1   /* page numbers */
#define	TEST_LENGTH	256	/* length */
uint8_t SrcArray[TEST_LENGTH];
uint8_t DestArray[TEST_LENGTH];

volatile uint32_t PDMA0_INT_Flag, PDMA1_INT_Flag;

void PDMA0_Callback(void);
void PDMA1_Callback(void);

void SpiFlash_ReadMidDid(void);
void SpiFlash_ChipErase(void);
uint32_t SpiFlash_ReadStatusReg1(void);
uint32_t SpiFlash_ReadStatusReg2(void);
void SpiFlash_WaitReady(void);
void SpiFlash_PageProgram(uint32_t, uint32_t);
void SpiFlash_ReadData(uint32_t, uint32_t);

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

/* Main */
int main(void)
{
	
	STR_PDMA_T sPDMA;  
    uint32_t  SPIPort;
    uint32_t u32ByteCount, u32FlashAddress, u32PageNumber;
	    
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

	/* ---------------------------------------------- */
	/* Configure SPI2 as a master to access SPI-Flash */
    /* ---------------------------------------------- */
	/* GPIO Init for SPI2 */
	DrvGPIO_InitFunction(E_FUNC_SPI2);
	/* Configure SPI2 as a master, Type1 waveform, 32-bit transaction */
	DrvSPI_Open(eDRVSPI_PORT2, eDRVSPI_MASTER, eDRVSPI_TYPE1, 32);
	/* MSB first */
	DrvSPI_SetEndian(eDRVSPI_PORT2, eDRVSPI_MSB_FIRST);
	/* Disable the automatic slave select function of SS0. */
	DrvSPI_DisableAutoSS(eDRVSPI_PORT2);
	/* Set the active level of slave select. */
	DrvSPI_SetSlaveSelectActiveLevel(eDRVSPI_PORT2, eDRVSPI_ACTIVE_LOW_FALLING);
	/* Enable the SPI0 interrupt and install the callback function. */
	//DrvSPI_EnableInt(eDRVSPI_PORT2, SPI2_Callback, 0);
	/* SPI clock rate 1MHz */
	DrvSPI_SetClockFreq(eDRVSPI_PORT2, 1000000, 0);
								  
	/* -------------------------------------------- */
    /* Configure PDMA Channel 0 to receive SPI2 Rx0 */
	/* -------------------------------------------- */
    /* PDMA Init */
    DrvPDMA_Init();
	/* SPI Port = SPI2 Rx0 */
	SPIPort = SPI2_BASE + 0x10;
	/* PDMA Setting */
	DrvPDMA_SetCHForAPBDevice(eDRVPDMA_CHANNEL_0, eDRVPDMA_SPI2, eDRVPDMA_READ_APB);
	/* CH0 RX Setting */
	sPDMA.sSrcCtrl.u32Addr 			= SPIPort;
    sPDMA.sDestCtrl.u32Addr 		= (uint32_t)DestArray;   
	sPDMA.u8TransWidth 				= eDRVPDMA_WIDTH_8BITS;
	sPDMA.u8Mode 					= eDRVPDMA_MODE_APB2MEM;
	sPDMA.sSrcCtrl.eAddrDirection 	= eDRVPDMA_DIRECTION_FIXED; 
	sPDMA.sDestCtrl.eAddrDirection 	= eDRVPDMA_DIRECTION_INCREMENTED; 
	sPDMA.i32ByteCnt                = TEST_LENGTH;  
	DrvPDMA_Open(eDRVPDMA_CHANNEL_0, &sPDMA);
	/* Enable INT */
    DrvPDMA_EnableInt(eDRVPDMA_CHANNEL_0, eDRVPDMA_BLKD);
	/* Install Callback function */   
 	DrvPDMA_InstallCallBack(eDRVPDMA_CHANNEL_0, eDRVPDMA_BLKD, (PFN_DRVPDMA_CALLBACK) PDMA0_Callback ); 
	
    /* -------------------------------------------- */
    /* Configure PDMA Channel 1 to receive SPI2 Tx0 */
	/* -------------------------------------------- */
	/* SPI Port = SPI2 Tx0 */
	SPIPort = SPI2_BASE + 0x20;
	/* PDMA Setting */
	DrvPDMA_SetCHForAPBDevice(eDRVPDMA_CHANNEL_1, eDRVPDMA_SPI2, eDRVPDMA_WRITE_APB);
	/* CH0 TX Setting */
	sPDMA.sSrcCtrl.u32Addr 			= (uint32_t)SrcArray;
    sPDMA.sDestCtrl.u32Addr 		= SPIPort;   
	sPDMA.u8TransWidth 				= eDRVPDMA_WIDTH_8BITS;
	sPDMA.u8Mode 					= eDRVPDMA_MODE_MEM2APB;
	sPDMA.sSrcCtrl.eAddrDirection 	= eDRVPDMA_DIRECTION_INCREMENTED; 
	sPDMA.sDestCtrl.eAddrDirection 	= eDRVPDMA_DIRECTION_FIXED; 
	sPDMA.i32ByteCnt                = TEST_LENGTH;  
	DrvPDMA_Open(eDRVPDMA_CHANNEL_1, &sPDMA);
	/* Enable INT */
    DrvPDMA_EnableInt(eDRVPDMA_CHANNEL_1, eDRVPDMA_BLKD);
	/* Install Callback function */   
 	DrvPDMA_InstallCallBack(eDRVPDMA_CHANNEL_1, eDRVPDMA_BLKD, (PFN_DRVPDMA_CALLBACK) PDMA1_Callback ); 
		
    print_lcd(0,"SPI Sample Code ");
	print_lcd(1," for Flash+PDMA ");
    print_lcd(3,"Press key 1 ....");

    while(1) {
		if (scan_key() == 1) break;
	}	     
						    
    /* read MID & DID */
	SpiFlash_ReadMidDid();
	SysTimerDelay(1000000);
	
	/* Erase SPI flash */
	print_lcd(2,"Erase Flash.....");
	print_lcd(3,"                ");
	SpiFlash_ChipErase();
	
	/* Wait ready */
	SpiFlash_WaitReady();
	
	/* Get the status of SPI flash */
	print_lcd(3,"Erase done !!!  ");
	SysTimerDelay(1000000);
	
    /* init source data buffer */
	for(u32ByteCount=0; u32ByteCount<256; u32ByteCount++)
	{
		SrcArray[u32ByteCount] = u32ByteCount+1;
	} 

	/* Program SPI flash */	
	print_lcd(2,"Program Flash...");
	print_lcd(3,"                ");	
	u32FlashAddress = 0;
	for(u32PageNumber=0; u32PageNumber<TEST_NUMBER; u32PageNumber++)
	{
		/* Trigger PDMA specified Channel */
		DrvPDMA_CHEnableTransfer(eDRVPDMA_CHANNEL_1);
		 
		/* page program */
		SpiFlash_PageProgram(u32FlashAddress, 256);
		SpiFlash_WaitReady();
		u32FlashAddress += 0x100;
	} 
    print_lcd(3,"Program done !!!");
	SysTimerDelay(1000000);	

	/* clear destination data buffer */
	for(u32ByteCount=0; u32ByteCount<256; u32ByteCount++)
	{
		DestArray[u32ByteCount] = 0;
	} 
		
	/* Read SPI flash */
	print_lcd(2,"Verify Flash....");
	print_lcd(3,"                ");
	u32FlashAddress = 0;
	for(u32PageNumber=0; u32PageNumber<TEST_NUMBER; u32PageNumber++)
	{
		/* Trigger PDMA specified Channel */
		DrvPDMA_CHEnableTransfer(eDRVPDMA_CHANNEL_0);
		/* page read */
		SpiFlash_ReadData(u32FlashAddress, 256);
		u32FlashAddress += 0x100;
		
		for(u32ByteCount=0; u32ByteCount<256; u32ByteCount++)
		{
		    if(DestArray[u32ByteCount]!=SrcArray[u32ByteCount])
			{
				print_lcd(3,"= Verify Error =");
				while(1);
			} 
		}
   
		/* clear destination data buffer */
		for(u32ByteCount=0; u32ByteCount<256; u32ByteCount++)
		{
			DestArray[u32ByteCount] = 0;
		}
	}
	print_lcd(3,"Verify done !!! ");
	
	DrvSPI_Close(eDRVSPI_PORT2);
	DrvPDMA_Close();
	
	return 1;
}


// **************************************
// For W25Q16BV, Manufacturer ID: 0xEF; Device ID: 0x14
// For W26X16, Manufacturer ID: 0xEF; Device ID: 0x14
void SpiFlash_ReadMidDid(void)
{
	uint32_t au32SourceData;
    uint32_t au32DestinationData; 	

    // configure transaction length as 8 bits
	DrvSPI_SetBitLength(eDRVSPI_PORT2, 8);

	// /CS: active
	DrvSPI_SetSS(eDRVSPI_PORT2, eDRVSPI_SS0);
	
	// send Command: 0x90, Read Manufacturer/Device ID
	au32SourceData = 0x90;
	DrvSPI_SingleWrite(eDRVSPI_PORT2, &au32SourceData); 
    	
	// wait
	while (DrvSPI_IsBusy(eDRVSPI_PORT2)) {}
	
	// configure transaction length as 24 bits
	DrvSPI_SetBitLength(eDRVSPI_PORT2, 24);	

	// send 24-bit '0', dummy
	au32SourceData = 0x0;
    DrvSPI_SingleWrite(eDRVSPI_PORT2, &au32SourceData); 	

	// wait
	while (DrvSPI_IsBusy(eDRVSPI_PORT2)) {}
    
	// configure transaction length as 16 bits
	DrvSPI_SetBitLength(eDRVSPI_PORT2, 16);	

	// receive
	au32SourceData = 0x0;
	DrvSPI_SingleWrite(eDRVSPI_PORT2, &au32SourceData);
	
	// wait
	while (DrvSPI_IsBusy(eDRVSPI_PORT2)) {}
	
	// /CS: de-active
	DrvSPI_ClrSS(eDRVSPI_PORT2, eDRVSPI_SS0); 	

	DrvSPI_DumpRxRegister(eDRVSPI_PORT2, &au32DestinationData, 1);

    if ((au32DestinationData & 0xffff) == 0xEF14) 
		print_lcd(3,"MID & DID=0xEF14");
	else 
		print_lcd(3,"MID & DID Error!");
  
}

// **************************************
void SpiFlash_ChipErase(void)
{
	
	uint32_t au32SourceData;

	// configure transaction length as 8 bits
	DrvSPI_SetBitLength(eDRVSPI_PORT2, 8); 	

	// /CS: active
	DrvSPI_SetSS(eDRVSPI_PORT2, eDRVSPI_SS0); 	

	// send Command: 0x06, Write enable
	au32SourceData = 0x06;
	DrvSPI_SingleWrite(eDRVSPI_PORT2, &au32SourceData); 	

	// wait
	while (DrvSPI_IsBusy(eDRVSPI_PORT2)) {}

	// /CS: de-active
	DrvSPI_ClrSS(eDRVSPI_PORT2, eDRVSPI_SS0); 

	// /CS: active
	DrvSPI_SetSS(eDRVSPI_PORT2, eDRVSPI_SS0);	

	// send Command: 0xC7, Chip Erase
	au32SourceData = 0xc7;
	DrvSPI_SingleWrite(eDRVSPI_PORT2, &au32SourceData); 	

	// wait
	while (DrvSPI_IsBusy(eDRVSPI_PORT2)) {}

	// /CS: de-active
	DrvSPI_ClrSS(eDRVSPI_PORT2, eDRVSPI_SS0); 
}

// **************************************
uint32_t SpiFlash_ReadStatusReg1(void)
{
    uint32_t au32SourceData;
    uint32_t au32DestinationData; 	

	// configure transaction length as 16 bits
	DrvSPI_SetBitLength(eDRVSPI_PORT2, 16);
		
	// /CS: active
	DrvSPI_SetSS(eDRVSPI_PORT2, eDRVSPI_SS0);
	
	// send Command: 0x05, Read status register 1
	au32SourceData = 0x0500;
	DrvSPI_SingleWrite(eDRVSPI_PORT2, &au32SourceData); 
	
	// wait
	while (DrvSPI_IsBusy(eDRVSPI_PORT2)) {}
		
	// /CS: de-active
	DrvSPI_ClrSS(eDRVSPI_PORT2, eDRVSPI_SS0);

	// dump Rx register
	DrvSPI_DumpRxRegister(eDRVSPI_PORT2, &au32DestinationData, 1);

	return (au32DestinationData & 0xFF);
}

// **************************************
uint32_t SpiFlash_ReadStatusReg2(void)
{
	uint32_t au32SourceData;
    uint32_t au32DestinationData; 	
	
	// configure transaction length as 16 bits
	DrvSPI_SetBitLength(eDRVSPI_PORT2, 16);
	
	// /CS: active
	DrvSPI_SetSS(eDRVSPI_PORT2, eDRVSPI_SS0);
	
	// send Command: 0x35, Read status register 2
	au32SourceData = 0x3500;
	DrvSPI_SingleWrite(eDRVSPI_PORT2, &au32SourceData); 
	
	// wait
	while (DrvSPI_IsBusy(eDRVSPI_PORT2)) {}
	
	// /CS: de-active
	DrvSPI_ClrSS(eDRVSPI_PORT2, eDRVSPI_SS0);

    // dump Rx register
	DrvSPI_DumpRxRegister(eDRVSPI_PORT2, &au32DestinationData, 1);

	return (au32DestinationData & 0xFF);
}

// **************************************
void SpiFlash_WaitReady(void)
{
	uint32_t ReturnValue;
	
	do{
		ReturnValue = SpiFlash_ReadStatusReg1();
		ReturnValue = ReturnValue & 1;
	}while(ReturnValue!=0); // check the BUSY bit
	
}

// **************************************
void SpiFlash_PageProgram(uint32_t StartAddress, uint32_t ByteCount)
{
	uint32_t au32SourceData;
	
    // configure transaction length as 8 bits
	DrvSPI_SetBitLength(eDRVSPI_PORT2, 8);
	
	// /CS: active
	DrvSPI_SetSS(eDRVSPI_PORT2, eDRVSPI_SS0);
	
	// send Command: 0x06, Write enable
	au32SourceData = 0x06;
	DrvSPI_SingleWrite(eDRVSPI_PORT2, &au32SourceData); 
	
	// wait
	while (DrvSPI_IsBusy(eDRVSPI_PORT2)) {}
	
	// /CS: de-active
	DrvSPI_ClrSS(eDRVSPI_PORT2, eDRVSPI_SS0);
	
	// /CS: active
	DrvSPI_SetSS(eDRVSPI_PORT2, eDRVSPI_SS0);
	
	// send Command: 0x02, Page program
	au32SourceData = 0x02;
	DrvSPI_SingleWrite(eDRVSPI_PORT2, &au32SourceData); 
	
	// wait
	while (DrvSPI_IsBusy(eDRVSPI_PORT2)) {}
	
	// configure transaction length as 24 bits
	DrvSPI_SetBitLength(eDRVSPI_PORT2, 24);	
	
	// send 24-bit start address
	au32SourceData = StartAddress;
	DrvSPI_SingleWrite(eDRVSPI_PORT2, &au32SourceData);
		
	// wait
	while (DrvSPI_IsBusy(eDRVSPI_PORT2)) {}
	
	// configure transaction length as 8 bits
	DrvSPI_SetBitLength(eDRVSPI_PORT2, 8); 
	
	// enable SPI PDMA 
	DrvSPI_SetPDMA(eDRVSPI_PORT2, eDRVSPI_TX_DMA, TRUE);

	// SPI go
	PDMA1_INT_Flag = 0;
	DrvSPI_SetGo(eDRVSPI_PORT2);
	
	// wait PDMA1 done
	while (1)
	{
		if (PDMA1_INT_Flag)
		{
			PDMA1_INT_Flag = 0;
			break;
		}
	}
		
	// /CS: de-active
	DrvSPI_ClrSS(eDRVSPI_PORT2, eDRVSPI_SS0);
	
}


// **************************************
void SpiFlash_ReadData(uint32_t StartAddress, uint32_t ByteCount)
{
	uint32_t au32SourceData;
    
	// configure transaction length as 8 bits
	DrvSPI_SetBitLength(eDRVSPI_PORT2, 8);	

	// /CS: active
	DrvSPI_SetSS(eDRVSPI_PORT2, eDRVSPI_SS0);

	// send Command: 0x03, Read data
	au32SourceData = 0x03;
	DrvSPI_SingleWrite(eDRVSPI_PORT2, &au32SourceData); 	

	// wait
	while (DrvSPI_IsBusy(eDRVSPI_PORT2)) {}
	
	// configure transaction length as 24 bits
	DrvSPI_SetBitLength(eDRVSPI_PORT2, 24);	

	// send 24-bit start address
	au32SourceData = StartAddress;
	DrvSPI_SingleWrite(eDRVSPI_PORT2, &au32SourceData); 	

	// wait
	while (DrvSPI_IsBusy(eDRVSPI_PORT2)) {}
	
	// configure transaction length as 8 bits
	DrvSPI_SetBitLength(eDRVSPI_PORT2, 8); 	

  	// enable SPI PDMA 
	DrvSPI_SetPDMA(eDRVSPI_PORT2, eDRVSPI_RX_DMA, TRUE);

	// SPI go
	PDMA0_INT_Flag = 0;
	DrvSPI_SetGo(eDRVSPI_PORT2);
	
	// wait PDMA0 done	
	while (1)
	{
		if (PDMA0_INT_Flag)
		{
			PDMA0_INT_Flag = 0;
			break;
		}
	}
	
	// /CS: de-active
	DrvSPI_ClrSS(eDRVSPI_PORT2, eDRVSPI_SS0); 	

}

/*---------------------------------------------------------------------------------------------------------*/
/* PDMA Callback function                                                                           	   */
/*---------------------------------------------------------------------------------------------------------*/
void PDMA0_Callback()
{
	PDMA0_INT_Flag = 1;	

}

void PDMA1_Callback()
{
	PDMA1_INT_Flag = 1;	

}
