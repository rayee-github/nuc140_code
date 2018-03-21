/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) 2009 Nuvoton Technology Corp. All rights reserved.                                         */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------------------------------------*/
/* Includes of system headers                                                                              */
/*---------------------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include "DrvGPIO.h"
#include "DrvSPI.h"
#include "NUC1xx.h"
#include "spiProg.h"


#define DRVSPIFLASH_PAGE_SIZE       256
#define DRVSPIFLASH_SECTOR_SIZE     4096

/* SPI Flash Status */
#define DRVSPIFLASH_SPR             0x80    /* Status Register Protect      */
#define DRVSPIFLASH_R               0x40    /* Reserved Bit                 */
#define DRVSPIFLASH_TB              0x20    /* Top / Bottom Block Protect   */
#define DRVSPIFLASH_BP2             0x10    /* Block Protect Bit 2          */
#define DRVSPIFLASH_BP1             0x8     /* Block Protect Bit 1          */
#define DRVSPIFLASH_BP0             0x4     /* Block Protect Bit 0          */
#define DRVSPIFLASH_WEL             0x2     /* Write Enable Latch           */
#define DRVSPIFLASH_BUSY            0x1     /* BUSY                         */


/* SPI Flash Command */
#define DRVSPIFLASH_ZERO            0x00
#define DRVSPIFLASH_DUMMY           0xFF
#define DRVSPIFLASH_WRITE_ENABLE    0x06
#define DRVSPIFLASH_WRITE_DISABLE   0x04
#define DRVSPIFLASH_READ_STATUS     0x05
#define DRVSPIFLASH_WRITE_STATUS    0x01
#define DRVSPIFLASH_FAST_READ       0x0B
#define DRVSPIFLASH_FAST_RD_DUAL    0x3B
#define DRVSPIFLASH_PAGE_PROGRAM    0x02
#define DRVSPIFLASH_BLOCK_ERASE     0xD8
#define DRVSPIFLASH_SECTOR_ERASE    0x20
#define DRVSPIFLASH_CHIP_ERASE      0xC7
#define DRVSPIFLASH_POWER_DOWN      0xB9
#define DRVSPIFLASH_RELEASE_PD_ID   0xAB
#define DRVSPIFLASH_FAST_READ_PARA  0x5B
#define DRVSPIFLASH_PROGRAM_PARA    0x52
#define DRVSPIFLASH_ERASE_PARA      0xD5
#define DRVSPIFLASH_DEVICE_ID       0x90
#define DRVSPIFLASH_JEDEC_ID        0x9F


static E_DRVSPI_PORT      g_SpiPort  = eDRVSPI_PORT0;
static E_DRVSPI_SLAVE_SEL g_SlaveSel = eDRVSPI_SS0;

uint32_t g_sectorBuf[1024]; /* 4096 bytes */

/*---------------------------------------------------------------------------------------------------------*/
/* Macro, type and constant definitions                                                                    */
/*---------------------------------------------------------------------------------------------------------*/

void WINBOND25X16A_SendCommandData(E_DRVSPI_PORT SpiPort, uint32_t u32Cmd, uint32_t u32CmdSize)
{
    DrvSPI_SetBitLength(SpiPort, u32CmdSize * 8);
    DrvSPI_SingleWrite(SpiPort, &u32Cmd);
    while(DrvSPI_IsBusy(SpiPort));
}

void WINBOND25X16A_ReadDataByByte(E_DRVSPI_PORT SpiPort, uint8_t * volatile pu8Data)
{
    uint32_t u32Data;

    DrvSPI_BurstTransfer(SpiPort, 1, 4);
    DrvSPI_SetBitLength(SpiPort, 8);
    DrvSPI_SetGo(SpiPort);
    while(DrvSPI_IsBusy(SpiPort));
    DrvSPI_DumpRxRegister(SpiPort, &u32Data, 1);
    *pu8Data = u32Data & 0xFF;
}

