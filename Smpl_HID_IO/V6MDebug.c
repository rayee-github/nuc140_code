#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "DrvUSB.h"
#include "DrvGPIO.h"
#include "DrvSys.h"
#include "LCD_Driver.h"
#include "HIDSysIO.h"
#include "V6MDebug.h"

extern void USB_SendBackData(uint8_t bError, const uint8_t *pu8Buffer, uint32_t u32Size);

static void VCMD_AckCommand(uint32_t u32Errno, const uint8_t *pu8Buffer, uint32_t u32Len)
{
    USB_SendBackData((u32Errno == 0 ? FALSE : TRUE), pu8Buffer, u32Len);
}

static uint32_t VCMD_WillResetCommandSerial(const uint8_t *pu8Buffer, uint32_t u32Len)
{
    return 0;	//Reset in V6M_ProcessCommand
}

static uint32_t V6M_ProcessOneCommand(const uint8_t *pu8Buffer, uint32_t u32Len)
{
    uint32_t u32Cmd;

    memcpy(&u32Cmd, pu8Buffer, sizeof(u32Cmd));
    switch (u32Cmd)
    {
    case V6M_CMD_LED_ON:
        return LED_on(pu8Buffer, u32Len, 15);

    case V6M_CMD_LED5_ON:
        return LED_on(pu8Buffer, u32Len, 5);

    case V6M_CMD_LED6_ON:
        return LED_on(pu8Buffer, u32Len, 6);

    case V6M_CMD_LED7_ON:
        return LED_on(pu8Buffer, u32Len, 7);

    case V6M_CMD_LED8_ON:
        return LED_on(pu8Buffer, u32Len, 8);

    case V6M_CMD_LED_OFF:
        return LED_off(pu8Buffer, u32Len, 15);

    case V6M_CMD_LED5_OFF:
        return LED_off(pu8Buffer, u32Len, 5);

    case V6M_CMD_LED6_OFF:
        return LED_off(pu8Buffer, u32Len, 6);

    case V6M_CMD_LED7_OFF:
        return LED_off(pu8Buffer, u32Len, 7);

    case V6M_CMD_LED8_OFF:
        return LED_off(pu8Buffer, u32Len, 8);

    case V6M_CMD_RESET_CMD_SRIAL:
        return VCMD_WillResetCommandSerial(pu8Buffer, u32Len);

    default:
        //DrvSIO_printf("Unknown cmd: %02x\n", u32Cmd);

        return 1;
    }
}


void V6M_ProcessCommand(const uint8_t *pu8Buffer, uint32_t u32Len)
{

    static uint8_t au8CmdBuffer[V6M_MAX_COMMAND_LENGTH];
    static uint32_t u32BufferLen = 0;

    memcpy(au8CmdBuffer, pu8Buffer, 62);

    V6M_ProcessOneCommand(au8CmdBuffer, u32BufferLen);

}

uint32_t LED_on(const uint8_t *pu8Buffer, uint32_t u32Len, uint8_t u8LedNum)
{
    uint8_t i, au32Data[62] = {0};
    uint32_t u32Errno = 0;

    clr_all_pannal();  //clear all pannel
    print_lcd(0, "PC Send command");

    if (u8LedNum == 5)
        print_lcd(1, "LED 5 on     ");
    else if (u8LedNum == 6)
        print_lcd(1, "LED 6 on     ");
    else if (u8LedNum == 7)
        print_lcd(1, "LED 7 on     ");
    else if (u8LedNum == 8)
        print_lcd(1, "LED 8 on     ");
    else
        print_lcd(1, "LED All on   ");

	if(u8LedNum != 15)
	{
	    //LED5~LED8 are defined GPC12~GPC15
	    DrvGPIO_Open(E_GPC, u8LedNum + 7, E_IO_OUTPUT);
	    DrvGPIO_ClrBit(E_GPC, u8LedNum + 7);
	}
	else
	{
	   for(i=12;i<16;i++)
	   {
	    DrvGPIO_Open(E_GPC, i, E_IO_OUTPUT);
	    DrvGPIO_ClrBit(E_GPC, i);
	   }
	}

    VCMD_AckCommand(u32Errno, (const uint8_t *)&au32Data, 62);
    return 0;
}

uint32_t LED_off(const uint8_t *pu8Buffer, uint32_t u32Len, uint8_t u8LedNum)
{
    uint8_t i, au32Data[62] = {0};
    uint32_t u32Errno = 0;

    clr_all_pannal();  //clear all pannel
    print_lcd(0, "PC Send command");

    if (u8LedNum == 5)
        print_lcd(1, "LED 5 off  	 ");
    else if (u8LedNum == 6)
        print_lcd(1, "LED 6 off  	 ");
    else if (u8LedNum == 7)
        print_lcd(1, "LED 7 off   	 ");
    else if (u8LedNum == 8)
        print_lcd(1, "LED 8 off   	 ");
    else
        print_lcd(1, "LED All off  	 ");

	if(u8LedNum != 15)
	{
		//LED5~LED8 are defined GPC12~GPC15
	    DrvGPIO_Open(E_GPC, u8LedNum + 7, E_IO_OUTPUT);
	    DrvGPIO_SetBit(E_GPC, u8LedNum + 7);
	}
	else
	{
		for(i=12;i<16;i++)
		{
		    DrvGPIO_Open(E_GPC, i, E_IO_OUTPUT);
		    DrvGPIO_SetBit(E_GPC, i);
		}
	}

    VCMD_AckCommand(u32Errno, (const uint8_t *)&au32Data, 62);
    return 0;
}

