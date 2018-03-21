/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright (c) Nuvoton Technology Corp. All rights reserved.                                             */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include "DrvUSB.h"
#include "VCOM_API.h"

//#define DBG_PRINTF      printf
//#define DBG_PRINTF(...)


uint32_t CFG_EP_SETTING[6] = {
CFG_CSTALL | CTRL_EP_NUM | CFG_EPT_IN,
CFG_CSTALL | CTRL_EP_NUM | CFG_EPT_OUT,
BULK_IN_EP_NUM | CFG_EPT_IN,
BULK_OUT_EP_NUM | CFG_EPT_OUT,
INT_IN_EP_NUM | CFG_EPT_IN,
CFG_CSTALL
};


S_DRVUSB_DEVICE gsUsbDevice ={
    0,//uint32_t u32ATTR;
    0,//uint32_t u32EPSTS;
    0,//uint32_t u32FLDET;
    0,//uint32_t u32INTSTS;

    {0, 0, 0, 0, 0, 0},//int32_t  abData0[MAX_EP_ID];  /* Index is EP identity   */
    
    {
        {CTRL_EP_NUM | EP_INPUT, VCOM_CTRL_MXPLD, (uint8_t *)USB_SRAM_BASE + 8},
        {CTRL_EP_NUM | EP_OUTPUT, VCOM_CTRL_MXPLD, (uint8_t *)USB_SRAM_BASE + 8 + VCOM_CTRL_MXPLD},
        {BULK_IN_EP_NUM | EP_INPUT, VCOM_CTRL_MXPLD, (uint8_t *)USB_SRAM_BASE + 8 + VCOM_CTRL_MXPLD*2},
        {BULK_OUT_EP_NUM| EP_OUTPUT, BULK_EP_MXPLD, (uint8_t *)USB_SRAM_BASE + 8 + VCOM_CTRL_MXPLD*2 + BULK_EP_MXPLD},
        {INT_IN_EP_NUM | EP_INPUT, BULK_EP_MXPLD, (uint8_t *)USB_SRAM_BASE + 8 + VCOM_CTRL_MXPLD*2 + BULK_EP_MXPLD*2},
        {0, 0, 0},
        {0, 8, (uint8_t *)USB_SRAM_BASE},
    }, //S_DRVUSB_EP_CTRL sEpCrl[MAX_EP_ID+1];
    
    eDRVUSB_DETACHED, //int32_t  eUsbState;           /* E_DRVUSB_STATE */

    0, //uint16_t u16MiscEventFlags;

    0, //uint16_t u16EPEventFlags;

    0, //S_DRVUSB_CTRL_CALLBACK_ENTRY    *pCtrlCallback;
    0, //uint32_t CtrlCallbackSize;
    {0}, //uint8_t  au8Setup[8];
    0, //uint8_t  u8UsbAddress;
    0, //uint8_t  u8UsbConfiguration;
        
    0, //int32_t  bSelfPowered;  /* TRUE : Self-Powereded, FALSE : BUS-Powereded */
    0, //int32_t  bRemoteWakeup; /* TRUE : Support RemoteWakeup, FALSE : no support */
    0, //uint8_t  u8MaxPower;        /* Maximum MaxPower 0xFA (500mA), default is 0x32 (100mA) */
        
    0, //S_DRVUSB_CLASS *psUsbClass;
};


E_DRVUSB_STATE eUsbState;

#define Maximum(a,b)		((a)>(b) ? (a) : (b))
//#define Minimum(a,b)		((a)<(b) ? (a) : (b))
int32_t Minimum(int32_t a, int32_t b)
{
    return ((a < b) ? a:b);
}

#define USB_VID			0x0416  /* Vendor ID */
#define USB_PID			0x5011  /* Product ID */


#define RXBUFSIZE			256 /* RX buffer size */  
#define TXBUFSIZE			256 /* RX buffer size */  
#define TX_FIFO_SIZE		64	/* TX Hardware FIFO size */         


/*---------------------------------------------------------------------------------------------------------*/
/* Global variables                                                                                        */
/*---------------------------------------------------------------------------------------------------------*/

/* To handle the data transfer size > maximum packet size */
static uint16_t gu16TransferLen = 0;
static uint16_t gu16TransferIndex = 0;


uint32_t USB_RX_Flag = 1;
uint32_t USB_TX_Flag;

#if 1
uint8_t Data_Tx_Buf[BULK_EP_MXPLD];
uint8_t Data_Rx_Buf[BULK_EP_MXPLD];
#else
uint8_t* Data_Tx_Buf = (uint8_t*)0x20000A00;
uint8_t* Data_Rx_Buf = (uint8_t*)0x20000900;
#endif

uint8_t USB_RX_Index;
uint8_t USB_TX_Index;

uint8_t *gpu8RxBuf = 0;
uint32_t gu32TxSize = 0;



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
	0x00, 0x03,		/* bcdDevice            */
	0x01,			/* iManufacture         */
	0x02,			/* iProduct             */
	0x03,			/* iSerialNumber        */
	0x01			/* bNumConfigurations   */
};


const uint8_t gau8ConfigDescriptor[] =
{
	LEN_CONFIG,		/* bLength              */
	DESC_CONFIG,	/* bDescriptorType      */
	0x43 - 5, 0x00,		/* wTotalLength         */
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

#if 0
	/* Communication Class Specified INTERFACE descriptor */
    0x05,           /* Size of the descriptor, in bytes */
    0x24,           /* CS_INTERFACE descriptor type */
    0x00,           /* Header functional descriptor subtype */
    0x10, 0x01,     /* Communication device compliant to the communication spec. ver. 1.10 */
#endif
    
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
	'n', 0 ,
};


const uint8_t gau8ProductStringDescriptor[] = {
	10,             /* bLength          */
	DESC_STRING,    /* bDescriptorType  */
	'V', 0, 
	'C', 0,
	'O', 0,
	'M', 0,
};


const uint8_t gau8StringSerial[] =
{
	6,				/* bLength          */
	DESC_STRING,	/* bDescriptorType  */
	'0', 0, 
	'1', 0 
};

