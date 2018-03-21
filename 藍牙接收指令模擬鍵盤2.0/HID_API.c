/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright (c) Nuvoton Technology Corp. All rights reserved.                                             */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include "Driver\DrvSYS.h"
#include "Driver\DrvUART.h"
#include "Driver\DrvUSB.h"
#include "HIDSys.h"
#include "HID_API.h"
#include "LCD_Driver.h"


#define DBG_PRINTF  printf
extern int IsDebugFifoEmpty(void);


#define HID_DIGITIZER   3
#define HID_MOUSE       2
#define HID_KEYBOARD    1
#define HID_FUNCTION    1

/* Define the vendor id and product id */
#define USB_VID         0x0416
#define USB_PID        (0xC140 + HID_FUNCTION)


#define HID_DEFAULT_INT_IN_INTERVAL 20
#define HID_IS_SELF_POWERED         0
#define HID_IS_REMOTE_WAKEUP        1
#define HID_MAX_POWER               50      /* The unit is in 2mA. ex: 50 * 2mA = 100mA */
uint16_t ADC_result;
#if(HID_FUNCTION == HID_MOUSE)
# define HID_REPORT_SIZE            3       /* The unit is in bytes */
#else
# define HID_REPORT_SIZE            9      /* The unit is in bytes */
#endif


#define LEN_CONFIG_AND_SUBORDINATE (LEN_CONFIG+LEN_INTERFACE+LEN_HID+LEN_ENDPOINT)

const S_DRVUSB_STRING_DESC
	g_sVendorStringDesc =
{
	16,
	DESC_STRING,
	{'N', 'U', 'V', 'O', 'T', 'O', 'N'}
};

const S_DRVUSB_STRING_DESC
	g_sProductStringDesc =
{
	22,
	DESC_STRING,
	{
		'A', 'R', 'M', ' ',
			's', 'e', 'r', 'i', 'e', 's'
	}
};

volatile uint8_t comRbuf[16] ={0x30};
char TEXT1[16] = "TX: sending...  ";
char TEXT2[16] = "RX:        ";
volatile uint16_t comRbytes = 0;
volatile uint16_t comRhead 	= 0;
volatile uint16_t comRtail 	= 0;   

#if (HID_FUNCTION == HID_MOUSE)
#if 1   /* relative movement */
const uint8_t g_HID_au8MouseReportDescriptor[] =
{
	0x05, 0x01,
	0x09, 0x02,
	0xA1, 0x01,
	0x09, 0x01,
	0xA1, 0x00,
	0x05, 0x09,
	0x19, 0x01,
	0x29, 0x03,
	0x15, 0x00,
	0x25, 0x01,
	0x95, 0x03,
	0x75, 0x01,
	0x81, 0x02,
	0x95, 0x01,
	0x75, 0x05,
	0x81, 0x01,
	0x05, 0x01,
	0x09, 0x30,
	0x09, 0x31,
	0x15, 0x81, //ligical minimun -127  
	0x25, 0x7F, //logical maximum 127
	0x75, 0x08,
	0x95, 0x02,
	0x81, 0x06,
	0xC0,
	0xC0
};
#else   /* absolute movement */
const uint8_t g_HID_au8MouseReportDescriptor[] = {
	0x05, 0x01,
	0x09, 0x02,
	0xA1, 0x01,
	0x09, 0x01,
	0xA1, 0x00,
	0x05, 0x09,
	0x19, 0x01,
	0x29, 0x03,
	0x15, 0x00,
	0x25, 0x01,
	0x95, 0x03,
	0x75, 0x01,
	0x81, 0x02,
	0x95, 0x01,
	0x75, 0x05,
	0x81, 0x01,
	0x05, 0x01,
	0x09, 0x30,
	0x09, 0x31,
	0x15, 0x81,             //logical min(-127)
	0x25, 0x7F,             //logical max(127)
	0x35, 0x00,             //physical min(0)
	0x46, 0xFF,0x00,        //physical max
	0x75, 0x08,             //report size (8 bits each)
	0x95, 0x02,             //report count (2 reports)
	0x81, 0x02,
	0xC0,
	0xC0      
};
#endif

