/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright (c) Nuvoton Technology Corp. All rights reserved.                                             */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include "Driver\DrvUART.h"
#include "Driver\DrvUSB.h"
#include "Driver\DrvGPIO.h"
#include "VCOMSys.h"
#include "VCOM_API.h"
#include "LCD_Driver.h"
#include "Seven_Segment.h"
#include "ScanKey.h"

#define USB_VID			0x0416  /* Vendor ID */
#define USB_PID			0x5011  /* Product ID */

#define USB_VID_LO      (USB_VID & 0xFF)
#define USB_VID_HI      ((USB_VID >> 8) & 0xFF)

#define USB_PID_LO      (USB_PID & 0xFF)
#define USB_PID_HI      ((USB_PID >> 8) & 0xFF)

const uint8_t gau8DeviceDescriptor[] =
{
	LEN_DEVICE,		/* bLength              */
	DESC_DEVICE,	/* bDescriptorType      */
	0x00, 0x02,		/* bcdUSB               */
	0x02,			/* bDeviceClass         */
	0x00,			/* bDeviceSubClass      */
	0x00,			/* bDeviceProtocol      */
	BULK_EP_MXPLD,	/* bMaxPacketSize0      */
	USB_VID_LO,                             
	USB_VID_HI,     /* Veondor ID           */
	USB_PID_LO,     
	USB_PID_HI,     /* Product ID           */
	0x00, 0x01,		/* bcdDevice            */
	0x01,			/* iManufacture         */
	0x02,			/* iProduct             */
	0x03,			/* iSerialNumber        */
	0x01			/* bNumConfigurations   */
};


const uint8_t gau8ConfigDescriptor[] =
{
	LEN_CONFIG,		/* bLength              */
	DESC_CONFIG,	/* bDescriptorType      */
	0x43, 0x00,		/* wTotalLength         */
	0x02,			/* bNumInterfaces       */
	0x01,			/* bConfigurationValue  */
	0x00,			/* iConfiguration       */
	0xC0,			/* bmAttributes         */
	0x32,			/* MaxPower             */

	/* INTERFACE descriptor */
	LEN_INTERFACE,	/* bLength              */
	DESC_INTERFACE,	/* bDescriptorType      */
	0x00,			/* bInterfaceNumber     */
	0x00,			/* bAlternateSetting    */
	0x01,			/* bNumEndpoints        */
	0x02,			/* bInterfaceClass      */
	0x02,			/* bInterfaceSubClass   */
	0x01,			/* bInterfaceProtocol   */
	0x00,			/* iInterface           */

	/* Communication Class Specified INTERFACE descriptor */
    0x05,           /* Size of the descriptor, in bytes */
    0x24,           /* CS_INTERFACE descriptor type */
    0x00,           /* Header functional descriptor subtype */
    0x10, 0x01,     /* Communication device compliant to the communication spec. ver. 1.10 */
    
	/* Communication Class Specified INTERFACE descriptor */
    0x05,           /* Size of the descriptor, in bytes */
    0x24,           /* CS_INTERFACE descriptor type */
    0x01,           /* Call management functional descriptor */
    0x00,           /* BIT0: Whether device handle call management itself. */
                    /* BIT1: Whether device can send/receive call management information over a Data Class Interface 0 */
    0x01,           /* Interface number of data class interface optionally used for call management */

	/* Communication Class Specified INTERFACE descriptor */
    0x04,           /* Size of the descriptor, in bytes */
    0x24,           /* CS_INTERFACE descriptor type */
    0x02,           /* Abstract control management funcational descriptor subtype */
    0x00,           /* bmCapabilities       */
    
	/* Communication Class Specified INTERFACE descriptor */
    0x05,           /* bLength              */
    0x24,           /* bDescriptorType: CS_INTERFACE descriptor type */
    0x06,           /* bDescriptorSubType   */
    0x00,           /* bMasterInterface     */
    0x01,           /* bSlaveInterface0     */
    
	/* ENDPOINT descriptor */
	LEN_ENDPOINT,	                /* bLength          */
	DESC_ENDPOINT,	                /* bDescriptorType  */
	(EP_INPUT | INT_IN_EP_NUM),     /* bEndpointAddress */
	EP_INT,		                    /* bmAttributes     */
	INT_EP_MXPLD, 0x00,	            /* wMaxPacketSize   */
	0x01,	                        /* bInterval        */
			
	/* INTERFACE descriptor */
	LEN_INTERFACE,	/* bLength              */
	DESC_INTERFACE,	/* bDescriptorType      */
	0x01,			/* bInterfaceNumber     */
	0x00,			/* bAlternateSetting    */
	0x02,			/* bNumEndpoints        */
	0x0A,			/* bInterfaceClass      */
	0x00,			/* bInterfaceSubClass   */
	0x00,			/* bInterfaceProtocol   */
	0x00,			/* iInterface           */
			
	/* ENDPOINT descriptor */
	LEN_ENDPOINT,	                /* bLength          */
	DESC_ENDPOINT,	                /* bDescriptorType  */
	(EP_INPUT | BULK_IN_EP_NUM),	/* bEndpointAddress */
	EP_BULK,		                /* bmAttributes     */
	BULK_EP_MXPLD, 0x00,	        /* wMaxPacketSize   */
	0x00,			                /* bInterval        */

	/* ENDPOINT descriptor */
	LEN_ENDPOINT,	                /* bLength          */
	DESC_ENDPOINT,	                /* bDescriptorType  */
	(EP_OUTPUT | BULK_OUT_EP_NUM),	/* bEndpointAddress */
	EP_BULK,		                /* bmAttributes     */
	BULK_EP_MXPLD, 0x00,	        /* wMaxPacketSize   */
	0x00,			                /* bInterval        */
};