const S_DRVUSB_CTRL_CALLBACK_ENTRY g_asCtrlCallbackEntry[] = {
    //request type,command     ,setup ack handler         , in ack handler      ,out ack handler,  parameter
	{REQ_STANDARD, SET_ADDRESS,      DrvUSB_CtrlSetupSetAddress,      DrvUSB_CtrlDataInSetAddress, DrvUSB_CtrlDataOutDefault},
	{REQ_STANDARD, CLEAR_FEATURE,    DrvUSB_CtrlSetupClearSetFeature, DrvUSB_CtrlDataInDefault,    DrvUSB_CtrlDataOutDefault},
	{REQ_STANDARD, SET_FEATURE,      DrvUSB_CtrlSetupClearSetFeature, DrvUSB_CtrlDataInDefault,    DrvUSB_CtrlDataOutDefault},
	{REQ_STANDARD, GET_CONFIGURATION,DrvUSB_CtrlSetupGetConfiguration,DrvUSB_CtrlDataInDefault,    DrvUSB_CtrlDataOutDefault},
	{REQ_STANDARD, GET_STATUS,       DrvUSB_CtrlSetupGetStatus,       DrvUSB_CtrlDataInDefault,    DrvUSB_CtrlDataOutDefault},
	{REQ_STANDARD, GET_INTERFACE,    DrvUSB_CtrlSetupGetInterface,    DrvUSB_CtrlDataInDefault,    DrvUSB_CtrlDataOutDefault},
	{REQ_STANDARD, SET_INTERFACE,    DrvUSB_CtrlDataOutDefault/*DrvUSB_CtrlSetupSetInterface*/,    DrvUSB_CtrlDataInDefault,    DrvUSB_CtrlDataOutDefault},

	{REQ_STANDARD, GET_DESCRIPTOR,   VCOM_CtrlSetupGetDescriptor,   DrvUSB_CtrlDataInDefault/*VCOM_CtrlDataInGetDescriptor*/,  DrvUSB_CtrlDataOutDefault},
	{REQ_STANDARD, SET_CONFIGURATION,VCOM_CtrlSetupSetConfiguration,DrvUSB_CtrlDataInDefault,      DrvUSB_CtrlDataOutDefault},
	{REQ_CLASS,    SET_LINE_CODE,    VCOM_SetLineCoding,            DrvUSB_CtrlDataInDefault,      VCOM_SetLineCodingDataOut},
    {REQ_CLASS,    GET_LINE_CODE,    VCOM_GetLineCoding,            DrvUSB_CtrlDataInDefault,      DrvUSB_CtrlDataOutDefault}, 
    {REQ_CLASS,    SET_CONTROL_LINE_STATE,VCOM_CtrlLineState,       DrvUSB_CtrlDataInDefault,      DrvUSB_CtrlDataOutDefault}
};


typedef struct { 
    uint32_t  u32DTERate;     /* Baud rate    */
    uint8_t   u8CharFormat;   /* stop bit     */
    uint8_t   u8ParityType;   /* parity       */
    uint8_t   u8DataBits;     /* data bits    */
} S_VCOM_LINE_CODING;

//S_VCOM_LINE_CODING gLineCoding = {0};
S_VCOM_LINE_CODING gLineCoding = {115200, 0, 0, 8};    /* Baud rate : 115200    */
													   /* Stop bit     */
       												   /* parity       */
  													   /* data bits    */
#if 0
/* Endpoint description */
S_DRVUSB_EP_CTRL sEpDescription[] = 
{
	{CTRL_EP_NUM    | EP_INPUT,  VCOM_CTRL_MXPLD, NULL},
	{CTRL_EP_NUM    | EP_OUTPUT, VCOM_CTRL_MXPLD, NULL},
	{BULK_IN_EP_NUM | EP_INPUT,  BULK_EP_MXPLD, NULL},
	{BULK_OUT_EP_NUM| EP_OUTPUT, BULK_EP_MXPLD, NULL},
	{INT_IN_EP_NUM  | EP_INPUT,  INT_EP_MXPLD, NULL},
	{0,0,0}
};
#endif

#if 0
/* Bus event call back functions */
S_DRVUSB_EVENT_PROCESS g_sBusOps[6] = 
{
	{NULL},                               /* Attach event callback        */
	{NULL},                               /* Detach event callback        */
	{DrvUSB_BusResetCallback},   /* Bus reset event callback     */
	{NULL},                               /* Bus suspend event callback   */
	{NULL},                               /* Bus resume event callback    */
	{DrvUSB_CtrlSetupAck},       /* Setup event callback         */
};

#endif

/* USB event call back functions */
S_DRVUSB_EVENT_PROCESS g_sUsbOps[12 - 5] = 
{
	{DrvUSB_CtrlDataInAck},
	{DrvUSB_CtrlDataOutAck},
	
	{VCOM_BulkInAckCallback},
	{NULL},

	{NULL},
	{VCOM_BulkOutAckCallback},

	{VCOM_IntInAckCallback},
};

//-------------------------------------------------------------------------------VCOM_BulkOutAckCallback
void VCOM_BulkOutAckCallback(void* pVoid)
{	
	uint8_t i;

	gpu8RxBuf = DrvUSB_GetOutData(BULK_OUT_EP_NUM,&USB_TX_Flag);
	for (i = 0;i < USB_TX_Flag;i++)
	{
	 	Data_Tx_Buf[i] =  gpu8RxBuf[i];
	}
	USB_TX_Index = 0; 	
//	DrvUSB_DataOutTrigger(BULK_OUT_EP_NUM, BULK_EP_MXPLD);
}
void VCOM_Tx_data(void)
{
	uint32_t u32Temp;
	if(USB_TX_Flag)
	{
		for(u32Temp=0;u32Temp<USB_TX_Flag;u32Temp++)
		{
			if(UART0->FSR.TX_FULL)
				break;	
 			UART0->DATA = Data_Tx_Buf[USB_TX_Index++];
		}
 		USB_TX_Flag -= u32Temp;
		if(USB_TX_Flag == 0)
			DrvUSB_DataOutTrigger(BULK_OUT_EP_NUM, BULK_EP_MXPLD);
	}
}
//-------------------------------------------------------------------------------VCOM_BulkInAckCallback
void VCOM_BulkInAckCallback(void* pVoid)
{
	// Reserved for user define 
	USB_RX_Flag = 1;
}