#define HID_MOUSE_REPORT_DESCRIPTOR_SIZE \
	sizeof (g_HID_au8MouseReportDescriptor) / sizeof(g_HID_au8MouseReportDescriptor[0])
const uint32_t g_HID_u32MouseReportDescriptorSize = HID_MOUSE_REPORT_DESCRIPTOR_SIZE;

#define HID_REPORT_DESCRIPTOR_SIZE HID_MOUSE_REPORT_DESCRIPTOR_SIZE

uint8_t g_au8MouseReport[HID_REPORT_SIZE];
const uint32_t g_u32MouseReportSize = sizeof(g_au8MouseReport) / sizeof(g_au8MouseReport[0]);
#endif

#if (HID_FUNCTION == HID_KEYBOARD)
//keyboard 101keys
const uint8_t g_HID_au8KeyboardReportDescriptor[] = {
	/*    0x05, 0x01,
	0x09, 0x06,
	0xA1, 0x01,
	0x05, 0x07,
	0x19, 0xE0,
	0x29, 0xE7,
	0x15, 0x00,
	0x25, 0x01,
	0x75, 0x01,
	0x95, 0x08,
	0x81, 0x02,
	0x95, 0x01,
	0x75, 0x08,
	0x81, 0x01,
	0x95, 0x05,
	0x75, 0x01,
	0x05, 0x08,
	0x19, 0x01,
	0x29, 0x05,
	0x91, 0x02,
	0x95, 0x01,
	0x75, 0x03,
	0x91, 0x01,
	0x95, 0x06,
	0x75, 0x08,
	0x15, 0x00,
	0x25, 0x65,
	0x05, 0x07,
	0x19, 0x00,
	0x29, 0x65,
	0x81, 0x00,
	0xC0  		 */
	0x05, 0x01,		/* Usage Page(Generic Desktop Controls) */
	0x09, 0x06,		/* Usage(Keyboard) */
	0xA1, 0x01,		/* Collection(Application) */
	0x85, 0x01,		/* Report ID (1) */
	0x05, 0x07,			/* Usage Page(Keyboard/Keypad) */
	0x19, 0xE0,			/* Usage Minimum(0xE0) */
	0x29, 0xE7,			/* Usage Maximum(0xE7) */
	0x15, 0x00,			/* Logical Minimum(0x0) */
	0x25, 0x01,			/* Logical Maximum(0x1) */
	0x75, 0x01,			/* Report Size(0x1) */
	0x95, 0x08,			/* Report Count(0x8) */
	0x81, 0x02,			/* Input (Data) => Modifier byte */ 
	0x95, 0x01,			/* Report Count(0x1) */
	0x75, 0x08,			/* Report Size(0x8) */
	0x81, 0x01,			/* Input (Constant) => Reserved byte */
	0x95, 0x05,			/* Report Count(0x5) */
	0x75, 0x01,			/* Report Size(0x1) */
	0x05, 0x08,			/* Usage Page(LEDs) */
	0x19, 0x01,			/* Usage Minimum(0x1) */
	0x29, 0x05,			/* Usage Maximum(0x5) */
	0x91, 0x02,			/* Output (Data) => LED report */
	0x95, 0x01,			/* Report Count(0x1) */
	0x75, 0x03,			/* Report Size(0x3) */
	0x91, 0x01,			/* Output (Constant) => LED report padding */
	0x95, 0x06,			/* Report Count(0x6) */
	0x75, 0x08,			/* Report Size(0x8) */
	0x15, 0x00,			/* Logical Minimum(0x0) */
	0x25, 0x65,			/* Logical Maximum(0x65) */
	0x05, 0x07,			/* Usage Page(Keyboard/Keypad) */
	0x19, 0x00,			/* Usage Minimum(0x0) */
	0x29, 0x65,			/* Usage Maximum(0x65) */
	0x81, 0x00,			/* Input (Data) */
	0xC0, 			/* End Collection */

	0x05, 0x0C,		/* Usage Page (Consumer Devices) */
	0x09, 0x01,		/* Usage (Consumer Control) */
	0xA1, 0x01,		/* Collection: (Application) */
	0x85, 0x02,		/* Report ID (2) */
	0x15, 0x00,			/* Logical Minimum */
	0x25, 0x01,			/* Logical Maximum */
	0x09, 0xE2,			/* Usage (Mute) */
	0x09, 0xE9,			/* Usage (Volume+) */
	0x09, 0xEA,			/* Usage (Volume-) */
	0x09, 0x6F,			/* Usage (Brightness+) */
	0x09, 0x70,			/* Usage (Brightness-) */
	0x75, 0x01,			/* Report Size (1) */
	0x95, 0x05,			/* Report Count (5) */
	0x81, 0x02,			/* Input: (Data, Variable, Absolute) */
	0x05, 0x01,		/* Usage Page (Generic Desktop Controls) */
	0x09, 0x82,			/* Usage (System Sleep) */
	0x75, 0x01,			/* Report Size (1) */
	0x95, 0x01,			/* Report Count (1) */
	0x81, 0x02,			/* Input: (Data, Variable, Absolute) */
	0x75, 0x01,			/* Report Size (1) */
	0x95, 0x02,			/* Report Count (2) */
	0x81, 0x03,			/* Input: (Constant, Variable, Absolute) */
	0xC0,			/*End Collection */
};

