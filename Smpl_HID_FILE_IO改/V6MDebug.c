#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "Driver\DrvUSB.h"
#include "Driver\DrvGPIO.h"
#include "Driver\DrvSys.h"
#include "Driver\DrvSPI.h"
#include "Driver\DrvPDMA.h"
#include "LCD_Driver.h"
#include "HIDSysIO.h"
#include "V6MDebug.h"
#include "SPI_FLASH.h"

extern void USB_SendBackData(const uint8_t *pu8Buffer, uint32_t u32Size);



static void VCMD_AckCommand(const uint8_t *pu8Buffer, uint32_t u32Len)
{
    USB_SendBackData(pu8Buffer, u32Len);
}

static uint32_t u32Checksum, u32PackNum, u32StartAddr, u32TotalLen, LastPackNum;

void V6M_ProcessCommand(const uint8_t *pu8Buffer)
{
    static uint8_t au8RetBuffer[64], au8DataBuffer[56]; //, au8SPIDataBuffer[56];
	static uint8_t au8SPIWriteDataBuffer[256], au8SPIReadDataBuffer[256];
	uint32_t i;
	uint32_t u32Cmd;
	_DRVUSB_DISABLE_USB_INT();
    u32Cmd = (uint32_t)pu8Buffer[0] + (uint32_t)(pu8Buffer[1] << 8) + (uint32_t)(pu8Buffer[2] << 16) + (uint32_t)(pu8Buffer[3] << 24);
    u32PackNum = (uint32_t)pu8Buffer[4] + (uint32_t)(pu8Buffer[5] << 8) + (uint32_t)(pu8Buffer[6] << 16) + (uint32_t)(pu8Buffer[7] << 24);

    for (i = 0;i < 64;i++)
        au8RetBuffer[i] = 0x00; // Clear return buffer

    for (i = 0;i < 56;i++)
	{
        au8DataBuffer[i] = 0x00; // Clear data buffer
	}

    for (i = 0;i < 256;i++)
	{
        au8SPIWriteDataBuffer[i] = 0x00; // Clear data buffer
		au8SPIReadDataBuffer[i] = 0x00; // Clear data buffer
	}

    if (u32Cmd == V6M_CMD_PROGRAM)
    {
        //CMD (4 bytes) + Packet Number (4 bytes) + Start Address (4 bytes) + Total Length (4 bytes) + Data (48 bytes)
        u32Checksum = 0; // fixed checksum issue.
		u32TotalLen = 0;
		u32StartAddr = 0;

        u32StartAddr = (uint32_t)pu8Buffer[8] + (uint32_t)(pu8Buffer[9] << 8) + (uint32_t)(pu8Buffer[10] << 16) + (uint32_t)(pu8Buffer[11] << 24);
        u32TotalLen = (uint32_t)pu8Buffer[12] + (uint32_t)(pu8Buffer[13] << 8) + (uint32_t)(pu8Buffer[14] << 16) + (uint32_t)(pu8Buffer[15] << 24);
	   	//1. Checksum & Data
		if (u32TotalLen > 48)
		{
	        for (i = 0;i < 48; i++)
	        {
	            u32Checksum += pu8Buffer[i+16];
	            au8DataBuffer[i] = pu8Buffer[i+16];
	        }
		}
		else
		{
	        for (i = 0;i < u32TotalLen; i++)
	        {
	            u32Checksum += pu8Buffer[i+16];
	            au8DataBuffer[i] = pu8Buffer[i+16];
	        }
		}

        //2. Packet Number
        if (u32TotalLen > 48)
        {
            if (((u32TotalLen - 48) % 56) == 0)
                LastPackNum = (u32TotalLen - 48) / 56 + 1;
            else
                LastPackNum = (u32TotalLen - 48) / 56 + 2;
        }
        else
            LastPackNum = 1;
    }
    else
    {
        //0x00000000 (4 bytes) + Packet Number (4 bytes) + Data (56 bytes)

		//1. Checksum & Data
		if (((u32TotalLen - 48) % 56) == 0)
		{
	        for (i = 0;i < 56;i++)
	        {
	            u32Checksum += pu8Buffer[i+8];
	            au8DataBuffer[i] = pu8Buffer[i+8];
	        }
		}
		else
		{
			if (u32PackNum != LastPackNum) // Finish
			{
		        for (i = 0;i < 56;i++)
		        {
		            u32Checksum += pu8Buffer[i+8];
		            au8DataBuffer[i] = pu8Buffer[i+8];
				}
			}
			else
			{
		        for (i = 0;i < ((u32TotalLen - 48) % 56);i++)
		        {
		            u32Checksum += pu8Buffer[i+8];
		            au8DataBuffer[i] = pu8Buffer[i+8];
				}
			}
		}
    }

    if (u32PackNum == LastPackNum) // Finish
    {
		// Checksum	(4 bytes) + Packet Number (4 bytes)
        au8RetBuffer[0] = (uint8_t)(u32Checksum);
        au8RetBuffer[1] = (uint8_t)(u32Checksum >> 8);
        au8RetBuffer[2] = (uint8_t)(u32Checksum >> 16);
        au8RetBuffer[3] = (uint8_t)(u32Checksum >> 24);

        au8RetBuffer[4] = (uint8_t)(u32PackNum);
        au8RetBuffer[5] = (uint8_t)(u32PackNum >> 8);
        au8RetBuffer[6] = (uint8_t)(u32PackNum >> 16);
        au8RetBuffer[7] = (uint8_t)(u32PackNum >> 24);
    }

	/*================== SPI FLASH =======================*/
	if (u32Cmd == V6M_CMD_PROGRAM)
	{
		if (u32TotalLen > 48)
		{    
			for (i = 0; i < 48; i++)
				au8SPIWriteDataBuffer[i] = au8DataBuffer[i];

            SpiFlash_PageProgram(au8SPIWriteDataBuffer, u32StartAddr, 256);
            SpiFlash_WaitReady();
            SpiFlash_ReadData(au8SPIReadDataBuffer, u32StartAddr, 256);
#ifdef DEBUG            
			for (i = 0; i < 48; i++)
            {
                if (au8SPIReadDataBuffer[i] != au8DataBuffer[i])
                {

                    printf("Verify Error!\n");
                    while (1);
                }
                printf("u32PackNum %2x au8SPIDataBuffer %2x \n", u32PackNum, au8SPIReadDataBuffer[i]);
            }
#endif
            u32StartAddr += 256;
		}
		else //Finish
		{
			for (i = 0; i < 48; i++)
				au8SPIWriteDataBuffer[i] = au8DataBuffer[i];

            SpiFlash_PageProgram(au8SPIWriteDataBuffer, u32StartAddr, 256);
            SpiFlash_WaitReady();
            SpiFlash_ReadData(au8SPIReadDataBuffer, u32StartAddr, 256);
#ifdef DEBUG            
			for (i = 0; i < (u32TotalLen % 48); i++)
            {
                if (au8SPIReadDataBuffer[i] != au8DataBuffer[i])
                {

                    printf("Verify Error!\n");
                    while (1);
                }
                printf("u32PackNum %2x au8SPIDataBuffer %2x \n", u32PackNum, au8SPIReadDataBuffer[i]);
            }
#endif
            u32StartAddr += 256;
		}
	}
	else
	{
		if (u32PackNum != LastPackNum)
		{
			for (i = 0; i < 56; i++)
				au8SPIWriteDataBuffer[i] = au8DataBuffer[i];

            SpiFlash_PageProgram(au8SPIWriteDataBuffer, u32StartAddr, 256);
            SpiFlash_WaitReady();
            SpiFlash_ReadData(au8SPIReadDataBuffer, u32StartAddr, 256);
#ifdef DEBUG            
			for (i = 0; i < 56; i++)
            {
                if (au8SPIReadDataBuffer[i] != au8DataBuffer[i])
                {
                    printf("Verify Error!\n");
                    while (1);
                }
                printf("u32PackNum %2x au8SPIDataBuffer %2x \n", u32PackNum, au8SPIReadDataBuffer[i]);
            }
#endif
            u32StartAddr += 256;
		}
		else
		{
			for (i = 0; i < ((u32TotalLen - 48) % 56); i++)
				au8SPIWriteDataBuffer[i] = au8DataBuffer[i];

            SpiFlash_PageProgram(au8SPIWriteDataBuffer, u32StartAddr, 256);
            SpiFlash_WaitReady();
            SpiFlash_ReadData(au8SPIReadDataBuffer, u32StartAddr, 256);
#ifdef DEBUG            
			for (i = 0; i < ((u32TotalLen - 48) % 56);i++)
            {
                if (au8SPIReadDataBuffer[i] != au8DataBuffer[i])
                {
                    printf("Verify Error!\n");
                    while (1);
                }
                printf("u32PackNum %2x au8SPIDataBuffer %2x \n", u32PackNum, au8SPIReadDataBuffer[i]);
            }
#endif
            u32StartAddr += 256;
		}

	}

    VCMD_AckCommand((const uint8_t *)&au8RetBuffer, 64);
	_DRVUSB_ENABLE_USB_INT();
}


// Copy Return data.
//void V6M_ProcessCommand(const uint8_t *pu8Buffer, uint32_t u32Len)
//{
//    static uint8_t au8CmdBuffer[64]= {0};
//
//    memcpy(au8CmdBuffer, pu8Buffer, 64);
//	VCMD_AckCommand((const uint8_t *)&au8CmdBuffer, 64);
//}