uint8_t u8Tempchar[2]={0x11,0x11};
void VCOM_Rx_data(void)
{

	while(UART0->FSR.RX_EMPTY == 0)					    /* Check RX empty => failed */
	{
		if(USB_RX_Index >= BULK_EP_MXPLD)
			break;											  
			   	
	    Data_Rx_Buf[USB_RX_Index++] = UART0->DATA;
	}

	if(USB_RX_Index)
	{

		if(USB_RX_Flag)
		{
			USB_RX_Flag = 0;
			DrvUSB_DataIn(BULK_IN_EP_NUM, Data_Rx_Buf, USB_RX_Index);
 			USB_RX_Index = 0;
		}
	}
}

void VCOM_IntInAckCallback(void* pVoid)
{
    uint8_t au8Buf[8] = {0};
    // Reserved for user define 
    DrvUSB_DataIn(INT_IN_EP_NUM, au8Buf, INT_EP_MXPLD);
}

void VCOM_MainProcess(void)
{	
  DrvUSB_Open(0);
  while(1)
  {			
    // Disable USB-related interrupts. 
    _DRVUSB_ENABLE_MISC_INT(0);
	
    // Enable float-detection interrupt. 
    _DRVUSB_ENABLE_FLDET_INT();
	 
    // Wait for USB connected.
    while (1)
    {
      // Order here is significant.
      // Give a chance to handle remaining events before exiting this loop.
      eUsbState = (E_DRVUSB_STATE)gsUsbDevice.eUsbState;
	
      //DrvUSB_DispatchEvent();
      if (eUsbState >= eDRVUSB_ATTACHED && eUsbState != eDRVUSB_SUSPENDED)
      {
        break;
       }
    }
	         
    _DRVUSB_ENABLE_MISC_INT(INTEN_WAKEUP | INTEN_WAKEUPEN | INTEN_FLDET | INTEN_USB | INTEN_BUS);
	
    // Poll and handle USB events.
    while(1)
    {
      eUsbState = (E_DRVUSB_STATE)gsUsbDevice.eUsbState;
      if (eUsbState == eDRVUSB_DETACHED)
        break;
      else if (eUsbState == eDRVUSB_CONFIGURED)
      {
	  	__disable_irq();
        VCOM_Tx_data();
        VCOM_Rx_data();
		__enable_irq();
      }
    }
  }
}


void VCOM_CtrlSetupGetDescriptor(void* pVoid)
{

	uint16_t u16Len, u16ConfigLen;
	
	u16Len = 0;
	u16Len = gsUsbDevice.au8Setup[7];
	u16Len <<= 8;
	u16Len += gsUsbDevice.au8Setup[6];
	
	switch (gsUsbDevice.au8Setup[3])
	{
		// Get Device Descriptor 
		case DESC_DEVICE:
		{
			u16Len = Minimum(u16Len, LEN_DEVICE);

			DrvUSB_DataIn(0, gau8DeviceDescriptor, u16Len);

            //DBG_PRINTF("Get device descriptor, %d\n", u16Len);

			break;
		}

		// Get Configuration Descriptor	
		case DESC_CONFIG:
		{
			u16ConfigLen = gau8ConfigDescriptor[3];
			u16ConfigLen <<= 8;
			u16ConfigLen |= gau8ConfigDescriptor[2];

            //DBG_PRINTF("Get config: %d\n", u16Len);

			gu16TransferLen = Minimum(u16Len, u16ConfigLen);

#if 1
			DrvUSB_DataIn(0,gau8ConfigDescriptor, gu16TransferLen);
#else
			gu16PacketLen = VCOM_CTRL_MXPLD;
			gu16TransferIndex = 0;		

			if(gu16TransferLen > gu16PacketLen)
			{
				DrvUSB_DataIn(0,gau8ConfigDescriptor + gu16TransferIndex, gu16PacketLen);
				gu16TransferLen -= gu16PacketLen;
				gu16TransferIndex += gu16PacketLen;

                //DBG_PRINTF("Get config data in, %d\n", gu16PacketLen);
			}
			else
			{
				DrvUSB_DataIn(0,gau8ConfigDescriptor + gu16TransferIndex, gu16TransferLen);
				gu16TransferLen = 0;

                //DBG_PRINTF("Get config data in, %d\n", gu16TransferLen);
			}
#endif

			break;
		}

		// Get String Descriptor 
		case DESC_STRING:
		{
			// Get Language	
			if (gsUsbDevice.au8Setup[2] == 0)
			{
				u16Len = Minimum(u16Len, 4);
				DrvUSB_DataIn(0, gau8StringLang, u16Len);
			}
			else
			{
				// Get String Descriptor 
				switch (gsUsbDevice.au8Setup[2])
				{
					case 1:
						u16Len = Minimum(u16Len, gau8VendorStringDescriptor[0]);
				
						DrvUSB_DataIn(0, (const uint8_t *)gau8VendorStringDescriptor, u16Len);
						break;
					case 2:
						u16Len = Minimum(u16Len,gau8ProductStringDescriptor[0]);
						DrvUSB_DataIn(0, (const uint8_t *)gau8ProductStringDescriptor, u16Len);
						break;
					case 3:
						u16Len = Minimum(u16Len, gau8StringSerial[0]);
						DrvUSB_DataIn(0, gau8StringSerial, u16Len);
						break;
					default:
						// Not support. Reply STALL.
						DrvUSB_ClrCtrlReadyAndTrigStall();
				}
			}
			break;
		}
		default:
			// Not support. Reply STALL.
			DrvUSB_ClrCtrlReadyAndTrigStall();
	}
	 

	return;
}

