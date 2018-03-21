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


#define DBG_PRINTF  printf
extern int IsDebugFifoEmpty(void);

#define HID_MOUSE       2
#define HID_KEYBOARD    1

/* Define the vendor id and product id */
#define USB_VID         0x0416
#define USB_PID        	(0xC140 + HID_MOUSE)


#define HID_DEFAULT_INT_IN_INTERVAL 20
#define HID_IS_SELF_POWERED         0
#define HID_IS_REMOTE_WAKEUP        1
#define HID_MAX_POWER               50      /* The unit is in 2mA. ex: 50 * 2mA = 100mA */

# define HID_REPORT_SIZE_MS         3       /* The unit is in bytes */
# define HID_REPORT_SIZE_KB         9       /* The unit is in bytes */

#define LEN_CONFIG_AND_SUBORDINATE (LEN_CONFIG+LEN_INTERFACE*2+LEN_HID*2+LEN_ENDPOINT*2)

volatile uint8_t comRbuf[16] ={0x30};
volatile uint8_t comRbuf2[16] ={0x30};
volatile uint8_t comRbuf3[16]={0x30};
volatile uint8_t comRbufk[16]={0x30};
char TEXT1[16] = "TX: sending...  ";
char TEXT2[16] = "RX:        ";
volatile uint16_t comRbytes = 0;
volatile uint16_t comRhead 	= 0;
volatile uint16_t comRtail 	= 0;   

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

/* Mouse Report Descriptor */
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
   
#define HID_MOUSE_REPORT_DESCRIPTOR_SIZE \
    sizeof (g_HID_au8MouseReportDescriptor) / sizeof(g_HID_au8MouseReportDescriptor[0])
const uint32_t g_HID_u32MouseReportDescriptorSize = HID_MOUSE_REPORT_DESCRIPTOR_SIZE;

uint8_t g_au8MouseReport[HID_REPORT_SIZE_MS];
const uint32_t g_u32MouseReportSize = sizeof(g_au8MouseReport) / sizeof(g_au8MouseReport[0]);


/* Keyboard Descriptor */
// const uint8_t g_HID_au8KeyboardReportDescriptor[] = {
//       0x05, 0x01,
//       0x09, 0x06,
//       0xA1, 0x01,
//       0x05, 0x07,
//       0x19, 0xE0,
//       0x29, 0xE7,
//       0x15, 0x00,
//       0x25, 0x01,
//       0x75, 0x01,
//       0x95, 0x08,
//       0x81, 0x02,
//       0x95, 0x01,
//       0x75, 0x08,
//       0x81, 0x01,
//       0x95, 0x05,
//       0x75, 0x01,
//       0x05, 0x08,
//       0x19, 0x01,
//       0x29, 0x05,
//       0x91, 0x02,
//       0x95, 0x01,
//       0x75, 0x03,
//       0x91, 0x01,
//       0x95, 0x06,
//       0x75, 0x08,
//       0x15, 0x00,
//       0x25, 0x65,
//       0x05, 0x07,
//       0x19, 0x00,
//       0x29, 0x65,
//       0x81, 0x00,
//       0xC0  
// };

