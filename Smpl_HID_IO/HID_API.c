/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) 2009 Nuvoton Technology Corp. All rights reserved.                                         */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "DrvUSB.h"
#include "HIDSysIO.h"
#include "HID_API.h"
#include "V6MDebug.h"

#define HID_FUNCTION    0

/* Define the vendor id and product id */
#define USB_VID			0x0416+0x104 // 0x051A
#define USB_PID			0x501B+0x100 // 0x511B

#define HID_DEFAULT_INT_IN_INTERVAL	1
#define HID_IS_SELF_POWERED         0
#define HID_IS_REMOTE_WAKEUP        0
#define HID_MAX_POWER               50  		/* The unit is in 2mA. ex: 50 * 2mA = 100mA */

//#define HID_REPORT_SIZE             64		/* 14 */
#define HID_REPORT_SIZE		    HID_MAX_PACKET_SIZE_EP0       /* The unit is in bytes */
#define LEN_CONFIG_AND_SUBORDINATE (LEN_CONFIG+LEN_INTERFACE+LEN_HID+LEN_ENDPOINT*2)

extern S_HID_DEVICE g_HID_sDevice;
const uint8_t g_HID_au8DeviceReportDescriptor[] =
{
    0x05, 0x01, // USAGE_PAGE (Generic Desktop)
    0x09, 0x00, // USAGE (0)
    0xa1, 0x01, // COLLECTION (Application)
    0x15, 0x00, //     LOGICAL_MINIMUM (0)
    0x25, 0xff, //     LOGICAL_MAXIMUM (255)
    0x19, 0x01, //     USAGE_MINIMUM (1)
    0x29, 0x08, //     USAGE_MAXIMUM (8)
    0x95, HID_MAX_PACKET_SIZE_EP0, //     REPORT_COUNT (8)
    0x75, 0x08, //     REPORT_SIZE (8)
    0x81, 0x02, //     INPUT (Data,Var,Abs)
    0x19, 0x01, //     USAGE_MINIMUM (1)
    0x29, 0x08, //     USAGE_MAXIMUM (8)
    0x91, 0x02, //   OUTPUT (Data,Var,Abs)
    0xc0        // END_COLLECTION
};

#define HID_DEVICE_REPORT_DESCRIPTOR_SIZE \
    sizeof (g_HID_au8DeviceReportDescriptor) / sizeof(g_HID_au8DeviceReportDescriptor[0])
const uint32_t g_HID_u32DeviceReportDescriptorSize = HID_DEVICE_REPORT_DESCRIPTOR_SIZE;


#define HID_REPORT_DESCRIPTOR_SIZE HID_DEVICE_REPORT_DESCRIPTOR_SIZE

static uint8_t g_au8DeviceReport[HID_REPORT_SIZE];
static const uint32_t g_u32DeviceReportSize = sizeof(g_au8DeviceReport) / sizeof(g_au8DeviceReport[0]);

const uint8_t gau8DeviceDescriptor[] =
{
	LEN_DEVICE,		// bLength
	DESC_DEVICE,	// bDescriptorType
	0x10, 0x01,		// bcdUSB
	0x00,			// bDeviceClass
	0x00,			// bDeviceSubClass
	0x00,			// bDeviceProtocol
	HID_MAX_PACKET_SIZE_EP0,	// bMaxPacketSize0
	// idVendor
	USB_VID & 0x00FF,
	(USB_VID & 0xFF00) >> 8,
	// idProduct
	USB_PID & 0x00FF,
	(USB_PID & 0xFF00) >> 8,
	0x00, 0x00,		// bcdDevice
	0x01,			// iManufacture
	0x02,			// iProduct
	0x03,			// iSerialNumber
	0x01			// bNumConfigurations
};

const uint8_t gau8ConfigDescriptor[] =
{
	LEN_CONFIG,		// bLength
	DESC_CONFIG,	// bDescriptorType
	// wTotalLength
	LEN_CONFIG_AND_SUBORDINATE & 0x00FF,
	(LEN_CONFIG_AND_SUBORDINATE & 0xFF00) >> 8,
	0x01,			// bNumInterfaces
	0x01,			// bConfigurationValue
	0x00,			// iConfiguration
	0x80 | (HID_IS_SELF_POWERED << 6) | (HID_IS_REMOTE_WAKEUP << 5),// bmAttributes
	HID_MAX_POWER,			// MaxPower

	// I/F descr: HID
	LEN_INTERFACE,	// bLength
	DESC_INTERFACE,	// bDescriptorType
    0x00,			// bInterfaceNumber
	0x00,			// bAlternateSetting
	0x02,			// bNumEndpoints
	0x03,			// bInterfaceClass
	0x00,			// bInterfaceSubClass
	0x00,			// bInterfaceProtocol
	0x00,			// iInterface

	// HID Descriptor
	LEN_HID,		// Size of this descriptor in UINT8s.
	DESC_HID,		// HID descriptor type.
	0x10, 0x01, 	// HID Class Spec. release number.
	0x00,			// H/W target country.
    0x01,			// Number of HID class descriptors to follow.
	DESC_HID_RPT,	// Dscriptor type.
	// Total length of report descriptor.
	HID_REPORT_DESCRIPTOR_SIZE & 0x00FF,
	(HID_REPORT_DESCRIPTOR_SIZE & 0xFF00) >> 8,

	// EP Descriptor: interrupt in.
	LEN_ENDPOINT,	// bLength
	DESC_ENDPOINT,	// bDescriptorType
	HID_IN_EP_NUM | EP_INPUT,   // bEndpointAddress
	EP_INT, // bmAttributes
	// wMaxPacketSize
	HID_MAX_PACKET_SIZE_EP1 & 0x00FF,
	(HID_MAX_PACKET_SIZE_EP1 & 0xFF00) >> 8,
	HID_DEFAULT_INT_IN_INTERVAL,		// bInterval

	// EP Descriptor: interrupt out.
	LEN_ENDPOINT,	// bLength
	DESC_ENDPOINT,	// bDescriptorType
	HID_OUT_EP_NUM | EP_OUTPUT,   // bEndpointAddress
	EP_INT, // bmAttributes
	// wMaxPacketSize
	HID_MAX_PACKET_SIZE_EP2 & 0x00FF,
	(HID_MAX_PACKET_SIZE_EP2 & 0xFF00) >> 8,
	HID_DEFAULT_INT_IN_INTERVAL		// bInterval

};