void VCOM_CtrlDataInGetDescriptor(void* pVoid)
{
	
	S_DRVUSB_DEVICE *psDevice = &gsUsbDevice;
	
	if (psDevice->au8Setup[0] & 0x80)
	{
		if(gu16TransferLen)
		{

			DrvUSB_DataIn(0, gau8ConfigDescriptor + gu16TransferIndex, gu16TransferLen);

            //DBG_PRINTF("control data in, %d\n", gu16TransferLen);

			gu16TransferLen = 0;
    		_DRVUSB_TRIG_EP(1, 0x00); //Ctl data out

		}
		else
		{
			_DRVUSB_TRIG_EP(1, 0x00);
		}
	}
	

}
void VCOM_CtrlSetupSetConfiguration(void* pVoid)
{
	
	S_DRVUSB_DEVICE *pDrvDevice = &gsUsbDevice;
//	S_DRVUSB_CLASS *psUsbClass = pDrvDevice->psUsbClass;
	int32_t bIsDeviceConfigure;
	
	//bIsDeviceConfigure = psUsbClass->pfnCompare ? psUsbClass->pfnCompare(pDrvDevice->au8Setup[2]) : 1;
    bIsDeviceConfigure = 1;	
#if 0
	if(pDrvDevice->au8Setup[2] == 0)
	{
		// USB address state.
		//DrvUSB_SetUsbState(eDRVUSB_ADDRESS);
		pDrvDevice->eUsbState = eDRVUSB_ADDRESS;
		pDrvDevice->u8UsbConfiguration = pDrvDevice->au8Setup[2];
		DrvUSB_DataIn(0, NULL, 0);
		
	}
	else
#endif    
    if(bIsDeviceConfigure)
	{
		pDrvDevice->eUsbState = eDRVUSB_CONFIGURED;
		pDrvDevice->u8UsbConfiguration = pDrvDevice->au8Setup[2];

		DrvUSB_DataIn(0, NULL, 0);
		
		// Reset bulk in/out endpoint 
		DrvUSB_DataOutTrigger(BULK_OUT_EP_NUM, 64);
		DrvUSB_DataIn(BULK_IN_EP_NUM, 0, 0);

        //DBG_PRINTF("Set config %d\n", pDrvDevice->au8Setup[2]);
	}
	else
	{
		// Not support. Reply STALL.
		DrvUSB_ClrCtrlReadyAndTrigStall();
	}
	

	
}

void VCOM_SetLineCoding(void* pVoid)
{

    gsUsbDevice.abData0[0] = FALSE;
    DrvUSB_DataOutTrigger(0, 7);
}

void VCOM_SetLineCodingDataOut(void* pVoid)
{
    uint8_t *buf;
    uint32_t size;
    
    buf = DrvUSB_GetOutData(0, &size);
    memcpy((void *)&gLineCoding, buf, size);

	//Set Data length
	UART0->LCR.WLS = gLineCoding.u8DataBits - 5;

	//Set stop bit
	if(gLineCoding.u8CharFormat)//1.5 or 2 stop bit
		UART0->LCR.NSB = 1;
	else 
		UART0->LCR.NSB = 0;// 1 stop bit

	//Set Parity
	if(gLineCoding.u8ParityType == 1)//Odd
	{
		UART0->LCR.PBE = 1;
		UART0->LCR.EPE = 0;
	}
	else if(gLineCoding.u8ParityType == 2)//Even
	{
		UART0->LCR.PBE = 1;
		UART0->LCR.EPE = 1;
	}else//None
	{
		UART0->LCR.PBE = 0;
	}
	//Set baudrate
    outp32(&UART0->BAUD, 0x30000000 | (12000000/gLineCoding.u32DTERate - 2));
    // ACK to end the transfer 
    gsUsbDevice.abData0[0] = FALSE;
    DrvUSB_DataIn(0, 0, 0);

}
void VCOM_GetLineCoding(void* pVoid)
{
    gsUsbDevice.abData0[0] = TRUE;
    DrvUSB_DataIn(0, (void *)&gLineCoding, 7);
}

void VCOM_CtrlLineState(void* pVoid)
{
    DrvUSB_DataIn(0, 0, 0); 
}

int32_t  DrvUSB_Open(void * pVoid)
{

    _DRVUSB_ENABLE_USB();
    _DRVUSB_ENABLE_SE0();   
     SysTimerDelay(10000);
    _DRVUSB_DISABLE_SE0();
    
    gsUsbDevice.u32FLDET = USBD->FLDET.FLDET;
    DrvUSB_PreDispatchFDTEvent(&gsUsbDevice);
    DrvUSB_DispatchMiscEvent(&gsUsbDevice);
    
    NVIC_SetPriority (USBD_IRQn,3);
    NVIC_EnableIRQ(USBD_IRQn);

    return 0;
}


void USBD_IRQHandler(void)
{
     DrvUSB_PreDispatchEvent();
     DrvUSB_DispatchMiscEvent(&gsUsbDevice);
     DrvUSB_DispatchEPEvent(&gsUsbDevice);

//     if (g_FnIntCallBack)
//         g_FnIntCallBack((void *)&gsUsbDevice);
}
void DrvUSB_PreDispatchEvent(void)
{
    gsUsbDevice.u32INTSTS = _DRVUSB_GET_EVENT_FLAG();

    if (gsUsbDevice.u32INTSTS & INTSTS_WAKEUP)
    {
        // Clear wakeup event. write one clear 
        _DRVUSB_SET_EVENT_FLAG(INTSTS_WAKEUP);
        
        // Pre-dispatch wakeup event. 
        //DrvUSB_PreDispatchWakeupEvent(&gsUsbDevice);
    }
    else if (gsUsbDevice.u32INTSTS & INTSTS_FLDET)
    {
        gsUsbDevice.u32FLDET = USBD->FLDET.FLDET;
        // Clear float-detection event. Write one clear 
        _DRVUSB_SET_EVENT_FLAG(INTSTS_FLDET);

        // Pre-dispatch float-detection event. 
        DrvUSB_PreDispatchFDTEvent(&gsUsbDevice);
    }
    else if (gsUsbDevice.u32INTSTS & INTSTS_BUS)
    {
        gsUsbDevice.u32ATTR = *((__IO uint32_t *)(&USBD->ATTR));
        // Clear bus event. Write one clear 
        _DRVUSB_SET_EVENT_FLAG(INTSTS_BUS);
        
        // Pre-dispatch bus event. 
        DrvUSB_PreDispatchBusEvent(&gsUsbDevice);
    }
    else if (gsUsbDevice.u32INTSTS & INTSTS_USB)
    {
        gsUsbDevice.u32EPSTS = _DRVUSB_GET_EPSTS();
        
        // Clear USB events individually instead of in total.  therwise, incoming USB events may be cleared mistakenly.   Pre-dispatch USB event. 
        DrvUSB_PreDispatchEPEvent(&gsUsbDevice);
    }
}