const uint8_t gau8StringLang[] = {
	4,				/* bLength                  */
	DESC_STRING,	/* bDescriptorType          */
	0x09, 0x04      /* Language ID: USA(0x0409) */
};


const uint8_t gau8VendorStringDescriptor[] = {
	16,             /* bLength          */
	DESC_STRING,    /* bDescriptorType  */
	'N', 0, 
	'u', 0, 
	'v', 0, 
	'o', 0, 
	't', 0, 
	'o', 0, 
	'n', 0
};

const uint8_t gau8ProductStringDescriptor[] = {
	32,             /* bLength          */
	DESC_STRING,    /* bDescriptorType  */
	'U', 0, 
	'S', 0,
	'B', 0,
	' ', 0,
	'V', 0,
	'i', 0,
	'r', 0,
	't', 0,
	'u', 0,
	'a', 0,
	'l', 0,
	' ', 0,
	'C', 0,
	'O', 0,
	'M', 0
};

const uint8_t gau8StringSerial[26] =
{
	26,				/* bLength          */
	DESC_STRING,	/* bDescriptorType  */
	'N', 0, 
	'T', 0, 
	'2', 0, 
	'0', 0, 
	'0', 0, 
	'9', 0, 
	'1', 0, 
	'0', 0, 
	'1', 0, 
	'4', 0, 
	'0', 0, 
	'0', 0
};


/* This callback will be executed whenever the interrupt IN transaction is received from HOST to get
   report. */

void VCOM_GetInData(uint8_t *pu8EpBuf)
{
	uint8_t u8KeyValue=0;
	char KeyValue[16]="Key Value:";   	

	u8KeyValue = Scankey();
	if(u8KeyValue != 0)
	{
		clr_all_pannal();
	
		print_lcd(0, "Virtual COM   ");	  
		print_lcd(1, "Key Pad test  ");
		print_lcd(2, "              ");	  

		sprintf(KeyValue+10,"%d",u8KeyValue);
		print_lcd(2, KeyValue);

		show_seven_segment(0,u8KeyValue);
		u8KeyValue += 0x30;

		gsVcomDevice.u32DataSize = 1;
		pu8EpBuf = &u8KeyValue;
		//gsVcomDevice.pu8Data = &u8KeyValue;
		//pu8EpBuf[0] = 0x30 + u8KeyValue;
		DrvUSB_DataIn(BULK_IN_EP_NUM, pu8EpBuf, gsVcomDevice.u32DataSize);
	}
	else
		DrvUSB_DataIn(BULK_IN_EP_NUM, pu8EpBuf, 0);
}

/* This callback will be executed whenever the interrupt OUT transaction is received from HOST to set
   report. */
