/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright (c) Nuvoton Technology Corp. All rights reserved.                                             */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "DrvSPI.h"
#include "DrvSYS.h"
#include "DrvUART.h"
#include "DrvUSB.h"
#include "CDROMHIDSys.h" 
#include "CDROM_HID_API.h"
#include "V6MDebug.h"

//#define USB_VID			0x0416  /* Vendor ID */
//#define USB_PID			0x1051  /* Product ID */
#define USB_VID			0x051A  /* Vendor ID */
#define USB_PID			0x511B  /* Product ID */

/*---------------------------------------------------------------------------------------------------------*/
/* Global variables                                                                                        */
/*---------------------------------------------------------------------------------------------------------*/
#define LEN_CONFIG_AND_SUBORDINATE (LEN_CONFIG+LEN_HID+LEN_INTERFACE*2+LEN_ENDPOINT*4)
//#define LEN_CONFIG_AND_SUBORDINATE (LEN_CONFIG+LEN_INTERFACE+LEN_ENDPOINT*2)

#define HID_REPORT_SIZE		    CDROM_MAX_PACKET_SIZE_EP0       /* The unit is in bytes */
#define HID_IS_SELF_POWERED         0
#define HID_IS_REMOTE_WAKEUP        0
#define HID_MAX_POWER               50  		/* The unit is in 2mA. ex: 50 * 2mA = 100mA */
#define HID_DEFAULT_INT_IN_INTERVAL	1

static uint8_t g_au8DeviceReport[HID_REPORT_SIZE];
static const uint32_t g_u32DeviceReportSize = sizeof(g_au8DeviceReport) / sizeof(g_au8DeviceReport[0]);

extern void SpiInit(E_DRVSPI_PORT eSpiPort);
extern uint8_t imageBegin0, imageEnd;