void DrvUSB_CtrlSetupSetAddress(void * pVoid)
{
    E_DRVUSB_STATE eUsbState;
    S_DRVUSB_DEVICE *pInfraDevice = &gsUsbDevice;   

	eUsbState = (E_DRVUSB_STATE)gsUsbDevice.eUsbState;
    if (eUsbState == eDRVUSB_DEFAULT)
    {
        pInfraDevice->u8UsbAddress = pInfraDevice->au8Setup[2];
        _DRVUSB_SET_EP_TOG_BIT(0, 0);
        _DRVUSB_TRIG_EP(0, 0);

		pInfraDevice->eUsbState = eDRVUSB_ADDRESS;

    }
    else
    {
        DrvUSB_ClrCtrlReadyAndTrigStall();
    }
}
void DrvUSB_CtrlDataInSetAddress(void * pVoid)
{
    S_DRVUSB_DEVICE *pInfraDevice = &gsUsbDevice;

    if (pInfraDevice->u8UsbAddress == 0x00)
    {
        //DrvUSB_SetUsbState(eDRVUSB_DEFAULT);
		pInfraDevice->eUsbState = eDRVUSB_DEFAULT;
    }
    else
    {
        //DrvUSB_SetUsbState(eDRVUSB_ADDRESS);
		pInfraDevice->eUsbState = eDRVUSB_ADDRESS;
        _DRVUSB_SET_FADDR(pInfraDevice->u8UsbAddress);
    }   
}
void DrvUSB_CtrlSetupClearSetFeature(void * pVoid)
{
//    uint32_t u32EpId;

#if 1
    DrvUSB_ClrCtrlReadyAndTrigStall();
#else
    // Device 
    if ((gsUsbDevice.au8Setup[0] == 0x00) && (gsUsbDevice.au8Setup[2] == FEATURE_DEVICE_REMOTE_WAKEUP))
    {
        if(gsUsbDevice.au8Setup[1] == SET_FEATURE)
            gsUsbDevice.bRemoteWakeup = 1;
        if(gsUsbDevice.au8Setup[1] == CLEAR_FEATURE)
            gsUsbDevice.bRemoteWakeup = 0;
    }
    // Interface 
    else if (gsUsbDevice.au8Setup[0] == 0x01);
    
    // Endpoint 
    else if ((gsUsbDevice.au8Setup[0] == 0x02) && (gsUsbDevice.au8Setup[2] == FEATURE_ENDPOINT_HALT))
    {
        u32EpId = DrvUSB_GetEpId(gsUsbDevice.au8Setup[4]);
        
        if (u32EpId < MAX_EP_ID)
        {
            _DRVUSB_CLEAR_EP_DSQ_SYNC(u32EpId);
            if (gsUsbDevice.au8Setup[1] == CLEAR_FEATURE)
            {
                _DRVUSB_CLEAR_EP_STALL(u32EpId);
            }
            else
            {
                _DRVUSB_TRIG_EP_STALL(u32EpId);
            }
        }
        else
        {
            DrvUSB_ClrCtrlReadyAndTrigStall();
        }
    }
    else
    {
        DrvUSB_ClrCtrlReadyAndTrigStall();
    }
#endif

    DrvUSB_DataIn(0, NULL, 0);  
}


void DrvUSB_CtrlSetupGetConfiguration(void * pVoid)
{
    DrvUSB_DataIn(0, &gsUsbDevice.u8UsbConfiguration, 1);   
}

void DrvUSB_CtrlSetupGetStatus(void * pVoid)
{
#if 1
    DrvUSB_ClrCtrlReadyAndTrigStall();
#else

    uint8_t au8Buf[4];
    S_DRVUSB_DEVICE *pInfraDevice = &gsUsbDevice;
    uint32_t u32EpId;

    u32EpId = DrvUSB_GetEpId(pInfraDevice->au8Setup[4]);

    if (pInfraDevice->au8Setup[0] == 0x80)
    {
        au8Buf[0] = pInfraDevice->bRemoteWakeup << 1 | pInfraDevice->bSelfPowered;
    }
    else if (pInfraDevice->au8Setup[0] == 0x81)
    {
        au8Buf[0] = 0x00;
    }
    else if (pInfraDevice->au8Setup[0] == 0x82)
    {
        if (u32EpId < MAX_EP_ID)
        {
            /* Interrupt-In Endpoint */
            au8Buf[0] = (_DRVUSB_GET_CFGP(u32EpId) & CFGP_SSTALL) ? 1 : 0;
        }
        else
        {
            DrvUSB_ClrCtrlReadyAndTrigStall();
            return;
        }
    }
    else
    {
        DrvUSB_ClrCtrlReadyAndTrigStall();
        return;
    }
    au8Buf[1] = 0x00;

    DrvUSB_DataIn(0, au8Buf, 2);    
#endif

}
void DrvUSB_CtrlSetupGetInterface(void * pVoid)
{
    const uint8_t c = 0x00;

    DrvUSB_DataIn(0, &c, 1);    
}

#if 0
void DrvUSB_CtrlSetupSetInterface(void * pVoid)
{
    DrvUSB_DataIn(0, NULL, 0);
}
#endif