#define HID_KEYBOARD_REPORT_DESCRIPTOR_SIZE \
	sizeof(g_HID_au8KeyboardReportDescriptor) / sizeof(g_HID_au8KeyboardReportDescriptor[0])
const uint32_t g_HID_u32KeyboardReportDescriptorSize = HID_KEYBOARD_REPORT_DESCRIPTOR_SIZE;

#define HID_REPORT_DESCRIPTOR_SIZE HID_KEYBOARD_REPORT_DESCRIPTOR_SIZE

uint8_t g_au8KeyboardReport[HID_REPORT_SIZE];
const uint32_t g_u32KeyboardReportSize = sizeof(g_au8KeyboardReport) / sizeof(g_au8KeyboardReport[0]);
#endif

#if (HID_FUNCTION == HID_DIGITIZER)
/* Multi-touch digitizer */
const uint8_t g_HID_au8DigitizerReportDescriptor[] = {
	0x05, 0x0d,                    // USAGE_PAGE (Digitizers)
	0x09, 0x04,                    // USAGE (Touch Screen)
	0xa1, 0x01,                    // COLLECTION (Application)
	0x85, 0x01,                    //   REPORT_ID (1)
	0x09, 0x22,                    //   USAGE (Finger)
	0xa1, 0x00,                    //   COLLECTION (Physical)
	0x09, 0x42,                    //     USAGE (Tip Switch)
	0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
	0x25, 0x01,                    //     LOGICAL_MAXIMUM (1)
	0x75, 0x01,                    //     REPORT_SIZE (1)
	0x95, 0x01,                    //     REPORT_COUNT (1)
	0x81, 0x02,                    //     INPUT (Data,Var,Abs)
	0x09, 0x32,                    //     USAGE (In Range)
	0x75, 0x01,                    //     REPORT_SIZE (1)
	0x95, 0x01,                    //     REPORT_COUNT (1)
	0x81, 0x02,                    //     INPUT (Data,Var,Abs)
	0x09, 0x51,                    //     USAGE (SwitchOffControl)
	0x75, 0x05,                    //     REPORT_SIZE (5)
	0x95, 0x01,                    //     REPORT_COUNT (1)
	0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
	0x26, 0x20, 0x00,//0x26, 0x14, 0x05,              //     LOGICAL_MAXIMUM (1300)
	0x81, 0x02,                    //     INPUT (Data,Var,Abs)
	0x09, 0x47,//0x09, 0x30,                    //     USAGE (Caller ID)
	0x75, 0x01,                    //     REPORT_SIZE (1)
	0x95, 0x01,                    //     REPORT_COUNT (1)
	0x81, 0x02,                    //     INPUT (Data,Var,Abs)
	0x05, 0x01,                    //     USAGE_PAGE (Generic Desktop)
	0x09, 0x30,                    //     USAGE (X)
	0x75, 0x10,                    //     REPORT_SIZE (16)
	0x95, 0x01,                    //     REPORT_COUNT (1)
	0x55, 0x0d,
	0x65, 0x33,
	0x36, 0x00, 0x00,//0x36, 0x00, 0xf0,              //     PHYSICAL_MINIMUM (-4096)
	0x46, 0x9a, 0x22,              //     PHYSICAL_MAXIMUM (8858)
	0x26, 0xff, 0x7f,              //     LOGICAL_MAXIMUM (32767)
	0x81, 0x02,                    //     INPUT (Data,Var,Abs)
	0x09, 0x31,                    //     USAGE (Y)
	0x75, 0x10,                    //     REPORT_SIZE (16)
	0x95, 0x01,                    //     REPORT_COUNT (1)
	0x55, 0x0d, 
	0x65, 0x33, 
	0x36, 0x00, 0x00,//0x36, 0x00, 0xf0,              //     PHYSICAL_MINIMUM (-4096)
	0x46, 0xc2, 0x14,              //     PHYSICAL_MAXIMUM (5314)
	0x26, 0xff, 0x7f,              //     LOGICAL_MAXIMUM (32767)
	0x81, 0x02,                    //     INPUT (Data,Var,Abs)
	0x05, 0x0d,                    //     USAGE_PAGE (Digitizers)
	0x09, 0x30,                    //     USAGE (Tip Pressure)
	0x75, 0x10,                    //     REPORT_SIZE (16)
	0x95, 0x01,                    //     REPORT_COUNT (1)
	0x65, 0x00,
	0x35, 0x01,                    //     PHYSICAL_MINIMUM (1)
	0x46, 0xff, 0x07,              //     PHYSICAL_MAXIMUM (2047)
	0x15, 0x01,                    //     LOGICAL_MINIMUM (1)
	0x26, 0xff, 0x07,              //     LOGICAL_MAXIMUM (2047)
	0x81, 0x02,                    //     INPUT (Data,Var,Abs)
	0x09, 0x55,//0x09, 0x46,                    //     USAGE (Tablet Pick)
	0x25, 0x08,                    //     LOGICAL_MAXIMUM (8)
	0x75, 0x08,                    //     REPORT_SIZE (8)
	0x95, 0x01,                    //     REPORT_COUNT (1)
	0xb1, 0x02,                    //     FEATURE (Data,Var,Abs)    
	0xc0,                          //   END_COLLECTION
	0xc0,                          // END_COLLECTION
	0x05, 0x0d,                    // USAGE_PAGE (Digitizers)
	0x09, 0x0e,//0x09, 0x0c,                    // USAGE (Multiple Point Digitizer)
	0xa1, 0x01,                    // COLLECTION (Application)
	0x85, 0x02,                    //   REPORT_ID (2)
	0x09, 0x22,                    //   USAGE (Finger)
	0xa1, 0x00,                    //   COLLECTION (Physical)
	0x09, 0x52,//0x09, 0x46,       //     USAGE (Tablet Pick)
	0x09, 0x53,//0x09, 0x80,       //     USAGE (System Control)
	0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
	0x25, 0x0a,                    //     LOGICAL_MAXIMUM (10)
	0x75, 0x08,                    //     REPORT_SIZE (8)
	0x95, 0x02,                    //     REPORT_COUNT (2)
	0xb1, 0x02,                    //     FEATURE (Data,Var,Abs)
	0xc0,                          //     END_COLLECTION
	0xc0,                          // END_COLLECTION

	0x06, 0x00, 0xff,              // USAGE_PAGE (Vendor Defined Page 1)
	0x09, 0x01,                    // USAGE (Vendor Usage 1)
	0xa1, 0x01,                    // COLLECTION (Application)

	0x85, 0x03,                    // Report ID
	0x95, 0x3F,                    //   REPORT_COUNT ()
	0x75, 0x08,                    //   REPORT_SIZE (8)
	0x26, 0xff, 0x00,              //   LOGICAL_MAXIMUM (255)
	0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
	0x09, 0x01,                    //   USAGE (Vendor Usage 1)
	0x91, 0x02,                    //   OUTPUT (Data,Var,Abs)
	0x09, 0x01,                    //   USAGE (Vendor Usage 1)
	0x81, 0x02,                    //   INPUT (Data,Var,Abs)

	0xC0                           //   end Application Collection
};

