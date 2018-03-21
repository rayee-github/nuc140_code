/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) 2009 Nuvoton Technology Corp. All rights reserved.                                         */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#ifndef _SPI_PROG_H
#define _SPI_PROG_H

#ifdef  __cplusplus
extern "C"
{
#endif	
	
	void WINBOND25X16A_SendCommandData(E_DRVSPI_PORT SpiPort, uint32_t u32Cmd, uint32_t u32CmdSize);
	
	
	void WINBOND25X16A_ReadDataByByte(E_DRVSPI_PORT SpiPort, uint8_t * volatile pu8Data);
	
	
	void WINBOND25X16A_ReadDataByWord(E_DRVSPI_PORT SpiPort, uint8_t * pu8Data);
	
	
	void WINBOND25X16A_GetStatus(E_DRVSPI_PORT SpiPort, E_DRVSPI_SLAVE_SEL Channel, uint8_t * pu8Status);
	
	uint8_t WINBOND25X16A_WaitIdle(E_DRVSPI_PORT SpiPort, E_DRVSPI_SLAVE_SEL Channel);
	
	void WINBOND25X16A_EnableWrite(E_DRVSPI_PORT SpiPort, E_DRVSPI_SLAVE_SEL Channel, uint8_t bEnable);
	
	void WINBOND25X16A_EraseSector(E_DRVSPI_PORT SpiPort, E_DRVSPI_SLAVE_SEL Channel, uint32_t u32StartSector);
	
	int32_t WINBOND25X16A_ProgramPage(E_DRVSPI_PORT SpiPort, E_DRVSPI_SLAVE_SEL Channel, uint32_t u32StartPage, uint8_t * pu8Data);
	
	int32_t WINBOND25X16A_ReadPage(
	    E_DRVSPI_PORT SpiPort,
	    E_DRVSPI_SLAVE_SEL  Channel, 
	    uint8_t u8ReadMode,
	    uint32_t u32StartPage, 
	    uint8_t * pu8Data
	);
	
	void SpiInit(E_DRVSPI_PORT eSpiPort);
	
	void SpiRead(uint32_t addr, uint32_t size, uint32_t buffer);
	
	void SpiWrite(uint32_t addr, uint32_t size, uint32_t buffer);

	void SpiChipErase(void);

	uint32_t SpiReadMidDid(void);

#ifdef  __cplusplus
}
#endif

#endif