void DrvUSB_DispatchMiscEvent(S_DRVUSB_DEVICE *psDevice)
{
    uint16_t u16MiscEventFlags;
//    int32_t i;
    
    u16MiscEventFlags = psDevice->u16MiscEventFlags;

#if 1
    if(u16MiscEventFlags & 0x4)
    {
        psDevice->u16MiscEventFlags ^= 0x4;
        DrvUSB_BusResetCallback(0);
    }
    else if(u16MiscEventFlags & 0x20)
    {
        psDevice->u16MiscEventFlags ^= 0x20;
        DrvUSB_CtrlSetupAck(0);
    }

#else
    for (i=0;i<6;i++)
    {
        if (u16MiscEventFlags & (1 << i))
        {           
            psDevice->u16MiscEventFlags &= ~(1 << i);
        
            if (g_sBusOps[i].apfnCallback != 0)
            {
                g_sBusOps[i].apfnCallback(0);
            }
        }
    }
#endif
       
}

void DrvUSB_DispatchEPEvent(S_DRVUSB_DEVICE *psDevice)
{
    int32_t i;
    uint32_t u32Mask;
    
    for (i=0;i<12 - 5;i++)
    {
        u32Mask = 1 << i;
        if (psDevice->u16EPEventFlags & u32Mask)
        {
            psDevice->u16EPEventFlags ^= u32Mask;
            // Call the corresponding event handler. 
            if (g_sUsbOps[i].apfnCallback != 0)
            {
                g_sUsbOps[i].apfnCallback(0);
            }       
        }
    }
}

void WordsCpy(void *dest, void *src, int32_t size)
{
    uint8_t *pu8Src, *pu8Dest;
    int32_t i;
    
    pu8Dest = (uint8_t *)dest;
    pu8Src  = (uint8_t *)src;
    
    for(i=0;i<size;i++)
        pu8Dest[i] = pu8Src[i]; 
}

int32_t DrvUSB_DataIn(uint32_t u32EpNum, const uint8_t * u8Buffer, uint32_t u32Size)
{
    
    uint32_t u32EpId;
    
    u32EpId = DrvUSB_GetEpIdentity(u32EpNum, EP_INPUT);

#if 0    
    if (u32Size > gsUsbDevice.sEpCrl[u32EpId].u32MaxPacketSize)
        return E_DRVUSB_SIZE_TOO_LONG;
#endif
    
    if (u8Buffer && u32Size)
    {
        WordsCpy(gsUsbDevice.sEpCrl[u32EpId].u8SramBuffer, (void *)u8Buffer, u32Size);
    }
    
    _DRVUSB_SET_EP_BUF(u32EpId, (uint32_t)gsUsbDevice.sEpCrl[u32EpId].u8SramBuffer);
    
    if (u8Buffer == NULL && u32Size == 0)
        gsUsbDevice.abData0[u32EpId] = 0;
    else
        gsUsbDevice.abData0[u32EpId] = !gsUsbDevice.abData0[u32EpId];
    
    _DRVUSB_SET_EP_TOG_BIT(u32EpId, gsUsbDevice.abData0[u32EpId]);
    _DRVUSB_TRIG_EP(u32EpId, u32Size);
    
    return 0;
}


void DrvUSB_BusResetCallback(void * pVoid)
{
   S_DRVUSB_DEVICE *psDevice =&gsUsbDevice;
    int i;

    // Reset function address of USB device 
    _DRVUSB_SET_FADDR(0x00);
    
    // Disable IN NAK Flag 
    USBD->INTEN.INNAK_EN = 0;

    _DRVUSB_SET_SETUP_BUF(USB_BUF_SETUP);
    
    // Initial USB EP according to EP description     
    i = 0;
    //while (sEpDescription[i].u32MaxPacketSize != 0)
    while (gsUsbDevice.sEpCrl[i].u32MaxPacketSize != 0)
    {
        // Write one to clear IN/OUT ready flag 
        _DRVUSB_SET_CFGP(i, 0x01);
        
        // Initial USB EP CFG Setting 
        _DRVUSB_SET_CFG(i, CFG_EP_SETTING[i]);
        _DRVUSB_SET_EP_BUF(i, (uint32_t)gsUsbDevice.sEpCrl[i].u8SramBuffer);
        i++;
    }

    psDevice->u8UsbAddress = 0;
    psDevice->u8UsbConfiguration = 0;
    /*
    if(psDevice->eUsbState > eDRVUSB_DEFAULT)
        psDevice->eUsbState = eDRVUSB_DEFAULT;*/
}
void DrvUSB_ClrCtrlReady(void)
{
    uint32_t u32EpId;

    u32EpId = DrvUSB_GetEpIdentity(0, EP_OUTPUT);
    _DRVUSB_CLEAR_EP_READY(u32EpId);
    u32EpId = DrvUSB_GetEpIdentity(0, EP_INPUT);
    _DRVUSB_CLEAR_EP_READY(u32EpId);
}

void DrvUSB_ClrCtrlReadyAndTrigStall(void)
{
    uint32_t u32EpId;

    u32EpId = DrvUSB_GetEpIdentity(0, EP_OUTPUT);
    _DRVUSB_CLEAR_EP_READY_AND_TRIG_STALL(u32EpId);
    u32EpId = DrvUSB_GetEpIdentity(0, EP_INPUT);
    _DRVUSB_CLEAR_EP_READY_AND_TRIG_STALL(u32EpId);
}