#define HID_DIGITIZER_REPORT_DESCRIPTOR_SIZE \
	sizeof(g_HID_au8DigitizerReportDescriptor) / sizeof(g_HID_au8DigitizerReportDescriptor[0])
const uint32_t g_HID_u32DigitizerReportDescriptorSize = HID_DIGITIZER_REPORT_DESCRIPTOR_SIZE;

#define HID_REPORT_DESCRIPTOR_SIZE HID_DIGITIZER_REPORT_DESCRIPTOR_SIZE

__align(4) uint8_t g_au8DigitizerReport[HID_REPORT_SIZE];
const uint32_t g_u32DigitizerReportSize = sizeof(g_au8DigitizerReport) / sizeof(g_au8DigitizerReport[0]);
#endif


const uint8_t g_HID_au8DeviceDescriptor[] =
{
	LEN_DEVICE,     // bLength
	DESC_DEVICE,    // bDescriptorType
	0x00, 0x02,     // bcdUSB
	0x00,           // bDeviceClass
	0x00,           // bDeviceSubClass
	0x00,           // bDeviceProtocol
	HID_MAX_PACKET_SIZE_CTRL,   // bMaxPacketSize0
	// idVendor
	USB_VID & 0x00FF,
	(USB_VID & 0xFF00) >> 8,
	// idProduct
	USB_PID & 0x00FF,
	(USB_PID & 0xFF00) >> 8,
	0x00, 0x00,     // bcdDevice
	0x01,           // iManufacture
	0x02,           // iProduct
	0x00,           // iSerialNumber
	0x01            // bNumConfigurations
};