void WINBOND25X16A_ReadDataByWord(E_DRVSPI_PORT SpiPort, uint8_t * pu8Data)
{
    uint32_t u32Data;
    
    DrvSPI_BurstTransfer(SpiPort, 1, 4);
    DrvSPI_SetBitLength(SpiPort, 32);
    DrvSPI_SetGo(SpiPort);
    while(DrvSPI_IsBusy(SpiPort));
    DrvSPI_DumpRxRegister(SpiPort, &u32Data, 1);
    
    pu8Data[0] = (u32Data >> 24) & 0xFF;
    pu8Data[1] = (u32Data >> 16) & 0xFF;
    pu8Data[2] = (u32Data >>  8) & 0xFF;
    pu8Data[3] = u32Data & 0xFF;
}

void WINBOND25X16A_GetStatus(E_DRVSPI_PORT SpiPort, E_DRVSPI_SLAVE_SEL Channel, uint8_t * pu8Status)
{   
    DrvSPI_SetSS(SpiPort, Channel);
    WINBOND25X16A_SendCommandData(SpiPort, DRVSPIFLASH_READ_STATUS, 0x1);
    WINBOND25X16A_ReadDataByByte(SpiPort, pu8Status);
    DrvSPI_ClrSS(SpiPort, Channel);
}


uint8_t WINBOND25X16A_WaitIdle(E_DRVSPI_PORT SpiPort, E_DRVSPI_SLAVE_SEL Channel)
{
    uint8_t u8Status = 0;
    
    do
    {
        WINBOND25X16A_GetStatus(SpiPort, Channel, &u8Status);
    }while ((u8Status & DRVSPIFLASH_BUSY));
    
    return u8Status;
}


void WINBOND25X16A_EnableWrite(E_DRVSPI_PORT SpiPort, E_DRVSPI_SLAVE_SEL Channel, uint8_t bEnable)
{
    DrvSPI_SetSS(SpiPort, Channel);
    
    if (bEnable)
        WINBOND25X16A_SendCommandData(SpiPort, DRVSPIFLASH_WRITE_ENABLE, 0x1);
    else
        WINBOND25X16A_SendCommandData(SpiPort, DRVSPIFLASH_WRITE_DISABLE, 0x1);
    
    DrvSPI_ClrSS(SpiPort, Channel);
    WINBOND25X16A_WaitIdle(SpiPort, Channel);
}

void WINBOND25X16A_EraseSector(E_DRVSPI_PORT SpiPort, E_DRVSPI_SLAVE_SEL Channel, uint32_t u32StartSector)
{   
    uint32_t u32StartAddr;   
    uint8_t  u8Status = 0;
    
    WINBOND25X16A_EnableWrite(SpiPort, Channel, TRUE);
    WINBOND25X16A_WaitIdle(SpiPort, Channel);
    DrvSPI_SetSS(SpiPort, Channel);
    u32StartAddr = u32StartSector & 0x0FFFF000;
    WINBOND25X16A_SendCommandData(SpiPort, ((DRVSPIFLASH_SECTOR_ERASE << 24) | u32StartAddr), 0x4);
    DrvSPI_ClrSS(SpiPort, Channel);
    
    do  
    {
        WINBOND25X16A_GetStatus(SpiPort, Channel, &u8Status);
    } while( (u8Status & DRVSPIFLASH_WEL) );

}

int32_t WINBOND25X16A_ProgramPage(E_DRVSPI_PORT SpiPort, E_DRVSPI_SLAVE_SEL Channel, uint32_t u32StartPage, uint8_t * pu8Data)
{
    uint32_t u32StartAddr;
    uint32_t u32LoopCounter;
    uint32_t u32Data;
    uint8_t  u8Status;
    
    WINBOND25X16A_EnableWrite(SpiPort, Channel, TRUE);
    
    u8Status = WINBOND25X16A_WaitIdle(SpiPort, Channel);
    
    if (u8Status & DRVSPIFLASH_WEL)
    {
        DrvSPI_SetSS(SpiPort, Channel);

        u32StartAddr = u32StartPage & 0x0FFFFF00;
        WINBOND25X16A_SendCommandData(SpiPort, ((DRVSPIFLASH_PAGE_PROGRAM << 24) | u32StartAddr), 0x4);
        
        for (u32LoopCounter = 0; u32LoopCounter < DRVSPIFLASH_PAGE_SIZE; u32LoopCounter += 4)
        {
            u32Data = ((((pu8Data[u32LoopCounter] << 24) | (pu8Data[u32LoopCounter + 1] << 16)) | 
            (pu8Data[u32LoopCounter + 2] << 8)) + pu8Data[u32LoopCounter + 3]);
            
            WINBOND25X16A_SendCommandData(SpiPort, u32Data, 0x4);
        }   
        
        DrvSPI_ClrSS(SpiPort, Channel);
        WINBOND25X16A_WaitIdle(SpiPort, Channel);   
        pu8Data += DRVSPIFLASH_PAGE_SIZE;
    }
       
    return 0;   
}