const uint8_t gau8StringLang[4] =
{
	4,				// bLength
	DESC_STRING,	// bDescriptorType
	0x09, 0x04
};

const uint8_t gau8VendorStringDescriptor[] =
{
	16,
	DESC_STRING,
	'N', 0,
    'U', 0,
    'V', 0,
    'O', 0,
    'T', 0,
    'O', 0,
    'N', 0
};

const uint8_t gau8ProductStringDescriptor[] =
{
	22,
	DESC_STRING,
	'U', 0,
    'S', 0,
    'B', 0,
    ' ', 0,
    'H', 0,
    'I', 0,
    'D', 0,
    ' ', 0,
    'I', 0,
    'O', 0
};

const uint8_t gau8StringSerial[26] =
{
	26,				// bLength
	DESC_STRING,	// bDescriptorType
	'B', 0, 
	'0', 0, 
	'2', 0, 
	'0', 0, 
	'0', 0, 
	'6', 0, 
	'0', 0, 
	'9', 0, 
	'2', 0, 
	'1', 0, 
	'1', 0, 
	'4', 0
};

void HID_Init(void *pfGetInReport, void *pfSetOutReport)
{
    /* Open HID to initial the descriptors and control handlers */
    HID_Open(HID_REPORT_SIZE, pfGetInReport, pfSetOutReport);

    /* Set the HID report descriptor */
    HID_SetReportDescriptor(g_HID_au8DeviceReportDescriptor, g_HID_u32DeviceReportDescriptorSize);

    /* Set the HID report buffer */
    HID_SetReportBuf(g_au8DeviceReport, g_u32DeviceReportSize);
}


/* Data for input & output */
static uint8_t g_u8CmdIndex = 0;
void USB_SendBackData(uint8_t bError, const uint8_t *pu8Buffer, uint32_t u32Size)
{
    uint8_t *pu8EpBuf;

    pu8EpBuf = (uint8_t *) & g_au8DeviceReport;
    if (u32Size > sizeof(g_au8DeviceReport))
        u32Size = sizeof(g_au8DeviceReport);

    pu8EpBuf[0] = (g_u8CmdIndex & (uint8_t)0x7F) | (bError ? (uint8_t)0x80 : (uint8_t)0x00);
    pu8EpBuf[1] = (uint8_t)u32Size;
    memcpy(&pu8EpBuf[2], pu8Buffer, u32Size);
}

/* This callback will be executed whenever the interrupt IN transaction is received from HOST to get
   report. */

void HID_GetInReport(uint8_t *buf)
{

}

/* This callback will be executed whenever the interrupt OUT transaction is received from HOST to set
   report. */
void HID_SetOutReport(uint8_t *pu8EpBuf)
{
    uint32_t u32Size = (uint32_t)pu8EpBuf[1];

    //Set new index
    g_u8CmdIndex = pu8EpBuf[0];

    V6M_ProcessCommand((uint8_t *)&pu8EpBuf[2], u32Size);									 
}

extern void Delay(uint32_t delayCnt);

int32_t HID_MainProcess(void)
{
    int32_t i32Ret = E_SUCCESS;
	E_DRVUSB_STATE eUsbState;
	    
    i32Ret = DrvUSB_Open((void *)DrvUSB_DispatchEvent);

    if(i32Ret != E_SUCCESS)
        return i32Ret;

    eUsbState = DrvUSB_GetUsbState();
    
    if (eUsbState >= eDRVUSB_ATTACHED)
    {
        _DRVUSB_ENABLE_SE0();
        Delay(1000);
        _DRVUSB_DISABLE_SE0();
    }

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
		
		/* Start HID and install the callback functions to handle in/out report */
		HID_Init((void *)HID_GetInReport, (void *)HID_SetOutReport);

		// Enable USB-related interrupts.
		_DRVUSB_ENABLE_MISC_INT(INTEN_WAKEUP | INTEN_WAKEUPEN | INTEN_FLDET | INTEN_USB | INTEN_BUS);

		// Poll and handle USB events.
		while(1)
		{
			eUsbState = DrvUSB_GetUsbState();
		
			if (eUsbState == eDRVUSB_DETACHED)
			{
				break;
			}
		}

		// Disable USB-related interrupts.
		_DRVUSB_ENABLE_MISC_INT(0);
    }
}