const uint8_t g_HID_au8ConfigDescriptor[] =
{
	LEN_CONFIG,     // bLength
	DESC_CONFIG,    // bDescriptorType
	// wTotalLength
	LEN_CONFIG_AND_SUBORDINATE & 0x00FF,
	(LEN_CONFIG_AND_SUBORDINATE & 0xFF00) >> 8,
	0x01,           // bNumInterfaces
	0x01,           // bConfigurationValue
	0x00,           // iConfiguration
	0x80 | (HID_IS_SELF_POWERED << 6) | (HID_IS_REMOTE_WAKEUP << 5),// bmAttributes
	HID_MAX_POWER,          // MaxPower

	// I/F descr: HID
	LEN_INTERFACE,  // bLength
	DESC_INTERFACE, // bDescriptorType
	0x00,           // bInterfaceNumber
	0x00,           // bAlternateSetting
	0x01,           // bNumEndpoints
	0x03,           // bInterfaceClass
	0x01,           // bInterfaceSubClass
	HID_FUNCTION,           // bInterfaceProtocol
	0x00,           // iInterface

	// HID Descriptor
	LEN_HID,        // Size of this descriptor in UINT8s.
	DESC_HID,       // HID descriptor type.
	0x10, 0x01,     // HID Class Spec. release number.
	0x00,           // H/W target country.
	0x01,           // Number of HID class descriptors to follow.
	DESC_HID_RPT,   // Dscriptor type.
	// Total length of report descriptor.
	HID_REPORT_DESCRIPTOR_SIZE & 0x00FF,
	(HID_REPORT_DESCRIPTOR_SIZE & 0xFF00) >> 8,

	// EP Descriptor: interrupt in.
	LEN_ENDPOINT,   // bLength
	DESC_ENDPOINT,  // bDescriptorType
	(INT_IN_EP_NUM | EP_INPUT), // bEndpointAddress
	EP_INT,         // bmAttributes
	// wMaxPacketSize
	HID_MAX_PACKET_SIZE_INT_IN & 0x00FF,
	(HID_MAX_PACKET_SIZE_INT_IN & 0xFF00) >> 8,
	HID_DEFAULT_INT_IN_INTERVAL     // bInterval
};

const uint8_t g_HID_au8StringLang[4] =
{
	4,              // bLength
	DESC_STRING,    // bDescriptorType
	0x09, 0x04
};

const S_DRVUSB_STRING_DESC g_HID_sVendorStringDesc =
{
	16,
	DESC_STRING,
	{'N', 'U', 'V', 'O', 'T', 'O', 'N'}
};

const S_DRVUSB_STRING_DESC g_HID_sProductStringDesc =
{
	16,
	DESC_STRING,
	{'W', 'P', 'M', ' ', 'U', 'S', 'B'}
};

