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
#include "LCD_Driver.h"

volatile uint32_t PDMA0_INT_Flag, PDMA1_INT_Flag;

void delay(void)
{
    int j;
    for (j = 0; j < 1000; j++);
}

void DelayCount(uint32_t delayCnt)
{
    while (delayCnt--)
    {
        __NOP();
        __NOP();
    }
}

// **************************************
// For W25Q16BV, Manufacturer ID: 0xEF; Device ID: 0x14	(2M byte)
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
        print_lcd(3, "MID & DID=0xEF14");
    else
        print_lcd(3, "MID & DID Error!");

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

    do
    {
        ReturnValue = SpiFlash_ReadStatusReg1();
        ReturnValue = ReturnValue & 1;
    }
    while (ReturnValue != 0); // check the BUSY bit

}

// **************************************
void SpiFlash_PageProgram(uint8_t *DataBuffer, uint32_t StartAddress, uint32_t ByteCount)
{
    uint32_t au32SourceData;
    uint32_t Counter;

    // configure transaction length as 8 bits
    DrvSPI_SetBitLength(eDRVSPI_PORT2, 8);

    // /CS: active
    DrvSPI_SetSS(eDRVSPI_PORT2, eDRVSPI_SS0);

    // send Command: 0x06, Write enable
    au32SourceData = 0x06;
    DrvSPI_SingleWrite(eDRVSPI_PORT2, &au32SourceData);

    // wait
    while (DrvSPI_IsBusy(eDRVSPI_PORT2));

    // /CS: de-active
    DrvSPI_ClrSS(eDRVSPI_PORT2, eDRVSPI_SS0);

    // /CS: active
    DrvSPI_SetSS(eDRVSPI_PORT2, eDRVSPI_SS0);

    // send Command: 0x02, Page program
    au32SourceData = 0x02;
    DrvSPI_SingleWrite(eDRVSPI_PORT2    , &au32SourceData);

    // wait
    while (DrvSPI_IsBusy(eDRVSPI_PORT2));

    // configure transaction length as 24 bits
    DrvSPI_SetBitLength(eDRVSPI_PORT2, 24);

    // send 24-bit start address
    au32SourceData = StartAddress;
    DrvSPI_SingleWrite(eDRVSPI_PORT2, &au32SourceData);

    // wait
    while (DrvSPI_IsBusy(eDRVSPI_PORT2));

    // configure transaction length as 8 bits
    DrvSPI_SetBitLength(eDRVSPI_PORT2, 8);

    for (Counter = 0; Counter < ByteCount; Counter++)
    {
        // send data to program
        au32SourceData = DataBuffer[Counter];
        DrvSPI_SingleWrite(eDRVSPI_PORT2, &au32SourceData);

        // wait
        while (DrvSPI_IsBusy(eDRVSPI_PORT2));
    }

    // /CS: de-active
    DrvSPI_ClrSS(eDRVSPI_PORT2, eDRVSPI_SS0);
}
// **************************************
void SpiFlash_ReadData(uint8_t *DataBuffer0, uint32_t StartAddress, uint32_t ByteCount)
{
    uint32_t au32SourceData;
    uint32_t au32DestinationData;
    uint32_t Counter;

    // configure transaction length as 8 bits
    DrvSPI_SetBitLength(eDRVSPI_PORT2, 8);

    // /CS: active
    DrvSPI_SetSS(eDRVSPI_PORT2, eDRVSPI_SS0);

    // send Command: 0x03, Read data
    au32SourceData = 0x03;
    DrvSPI_SingleWrite(eDRVSPI_PORT2, &au32SourceData);

    // wait
    while (DrvSPI_IsBusy(eDRVSPI_PORT2));

    // configure transaction length as 24 bits
    DrvSPI_SetBitLength(eDRVSPI_PORT2, 24);

    // send 24-bit start address
    au32SourceData = StartAddress;
    DrvSPI_SingleWrite(eDRVSPI_PORT2, &au32SourceData);

    // wait
    while (DrvSPI_IsBusy(eDRVSPI_PORT2));

    // configure transaction length as 8 bits
    DrvSPI_SetBitLength(eDRVSPI_PORT2, 8);

    for (Counter = 0; Counter < ByteCount; Counter++)
    {
        // receive
        au32SourceData = 0x0;
        DrvSPI_SingleWrite(eDRVSPI_PORT2, &au32SourceData);

        // wait
        while (DrvSPI_IsBusy(eDRVSPI_PORT2));

        // dump Rx register
        DrvSPI_DumpRxRegister(eDRVSPI_PORT2, &au32DestinationData, 1);
        DataBuffer0[Counter] = (uint8_t) au32DestinationData;
    }

    // /CS: de-active
    DrvSPI_ClrSS(eDRVSPI_PORT2, eDRVSPI_SS0);
}
