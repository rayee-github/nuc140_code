#ifndef _SPI_PDMA_H
#define _SPI_PDMA_H
#ifdef __cplusplus
extern "C"
{
#endif

    /*---------------------------------------------------------------------------------------------------------*/
    /*                                                                                                         */
    /* Copyright (c) Nuvoton Technology Corp. All rights reserved.                                             */
    /*                                                                                                         */
    /*---------------------------------------------------------------------------------------------------------*/

    void PDMA0_Callback(void);
    void PDMA1_Callback(void);

    void SpiFlash_ReadMidDid(void);
    void SpiFlash_ChipErase(void);
    uint32_t SpiFlash_ReadStatusReg1(void);
    uint32_t SpiFlash_ReadStatusReg2(void);
    void SpiFlash_WaitReady(void);
    void SpiFlash_PageProgram(uint8_t *DataBuffer, uint32_t StartAddress, uint32_t ByteCount);
    void SpiFlash_ReadData(uint8_t *DataBuffer0, uint32_t StartAddress, uint32_t ByteCount);

//void Initial_pannel(void);
//void clr_all_pannal(void);
//void print_lcd(unsigned char, char *);

    void SysTimerDelay(uint32_t);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