int32_t WINBOND25X16A_ReadPage(
    E_DRVSPI_PORT SpiPort,
    E_DRVSPI_SLAVE_SEL  Channel, 
    uint8_t u8ReadMode,
    uint32_t u32StartPage, 
    uint8_t * pu8Data
)
{
    uint32_t u32StartAddr = (u32StartPage & 0x0FFFFF00);
    uint32_t u32DataSize  = DRVSPIFLASH_PAGE_SIZE;
    uint32_t u32LoopCounter;
        
    DrvSPI_SetSS(SpiPort, Channel);

    if (u8ReadMode == 0)
        WINBOND25X16A_SendCommandData(SpiPort, ((DRVSPIFLASH_FAST_READ << 24) | u32StartAddr), 0x4);   
    else            
        WINBOND25X16A_SendCommandData(SpiPort, ((DRVSPIFLASH_FAST_RD_DUAL << 24) | u32StartAddr), 0x4); 

    WINBOND25X16A_SendCommandData(SpiPort, DRVSPIFLASH_DUMMY, 0x1);

    for (u32LoopCounter = 0; u32LoopCounter < u32DataSize; u32LoopCounter += 4)
        WINBOND25X16A_ReadDataByWord(SpiPort, &pu8Data[u32LoopCounter]);

    DrvSPI_ClrSS(SpiPort, Channel);
    return 0;
}


void SpiInit(E_DRVSPI_PORT eSpiPort)
{
    E_DRVGPIO_FUNC eGpioFunc = E_FUNC_SPI0;

    /* Configure SPI related multi-function pins */
    eGpioFunc = (E_DRVGPIO_FUNC) (E_FUNC_SPI0 + (eSpiPort * 3));
    DrvGPIO_InitFunction(eGpioFunc);
    
    g_SpiPort  = eSpiPort;
    g_SlaveSel = eDRVSPI_SS0;

    /* Configure SPI as a master, 8-bit transaction */
    DrvSPI_Open(eSpiPort, eDRVSPI_MASTER, eDRVSPI_TYPE1, 32);
    
	/* MSB first */
    DrvSPI_SetEndian(eSpiPort, eDRVSPI_MSB_FIRST);
	        
    /* Disable the Automatic Slave Select */
    DrvSPI_DisableAutoSS(eSpiPort);    

    /* Set the active level of slave select. */
    DrvSPI_SetSlaveSelectActiveLevel(eSpiPort, eDRVSPI_ACTIVE_LOW_FALLING);
    
    /* SPI clock rate 12MHz */
    DrvSPI_SetClockFreq(eSpiPort, 12000000, 0);
}

void SpiRead(uint32_t addr, uint32_t size, uint32_t buffer)
{
    /* This is low level read function of USB Mass Storage */
    int32_t len;

    len = (int32_t)size;

    while(len >= DRVSPIFLASH_PAGE_SIZE)
    {
        WINBOND25X16A_ReadPage(g_SpiPort, g_SlaveSel, 0, addr, (uint8_t *)buffer);
        addr   += DRVSPIFLASH_PAGE_SIZE;
        buffer += DRVSPIFLASH_PAGE_SIZE;
        len    -= DRVSPIFLASH_PAGE_SIZE;
    }
}