void DrvUSB_CtrlSetupAck(void * pArgu)
{
    uint32_t i;
    E_DRVUSB_STATE eUsbState;
    const S_DRVUSB_CTRL_CALLBACK_ENTRY *psEntry;
    S_DRVUSB_DEVICE *psDevice = &gsUsbDevice;
    uint8_t * SetupBuffer;
    volatile int32_t u32Delay;

    DrvUSB_ClrCtrlReady();
    

    // check if after DEFAULT state (RESET) 
    //eUsbState = DrvUSB_GetUsbState();
	eUsbState = (E_DRVUSB_STATE)gsUsbDevice.eUsbState;
    if (eUsbState < eDRVUSB_DEFAULT)
    {
        DrvUSB_ClrCtrlReadyAndTrigStall();
        return;
    }
    SetupBuffer = (uint8_t *)DrvUSB_GetSetupBuffer();
    
    //u32Delay = 0x1000;
    //while(u32Delay--);


#if 0
    //DBG_PRINTF("setup: ");
    for (i = 0; i < 8; i++)
    {
        psDevice->au8Setup[i] = SetupBuffer[i];
        //DBG_PRINTF("%02x ", SetupBuffer[i]);
    }
    //DBG_PRINTF("\n");
#else
    WordsCpy(psDevice->au8Setup, SetupBuffer, 8);
#endif
    
    //u32Delay = 0x1000;
    //while(u32Delay--);

    for (i = 0; i < sizeof(g_asCtrlCallbackEntry) / sizeof(g_asCtrlCallbackEntry[0]); i++)
    {
        psEntry = &g_asCtrlCallbackEntry[i];
        if (psEntry->u8RequestType == (psDevice->au8Setup[0] & 0x60) &&
            psEntry->u8Request == psDevice->au8Setup[1])
        {
            psEntry->pfnCtrlSetupCallback(0);
            return;
        }
    }

    DrvUSB_ClrCtrlReadyAndTrigStall();

    return;
}

void DrvUSB_CtrlDataInAck(void * pArgu)
{
    uint32_t i;
    const S_DRVUSB_CTRL_CALLBACK_ENTRY *psEntry;
    //S_DRVUSB_DEVICE *psDevice = &gsUsbDevice;

    for (i = 0; i < sizeof(g_asCtrlCallbackEntry) / sizeof(g_asCtrlCallbackEntry[0]); i++)
    {
        psEntry = &g_asCtrlCallbackEntry[i];
        if (psEntry->u8RequestType == (gsUsbDevice.au8Setup[0] & 0x60) &&
            psEntry->u8Request == gsUsbDevice.au8Setup[1])
        {
            psEntry->pfnCtrlDataInCallback(0);
            return;
        }
    }
    return;
}



void DrvUSB_CtrlDataOutAck(void * pArgu)
{
    uint32_t i;
    const S_DRVUSB_CTRL_CALLBACK_ENTRY *psEntry;
//    S_DRVUSB_DEVICE *psDevice = &gsUsbDevice;

    for (i = 0; i < sizeof(g_asCtrlCallbackEntry) / sizeof(g_asCtrlCallbackEntry[0]); i++)
    {
        psEntry = &g_asCtrlCallbackEntry[i];
        if (psEntry->u8RequestType == (gsUsbDevice.au8Setup[0] & 0x60) &&
            psEntry->u8Request == gsUsbDevice.au8Setup[1])
        {
            psEntry->pfnCtrlDataOutCallback(0);
            return;
        }
    }
    return;
}
/*
E_DRVUSB_STATE DrvUSB_GetUsbState(void)
{
    return (E_DRVUSB_STATE)gsUsbDevice.eUsbState;
}
  */
/*void DrvUSB_SetUsbState(E_DRVUSB_STATE eUsbState)
{
    gsUsbDevice.eUsbState = eUsbState;
} */


int32_t DrvUSB_GetEpIdentity(uint32_t u32EpNum, uint32_t u32EpAttr)
{
    uint32_t i;

    //S_DRVUSB_DEVICE *psDevice =&gsUsbDevice;
	
    for ( i = 0; i < MAX_EP_ID; i++)
    {
        if (gsUsbDevice.sEpCrl[i].u32EpNum == (u32EpNum | u32EpAttr))
            return i;
    }

	    
    return E_DRVUSB_INVALID_EP_NUM;

}
int32_t DrvUSB_GetEpId(uint32_t u32EpNum)
{
    uint32_t i;
    //S_DRVUSB_DEVICE *psDevice =&gsUsbDevice;
    
    for ( i = 0; i < MAX_EP_ID; i++)
    {
        if (gsUsbDevice.sEpCrl[i].u32EpNum == u32EpNum)
            return i;
    }
    
    return E_DRVUSB_INVALID_EP_NUM; 
}


int32_t DrvUSB_DataOutTrigger(uint32_t u32EpNum, uint32_t u32Size)
{
    uint32_t u32EpId;
    //S_DRVUSB_DEVICE *psDevice = &gsUsbDevice;
    
    u32EpId = DrvUSB_GetEpIdentity(u32EpNum, EP_OUTPUT);

#if 0    
    if (u32Size > gsUsbDevice.sEpCrl[u32EpId].u32MaxPacketSize)
        return E_DRVUSB_SIZE_TOO_LONG;
#endif
        
    _DRVUSB_SET_EP_BUF(u32EpId, (uint32_t)gsUsbDevice.sEpCrl[u32EpId].u8SramBuffer);
    _DRVUSB_TRIG_EP(u32EpId, u32Size);
    
    return 0;
}


uint8_t * DrvUSB_GetOutData(uint32_t u32EpNum, uint32_t *u32Size)
{
    uint32_t u32EpId;
    
    u32EpId = DrvUSB_GetEpIdentity(u32EpNum, EP_OUTPUT);
    *u32Size = _DRVUSB_GET_EP_DATA_SIZE(u32EpId);

    //return sEpDescription[u32EpId].u8SramBuffer;
    return gsUsbDevice.sEpCrl[u32EpId].u8SramBuffer;
}

void DrvUSB_CtrlDataInDefault(void * pVoid)
{
    //S_DRVUSB_DEVICE *pInfraDevice = &gsUsbDevice;
    
    //if (gsUsbDevice.au8Setup[0] & 0x80)
    //{
        _DRVUSB_TRIG_EP(1, 0x00);
    //}
}

void DrvUSB_CtrlDataOutDefault(void * pVoid)
{
    //S_DRVUSB_DEVICE *pInfraDevice = &gsUsbDevice;
    
    //if ((gsUsbDevice.au8Setup[0] & 0x80) == 0)
    //{
        DrvUSB_DataIn(0, NULL, 0);
    //}
}

uint32_t DrvUSB_GetSetupBuffer(void)
{
    return (uint32_t)gsUsbDevice.sEpCrl[MAX_EP_ID].u8SramBuffer;
}