void VCOM_SetOutData(uint8_t *pu8EpBuf)
{
    static uint8_t au8CmdBuffer[BULK_EP_MXPLD];
	uint8_t u8Cmd;

    memcpy(au8CmdBuffer, pu8EpBuf, BULK_EP_MXPLD);
	memcpy(&u8Cmd, pu8EpBuf, sizeof(u8Cmd));

    switch (u8Cmd)
    {
    case '9':
        LED_on(pu8EpBuf, 15);
		show_seven_segment(0,9);
		break;
    case '1':
        LED_on(pu8EpBuf, 5);
		show_seven_segment(0,1);
		break;
    case '2':
        LED_on(pu8EpBuf, 6);
		show_seven_segment(0,2);
		break;
    case '3':
        LED_on(pu8EpBuf, 7);
		show_seven_segment(0,3);
		break;
    case '4':
        LED_on(pu8EpBuf, 8);
		show_seven_segment(0,4);
		break;
    case '0':
        LED_off(pu8EpBuf, 15);
		show_seven_segment(0,0);
		break;
    case '5':
        LED_off(pu8EpBuf, 5);
		show_seven_segment(0,5);
		break;
    case '6':
        LED_off(pu8EpBuf, 6);
		show_seven_segment(0,6);
		break;
    case '7':
        LED_off(pu8EpBuf, 7);
		show_seven_segment(0,7);
		break;
    case '8':
        LED_off(pu8EpBuf, 8);
		show_seven_segment(0,8);
		break;
    default:
		close_seven_segment();
		break;
    }

}

void VCOM_IntInAckCallback(void* pVoid)
{
    /* Reserved for user define */
}

void LED_on(const uint8_t *pu8Buffer, uint8_t u8LedNum)
{
    uint8_t i;

    clr_all_pannal();  //clear all pannel
    print_lcd(0, "PC Send command");

    if (u8LedNum == 5)
        print_lcd(1, "LED 5 on    ");
    else if (u8LedNum == 6)
        print_lcd(1, "LED 6 on    ");
    else if (u8LedNum == 7)
        print_lcd(1, "LED 7 on    ");
    else if (u8LedNum == 8)
        print_lcd(1, "LED 8 on    ");
    else
        print_lcd(1, "LED All on  ");

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
}

void LED_off(const uint8_t *pu8Buffer, uint8_t u8LedNum)
{
    uint8_t i;

    clr_all_pannal();  //clear all pannel
    print_lcd(0, "PC Send command");

    if (u8LedNum == 5)
        print_lcd(1, "LED 5 off   ");
    else if (u8LedNum == 6)
        print_lcd(1, "LED 6 off   ");
    else if (u8LedNum == 7)
        print_lcd(1, "LED 7 off    ");
    else if (u8LedNum == 8)
        print_lcd(1, "LED 8 off    ");
    else
        print_lcd(1, "LED All off  ");

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
}

//static void RoughDelay(uint32_t t)
//{
//    volatile int32_t delay;
//    delay = t;
//    while(delay-- >= 0);
//}

int32_t VCOM_MainProcess(void)
{
    int32_t i32Ret = E_SUCCESS;
	E_DRVUSB_STATE eUsbState;
	    
    i32Ret = DrvUSB_Open((void*)DrvUSB_DispatchEvent);
    if(i32Ret != E_SUCCESS)
        return i32Ret;

 	while(1)
	{
		/* Disable USB-related interrupts. */
		_DRVUSB_ENABLE_MISC_INT(0);

		/* Enable float-detection interrupt. */
		_DRVUSB_ENABLE_FLDET_INT();

		// Wait for USB connected.
		while (1)
		{
			// Order here is significant.
			// Give a chance to handle remaining events before exiting this loop.
			eUsbState = DrvUSB_GetUsbState();

			//DrvUSB_DispatchEvent();

			if (eUsbState >= eDRVUSB_ATTACHED &&
				eUsbState != eDRVUSB_SUSPENDED)
			{
				break;
			}
		}

		/* Start VCOM and install the callback functions to handle in/out data */
		VCOM_Open((void *)VCOM_GetInData, (void *)VCOM_SetOutData);

		// Enable USB-related interrupts.
		_DRVUSB_ENABLE_MISC_INT(INTEN_WAKEUP | INTEN_WAKEUPEN | INTEN_FLDET | INTEN_USB | INTEN_BUS);

		// Poll and handle USB events.
		while(1)
		{
			eUsbState = DrvUSB_GetUsbState();

			if (eUsbState == eDRVUSB_DETACHED)
				break;
		}

		// Disable USB-related interrupts.
		_DRVUSB_ENABLE_MISC_INT(0);
	}
}