void SpiWrite(uint32_t addr, uint32_t size, uint32_t buffer)
{
    /* This is low level write function of USB Mass Storage */
    int32_t len, i, offset;
    uint32_t *pu32;
    uint32_t alignAddr;

    len = (int32_t)size;
    
    if (len == DRVSPIFLASH_SECTOR_SIZE && ((addr & (DRVSPIFLASH_SECTOR_SIZE-1)) == 0))
    {   
        /* one-sector length & the start address is sector-alignment */
        WINBOND25X16A_EraseSector(g_SpiPort, g_SlaveSel, addr);
        
        //WINBOND25X16A_EnableWrite(g_SpiPort, g_SlaveSel, TRUE);  
        
        while (len >= DRVSPIFLASH_PAGE_SIZE)
        {
            WINBOND25X16A_ProgramPage(g_SpiPort, g_SlaveSel, addr, (uint8_t *)buffer);
            len    -= DRVSPIFLASH_PAGE_SIZE;
            buffer += DRVSPIFLASH_PAGE_SIZE;
            addr   += DRVSPIFLASH_PAGE_SIZE;
        }
    }
    else
    {
        do
        {
            /* alignAddr: sector address */
            alignAddr = addr & 0x1FFFF000;
            
             /* Get the sector offset*/
            offset = (addr & (DRVSPIFLASH_SECTOR_SIZE-1));  
            
                if (offset || (size < DRVSPIFLASH_SECTOR_SIZE))
                {
                    /* if the start address is not sector-alignment or the size is less than one sector, */
                    /* read back the data of the destination sector to g_sectorBuf[].                    */
                    SpiRead(alignAddr, DRVSPIFLASH_SECTOR_SIZE, (uint32_t)&g_sectorBuf[0]);
                }
            
            /* Update the data */
            pu32 = (uint32_t *)buffer;
            len = DRVSPIFLASH_SECTOR_SIZE - offset; /* len: the byte count between the start address and the end of a sector. */
            if (size < len) /* check if the range of data arrive at the end of a sector. */
                len = size; /* Not arrive at the end of a sector. "len" indicate the actual byte count of data. */
            
            for (i=0; i<len/4; i++)
            {
                g_sectorBuf[offset/4 + i] = pu32[i];
            }
            
            WINBOND25X16A_EraseSector(g_SpiPort, g_SlaveSel, alignAddr);
            
            
            //WINBOND25X16A_EnableWrite(g_SpiPort, g_SlaveSel, TRUE);  
            
            for (i=0;i<16;i++) /* one sector (16 pages) */
            {
                WINBOND25X16A_ProgramPage(g_SpiPort, g_SlaveSel, alignAddr + (i << 8), (uint8_t *)g_sectorBuf + (i << 8));
            }
            
            size -= len;
            addr += len;
            buffer += len;
            
        }while (size > 0);       
    }
}



void SpiChipErase(void)
{
	WINBOND25X16A_EnableWrite(g_SpiPort, g_SlaveSel, TRUE);

	DrvSPI_SetSS(g_SpiPort, g_SlaveSel);

	WINBOND25X16A_SendCommandData(g_SpiPort, DRVSPIFLASH_CHIP_ERASE, 0x1);

	DrvSPI_ClrSS(g_SpiPort, g_SlaveSel);

	WINBOND25X16A_WaitIdle(g_SpiPort, g_SlaveSel);
}

uint32_t SpiReadMidDid(void)
{
    uint32_t au32DestinationData;

	DrvSPI_SetSS(g_SpiPort, g_SlaveSel);

	WINBOND25X16A_SendCommandData(g_SpiPort, DRVSPIFLASH_DEVICE_ID, 0x1);

	WINBOND25X16A_SendCommandData(g_SpiPort, DRVSPIFLASH_ZERO, 0x3);

	WINBOND25X16A_SendCommandData(g_SpiPort, DRVSPIFLASH_ZERO, 0x2);

	DrvSPI_ClrSS(g_SpiPort, g_SlaveSel);

    // configure transaction length as 16 bits
    DrvSPI_SetBitLength(g_SpiPort, 16);

	DrvSPI_DumpRxRegister(g_SpiPort, &au32DestinationData, 0x1);

	return au32DestinationData;
}