void DrvUSB_PreDispatchFDTEvent(S_DRVUSB_DEVICE *psDevice)
{
    // Clear all pending events on USB attach/detach to handle the scenario that the time sequence of event happening is different from that of event handling. 
    uint32_t u32FLDET = psDevice->u32FLDET;
    
    if(u32FLDET & 1)
    {
        // attached 
        if (psDevice->eUsbState == eDRVUSB_DETACHED)
        {
            psDevice->eUsbState = eDRVUSB_ATTACHED;
            _DRVUSB_ENABLE_USB();
        }

        psDevice->u16MiscEventFlags |= DRVUSB_EVENT_FLAG_ATTACH;
    }
    else
    {
        // detached 
        psDevice->eUsbState = eDRVUSB_DETACHED;
        _DRVUSB_DISABLE_USB();

        psDevice->u16MiscEventFlags |= DRVUSB_EVENT_FLAG_DETACH;
    }

}
void DrvUSB_PreDispatchBusEvent(S_DRVUSB_DEVICE *psDevice)
{
    uint32_t u32Attr = psDevice->u32ATTR;

#if 0
    if (psDevice->eUsbState == eDRVUSB_DETACHED)
    {
        // There should be something wrong 
        return;
    }
#endif

    if (u32Attr & ATTR_USBRST)
    {
        // Bus reset 
        _DRVUSB_ENABLE_USB(); // enable USB & PHY 
        psDevice->eUsbState = eDRVUSB_DEFAULT;
        psDevice->u16MiscEventFlags |= DRVUSB_EVENT_FLAG_BUS_RESET;
    }
    else if (u32Attr & ATTR_SUSPEND)
    {
        // Bus suspend 
        _DRVUSB_DISABLE_PHY(); // disable PHY 
        if (psDevice->eUsbState >= eDRVUSB_ATTACHED)
        {
            psDevice->eUsbState |= eDRVUSB_SUSPENDED;
        }
        psDevice->u16MiscEventFlags |= DRVUSB_EVENT_FLAG_BUS_SUSPEND;

    }
    else if (u32Attr & ATTR_RESUME)
    {
        // Bus resume 
        _DRVUSB_ENABLE_USB(); // enable PHY 
        if (psDevice->eUsbState >= eDRVUSB_ATTACHED)
        {
            psDevice->eUsbState &= ~eDRVUSB_SUSPENDED;
        }
        psDevice->u16MiscEventFlags |= DRVUSB_EVENT_FLAG_BUS_RESUME;
    }



}
void DrvUSB_PreDispatchEPEvent(S_DRVUSB_DEVICE *psDevice)
{
    uint32_t u32EpNum;
    uint32_t u32EpId;
    uint32_t u32INTSTS = psDevice->u32INTSTS;
    uint32_t u32EPSTS = psDevice->u32EPSTS;
    uint32_t u32PacketType;
//    uint32_t u32ISOPacketType;
    int32_t i;
        
#if 0
    if (psDevice->eUsbState == eDRVUSB_DETACHED)
    {
        // There should be something wrong. 
        return;
    }
#endif

    // Only care EP events and Setup event 
    u32INTSTS &= (INTSTS_EPTF0 | INTSTS_EPTF1 | INTSTS_EPTF2 | INTSTS_EPTF3 | INTSTS_EPTF4 | INTSTS_EPTF5 | INTSTS_SETUP);

    if (u32INTSTS & INTSTS_SETUP)
    {
        // Record the event to procee it later 
        psDevice->u16MiscEventFlags |= DRVUSB_EVENT_FLAG_SETUP;
        
        // Clear setup event flag 
        _DRVUSB_SET_EVENT_FLAG(INTSTS_SETUP);
        
        // Assign the toggle bit for data phase 
        psDevice->abData0[0] = 1;
    }
    
    u32INTSTS = inp32(&USBD->INTSTS); 
    outp32(&USBD->INTSTS, u32INTSTS & 0x00FF0000UL);

    for (i=0;i<MAX_EP_ID;i++)
    {
        if (u32INTSTS & (0x10000 << i))
        {
            u32EpId = i;

            // Get the address of EP 
            u32EpNum = (_DRVUSB_GET_CFG(u32EpId) & CFG_EPT);    
            
            u32PacketType = ((u32EPSTS >> (4 + u32EpId * 3)) & EPSTS_STS);

#if 1
            if(u32PacketType)
            {
                // Set this EP event as non-handled. 
                psDevice->u16EPEventFlags |= (1 << (u32EpNum*2 + 1));
                psDevice->abData0[u32EpId] = (u32PacketType == EPSTS_OUT0_ACK)?1:0;
            }
            else
            {
                psDevice->u16EPEventFlags |= (1 << (u32EpNum*2));
            }

#else
            
            if (u32PacketType == EPSTS_IN_ACK)
            {
                // Set this EP event as non-handled. 
                psDevice->u16EPEventFlags |= (1 << (u32EpNum*2));
            }
            else if(u32PacketType == EPSTS_OUT0_ACK)
            {
                // Set this EP event as non-handled. 
                psDevice->u16EPEventFlags |= (1 << (u32EpNum*2 + 1));
                
                // Assign the toggle bit of data packet 
                psDevice->abData0[u32EpId] = 1;
            }
            else if(u32PacketType == EPSTS_OUT1_ACK)
            {
                // Set this EP event as non-handled. 
                psDevice->u16EPEventFlags |= (1 << (u32EpNum*2 + 1));
                
                // Assign the toggle bit of data packet 
                psDevice->abData0[u32EpId] = 0;
            }
            else if(u32PacketType == EPSTS_ISO)
            {
                
                /* It should be No ISO here */
                
            }       
#endif
        }

    }
}

void SysTimerDelay(uint32_t us)
{
	SysTick->LOAD = us * 24; /* using 48MHz cpu clock*/
    SysTick->VAL   =  (0x00);
    SysTick->CTRL = (1 << SysTick_CTRL_CLKSOURCE_Pos) | (1<<SysTick_CTRL_ENABLE_Pos);//using cpu clock

    /* Waiting for down-count to zero */
    while((SysTick->CTRL & (1 << 16)) == 0);
}