const uint8_t g_HID_au8StringSerial[26] =
{
	26,             // bLength
	DESC_STRING,    // bDescriptorType
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

void HID_Init(void)
{
	/* Open HID to initial the descriptors and control handlers */
	HID_Open();

#if(HID_FUNCTION == HID_MOUSE)

	/* Set the HID report descriptor */
	HID_SetReportDescriptor(g_HID_au8MouseReportDescriptor, g_HID_u32MouseReportDescriptorSize);

	/* Set the HID report buffer */
	HID_SetReportBuf(g_au8MouseReport, g_u32MouseReportSize);

#elif(HID_FUNCTION == HID_DIGITIZER)
	/* Set the HID report descriptor */
	HID_SetReportDescriptor(g_HID_au8DigitizerReportDescriptor, g_HID_u32DigitizerReportDescriptorSize);

	/* Set the HID report buffer */
	HID_SetReportBuf(g_au8DigitizerReport, g_u32DigitizerReportSize);
#else
	/* Set the HID report descriptor */
	HID_SetReportDescriptor(g_HID_au8KeyboardReportDescriptor, g_HID_u32KeyboardReportDescriptorSize);

	/* Set the HID report buffer */
	HID_SetReportBuf(g_au8KeyboardReport, g_u32KeyboardReportSize);
#endif

}


#if(HID_FUNCTION == HID_MOUSE)



/* This function is used to prepare IN report. The g_HID_sDevice.u32ReportSize will be reset when it has been done. */
void HID_SetInReport()
{
	uint8_t *buf;
	/* To check if previous report data is processed or not */
	if(g_HID_sDevice.isReportReady)
		return;

	buf = g_HID_sDevice.pu8Report;

	/* Move mouse when GPD1 == 0 */
	if ((GPIOD->PIN & (1 << 1)) == 0)
	{
		/* Update new report data */
		// 	if(ADC_result>=2000)
		{
			buf[0] = 0x00;
			buf[1] = 0x01;
			buf[2] = 0x00;
		}
		/*	else
		{
		buf[0] = 0x00;
		buf[1] = 0x00;
		buf[2] = 0x01;
		}	  */
		g_HID_sDevice.isReportReady = 1; /* To note the report data has been updated */
		DrvUSB_DataIn(INT_IN_EP_NUM, g_HID_sDevice.pu8Report, g_HID_sDevice.u32ReportSize);
	}
}

#elif(HID_FUNCTION == HID_DIGITIZER)

void HID_SetInReport()
{
	uint8_t *buf;
	uint32_t key=0;
	static int16_t x = 0, y = 700, z = 0;
	static int16_t xinc = 1, yinc = 1;
	int16_t tmp;


	if(g_HID_sDevice.isReportReady)
		return;

	buf = g_HID_sDevice.pu8Report;

	key = (GPIOD->PIN & (1 << 1))? 0 : 1;

	{
		/* No any key pressed */
		if(key == 0)
		{
			if(z)
			{
				if(buf[1] == 0x07)
					buf[1] = 0x04;
				z = 0;
				g_HID_sDevice.isReportReady = 1;
				DrvUSB_DataIn(INT_IN_EP_NUM, g_HID_sDevice.pu8Report, g_HID_sDevice.u32ReportSize);

			}
			else
			{
				buf[3] = buf[4] = buf[5] = buf[6] = 0;
			}
		}
		else
		{
			z++;
		}

		if(key)
		{
			g_HID_sDevice.isReportReady = 1;

			buf[0] = 0x01;

			buf[1] = (z > 4)?0x07:0x00;
			buf[2] = 0x00;


			if(z > 4)
			{
				if(z > 8)
					x+=xinc;
				if(x >= 5091){ x = 5091; xinc =-1;}
				if(x < 0){x = 0;xinc = 1;}
				tmp = x >> 0;
				buf[3] = tmp & 0xFF;
				buf[4] = tmp >> 8;

				y+=yinc;
				if(y >= 2863){ y = 2863; yinc = -1;}
				if(y < 0){y = 0; yinc = 1;}
				tmp = y >> 1;
				buf[5] = tmp & 0xFF;
				buf[6] = tmp >> 8;
			}

			buf[7] = 0x00;
			buf[8] = 0x01;
			buf[9] = 0x00;

			buf[10]= 0x00;
			buf[11]= 0x00;

			buf[12]= 0x00;
			buf[13]= (z > 4)?1:0;

			DrvUSB_DataIn(INT_IN_EP_NUM, g_HID_sDevice.pu8Report, g_HID_sDevice.u32ReportSize);
		}
	}
}
#else

void HID_SetInReport()
{
	/*  uint8_t *buf;
	int32_t i;
	uint32_t key = 0xF;
	static uint32_t preKey;


	if(g_HID_sDevice.isReportReady)
	//    return;


	//  If GPD1 = 0, just report it is key 'a' 
	key = (GPIOD->PIN & (1 << 1))? 0 : 1;

	buf = g_HID_sDevice.pu8Report;

	if (key == 0)
	{
	for (i=0;i<8;i++)
	{
	buf[i] = 0;
	}

	if(key != preKey)
	{
	// Trigger to note key release 
	g_HID_sDevice.isReportReady = 1;
	DrvUSB_DataIn(INT_IN_EP_NUM, g_HID_sDevice.pu8Report, g_HID_sDevice.u32ReportSize);
	}
	}
	else
	{

	preKey = key;
	buf[2] = 0x04;// Key A 
	g_HID_sDevice.isReportReady = 1;
	DrvUSB_DataIn(INT_IN_EP_NUM, g_HID_sDevice.pu8Report, g_HID_sDevice.u32ReportSize);
	}  															  */
	uint8_t *buf;
	int32_t i;
	uint32_t key = 0xF;
	uint32_t key1 = 0xF;
	uint32_t key2 = 0xF;

	static uint32_t preKey;

	/* To check if previous report data is processed or not */
	if(g_HID_sDevice.isReportReady)
		return;
	/* Update new report data */

	/* If GPD2 = 0, just report it is key 'a' */
	key = (GPIOD->PIN & (1 << 2))? 0 : 1;
	key=1;
	buf = g_HID_sDevice.pu8Report;

	if (key == 0)
	{

		buf[0] = 0x01;	/* Report ID */ 

		for (i=1;i<9;i++)
		{
			buf[i] = 0;										 
		}

		if((key != preKey) && (key1 != preKey) && (key2 != preKey))
		{
			/* Trigger to note key release */
			g_HID_sDevice.isReportReady = 1;
			DrvUSB_DataIn(INT_IN_EP_NUM, g_HID_sDevice.pu8Report, g_HID_sDevice.u32ReportSize);
		}
	}
	else if (key == 1)
	{
		// Key A    2c =space      4f,50,51,52			  w=1a s=16 a=04 d=07
		buf[0] = 0x01;	/* Report ID */

		if(comRbuf[0]=='1')
			buf[3]=0x4f;
		else if(comRbuf[0]=='2')
			buf[3]=0x50;
		else
			buf[3]=0x00;

		if(comRbuf[1]=='1')
			buf[4]=0x51;
		else if(comRbuf[1]=='2')
			buf[4]=0x52;										  
		else
			buf[4]=0x00;

		if(comRbuf[0]=='3') 
			buf[3] = 0x04; 
		else if(comRbuf[0]=='4') 
			buf[3] = 0x05; 
		else if(comRbuf[0]=='5') 
			buf[3] = 0x06; 
		
		//DrvSYS_Delay(2000);	
		g_HID_sDevice.isReportReady = 1;   
		DrvUSB_DataIn(INT_IN_EP_NUM, g_HID_sDevice.pu8Report, g_HID_sDevice.u32ReportSize);
	}  
}

#endif


/* This function is used to prepare the first out report. */
void HID_SetFirstOutReport(void)
{

}

/* This function is used to prepare the first in report */
void HID_SetFirstInReport(void)
{
	DrvUSB_DataIn(INT_IN_EP_NUM, g_HID_sDevice.pu8Report, g_HID_sDevice.u32ReportSize);
}


/* This callback will be executed whenever the interrupt OUT transaction is received from HOST and device response ACK.
Therefore, we could prepare next out report here. */
void HID_GetOutReportAck(uint8_t *buf, uint32_t size)
{

}

void PowerDown()
{
	DBG_PRINTF("Enter power down ...\n");

	UNLOCKREG();

	while ( IsDebugFifoEmpty() == FALSE );

	/* Wakeup Enable */
	USBD->INTEN.WAKEUP_EN = 1;

	/* Deep sleep */
	outp32(0xE000ED10, 4);
	SYSCLK->PWRCON.PD_WAIT_CPU = 1;
	SYSCLK->PWRCON.PWR_DOWN_EN = 1;
	__WFI();
	SYSCLK->PWRCON.PWR_DOWN_EN = 0;

	if(DrvUSB_GetUsbState() & eDRVUSB_SUSPENDED)
	{
		/* Note HOST to resume USB tree if it is suspended and remote wakeup enabled */
		if(DrvUSB_IsRemoteWakeupEnabled())
		{
			/* Enable PHY before sending Resume('K') state */
			USBD->ATTR.PHY_EN = 1;

			/* Keep remote wakeup for 1 ms */
			USBD->ATTR.RWAKEUP = 1;
			DrvSYS_Delay(1000); /* Delay 1ms */
			USBD->ATTR.RWAKEUP = 0;
		}
	}

	DBG_PRINTF("device wakeup!\n");

}


void UART_INT_HANDLE(void)
{
	uint8_t bInChar[1] = {0xFF};

	while(UART0->ISR.RDA_IF==1) 
	{
		DrvUART_Read(UART_PORT0,bInChar,1);	
		if(bInChar[0]=='s') // check if Buffer is full
		{
			DrvUART_Read(UART_PORT0,bInChar,1);	
			comRbuf[0] = bInChar[0];
			DrvUART_Read(UART_PORT0,bInChar,1);	
			comRbuf[1] = bInChar[0];
			sprintf(TEXT2+4,"%s",comRbuf);
			print_lcd(2,TEXT2);
		}	
	}
}


int32_t HID_MainProcess(void)
{

	int32_t i32Ret = 0;
	E_DRVUSB_STATE eUsbState;

	//GPIOD->IEN = (1 << 1);
	NVIC_EnableIRQ(GPCDE_IRQn);

	i32Ret = DrvUSB_Open((void *)DrvUSB_DispatchEvent);
	if(i32Ret != 0)
		return i32Ret;

	DrvUART_EnableInt(UART_PORT0, DRVUART_RDAINT, UART_INT_HANDLE);  



	while(1)
	{
		/* Disable USB-related interrupts. */
		_DRVUSB_ENABLE_MISC_INT(0);

		/* Enable float-detection interrupt. */
		_DRVUSB_ENABLE_FLDET_INT();

		/* Start HID and install the callback functions to handle in/out report */
		HID_Init();

		/* Enable USB-related interrupts. */
		_DRVUSB_ENABLE_MISC_INT(INTEN_WAKEUP | INTEN_WAKEUPEN | INTEN_FLDET | INTEN_USB | INTEN_BUS);

		/* Enter power down to wait USB attached */
		// PowerDown();

		/* Poll and handle USB events. */
		while(1)
		{	

			// show_string(0,10,"dx");
			eUsbState = DrvUSB_GetUsbState();

			if (eUsbState == eDRVUSB_DETACHED)
			{	 
				DBG_PRINTF("USB Detached!\n");
				break;
			}

			if (eUsbState & eDRVUSB_SUSPENDED)
			{	
				/* Enter suspend here */
				// PowerDown();
			}

			if (eUsbState == eDRVUSB_CONFIGURED)
			{	 
				/* Set HID IN report for interrupt IN transfer */

				HID_SetInReport();
			}
		}

		/* Disable USB-related interrupts. */
		_DRVUSB_ENABLE_MISC_INT(0);
	}
}

void DrvUSB_BusSuspendCallback(void * pVoid)
{
	/* Note!! We should not power down or idle in Handler mode */
	DrvUSB_SetUsbState((E_DRVUSB_STATE)((uint32_t)DrvUSB_GetUsbState() | (uint32_t)eDRVUSB_SUSPENDED));
}

void DrvUSB_BusResumeCallback(void * pVoid)
{
	DrvUSB_SetUsbState((E_DRVUSB_STATE)((uint32_t)DrvUSB_GetUsbState() & (uint32_t)(~eDRVUSB_SUSPENDED)));
}