const uint8_t g_HID_au8DeviceReportDescriptor[] =
{
    0x05, 0x01, // USAGE_PAGE (Generic Desktop)
    0x09, 0x00, // USAGE (0)
    0xa1, 0x01, // COLLECTION (Application)
    0x15, 0x00, //     LOGICAL_MINIMUM (0)
    0x25, 0xff, //     LOGICAL_MAXIMUM (255)
    0x19, 0x01, //     USAGE_MINIMUM (1)
    0x29, 0x08, //     USAGE_MAXIMUM (8)
    0x95, CDROM_MAX_PACKET_SIZE_EP0, //     REPORT_COUNT (8)
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

const uint8_t g_CDROM_au8DeviceDescriptor[LEN_DEVICE] =
{
	LEN_DEVICE,		           /* bLength            */
	DESC_DEVICE,	           /* bDescriptorType    */
	0x10, 0x01,		           /* bcdUSB             */
	0x00,			           /* bDeviceClass       */
	0x00,			           /* bDeviceSubClass    */
	0x00,			           /* bDeviceProtocol    */
	CDROM_MAX_PACKET_SIZE_EP0, /* bMaxPacketSize0    */
	USB_VID & 0x00FF,          /* Veondor ID         */
	(USB_VID & 0xFF00) >> 8,
	USB_PID & 0x00FF,          /* Product ID         */
	(USB_PID & 0xFF00) >> 8,
	0x00, 0x00,		           /* bcdDevice          */
	0x01,			           /* iManufacture       */
	0x02,			           /* iProduct           */
	0x03,			           /* iSerialNumber      */
	0x01			           /* bNumConfigurations */
};

const uint8_t g_CDROM_au8ConfigDescriptor[] =
{
	LEN_CONFIG,		                            /* bLength             */
	DESC_CONFIG,	                            /* bDescriptorType     */
	LEN_CONFIG_AND_SUBORDINATE & 0x00FF,        /* wTotalLength        */
	(LEN_CONFIG_AND_SUBORDINATE & 0xFF00) >> 8,
	0x02,			                            /* bNumInterfaces      */
	0x01,			                            /* bConfigurationValue */
	0x00,			                            /* iConfiguration      */
    0x80 | (HID_IS_SELF_POWERED << 6) | (HID_IS_REMOTE_WAKEUP << 5),// bmAttributes
	HID_MAX_POWER,			                            /* MaxPower            */

	/* INTERFACE descriptor */
	LEN_INTERFACE,	                            /* bLength             */
	DESC_INTERFACE,	                            /* bDescriptorType     */
	0x00,			                            /* bInterfaceNumber    */
	0x00,			                            /* bAlternateSetting   */
	0x02,			                            /* bNumEndpoints       */
	0x08,			                            /* bInterfaceClass: Mass-Storage Class     */ 
	0x02,			                            /* bInterfaceSubClass: MMC-5               */ 
	0x50,			                            /* bInterfaceProtocol: Bulk Only Transport */ 
	0x00,			                            /* iInterface          */

	/* ENDPOINT descriptor: In */
	LEN_ENDPOINT,	                            /* bLength             */
	DESC_ENDPOINT,	                            /* bDescriptorType     */
	(EP_INPUT | BULK_IN_EP_NUM),			    /* bEndpointAddress    */
	EP_BULK,			                        /* bmAttributes        */
	BULK_EP_MXPLD & 0x00FF,                     /* wMaxPacketSize      */
	(BULK_EP_MXPLD & 0xFF00) >> 8,
	0x00,		                                /* bInterval           */
	
	/* ENDPOINT descriptor: Out */
	LEN_ENDPOINT,	                            /* bLength             */
	DESC_ENDPOINT,	                            /* bDescriptorType     */
	(EP_OUTPUT | BULK_OUT_EP_NUM),			    /* bEndpointAddress    */
	EP_BULK,			                        /* bmAttributes        */
	BULK_EP_MXPLD & 0x00FF,                     /* wMaxPacketSize      */
	(BULK_EP_MXPLD & 0xFF00) >> 8,
	0x00,		                                /* bInterval           */	

    // I/F descr: HID
    LEN_INTERFACE,	// bLength
    DESC_INTERFACE,	// bDescriptorType
    0x01,			// bInterfaceNumber
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
    INT_EP_MXPLD & 0x00FF,
    (INT_EP_MXPLD & 0xFF00) >> 8,
    HID_DEFAULT_INT_IN_INTERVAL,		// bInterval

    // EP Descriptor: interrupt out.
    LEN_ENDPOINT,	// bLength
    DESC_ENDPOINT,	// bDescriptorType
    HID_OUT_EP_NUM | EP_OUTPUT,   // bEndpointAddress
    EP_INT, // bmAttributes
    // wMaxPacketSize
    INT_EP_MXPLD & 0x00FF,
    (INT_EP_MXPLD & 0xFF00) >> 8,
    HID_DEFAULT_INT_IN_INTERVAL		// bInterval
};


const uint8_t gau8StringLang[] = {
	4,				/* bLength                  */
	DESC_STRING,	/* bDescriptorType          */
	0x09, 0x04      /* Language ID: USA(0x0409) */
};


const uint8_t gau8VendorStringDescriptor[] = {
	16,             /* bLength         */
	DESC_STRING,    /* bDescriptorType */
	'N', 0, 
	'u', 0, 
	'v', 0, 
	'o', 0, 
	't', 0, 
	'o', 0, 
	'n', 0
};

const uint8_t gau8ProductStringDescriptor[] =
{
    28,
    DESC_STRING,
    'U', 0,
    'S', 0,
    'B', 0,
    ' ', 0,
    'C', 0,
    'D', 0,
    'R', 0,
    'O', 0,
    'M', 0,
    ' ', 0,
	'H', 0,
	'I', 0,
	'D', 0
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
    '5', 0
};

void CDROM_BulkOutAckCallback(void * pVoid)
{	
	UsbBulkOutAck();
}

void CDROM_BulkInAckCallback(void * pVoid)
{
	UsbBulkInAck();
}

void USB_SendBackData(const uint8_t *pu8Buffer, uint32_t u32Size)
{
    uint8_t *pu8EpBuf;

    pu8EpBuf = (uint8_t *) & g_au8DeviceReport;
    if (u32Size > sizeof(g_au8DeviceReport))
        u32Size = sizeof(g_au8DeviceReport);

    memcpy(&pu8EpBuf[0], pu8Buffer, u32Size);
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
    V6M_ProcessCommand((uint8_t *)&pu8EpBuf[0]);
}

void CDROMHID_MainProcess(void)
{
	E_DRVUSB_STATE eUsbState;
		
    /* Initialize SPI interface */
    //SpiInit(eDRVSPI_PORT2);
    
    DrvUSB_Open((void*)DrvUSB_DispatchEvent);
 
	CDROM_Open(HID_REPORT_SIZE,(void *)HID_GetInReport, (void *)HID_SetOutReport);

    /* Set the HID report descriptor */
    HID_SetReportDescriptor(g_HID_au8DeviceReportDescriptor, g_HID_u32DeviceReportDescriptorSize);

    /* Set the HID report buffer */
    HID_SetReportBuf(g_au8DeviceReport, g_u32DeviceReportSize);    
	
	eUsbState = DrvUSB_GetUsbState();
    
	if (eUsbState >= eDRVUSB_ATTACHED)
    {
        /* Force Bus Reset for 150 ms*/
        _DRVUSB_ENABLE_SE0();
        DrvSYS_Delay(150000);
        _DRVUSB_DISABLE_SE0();
    }
    		
	/* Disable USB-related interrupts. */
	_DRVUSB_ENABLE_MISC_INT(0);

	/* Enable float-detection interrupt. */
	_DRVUSB_ENABLE_FLDET_INT();
        
	_DRVUSB_ENABLE_MISC_INT(INTEN_WAKEUP | INTEN_WAKEUPEN | INTEN_FLDET | INTEN_USB | INTEN_BUS);
 
    while(1)
    {
        eUsbState = DrvUSB_GetUsbState();

        if (eUsbState == eDRVUSB_DETACHED)
        {
            //printf("USB Detached.\n");
            /* Just waiting for USB attach */
            while(eUsbState == eDRVUSB_DETACHED)
            {
                eUsbState = DrvUSB_GetUsbState();
            }
            //printf("USB Attached.\n");
        }
    }
}