const uint8_t g_HID_au8KeyboardReportDescriptor[] =
{
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

uint8_t g_au8KeyboardReport[HID_REPORT_SIZE_KB];
const uint32_t g_u32KeyboardReportSize = sizeof(g_au8KeyboardReport) / sizeof(g_au8KeyboardReport[0]);


const uint8_t g_HID_au8DeviceDescriptor[] =
{
    LEN_DEVICE,     // bLength
    DESC_DEVICE,    // bDescriptorType
    0x10, 0x01,     // bcdUSB
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
    0x02,           // bNumInterfaces
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
    HID_MOUSE,      // bInterfaceProtocol
    0x00,           // iInterface

    // HID Descriptor
    LEN_HID,        // Size of this descriptor in UINT8s.
    DESC_HID,       // HID descriptor type.
    0x10, 0x01,     // HID Class Spec. release number.
    0x00,           // H/W target country.
    0x01,           // Number of HID class descriptors to follow.
    DESC_HID_RPT,   // Dscriptor type.
    // Total length of report descriptor.
    HID_MOUSE_REPORT_DESCRIPTOR_SIZE & 0x00FF,
    (HID_MOUSE_REPORT_DESCRIPTOR_SIZE & 0xFF00) >> 8,

    // EP Descriptor: interrupt in.
    LEN_ENDPOINT,   // bLength
    DESC_ENDPOINT,  // bDescriptorType
    (INT_IN_EP_NUM_MS | EP_INPUT), // bEndpointAddress
    EP_INT,         // bmAttributes
    // wMaxPacketSize
    HID_MAX_PACKET_SIZE_INT_IN & 0x00FF,
    (HID_MAX_PACKET_SIZE_INT_IN & 0xFF00) >> 8,
    HID_DEFAULT_INT_IN_INTERVAL,    // bInterval
		
		// I/F descr: HID
    LEN_INTERFACE,  // bLength
    DESC_INTERFACE, // bDescriptorType
    0x01,           // bInterfaceNumber
    0x00,           // bAlternateSetting
    0x01,           // bNumEndpoints
    0x03,           // bInterfaceClass
    0x01,           // bInterfaceSubClass
    HID_KEYBOARD,   // bInterfaceProtocol
    0x00,           // iInterface

    // HID Descriptor
    LEN_HID,        // Size of this descriptor in UINT8s.
    DESC_HID,       // HID descriptor type.
    0x10, 0x01,     // HID Class Spec. release number.
    0x00,           // H/W target country.
    0x01,           // Number of HID class descriptors to follow.
    DESC_HID_RPT,   // Dscriptor type.
    // Total length of report descriptor.
    HID_KEYBOARD_REPORT_DESCRIPTOR_SIZE & 0x00FF,
    (HID_KEYBOARD_REPORT_DESCRIPTOR_SIZE & 0xFF00) >> 8,

    // EP Descriptor: interrupt in.
    LEN_ENDPOINT,   // bLength
    DESC_ENDPOINT,  // bDescriptorType
    (INT_IN_EP_NUM_KB | EP_INPUT), // bEndpointAddress
    EP_INT,         // bmAttributes
    // wMaxPacketSize
    HID_MAX_PACKET_SIZE_INT_IN & 0x00FF,
    (HID_MAX_PACKET_SIZE_INT_IN & 0xFF00) >> 8,
    HID_DEFAULT_INT_IN_INTERVAL,     // bInterval
		
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

void show_string(unsigned char x, unsigned char y, char *str)
{
	int i=y;
	do{
		Show_Word(x,i,*str++); //display a character at (x,i)
		i++; // next character
		if(i>15) break; // max 16 character
	} while(*str!='\0'); //
}

void delay_time(int time)		     // 1 secend
{
	int i=0;
	for(i=0;i<time*30;i++)
	{
		DrvSYS_Delay(330000);
	}
}

void HID_Init(void)
{
    /* Open HID to initial the descriptors and control handlers */
    HID_Open();
    
    /* Set the HID report descriptor */
    HID_SetReportDescriptor_MS(g_HID_au8MouseReportDescriptor, g_HID_u32MouseReportDescriptorSize);

    /* Set the HID report buffer */
    HID_SetReportBuf_MS(g_au8MouseReport, g_u32MouseReportSize);

    /* Set the HID report descriptor */
    HID_SetReportDescriptor_KB(g_HID_au8KeyboardReportDescriptor, g_HID_u32KeyboardReportDescriptorSize);

    /* Set the HID report buffer */
    HID_SetReportBuf_KB(g_au8KeyboardReport, g_u32KeyboardReportSize);

}


/* This function is used to prepare IN report. The g_HID_sDevice.u32ReportSize will be reset when it has been done. */
void HID_SetInReport_MS()
{
	uint8_t *buf;
	
  /* To check if previous report data is processed or not */
  if(g_HID_sDevice.isReportReady_MS)
        return;

  buf = g_HID_sDevice.pu8Report_MS;

  buf[0] = atoi(comRbuf3); 														   //s10t8a0  ¤W,¥k,®×¥ó
	buf[1] = -atoi(comRbuf); 
	buf[2] = -atoi(comRbuf2); 
	comRbuf[0]=0;
	comRbuf[1]=0;
	comRbuf[2]=0;
	comRbuf2[0]=0;
	comRbuf2[1]=0;
	comRbuf2[2]=0;
  g_HID_sDevice.isReportReady_MS = 1; /* To note the report data has been updated */
  DrvUSB_DataIn(INT_IN_EP_NUM_MS, g_HID_sDevice.pu8Report_MS, g_HID_sDevice.u32ReportSize_MS);
}

void HID_SetInReport_KB()
{
	uint8_t *buf;
	uint8_t bInChar[10] = {0xFF};
	int32_t i;
	/* To check if previous report data is processed or not */
	if(g_HID_sDevice.isReportReady_KB)
			return;              
	buf = g_HID_sDevice.pu8Report_KB;

	// Key A    2c =space      4f,50,51,52			  w=1a s=16 a=04 d=07
	
	/*sprintf(TEXT2,"s%c%c ",comRbuf[0],comRbuf[1]);		
	show_string(1,10,TEXT2);*/
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
		g_HID_sDevice.isReportReady_KB = 1;
		DrvUSB_DataIn(INT_IN_EP_NUM_KB, g_HID_sDevice.pu8Report_KB, g_HID_sDevice.u32ReportSize_KB);  
}


/* This function is used to prepare the first out report. */
void HID_SetFirstOutReport(void)
{
		
}

/* This function is used to prepare the first in report */
void HID_SetFirstInReport(void)
{
    DrvUSB_DataIn(INT_IN_EP_NUM_MS, g_HID_sDevice.pu8Report_MS, g_HID_sDevice.u32ReportSize_MS);
		DrvUSB_DataIn(INT_IN_EP_NUM_KB, g_HID_sDevice.pu8Report_KB, g_HID_sDevice.u32ReportSize_KB);
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
		}
	}
}

int32_t HID_MainProcess(void)
{
    int32_t i32Ret = 0;
    E_DRVUSB_STATE eUsbState;

    DBG_PRINTF("Use GPD1 to move mouse pointer right when GPD1 = 0\n");
    DBG_PRINTF("Use GPD2 to report a key 'a' when GPD2 = 0\n");
    
    GPIOD->IEN = (3 << 1);
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
      	 PowerDown();
        /* Poll and handle USB events. */
        while(1)
        {
            eUsbState = DrvUSB_GetUsbState();

            if (eUsbState == eDRVUSB_DETACHED)
            {
                DBG_PRINTF("USB Detached!\n");
                break;
            }
            
            if (eUsbState & eDRVUSB_SUSPENDED)
            {
                /* Enter suspend here */
								PowerDown();
            }
            
            if (eUsbState == eDRVUSB_CONFIGURED)
            {
                /* Set HID IN report for interrupt IN transfer */
                //HID_SetInReport_MS();
								HID_SetInReport_KB();
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

