#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "Driver\DrvUSB.h"
#include "Driver\DrvGPIO.h"
#include "Driver\DrvSys.h"
#include "Driver\DrvSPI.h"
#include "Driver\DrvPDMA.h"
#include "LCD_Driver.h"
#include "CDROMHIDSys.h"
#include "V6MDebug.h"
#include "SpiProg.h"

extern void USB_SendBackData(const uint8_t *pu8Buffer, uint32_t u32Size);



static void VCMD_AckCommand(const uint8_t *pu8Buffer, uint32_t u32Len)
{
    USB_SendBackData(pu8Buffer, u32Len);
}

static uint32_t g_u32Checksum, g_u32PackNum, g_u32StartAddr, g_u32TotalLen, g_LastPackNum;
volatile uint8_t u8CmdOK = FALSE;

void V6M_ProcessCommand(const uint8_t *pu8Buffer)
{
    static uint8_t au8RetBuffer[64];
	static uint8_t au8SPIWriteDataBuffer[256], au8SPIReadDataBuffer[256];
	uint32_t i;
	uint32_t u32Cmd=0, u32CmdPackNum=0xFF;

	_DRVUSB_DISABLE_USB_INT();
    u32Cmd = (uint32_t)pu8Buffer[0] + (uint32_t)(pu8Buffer[1] << 8) + (uint32_t)(pu8Buffer[2] << 16) + (uint32_t)(pu8Buffer[3] << 24);
    u32CmdPackNum = (uint32_t)pu8Buffer[4] + (uint32_t)(pu8Buffer[5] << 8) + (uint32_t)(pu8Buffer[6] << 16) + (uint32_t)(pu8Buffer[7] << 24);

	if ((u32Cmd == V6M_CMD_PROGRAM ) && (u32CmdPackNum == 0) && (!u8CmdOK))
    {
	    /* Erase SPI flash */
	    print_lcd(2, "Erase Flash.....");
	    print_lcd(3, "                ");
	
		SpiChipErase();
	    //SysTimerDelay(1000000);
	
	    /* Get the status of SPI flash */
	    print_lcd(3, "Erase done !!!  ");

        //CMD (4 bytes) + CMD Packet Number (4 bytes) + Start Address (4 bytes) + Total Length (4 bytes) + Data (48 bytes)
        g_u32Checksum = 0; // fixed checksum issue.
		g_u32PackNum = 0;
		g_u32TotalLen = 0;

		g_u32StartAddr = (uint32_t)pu8Buffer[8] + (uint32_t)(pu8Buffer[9] << 8) + (uint32_t)(pu8Buffer[10] << 16) + (uint32_t)(pu8Buffer[11] << 24);
        g_u32TotalLen = (uint32_t)pu8Buffer[12] + (uint32_t)(pu8Buffer[13] << 8) + (uint32_t)(pu8Buffer[14] << 16) + (uint32_t)(pu8Buffer[15] << 24);
		u8CmdOK = TRUE;

        //1. Data Packet Number from 0 to N-1
        if (g_u32TotalLen % 64 == 0)
            g_LastPackNum = g_u32TotalLen / 64;
        else
            g_LastPackNum = g_u32TotalLen / 64 + 1;

	    for (i = 0;i < 64;i++)
	        au8RetBuffer[i] = 0x00; // Clear return buffer
	
	    for (i = 0;i < 256;i++)
		{
	        au8SPIWriteDataBuffer[i] = 0x00; // Clear data buffer
			au8SPIReadDataBuffer[i] = 0x00; // Clear data buffer
		}
    }
	else
    {
        //Data (64 bytes)
		//1. Checksum & Data
		if (g_u32PackNum < (g_LastPackNum-1)) // Not Finish 0~N-1
		{
	        for (i = 0;i < 64;i++)
	        {
	            g_u32Checksum += pu8Buffer[i];
	            au8SPIWriteDataBuffer[(g_u32PackNum % 4) * 64 + i] = pu8Buffer[i];
			}
			g_u32PackNum ++;
		}
		else
		{	// Finish  N-1
			if (g_u32TotalLen % 64 == 0)
			{
		        for (i = 0;i < 64;i++)
		        {
		            g_u32Checksum += pu8Buffer[i];
		            au8SPIWriteDataBuffer[(g_u32PackNum % 4) * 64 + i] = pu8Buffer[i];
				}
			}
			else
			{
		        for (i = 0;i < (g_u32TotalLen % 64);i++)
		        {
		            g_u32Checksum += pu8Buffer[i];
		            au8SPIWriteDataBuffer[(g_u32PackNum % 4) * 64 + i] = pu8Buffer[i];
				}
			}
			g_u32PackNum ++;
//			u8CmdOK = FALSE;
			// Checksum	(4 bytes) + Packet Number (4 bytes)
	        au8RetBuffer[0] = (uint8_t)(g_u32Checksum);
	        au8RetBuffer[1] = (uint8_t)(g_u32Checksum >> 8);
	        au8RetBuffer[2] = (uint8_t)(g_u32Checksum >> 16);
	        au8RetBuffer[3] = (uint8_t)(g_u32Checksum >> 24);
	
	        au8RetBuffer[4] = (uint8_t)(g_u32PackNum);
	        au8RetBuffer[5] = (uint8_t)(g_u32PackNum >> 8);
	        au8RetBuffer[6] = (uint8_t)(g_u32PackNum >> 16);
	        au8RetBuffer[7] = (uint8_t)(g_u32PackNum >> 24);
		}
		/*================== SPI FLASH =======================*/
		if (g_u32PackNum < g_LastPackNum) // Not Finish  Data Packet Number from 1 to N
		{
			if(g_u32PackNum % 4 == 0)
			{
				SpiWrite(g_u32StartAddr, 256, (uint32_t) au8SPIWriteDataBuffer);
				SpiRead(g_u32StartAddr, 256, (uint32_t) au8SPIReadDataBuffer);
	            g_u32StartAddr += 256;
#ifdef DEBUG            
				for (i = 0; i < 256; i++)
	            {
	                if (au8SPIReadDataBuffer[i] != au8SPIWriteDataBuffer[i])
	                {
	                    printf("Verify Error!\n");
	                    while (1);
	                }
	                printf("g_u32PackNum %2x au8SPIDataBuffer %2x \n", g_u32PackNum, au8SPIReadDataBuffer[i]);
	            }
#endif
			}
		}
		else
		{
			if ((g_u32TotalLen % 256 == 0) && (g_u32PackNum % 4 == 0))
			{
				SpiWrite(g_u32StartAddr, 256, (uint32_t) au8SPIWriteDataBuffer);
				SpiRead(g_u32StartAddr, 256, (uint32_t) au8SPIReadDataBuffer);
	            g_u32StartAddr += 256;
#ifdef DEBUG            
				for (i = 0; i < 256;i++)
	            {
	                if (au8SPIReadDataBuffer[i] != au8SPIWriteDataBuffer[i])
	                {
	                    printf("Verify Error!\n");
	                    while (1);
	                }
	                printf("g_u32PackNum %2x au8SPIDataBuffer %2x \n", g_u32PackNum, au8SPIReadDataBuffer[i]);
	            }
#endif
			}
			else
			{
				SpiWrite(g_u32StartAddr, g_u32TotalLen % 256, (uint32_t) au8SPIWriteDataBuffer);
				SpiRead(g_u32StartAddr, g_u32TotalLen % 256, (uint32_t) au8SPIReadDataBuffer);
	            g_u32StartAddr += g_u32TotalLen % 256;
#ifdef DEBUG            
				for (i = 0; i < (g_u32TotalLen % 256);i++)
	            {
	                if (au8SPIReadDataBuffer[i] != au8SPIWriteDataBuffer[i])
	                {
	                    printf("Verify Error!\n");
	                    while (1);
	                }
	                printf("g_u32PackNum %2x au8SPIDataBuffer %2x \n", g_u32PackNum, au8SPIReadDataBuffer[i]);
	            }
#endif
			}
			u8CmdOK = FALSE;
		}
    }

    VCMD_AckCommand((const uint8_t *)&au8RetBuffer, 64);
	_DRVUSB_ENABLE_USB_INT();
}
