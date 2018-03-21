/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright (c) Nuvoton Technology Corp. All rights reserved.                                             */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include "Driver\DrvUSB.h"
#include "CDROMHIDSys.h"
#include "CDROM_HID_API.h"

//#define DEBUG

#ifdef 	DEBUG
# define DBG_PRINTF			printf
#else
# define DBG_PRINTF(...)  
#endif

#define Maximum(a,b)		((a)>(b) ? (a) : (b))
#define Minimum(a,b)		((a)<(b) ? (a) : (b))


#define STORAGE_BUFFER_SIZE     2048         /* Data transfer buffer size in 2048 bytes alignment */
#define CDROM_BLOCK_SIZE		2048         /* logic block size */

uint32_t Storage_Block[STORAGE_BUFFER_SIZE / 4];

#define MASS_STORAGE_OFFSET	    0
#define STORAGE_DATA_BUF        ((uint32_t)&Storage_Block[0])


/* SPI Flash Programming API */
extern void SpiRead(uint32_t addr, uint32_t size, uint32_t buffer);
extern void SpiWrite(uint32_t addr, uint32_t size, uint32_t buffer);


/* Endpoint Description */
S_DRVUSB_EP_CTRL sEpDescription[] = 
{                                    
	{CTRL_EP_NUM    | EP_INPUT , CDROM_MAX_PACKET_SIZE_EP0, NULL},
	{CTRL_EP_NUM    | EP_OUTPUT, CDROM_MAX_PACKET_SIZE_EP0, NULL},
    {HID_IN_EP_NUM  | EP_INPUT,  INT_EP_MXPLD, NULL},
    {HID_OUT_EP_NUM | EP_OUTPUT, INT_EP_MXPLD, NULL},
	{BULK_IN_EP_NUM | EP_INPUT , BULK_EP_MXPLD, NULL},
	{BULK_OUT_EP_NUM| EP_OUTPUT, BULK_EP_MXPLD, NULL},
	{0, 0, 0}
};

/* Bus event call back functions */
S_DRVUSB_EVENT_PROCESS g_sBusOps[6] = 
{
	{NULL, NULL},                                /* Attach event callback        */
	{NULL, NULL},                                /* Detach event callback        */
	{DrvUSB_BusResetCallback, &gsCdromDevice},   /* Bus reset event callback     */
	{NULL, NULL},                                /* Bus suspend event callback   */
	{NULL, NULL},                                /* Bus resume event callback    */
	{DrvUSB_CtrlSetupAck, &gsCdromDevice}        /* Setup event callback         */
};

/* USB event call back functions */
S_DRVUSB_EVENT_PROCESS g_sUsbOps[12] = 
{
	{DrvUSB_CtrlDataInAck,   &gsCdromDevice},   /* Ctrl pipe0         */
	{DrvUSB_CtrlDataOutAck,  &gsCdromDevice},	

    {HID_IntInCallback      , &gsCdromDevice},	/* Endpoint address 3 */
	{NULL, NULL},

	{NULL, NULL},                               /* Endpoint address 4 */
	{HID_IntOutCallback		, &gsCdromDevice},							
	
	{CDROM_BulkInAckCallback,  &gsCdromDevice},	/* Endpoint address 1 */
	{NULL, NULL},

	{NULL, NULL},							    /* Endpoint address 2 */
	{CDROM_BulkOutAckCallback, &gsCdromDevice},
	
	{NULL, NULL},                               /* Endpoint address 5 */
	{NULL, NULL}								
};

/* Ctrl pipe call back.                                                                   */
/* it will be call by DrvUSB_CtrlSetupAck, DrvUSB_CtrlDataInAck and DrvUSB_CtrlDataOutAck */
/* if in ack handler and out ack handler is 0, default handler will be called             */
S_DRVUSB_CTRL_CALLBACK_ENTRY g_asCtrlCallbackEntry[] = {
    //request type,command          ,setup ack handler                , in ack handler               ,out ack handler, parameter
	{REQ_STANDARD, SET_ADDRESS      , DrvUSB_CtrlSetupSetAddress      , DrvUSB_CtrlDataInSetAddress  , 0             , &gsCdromDevice},
	{REQ_STANDARD, CLEAR_FEATURE    , DrvUSB_CtrlSetupClearSetFeature , 0                            , 0             , &gsCdromDevice},
	{REQ_STANDARD, SET_FEATURE      , DrvUSB_CtrlSetupClearSetFeature , 0                            , 0             , &gsCdromDevice},
	{REQ_STANDARD, GET_CONFIGURATION, DrvUSB_CtrlSetupGetConfiguration, 0                            , 0             , &gsCdromDevice},
	{REQ_STANDARD, GET_STATUS       , DrvUSB_CtrlSetupGetStatus       , 0                            , 0             , &gsCdromDevice},
	{REQ_STANDARD, GET_INTERFACE    , DrvUSB_CtrlSetupGetInterface    , 0                            , 0             , &gsCdromDevice},
	{REQ_STANDARD, SET_INTERFACE    , DrvUSB_CtrlSetupSetInterface    , 0                            , 0             , &gsCdromDevice},
	{REQ_STANDARD, GET_DESCRIPTOR   , CDROM_CtrlSetupGetDescriptor    , CDROM_CtrlGetDescriptorIn, CDROM_CtrlGetDescriptorOut , &gsCdromDevice},
	{REQ_STANDARD, SET_CONFIGURATION, DrvUSB_CtrlSetupSetConfiguration , 0                            , 0             , &gsCdromDevice},
	
	{REQ_CLASS   , BULK_ONLY_MASS_STORAGE_RESET, 0, 0, 0, &gsCdromDevice},
	{REQ_CLASS   , GET_MAX_LUN                 , CDROM_GetMaxLUN, 0, 0, &gsCdromDevice},

    /* To support boot protocol */
    {REQ_CLASS	, GET_REPORT		, HID_CtrlSetupGetReport, 0, 0, &gsCdromDevice},
    {REQ_CLASS	, SET_REPORT		, HID_CtrlSetupSetReport, 0, 0, &gsCdromDevice},
    {REQ_CLASS	, SET_IDLE			, HID_CtrlSetupSetIdle, 0, 0, &gsCdromDevice},
    {REQ_CLASS	, SET_PROTOCOL		, HID_CtrlSetupSetProtocol, 0, 0, &gsCdromDevice},
};

static void CDROM_UsbStartCallBack(void * pVoid);

/* The callback is called when Host issues the interrupt IN transfer to get report */
void (*pfHID_GetInReport)(uint8_t *buf) = NULL;

/* The callback is called when Host issues the interrupt OUT transfer to set report */
void (*pfHID_SetOutReport)(uint8_t *buf) = NULL;

S_CDROM_DEVICE gsCdromDevice;

/* The structure registered to USB driver */
S_DRVUSB_CLASS sCdromClass = 
{
	(void*) &gsCdromDevice, 
	CDROM_UsbStartCallBack,
	CDROM_IsConfigureValue,
	NULL
};

const uint8_t InquiryID[36] = 
{
	0x05,					// Peripheral Device Type
	0x80,					// RMB
	0x00,					// ISO/ECMA, ANSI Version
	0x32,					// Response Data Format
	0x1F, 0x00, 0x00, 0x00,	// Additional Length

	/* Vendor Identification */
	'N', 'u', 'v', 'o', 't', 'o', 'n', ' ',

	/* Product Identification */
	'N', 'U', 'C', '1', '0', '0', ' ', 'C', 'D', 'R', 'O', 'M', ' ', ' ', ' ', ' ',

	/* Product Revision */
	'1', '.', '0', '0'
};

/* To handle the data transfer size > maximum packet size */
static uint16_t gu16TransferLen   = 0;
static uint16_t gu16PacketLen     = 0;
static uint16_t gu16TransferIndex = 0;

int32_t g_bCBWInvalid;
int32_t volatile bFirstCMD = 0xFF;
int32_t volatile bIsDeviceReady = FALSE;

uint8_t g_u8Size;
uint8_t g_u8BulkState;
uint8_t g_au8SenseKey[3];
uint8_t volatile MassLUN=0;
uint8_t volatile F_SPI_LUN=0;
uint8_t preventflag=0;

int32_t  gTotalSectors = 44;
uint32_t g_u32Length;
uint32_t g_u32Address;
uint32_t g_u32LbaAddress;
uint32_t g_u32BytesInStorageBuf;

/* CBW/CSW variables */
struct CBW g_sCBW;
struct CSW g_sCSW;

const uint8_t * gpu8UsbBuf = 0;
uint32_t gu32BytesInUsbBuf = 0;
int8_t gIsOverRequest = 0;

/* Mass_Storage command base address */
__align(4) uint8_t Mass_Base_Addr[64];
uint8_t Storage_Base_Addr[512];

extern int8_t imageBegin0, imageEnd;

/*---------------------------------------------------------------------------------------------------------*/
/* Function: CDROM_CtrlDataInGetDescriptor                                                                 */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*      pVoid - [in],   A pointer to USB class device structure (gsCdromDevice).                           */
/*                                                                                                         */
/* Returns:                                                                                                */
/*      None                                                                                               */
/*                                                                                                         */
/* Description:                                                                                            */
/*      The USB get descriptor  data in event callback function.                                           */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
void CDROM_CtrlDataInGetDescriptor(void * pVoid)
{
	S_DRVUSB_DEVICE *psDevice = &gsUsbDevice;
	
    if (psDevice->au8Setup[0] & 0x80)
	{
		if (gu16TransferLen)
		{
			if (gu16TransferLen >= gu16PacketLen)
			{
				
				DrvUSB_DataIn(0, g_CDROM_au8ConfigDescriptor + gu16TransferIndex, gu16PacketLen);
				gu16TransferLen -= gu16PacketLen;
				gu16TransferIndex += gu16PacketLen;
			}
			else
			{
				DrvUSB_DataIn(0, g_CDROM_au8ConfigDescriptor + gu16TransferIndex, gu16TransferLen);
				gu16TransferLen = 0;
			}
			
			if (gu16TransferLen == 0)
				_DRVUSB_TRIG_EP(1,0x00); //Ctl data out
		}
		else
		{
			_DRVUSB_TRIG_EP(1,0x00);
		}
	}
}



void CDROM_PrepareDescriptors(const uint8_t *pu8Descriptor, uint32_t u32DescriptorSize, uint32_t u32RequestSize, uint32_t u32MaxPacketSize)
{

    gu32BytesInUsbBuf = u32RequestSize;
    if (u32RequestSize > u32DescriptorSize)
    {
        gu32BytesInUsbBuf = u32DescriptorSize;
        gIsOverRequest = 1;
    }
    gpu8UsbBuf = pu8Descriptor;

    DBG_PRINTF("Get descriptor 0x%08x %d size.\n", pu8Descriptor, u32DescriptorSize);

    if (gu32BytesInUsbBuf < u32MaxPacketSize)
    {
        DrvUSB_DataIn(0, gpu8UsbBuf, gu32BytesInUsbBuf);
        gpu8UsbBuf = 0;
        gu32BytesInUsbBuf = 0;
    }
    else
    {
        DrvUSB_DataIn(0, gpu8UsbBuf, u32MaxPacketSize);
        gpu8UsbBuf += u32MaxPacketSize;
        gu32BytesInUsbBuf -= u32MaxPacketSize;
    }

}


void CDROM_CtrlGetDescriptorOut(void * pVoid)
{
    gu32BytesInUsbBuf = 0;
    gpu8UsbBuf = 0;
    gIsOverRequest = 0;
}

void CDROM_CtrlGetDescriptorIn(void * pVoid)
{
    uint32_t u32Len;

    DBG_PRINTF(" >>> 0x%08x %d size.\n", gpu8UsbBuf, gu32BytesInUsbBuf);

    if (gpu8UsbBuf)
    {
        if (gu32BytesInUsbBuf == 0)
        {
            /* Zero packet */
            DrvUSB_DataIn(0, gpu8UsbBuf, 0);
            gpu8UsbBuf = 0;
            _DRVUSB_TRIG_EP(1,0x00);

        }
        else
        {
            u32Len = Minimum(gu32BytesInUsbBuf, CDROM_MAX_PACKET_SIZE_EP0);
            DrvUSB_DataIn(0, gpu8UsbBuf, u32Len);
            gpu8UsbBuf += u32Len;
            gu32BytesInUsbBuf -= u32Len;

            if (gu32BytesInUsbBuf == 0)
            {
                if (u32Len < CDROM_MAX_PACKET_SIZE_EP0)
                {
                    /* This should be last IN packet due to it is less than UAC_MAX_PACKET_SIZE_EP0 */
                    gpu8UsbBuf = 0;
                }
                else
                {
                    if (!gIsOverRequest)
                    {
                        /* This should be the last IN packet because there is no more data to
                           transfer and it is not over request transfer */
                        gpu8UsbBuf = 0;
                    }
                }
                _DRVUSB_TRIG_EP(1,0x00);
            }
        }
    }
    else
    {
        /* The EP id 1 should always be used as control (OUT) endpoint */
        _DRVUSB_TRIG_EP(1, 0x00);
    }
}

/*************************************************************************/
/*                                                                       */
/* DESCRIPTION                                                           */
/*      whether or not the configure value is configure value of HID     */
/*                                                                       */
/* INPUTS                                                                */
/*      pVoid     parameter passed by DrvUSB_RegisterCtrl                */
/*                                                                       */
/* OUTPUTS                                                               */
/*      None                                                             */
/*                                                                       */
/* RETURN                                                                */
/*      0           Success                                              */
/*		Otherwise	error												 */
/*                                                                       */
/*************************************************************************/
static int32_t CDROM_IsConfigureValue(uint8_t u8ConfigureValue)
{
    return (u8ConfigureValue == g_CDROM_au8ConfigDescriptor[5]);
}

/*---------------------------------------------------------------------------------------------------------*/
/* Function: CDROM_CtrlSetupGetDescriptor                                                                  */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*      pVoid - [in],   A pointer to USB class device structure (gsCdromDevice).                           */
/*                                                                                                         */
/* Returns:                                                                                                */
/*      None                                                                                               */
/*                                                                                                         */
/* Description:                                                                                            */
/*      The USB get descriptor event callback function.                                                    */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
void CDROM_CtrlSetupGetDescriptor(void * pVoid)
{
    S_CDROM_DEVICE *psCdromDevice = (S_CDROM_DEVICE *)pVoid;
	S_DRVUSB_DEVICE *psDevice = (S_DRVUSB_DEVICE *)psCdromDevice->device;
	uint16_t u16Len;
	
	u16Len = 0;
	u16Len = psDevice->au8Setup[7];
	u16Len <<= 8;
	u16Len += psDevice->au8Setup[6];

    gIsOverRequest = 0;
    gu32BytesInUsbBuf = 0;
    gpu8UsbBuf = 0;
	switch (psDevice->au8Setup[3])
	{
		/* Get Device Descriptor */
		case DESC_DEVICE:
		{
	        CDROM_PrepareDescriptors(g_CDROM_au8DeviceDescriptor, LEN_DEVICE, u16Len, CDROM_MAX_PACKET_SIZE_EP0);
	
	        /* Prepare the OUT to avoid HOST stop data phase without all data transfered. */
	        //_DRVUSB_TRIG_EP(1, 0x00);
			break;
		}

		/* Get Configuration Descriptor */
		case DESC_CONFIG:
		{
        	CDROM_PrepareDescriptors(g_CDROM_au8ConfigDescriptor, g_CDROM_au8ConfigDescriptor[2]+g_CDROM_au8ConfigDescriptor[3]*256, u16Len, CDROM_MAX_PACKET_SIZE_EP0);
			break;
		}
	    // Get HID Descriptor
	    case DESC_HID:
	    {
	        CDROM_PrepareDescriptors(gsCdromDevice.pu8HIDDescriptor, LEN_HID, u16Len, CDROM_MAX_PACKET_SIZE_EP0);
	        break;
	    }
	    // Get Report Descriptor
	    case DESC_HID_RPT:
	    {
	        CDROM_PrepareDescriptors(gsCdromDevice.pu8ReportDescriptor, gsCdromDevice.u32ReportDescriptorSize, u16Len, CDROM_MAX_PACKET_SIZE_EP0);
	        break;
	    }
	    // Get String Descriptor
	    case DESC_STRING:
	    {
	        // Get Language
	        if (psDevice->au8Setup[2] == 0)
	        {
	            CDROM_PrepareDescriptors(gau8StringLang, 4, u16Len, CDROM_MAX_PACKET_SIZE_EP0);
	        }
	        else
	        {
	            // Get String Descriptor
	            switch (psDevice->au8Setup[2])
	            {
	            case 1:
	                CDROM_PrepareDescriptors((const uint8_t *)gau8VendorStringDescriptor, gau8VendorStringDescriptor[0], u16Len, CDROM_MAX_PACKET_SIZE_EP0);
	                break;
	
	            case 2:
	                CDROM_PrepareDescriptors((const uint8_t *)gau8ProductStringDescriptor, gau8ProductStringDescriptor[0], u16Len, CDROM_MAX_PACKET_SIZE_EP0);

	                break;
	
	            case 3:
	                CDROM_PrepareDescriptors((const uint8_t *)gau8StringSerial, gau8StringSerial[0], u16Len, CDROM_MAX_PACKET_SIZE_EP0);
	                break;
	
	            default:
	                /* Not support. Reply STALL. */
	                DrvUSB_ClrCtrlReadyAndTrigStall();
	            }
	        }
	
	        break;
	    }
		default:
			// Not support. Reply STALL.
			DrvUSB_ClrCtrlReadyAndTrigStall();
			break;
	}
}


/*---------------------------------------------------------------------------------------------------------*/
/* Function: CDROM_CtrlSetupSetConfiguration                                                               */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*      pVoid - [in],   A pointer to USB class device structure (gsCdromDevice).                           */
/*                                                                                                         */
/* Returns:                                                                                                */
/*      None                                                                                               */
/*                                                                                                         */
/* Description:                                                                                            */
/*      The callback function when get SET_CONFIGURATION request.                                          */
/*---------------------------------------------------------------------------------------------------------*/
void CDROM_CtrlSetupSetConfiguration(void * pVoid)
{
	S_DRVUSB_DEVICE *pDrvDevice = &gsUsbDevice;
	S_DRVUSB_CLASS *psUsbClass = pDrvDevice->psUsbClass;
	int32_t bIsDeviceConfigure;
	
	bIsDeviceConfigure = psUsbClass->pfnCompare ? psUsbClass->pfnCompare(pDrvDevice->au8Setup[2]) : 1;
	
	if (pDrvDevice->au8Setup[2] == 0)
	{
		// USB address state.
		DrvUSB_SetUsbState(eDRVUSB_ADDRESS);
		pDrvDevice->u8UsbConfiguration = pDrvDevice->au8Setup[2];
		DrvUSB_DataIn(0, NULL, 0);
		
	}
	else if(bIsDeviceConfigure)
	{
		// USB configured state.
		DrvUSB_SetUsbState(eDRVUSB_CONFIGURED);

		//call USB class's start function
		if (psUsbClass->pfnStart)
			psUsbClass->pfnStart(pVoid);

		pDrvDevice->u8UsbConfiguration = pDrvDevice->au8Setup[2];

		DrvUSB_DataIn(0, NULL, 0);		
	}
	else
	{
		// Not support. Reply STALL.
		DrvUSB_ClrCtrlReadyAndTrigStall();
	}
}



/*---------------------------------------------------------------------------------------------------------*/
/* Function: CDROM_Open                                                                                     */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*      None                                                                                               */
/*                                                                                                         */
/* Returns:                                                                                                */
/*      E_SUCCESS                                                                                          */
/*                                                                                                         */
/* Description:                                                                                            */
/*      To install the virtual com to USB device driver and install the relative descriptors.              */
/*---------------------------------------------------------------------------------------------------------*/
int32_t CDROM_Open(uint32_t inReportSize, void *pfGetInReport, void *pfSetOutReport)
{
    S_CDROM_DEVICE *psCdromDevice = &gsCdromDevice;

    if ((uint32_t)pfGetInReport != NULL)
        pfHID_GetInReport = (void(*)(uint8_t *))pfGetInReport;

    if ((uint32_t)pfSetOutReport != NULL)
        pfHID_SetOutReport = (void(*)(uint8_t *))pfSetOutReport;

	gsCdromDevice.u32ReportSize = inReportSize;
	
	/* Install the virtual com to USB device */
	psCdromDevice->device = (void *)DrvUSB_InstallClassDevice(&sCdromClass);
	
	/* Configure the descritors */
	psCdromDevice->au8DeviceDescriptor = g_CDROM_au8DeviceDescriptor;
	psCdromDevice->au8ConfigDescriptor = g_CDROM_au8ConfigDescriptor;			
	psCdromDevice->sVendorStringDescriptor = gau8VendorStringDescriptor;

    psCdromDevice->pu8HIDDescriptor = psCdromDevice->au8ConfigDescriptor + LEN_CONFIG + LEN_INTERFACE*2 + LEN_ENDPOINT*2;
    psCdromDevice->pu8IntInEPDescriptor = psCdromDevice->au8ConfigDescriptor + LEN_CONFIG + LEN_INTERFACE*2 + LEN_ENDPOINT*2 + LEN_HID;
		
	/* Install the USB event callbacks */
	DrvUSB_InstallCtrlHandler(psCdromDevice->device, g_asCtrlCallbackEntry, 
	    sizeof(g_asCtrlCallbackEntry) / sizeof(g_asCtrlCallbackEntry[0]));

	return E_SUCCESS;
}


/*---------------------------------------------------------------------------------------------------------*/
/* Function: CDROM_Close                                                                                    */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*      None                                                                                               */
/*                                                                                                         */
/* Returns:                                                                                                */
/*      None                                                                                               */
/*                                                                                                         */
/* Description:                                                                                            */
/*      To close CDROM.                                                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
void CDROM_Close(void)
{

}

static void CDROM_UsbStartCallBack(void * pVoid)
{
    CDROM_CtrlReset((S_CDROM_DEVICE *)pVoid);
    CDROM_Start((S_CDROM_DEVICE *)pVoid);
}

void CDROM_Start(S_CDROM_DEVICE *psDevice)
{
    DrvUSB_DataIn(HID_IN_EP_NUM, psDevice->pu8Report, psDevice->u32ReportSize);
    DrvUSB_DataOutTrigger(HID_OUT_EP_NUM, INT_EP_MXPLD);

	g_u8BulkState = BULK_CBW;
	g_au8SenseKey[0] = 0;
	g_au8SenseKey[1] = 0;
	g_au8SenseKey[2] = 0;
	g_bCBWInvalid = FALSE;	
	g_u32Address = 0;
	g_u32Length = 0;
	g_u8Size = 0;

	g_sCSW.dCSWSignature = CSW_SIGNATURE;

	g_bCBWInvalid = FALSE;
	DrvUSB_DataIn(CTRL_EP_NUM, NULL, 0);
	g_u8BulkState = BULK_CBW;
	DrvUSB_DataOutTrigger(BULK_OUT_EP_NUM, 31);

	//S_DRVUSB_DEVICE *psDevice = (S_DRVUSB_DEVICE *)((S_UDC_DEVICE *)pVoid)->device;
	g_bCBWInvalid = FALSE;
	DrvUSB_DataIn(CTRL_EP_NUM, NULL, 0);
	g_u8BulkState = BULK_CBW;
	DrvUSB_DataOutTrigger(BULK_OUT_EP_NUM, 31);
}

void CDROM_CtrlReset(S_CDROM_DEVICE *psDevice)
{
    DrvUSB_Reset(HID_IN_EP_NUM);
    DrvUSB_Reset(HID_OUT_EP_NUM);

    DrvUSB_Reset(BULK_IN_EP_NUM);
    DrvUSB_Reset(BULK_OUT_EP_NUM);
}


void CDROM_GetMaxLUN(void * pVoid)
{
	S_DRVUSB_DEVICE *psDevice = (S_DRVUSB_DEVICE *)((S_CDROM_DEVICE *)pVoid)->device;
	uint8_t u8LUN;
	
    u8LUN = 0;
	DrvUSB_DataIn(CTRL_EP_NUM, &u8LUN, 1);
	psDevice->abData0[BULK_IN_EP_NUM] = FALSE;
}

static __inline uint32_t get_be32(uint8_t * buf)
{
	return ((uint32_t) buf[0] << 24) | ((uint32_t) buf[1] << 16) |
			((uint32_t) buf[2] << 8) | ((uint32_t) buf[3]);
}

static __inline uint16_t get_be16(uint8_t * buf)
{
	return (((uint16_t) buf[0] << 8) | ((uint16_t) buf[1]));
}


static uint8_t Command_51_01[8] = 
{
 	0x00, 0x20, 0x01, 0x01, 0x02, 0x02, 0x02, 0x80 
};

void Command_51(void)
{
	uint32_t u32Len;

	u32Len = g_sCBW.dCBWDataTransferLength;
	if (u32Len > 8)
		u32Len = 8;
		
	DrvUSB_DataIn(BULK_IN_EP_NUM, Command_51_01, u32Len);
}

static uint8_t Command_4A_01[8] = 
{
 	0x00, 0x02, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00 
};

void Command_4A(void)
{
	uint32_t u32Len;
	
	u32Len = g_sCBW.dCBWDataTransferLength;
	
    if (u32Len > 8)
		u32Len = 8;
		
	DrvUSB_DataIn(BULK_IN_EP_NUM, Command_4A_01, u32Len);	
}

static uint8_t Command_A4_01[8] = 
{
 	0x00, 0x06, 0x00, 0x00, 0x64, 0xFE, 0x01, 0x00 
};

void Command_A4(void)
{
	uint32_t u32Len;
	
	u32Len = g_sCBW.dCBWDataTransferLength;
	if (u32Len > 8)
		u32Len = 8;
		
	DrvUSB_DataIn(BULK_IN_EP_NUM, Command_A4_01, u32Len);	
}

static uint8_t Command_46_00[328] = 
{
 	0x00, 0x00, 0x01, 0x44, 0x00, 0x00, 0x00, 0x09,
 	0x00, 0x00, 0x03, 0x34, 0x00, 0x2B, 0x00, 0x00,
 	0x00, 0x1B, 0x00, 0x00, 0x00, 0x1A, 0x00, 0x00,
 	0x00, 0x15, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00,
 	0x00, 0x13, 0x00, 0x00, 0x00, 0x12, 0x00, 0x00,
 	0x00, 0x11, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00,
 	0x00, 0x0A, 0x00, 0x00, 0x00, 0x09, 0x01, 0x00,
 	0x00, 0x08, 0x01, 0x00, 0x00, 0x02, 0x00, 0x00,
 	0x00, 0x01, 0x03, 0x04, 0x00, 0x00, 0x00, 0x02,
 	0x00, 0x02, 0x03, 0x04, 0x00, 0x00, 0x00, 0x00,
 	0x00, 0x03, 0x03, 0x04, 0x29, 0x00, 0x00, 0x00,
 	0x00, 0x04, 0x04, 0x04, 0x02, 0x00, 0x00, 0x00,
 	0x00, 0x10, 0x01, 0x08, 0x00, 0x00, 0x08, 0x00,
 	0x00, 0x01, 0x01, 0x00, 0x00, 0x1D, 0x01, 0x00,
 	0x00, 0x1E, 0x05, 0x04, 0x03, 0x00, 0x00, 0x00,
 	0x00, 0x1F, 0x04, 0x04, 0x00, 0x00, 0x00, 0x00,
 	0x00, 0x20, 0x04, 0x0C, 0x00, 0x00, 0x00, 0x00,
 	0x00, 0x00, 0x08, 0x00, 0x00, 0x01, 0x01, 0x00,
 	0x00, 0x21, 0x0D, 0x08, 0x3D, 0x01, 0x01, 0x01,
 	0x07, 0x00, 0x00, 0x00, 0x00, 0x23, 0x00, 0x00,
 	0x00, 0x24, 0x04, 0x04, 0x80, 0x00, 0x00, 0x00,
 	0x00, 0x26, 0x00, 0x00, 0x00, 0x28, 0x00, 0x04,
 	0x01, 0x00, 0x00, 0x00, 0x00, 0x2A, 0x00, 0x04,
 	0x01, 0x00, 0x00, 0x00, 0x00, 0x2B, 0x00, 0x04,
 	0x01, 0x00, 0x00, 0x00, 0x00, 0x2C, 0x00, 0x04,
 	0x03, 0x00, 0x00, 0x00, 0x00, 0x2D, 0x09, 0x04,
 	0x46, 0x00, 0x3D, 0x01, 0x00, 0x2E, 0x05, 0x04,
 	0x66, 0x00, 0x24, 0x00, 0x00, 0x2F, 0x08, 0x04,
 	0x4E, 0x00, 0x00, 0x00, 0x00, 0x37, 0x00, 0x04,
	0x00, 0x0F, 0x00, 0x00, 0x00, 0x3B, 0x00, 0x04,
	0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x03, 0x00,
	0x01, 0x01, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00
};

static uint8_t Command_46_1E[8] = 
{
 	0x00, 0x1E, 0x05, 0x04, 0x03, 0x00, 0x00, 0x00
};

static uint8_t Command_46_1F[8] = 
{
 	0x00, 0x1F, 0x04, 0x04, 0x00, 0x00, 0x00, 0x00
};

static uint8_t Command_46_20[16] =
{
 	0x00, 0x20, 0x04, 0x0C, 0x00, 0x00, 0x00, 0x00,
 	0x00, 0x00, 0x08, 0x00, 0x00, 0x01, 0x01, 0x00
};

static uint8_t Command_46_21[12] = 
{
 	0x00, 0x21, 0x0D, 0x08, 0x3D, 0x01, 0x01, 0x01,
 	0x07, 0x00, 0x00, 0x00
};

static uint8_t Command_46_23[4] = 
{
 	0x00, 0x23, 0x00, 0x00
};

static uint8_t Command_46_24[8] = 
{
 	0x00, 0x24, 0x04, 0x04, 0x80, 0x00, 0x00, 0x00
};

static uint8_t Command_46_26[4] = 
{
 	0x00, 0x26, 0x00, 0x00
};

static uint8_t Command_46_2A[8] = 
{
 	0x00, 0x2A, 0x00, 0x04,	0x01, 0x00, 0x00, 0x00
};

static uint8_t Command_46_2B[8] = 
{
 	0x00, 0x2B, 0x00, 0x04,	0x01, 0x00, 0x00, 0x00
};

static uint8_t Command_46_2D[8] = 
{
 	0x00, 0x2D, 0x09, 0x04,	0x46, 0x00, 0x3D, 0x01
};

static uint8_t Command_46_2E[8] = 
{
 	0x00, 0x2E, 0x05, 0x04,	0x66, 0x00, 0x24, 0x00
};

static uint8_t Command_46_2F[8] = 
{
 	0x00, 0x2F, 0x08, 0x04,	0x4E, 0x00, 0x00, 0x00
};

static uint8_t Command_46_3B[8] = 
{
 	0x00, 0x3B, 0x00, 0x04, 0x01, 0x00, 0x00, 0x00
};

static uint8_t Command_46_01[176] = 
{
 	0x00, 0x00, 0x00, 0xAC, 0x00, 0x00, 0x00, 0x09,
	0x00, 0x00, 0x03, 0x34, 0x00, 0x2B, 0x00, 0x00,
 	0x00, 0x1B, 0x00, 0x00, 0x00, 0x1A, 0x00, 0x00,
 	0x00, 0x15, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00,
 	0x00, 0x13, 0x00, 0x00, 0x00, 0x12, 0x00, 0x00,
 	0x00, 0x11, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00,
 	0x00, 0x0A, 0x00, 0x00, 0x00, 0x09, 0x01, 0x00,
 	0x00, 0x08, 0x01, 0x00, 0x00, 0x02, 0x00, 0x00,
 	0x00, 0x01, 0x03, 0x04, 0x00, 0x00, 0x00, 0x02,
 	0x00, 0x02, 0x03, 0x04, 0x00, 0x00, 0x00, 0x00,
 	0x00, 0x03, 0x03, 0x04, 0x29, 0x00, 0x00, 0x00,
 	0x00, 0x10, 0x01, 0x08, 0x00, 0x00, 0x08, 0x00,
 	0x00, 0x01, 0x01, 0x00, 0x00, 0x1D, 0x01, 0x00,
 	0x00, 0x1E, 0x05, 0x04, 0x03, 0x00, 0x00, 0x00,
 	0x00, 0x21, 0x0D, 0x08, 0x3D, 0x01, 0x01, 0x01,
 	0x07, 0x00, 0x00, 0x00, 0x00, 0x2D, 0x09, 0x04,
 	0x46, 0x00, 0x3D, 0x01, 0x00, 0x2E, 0x05, 0x04,
 	0x66, 0x00, 0x24, 0x00,	0x01, 0x00, 0x03, 0x00,
 	0x01, 0x05, 0x07, 0x04, 0x00, 0x00, 0x00, 0x00,
 	0x01, 0x07, 0x0D, 0x04, 0x1F, 0x00, 0x00, 0x00,
 	0x01, 0x08, 0x03, 0x0C, 0x54, 0x43, 0x4A, 0x49,
 	0x41, 0x34, 0x30, 0x31, 0x34, 0x42, 0x36, 0x33
};

void Command_46(uint32_t len, uint8_t *buff)
{
	if (!bIsDeviceReady)
	{
		memset((int8_t *)buff, 0, len);
		return;
	}

	if (g_sCBW.u8LUN == 0x02)
	{
		memset(buff, 0x0, len);
		if (get_be16(&g_sCBW.au8Data[0]) == 0x0000)
			memcpy((int8_t *)buff, &Command_46_00[0], len);
		else
		{
			memcpy((int8_t *)buff, &Command_46_00[0], 8);
			buff += 8;
			switch(get_be16(&g_sCBW.au8Data[0]))
			{
				case 0x001E:
					memcpy((int8_t *)buff, &Command_46_1E[0], 8);
					break;
				case 0x001F:
					memcpy((int8_t *)buff, &Command_46_1F[0], 8);
					break;
				case 0x0020:
					memcpy((int8_t *)buff, &Command_46_20[0], 8);
					break;
				case 0x0021:
					memcpy((int8_t *)buff, &Command_46_21[0], 8);
					break;
				case 0x0023:
					memcpy((int8_t *)buff, &Command_46_23[0], 8);
					break;
				case 0x0024:
					memcpy((int8_t *)buff, &Command_46_24[0], 8);
					break;
				case 0x0026:
					memcpy((int8_t *)buff, &Command_46_26[0], 4);
					break;
				case 0x002A:
					memcpy((int8_t *)buff, &Command_46_2A[0], 8);
					break;
				case 0x002B:
					memcpy((int8_t *)buff, &Command_46_2B[0], 8);
					break;
				case 0x002D:
					memcpy((int8_t *)buff, &Command_46_2D[0], 8);
					break;
				case 0x002E:
					memcpy((int8_t *)buff, &Command_46_2E[0], 8);
					break;
				case 0x002F:
					memcpy((int8_t *)buff, &Command_46_2F[0], 8);
					break;
				case 0x003B:
					memcpy((int8_t *)buff, &Command_46_3B[0], 8);
					break;
				default:
					g_au8SenseKey[0] = 0x05;
					g_au8SenseKey[1] = 0x24;
					g_au8SenseKey[2] = 0x00;
			}
		}
	}
	else if (g_sCBW.u8LUN == 0x01)
	{
		if (g_u32Length > 176)
			g_u32Length = 176;
		memcpy((char *)buff, &Command_46_01[0], len);
	}
	else if (g_sCBW.u8LUN == 0x00)
	{
		if (g_u32Length > 328)
			g_u32Length = 328;

		memset(buff, 0x0, len);
		if (get_be16(&g_sCBW.au8Data[0]) == 0x0000)
			memcpy((char *)buff, &Command_46_00[0], len);
		else
		{
			memcpy((char *)buff, &Command_46_00[0], 8);
			buff += 8;
			switch(get_be16(&g_sCBW.au8Data[0]))
			{
				case 0x001E:
					memcpy((int8_t *)buff, &Command_46_1E[0], 8);
					break;
				case 0x001F:
					memcpy((int8_t *)buff, &Command_46_1F[0], 8);
					break;
				case 0x0020:
					memcpy((int8_t *)buff, &Command_46_20[0], 8);
					break;
				case 0x0021:
					memcpy((int8_t *)buff, &Command_46_21[0], 8);
					break;
				case 0x0023:
					memcpy((int8_t *)buff, &Command_46_23[0], 8);
					break;
				case 0x0024:
					memcpy((int8_t *)buff, &Command_46_24[0], 8);
					break;
				case 0x0026:
					memcpy((int8_t *)buff, &Command_46_26[0], 4);
					break;
				case 0x002A:
					memcpy((int8_t *)buff, &Command_46_2A[0], 8);
					break;
				case 0x002B:
					memcpy((int8_t *)buff, &Command_46_2B[0], 8);
					break;
				case 0x002D:
					memcpy((int8_t *)buff, &Command_46_2D[0], 8);
					break;
				case 0x002E:
					memcpy((int8_t *)buff, &Command_46_2E[0], 8);
					break;
				case 0x002F:
					memcpy((int8_t *)buff, &Command_46_2F[0], 8);
					break;
				case 0x003B:
					memcpy((int8_t *)buff, &Command_46_3B[0], 8);
					break;
				default:
					g_au8SenseKey[0] = 0x05;
					g_au8SenseKey[1] = 0x24;
					g_au8SenseKey[2] = 0x00;
			}
		}
	}
	else
	{
		g_au8SenseKey[0] = 0x05;
		g_au8SenseKey[1] = 0x24;
		g_au8SenseKey[2] = 0x00;
	}
}


static uint8_t Command_43_00_01[12] = 
{
	0x00, 0x0A, 0x01, 0x01, 0x00, 0x14, 0x01, 0x00,
	0x00, 0x00, 0x00, 0x00 
};

static uint8_t Command_43_00_00[20] = 
{
	0x00, 0x12, 0x01, 0x01, 0x00, 0x14, 0x01, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0xAA, 0x00,
	0x00, 0x00, 0x01, 0x2E 
};

static uint8_t Command_43_02[20] = 
{
	0x00, 0x12, 0x01, 0x01, 0x00, 0x14, 0x01, 0x00,
	0x00, 0x00, 0x02, 0x00, 0x00, 0x14, 0xAA, 0x00,
	0x00, 0x00, 0x06, 0x02 
};

void Command_43(void)
{
	uint32_t u32Len;

	if (g_sCBW.u8LUN == 0x02)
	{	
		u32Len = g_sCBW.dCBWDataTransferLength;
		if (u32Len > 20)
			u32Len = 20;
		
		DrvUSB_DataIn(BULK_IN_EP_NUM, Command_43_02, u32Len);	
		
	}
	else if (g_sCBW.u8LUN == 0x00)
	{
		switch (g_sCBW.au8Data[0])
		{
			case 0x01:
			{
				u32Len = g_sCBW.dCBWDataTransferLength;
				if (u32Len > 12)
					u32Len = 12;
		
				DrvUSB_DataIn(BULK_IN_EP_NUM, Command_43_00_01, u32Len);
				break;
			}
			case 0x00:
			{
				u32Len = g_sCBW.dCBWDataTransferLength;
				if (u32Len > 20)
					u32Len = 20;
		
				DrvUSB_DataIn(BULK_IN_EP_NUM, Command_43_00_00, u32Len);				
				break;
			}
			default:
				g_au8SenseKey[0] = 0x05;
				g_au8SenseKey[1] = 0x24;
				g_au8SenseKey[2] = 0x00;
		}
	}
	else
	{
		g_au8SenseKey[0] = 0x05;
		g_au8SenseKey[1] = 0x24;
		g_au8SenseKey[2] = 0x00;
	}
}

uint8_t ModeSense_2A[40] = 
{
	0x00, 0x36, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x2A, 0x2E, 0x3F, 0x37, 0xF5, 0x73, 0x29, 0x23,
	0x1B, 0x90, 0x01, 0x00, 0x05, 0x8B, 0x1B, 0x90,
	0x00, 0x00, 0x1B, 0x90, 0x1B, 0x90, 0x00, 0x01,
	0x00, 0x00, 0x00, 0x01, 0x1B, 0x90, 0x00, 0x04
};

uint8_t ModeSense_1A[12] = 
{
	0x00, 0x12, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x1A, 0x0A, 0x00, 0x03
};

void nvtModeSense(uint32_t len, uint8_t *buff)
{
	memset(buff, 0x0, len);
	switch (g_sCBW.au8Data[0])
	{
		case 0x2A:
		{
			memcpy((int8_t *)buff, &ModeSense_2A[0], 40);
			break;
		}
		case 0x1A:
		{
			memcpy((int8_t *)buff, &ModeSense_1A[0], 12);
			break;
		}
		default:
			g_au8SenseKey[0] = 0x05;
			g_au8SenseKey[1] = 0x24;
			g_au8SenseKey[2] = 0x00;
	}
}

void ReqSenCommand(uint8_t * buff)
{
    if (preventflag == 1)
    {
        preventflag = 0;
		buff[0] = 0x70;
    }
    else
		buff[0] = 0xf0;

	buff[1] = 0;
	buff[3] = 0;
	buff[4] = 0;
	buff[5] = 0;
	buff[6] = 0;
	buff[7] = 0x0A;
	buff[8] = 0;
	buff[9] = 0;
	buff[10] = 0;
	buff[11] = 0;
	buff[14] = 0;
	buff[15] = 0;
	buff[16] = 0;
	buff[17] = 0;

	buff[2] = g_au8SenseKey[0];
	buff[12] = g_au8SenseKey[1];
	buff[13] = g_au8SenseKey[2];

	/* Sense Key */
	g_au8SenseKey[0] = 0x00;
	g_au8SenseKey[1] = 0x00;
	g_au8SenseKey[2] = 0x00;
}

void UsbRead(void)
{
	if (g_u32Length > BULK_EP_MXPLD)
		g_u8Size = BULK_EP_MXPLD;
	else
		g_u8Size = g_u32Length;

	DrvUSB_DataIn(BULK_IN_EP_NUM, (uint8_t *)g_u32Address, g_u8Size);
	//DBG_PRINTF("BULK IN: 0x%08x size:%d\n", g_u32Address, g_u32Length);
	//DBG_PRINTF(".");
	g_u32Address += g_u8Size;
	g_u32Length -= g_u8Size;
	g_u32BytesInStorageBuf -= g_u8Size;
}

void UsbBulkInAck(void)
{
	g_sCSW.dCSWSignature = CSW_SIGNATURE;
	
	if (g_u8BulkState == BULK_CSW)
	{
	    /* Prepare to receive the CBW */
		g_u8BulkState = BULK_CBW;
		DrvUSB_DataOutTrigger(BULK_OUT_EP_NUM, 31);
	}
	else if (g_u8BulkState == BULK_IN)
	{
		switch (g_sCBW.u8OPCode)
		{
			case UFI_READ_CAPACITY:
			case UFI_MODE_SENSE_10:
			case UFI_READ_10:
			case 0x46:
			case 0xBE:
			{
				if(g_u32Length > 0)
				{
					UsbRead();
					return;
				}
				else
				{
	                DBG_PRINTF("Finish!\n");
				}
			}
			case UFI_REQUEST_SENSE:
			case UFI_INQUIRY:
			{
				g_sCSW.dCSWDataResidue = 0;
				g_sCSW.bCSWStatus = 0;
				break;
			}

			case UFI_PREVENT_ALLOW_MEDIUM_REMOVAL:
			case UFI_START_STOP:
			case 0x4A:	// debug1209
			case 0xA4:	// debug1209
			case 0x51:	// debug1209
			case 0xBB:
			{
				g_sCSW.dCSWDataResidue = 0;
				g_sCSW.bCSWStatus = 0;
				break;
			}
			case 0x43:
			{
				if (g_sCBW.u8LUN == 0x02)
				{
					if (g_sCBW.dCBWDataTransferLength > 20)
						g_sCSW.dCSWDataResidue = g_sCBW.dCBWDataTransferLength - 20;
					else
						g_sCSW.dCSWDataResidue = 0;
				}
				else if (g_sCBW.u8LUN == 0x00)
				{
					if (g_sCBW.au8Data[0] == 0x00)
					{
						if (g_sCBW.dCBWDataTransferLength > 20)
							g_sCSW.dCSWDataResidue = g_sCBW.dCBWDataTransferLength - 20;
						else
							g_sCSW.dCSWDataResidue = 0;
					}
					if (g_sCBW.au8Data[0] == 0x01)
					{
						if (g_sCBW.dCBWDataTransferLength > 12)
							g_sCSW.dCSWDataResidue = g_sCBW.dCBWDataTransferLength - 12;
						else
							g_sCSW.dCSWDataResidue = 0;
					}
				}
				else
					g_sCSW.dCSWDataResidue = 0;
				
				g_sCSW.bCSWStatus = 0;
				break;
			}			
			case UFI_TEST_UNIT_READY:
			{
				if (bIsDeviceReady)
				{
					g_sCSW.dCSWDataResidue = 0;
					g_sCSW.bCSWStatus = 0;
					break;
				}			
			}
			default:
			{
			    /* Unsupported commmand. Return command fail status */
				g_sCSW.dCSWDataResidue = g_sCBW.dCBWDataTransferLength;
				g_sCSW.bCSWStatus = 0x01;
				break;
			}
		}
		
		/* Return the CSW */
		g_u8BulkState = BULK_CSW;
		DrvUSB_DataIn(BULK_IN_EP_NUM, (uint8_t *)&g_sCSW.dCSWSignature, 13);
	}
}

void UsbWrite(void)
{
	uint32_t u32Len = 0;
	uint8_t * bufferAddr;

	if (g_u32Length > BULK_EP_MXPLD)
	{

		DrvUSB_DataOutTrigger(BULK_OUT_EP_NUM, BULK_EP_MXPLD);
		bufferAddr = DrvUSB_GetOutData(BULK_OUT_EP_NUM, &u32Len);
		memcpy((char *)g_u32Address, bufferAddr, BULK_EP_MXPLD);
		
		g_u32Address += BULK_EP_MXPLD;
		g_u32Length -= BULK_EP_MXPLD;
		
	}
	else
	{
		bufferAddr = DrvUSB_GetOutData(BULK_OUT_EP_NUM, &u32Len);
		memcpy((char *)g_u32Address, bufferAddr, g_u32Length);
		
		g_u32Address += g_u32Length;
		g_u32Length = 0;

		g_u8BulkState = BULK_IN;
		UsbBulkInAck ();
	}
}

int32_t UsbBulkOutAck(void)
{
	uint32_t  u32Len = 0;
	uint8_t * bufferAddr;
	int32_t i;
	//uint32_t u32ByteCount;

	if (g_u8BulkState == BULK_CBW)
	{
		bufferAddr = DrvUSB_GetOutData(BULK_OUT_EP_NUM, &u32Len);

		if ( (*(uint32_t *)bufferAddr != CBW_SIGNATURE) || (u32Len != 31) )
		{
			return FALSE;
		}		
		
		/* Get the CBW */
		for (i = 0; i < u32Len; i++)
			*((uint8_t *) (&g_sCBW.dCBWSignature) + i) = bufferAddr[i];	
		
		/* Prepare to echo the tag from CBW to CSW */
		g_sCSW.dCSWTag = g_sCBW.dCBWTag;		
		
		/* Parse Op-Code of CBW */
		switch (g_sCBW.u8OPCode)
		{
			case UFI_PREVENT_ALLOW_MEDIUM_REMOVAL:
			{
				if (g_sCBW.au8Data[2] & 0x01)
				{
					g_au8SenseKey[0] = 0x05;  //INVALID COMMAND
					g_au8SenseKey[1] = 0x24;
					g_au8SenseKey[2] = 0;
					preventflag = 1;
				}
				else
					preventflag = 0;
				
				g_u8BulkState = BULK_IN;
				UsbBulkInAck();
				return TRUE;
			}
			case UFI_START_STOP:
			case 0xBB:
			{
				g_u8BulkState = BULK_IN;
				UsbBulkInAck ();
				return TRUE;
			}			
			case UFI_TEST_UNIT_READY:
			{
				if (bFirstCMD == 0x25)
					bIsDeviceReady = TRUE;

				// Medium Not Present
				if (!bIsDeviceReady)
				{
					g_au8SenseKey[0] = 0x02;
					g_au8SenseKey[1] = 0x3A;
					g_au8SenseKey[2] = 0x00;
				}
				g_u8BulkState = BULK_IN;
				UsbBulkInAck ();
				return TRUE;
			}
			case UFI_REQUEST_SENSE:
			{
				uint8_t u8ReqSenCmd[18] = {0};

				ReqSenCommand(u8ReqSenCmd);
				g_u8BulkState = BULK_IN;
				DrvUSB_DataIn(BULK_IN_EP_NUM, (const uint8_t *)u8ReqSenCmd, 18);
				return TRUE;
			}
			case 0x51:
			{
				Command_51();
				g_u8BulkState = BULK_IN;
				return TRUE;
			}
			case 0xA4:
			{
				Command_A4();
				g_u8BulkState = BULK_IN;
				return TRUE;
			}
			case 0x4A:
			{
				Command_4A();
				g_u8BulkState = BULK_IN;
				return TRUE;
			}
			case 0x46:
			{
				if (g_u32Length == 0)
				{
					// LBA
					g_u32Address = get_be32(&g_sCBW.au8Data[0]);
					g_u32Length = g_sCBW.dCBWDataTransferLength;
					Command_46(g_u32Length, (uint8_t *)Storage_Base_Addr);
				}
				g_u32Address = (uint32_t)Storage_Base_Addr;
				g_u8BulkState = BULK_IN;
				if (g_u32Length > 0)
				{
					if (g_u32Length > BULK_EP_MXPLD)
						g_u8Size = BULK_EP_MXPLD;
					else
						g_u8Size = g_u32Length;

					DrvUSB_DataIn(BULK_IN_EP_NUM, (const uint8_t *)g_u32Address, g_u8Size);
					g_u32Address += g_u8Size;
					g_u32Length -= g_u8Size;
					
					g_u32BytesInStorageBuf -= g_u8Size;
				}
				else
					UsbBulkInAck ();
				return TRUE;
			}
			case 0x43:
			{
				Command_43();
				g_u8BulkState = BULK_IN;
				return TRUE;
			}			
			case UFI_READ_CAPACITY:
			{
				if (bFirstCMD == 0xFF)
					bFirstCMD = 0x25;
				
				if(g_u32Length == 0)
				{
    				g_u32Length = g_sCBW.dCBWDataTransferLength;
					g_u32Address = (uint32_t)Mass_Base_Addr;
				}
				
			    *((uint32_t *)Mass_Base_Addr) = 0;
			    *((uint32_t *)Mass_Base_Addr) = 2048;
			    	
				g_u8BulkState = BULK_IN;
				
				if (g_u32Length > 0)
				{
					if (g_u32Length > BULK_EP_MXPLD)
						g_u8Size = BULK_EP_MXPLD;
					else
						g_u8Size = g_u32Length;

					DrvUSB_DataIn(BULK_IN_EP_NUM, (const uint8_t *)g_u32Address, g_u8Size);
					g_u32Address += g_u8Size;
					g_u32Length -= g_u8Size;
					
					g_u32BytesInStorageBuf -= g_u8Size;
					
				}
				else
					UsbBulkInAck ();
				return TRUE;
			}
			case UFI_MODE_SENSE_10:
			{
				if (bFirstCMD == 0xFF)
					bFirstCMD = 0x5A;

				if (g_u32Length == 0)
				{
					// LBA
					g_u32Address = get_be32(&g_sCBW.au8Data[0]);
					g_u32Length = g_sCBW.dCBWDataTransferLength;
					nvtModeSense(g_u32Length, (uint8_t *)Storage_Base_Addr);
				}
				g_u32Address = (uint32_t)Storage_Base_Addr;
				g_u8BulkState = BULK_IN;
				if (g_u32Length > 0)
				{
					if (g_u32Length > BULK_EP_MXPLD)
						g_u8Size = BULK_EP_MXPLD;
					else
						g_u8Size = g_u32Length;

					DrvUSB_DataIn(BULK_IN_EP_NUM, (const uint8_t *)g_u32Address, g_u8Size);
					g_u32Address += g_u8Size;
					g_u32Length -= g_u8Size;	
					g_u32BytesInStorageBuf -= g_u8Size;			
				}
				else
					UsbBulkInAck ();
				return TRUE;
			}
			case 0xBE:
			{
				if (g_u32Length == 0)
				{
					// LBA
					g_u32Address = get_be32(&g_sCBW.au8Data[0]);
					g_u32Length = g_sCBW.dCBWDataTransferLength;
					memset((char *)Storage_Base_Addr, 0x00, g_u32Length);
				}
				g_u32Address = (uint32_t)Storage_Base_Addr;
				g_u8BulkState = BULK_IN;
				if (g_u32Length > 0)
				{
					if (g_u32Length > BULK_EP_MXPLD)
						g_u8Size = BULK_EP_MXPLD;
					else
						g_u8Size = g_u32Length;

					DrvUSB_DataIn(BULK_IN_EP_NUM, (const uint8_t *)g_u32Address, g_u8Size);
					g_u32Address += g_u8Size;
					g_u32Length -= g_u8Size;	
					g_u32BytesInStorageBuf -= g_u8Size;								
				}
				else
					UsbBulkInAck ();
				return TRUE;
			}			
			case UFI_INQUIRY:
			{
				g_u8BulkState = BULK_IN;
				
				u32Len = g_sCBW.dCBWDataTransferLength;
				if (u32Len > 36)
					u32Len = 36;
		
				DrvUSB_DataIn(BULK_IN_EP_NUM, InquiryID, u32Len);
				return TRUE;
			}
			case UFI_READ_10:
			{
			    /* Check if it is a new transfer */
				if (g_u32Length == 0)
				{
					/* Get LBA address */
					g_u32Address = get_be32(&g_sCBW.au8Data[0]);
					g_u32LbaAddress = g_u32Address * CDROM_BLOCK_SIZE;
					g_u32Length = g_sCBW.dCBWDataTransferLength;
					
					DBG_PRINTF("READ: 0x%08x size %d ... ", g_u32Address * UDC_BLOCK_SIZE + MASS_STORAGE_OFFSET, g_u32Length);
								
					g_u32BytesInStorageBuf = g_u32Length;

					if (g_sCBW.bCBWLUN == F_SPI_LUN)
					{
					    i = g_u32Length;
					    
                        if (i > STORAGE_BUFFER_SIZE)
                            i = STORAGE_BUFFER_SIZE;

                        SpiRead(g_u32Address * CDROM_BLOCK_SIZE + MASS_STORAGE_OFFSET, i, (uint32_t)STORAGE_DATA_BUF);
                        g_u32BytesInStorageBuf = i;
                        g_u32LbaAddress += i;
					}
				}

                g_u32Address = STORAGE_DATA_BUF; 
                                
                /* Indicate the next packet should be Bulk IN Data packet */
				g_u8BulkState = BULK_IN;
				
				if (g_u32BytesInStorageBuf > 0)
				{
				    /* Set the packet size */
					if (g_u32BytesInStorageBuf > BULK_EP_MXPLD)
						g_u8Size = BULK_EP_MXPLD;
					else
						g_u8Size = g_u32BytesInStorageBuf;					
					
					DrvUSB_DataIn(BULK_IN_EP_NUM, (const uint8_t *)g_u32Address, g_u8Size);
					//DBG_PRINTF("BULK IN: 0x%08x size:%d\n", g_u32Address, g_u8Size);
					//DBG_PRINTF(".");
								
					g_u32Address += g_u8Size;
					g_u32Length -= g_u8Size;										
					g_u32BytesInStorageBuf -= g_u8Size;
				}
				else
				{				    
					UsbBulkInAck();				
				}
					
				return TRUE;
			}
			default:
			{
			
			    DBG_PRINTF("WARNING: Un-supported command! code:%d\n", g_sCBW.u8OPCode);
			     
        		/* Unsupported command */
        		g_au8SenseKey[0] = 0x05;
        		g_au8SenseKey[1] = 0x20;
        		g_au8SenseKey[2] = 0x00;
        		
        		/* If CBW request for data phase, just return zero packet to end data phase */
        		if (g_sCBW.dCBWDataTransferLength > 0)
        		{
        			/* Data Phase, zero/short packet */
        			if ((g_sCBW.bmCBWFlags & 0x80) != 0)
        			{	
        			    /* Data-In */
        				g_u8BulkState = BULK_IN;
        				DrvUSB_DataIn(BULK_IN_EP_NUM, NULL, 0);
        			}
        		}
        		else
        		{	/* Status Phase */
        			g_u8BulkState = BULK_IN;
        			UsbBulkInAck ();
        		}
			    return TRUE;
			}
		}
	}
	else if (g_u8BulkState == BULK_OUT)
	{
	
		switch (g_sCBW.u8OPCode)
		{
			case UFI_WRITE_10:
			case UFI_MODE_SELECT_10:
			{
				UsbWrite();
				return TRUE;
			}
		}
	}
	return TRUE;
}





/*************************************************************************/
/*                                                                       */
/* DESCRIPTION                                                           */
/*      interrupt pipe call back function                                */
/*                                                                       */
/* INPUTS                                                                */
/*      pVoid     parameter passed by g_sUsbOps[]                        */
/*                                                                       */
/* OUTPUTS                                                               */
/*      None                                                             */
/*                                                                       */
/* RETURN                                                                */
/*      None                                                             */
/*                                                                       */
/*************************************************************************/
void HID_IntInCallback(void * pVoid)
{
    S_CDROM_DEVICE* psDevice = (S_CDROM_DEVICE*) pVoid;

    if ((uint32_t)pfHID_GetInReport)
        pfHID_GetInReport(psDevice->pu8Report);

    DrvUSB_DataIn(HID_IN_EP_NUM, psDevice->pu8Report, psDevice->u32ReportSize);
}

void HID_IntOutCallback(void * pVoid)
{
    uint8_t *pu8EpBuf;
    uint32_t u32Size;

    pu8EpBuf = DrvUSB_GetOutData(HID_OUT_EP_NUM, &u32Size);

    if ((uint32_t)pfHID_SetOutReport)
        pfHID_SetOutReport(pu8EpBuf);

    DrvUSB_DataOutTrigger(HID_OUT_EP_NUM, INT_EP_MXPLD);
}

/*************************************************************************/
/*                                                                       */
/* DESCRIPTION                                                           */
/*      Set report descriptor. if not set, default will be used          */
/*      default HID report descriptor is mouse.                          */
/*                                                                       */
/* INPUTS                                                                */
/*      pu8ReportDescriptor     report descriptor buffer                 */
/*      u32ReportDescriptorSize report descriptor size                   */
/*                                                                       */
/* OUTPUTS                                                               */
/*      None                                                             */
/*                                                                       */
/* RETURN                                                                */
/*      0           Success                                              */
/*		Otherwise	error												 */
/*                                                                       */
/*************************************************************************/
int32_t HID_SetReportDescriptor(const uint8_t* pu8ReportDescriptor, uint32_t u32ReportDescriptorSize)
{
    if (pu8ReportDescriptor == NULL)
    {
        return (E_HID_NULL_POINTER);
    }

    gsCdromDevice.pu8ReportDescriptor = pu8ReportDescriptor;
    gsCdromDevice.u32ReportDescriptorSize = u32ReportDescriptorSize;

    return E_SUCCESS;
}

/*************************************************************************/
/*                                                                       */
/* DESCRIPTION                                                           */
/*      Set report buffer and size for interrupt pipe                    */
/*                                                                       */
/* INPUTS                                                                */
/*      pu8Report     buffer that will be sent to host when interupt IN  */
/*						happen                                           */
/*      u32ReportSize     buffer size                                    */
/*                                                                       */
/* OUTPUTS                                                               */
/*      None                                                             */
/*                                                                       */
/* RETURN                                                                */
/*      0           Success                                              */
/*		Otherwise	error												 */
/*                                                                       */
/*************************************************************************/
int32_t HID_SetReportBuf(uint8_t* pu8Report, uint32_t u32ReportSize)
{
    if (pu8Report == NULL)
    {
        return (E_HID_NULL_POINTER);
    }
    if (u32ReportSize > INT_EP_MXPLD)
    {
        return (E_HID_EXCEED_INT_IN_PACKET_SIZE);
    }

    gsCdromDevice.pu8Report = pu8Report;
    gsCdromDevice.u32ReportSize = u32ReportSize;

    return E_SUCCESS;
}

void HID_CtrlSetupGetReport(void * pVoid)
{
    S_DRVUSB_DEVICE *psDevice = (S_DRVUSB_DEVICE *)((S_CDROM_DEVICE *)pVoid)->device;

    DBG_PRINTF("HID - Get Report");
    if (psDevice->au8Setup[3] == 3)
    {
        /* Request Type = Feature */
        const uint8_t au8Data[2] = {0x01, 0x02};

        gpu8UsbBuf = au8Data;
        gu32BytesInUsbBuf = 0;
        DrvUSB_DataIn(0, au8Data, 2);
    }
    else
    {
        DBG_PRINTF(" - Unknown\n");
        _HID_CLR_CTRL_READY_AND_TRIG_STALL();
    }
}

/*************************************************************************/
/*                                                                       */
/* DESCRIPTION                                                           */
/*      The handler of Set Report request of HID request.                */
/*                                                                       */
/* INPUTS                                                                */
/*      pVoid     parameter passed by DrvUSB_InstallCtrlHandler          */
/*                                                                       */
/* OUTPUTS                                                               */
/*      None                                                             */
/*                                                                       */
/* RETURN                                                                */
/*      0           Success                                              */
/*		Otherwise	error												 */
/*                                                                       */
/*************************************************************************/
// 21 09 02 03 02 00 03 00	 (Report ID = 0x02)
// 02(ID) 02(MODE) 78(contact ID)
//
void HID_CtrlSetupSetReport(void * pVoid)
{
    S_DRVUSB_DEVICE *psDevice = (S_DRVUSB_DEVICE *)((S_CDROM_DEVICE *)pVoid)->device;

    DBG_PRINTF("HID - Set Report");
    if (psDevice->au8Setup[3] == HID_RPT_TYPE_INPUT)
    {
        /* Report Type = input */
//		_DRVUSB_SET_EP_TOG_BIT(1,FALSE);
//		_DRVUSB_TRIG_EP(1, 0x00);
        DBG_PRINTF(" - Input\n");
    }
    else if (psDevice->au8Setup[3] == HID_RPT_TYPE_OUTPUT)
    {
        /* Report Type = Output */
//		_DRVUSB_SET_EP_TOG_BIT(1,FALSE);
//		_DRVUSB_TRIG_EP(1,0x00);
        DBG_PRINTF(" - Output\n");
    }
    else if (psDevice->au8Setup[3] == HID_RPT_TYPE_FEATURE)
    {
        /* Request Type = Feature */

        DrvUSB_DataOutTrigger(CTRL_EP_NUM, CDROM_MAX_PACKET_SIZE_EP0);

        DBG_PRINTF(" - Feature\n");
    }
    else
    {
        // Not support. Reply STALL.
        DBG_PRINTF(" - Unknown\n");

        _HID_CLR_CTRL_READY_AND_TRIG_STALL();
    }
    DBG_PRINTF("\n");
}


/*************************************************************************/
/*                                                                       */
/* DESCRIPTION                                                           */
/*      The handler of Set Idle request of HID request.                  */
/*                                                                       */
/* INPUTS                                                                */
/*      pVoid     parameter passed by DrvUSB_InstallCtrlHandler          */
/*                                                                       */
/* OUTPUTS                                                               */
/*      None                                                             */
/*                                                                       */
/* RETURN                                                                */
/*      0           Success                                              */
/*		Otherwise	error												 */
/*                                                                       */
/*************************************************************************/
void HID_CtrlSetupSetIdle(void * pVoid)
{

    _DRVUSB_SET_EP_TOG_BIT(0, FALSE);
    _DRVUSB_TRIG_EP(0, 0x00);


    DBG_PRINTF("Set idle\n");
}


/*************************************************************************/
/*                                                                       */
/* DESCRIPTION                                                           */
/*      The handler of Set Protocol request of HID request.              */
/*                                                                       */
/* INPUTS                                                                */
/*      pVoid     parameter passed by DrvUSB_InstallCtrlHandler          */
/*                                                                       */
/* OUTPUTS                                                               */
/*      None                                                             */
/*                                                                       */
/* RETURN                                                                */
/*      0           Success                                              */
/*		Otherwise	error												 */
/*                                                                       */
/*************************************************************************/
void HID_CtrlSetupSetProtocol(void * pVoid)
{

    _DRVUSB_SET_EP_TOG_BIT(0, FALSE);
    _DRVUSB_TRIG_EP(0, 0x00);

    DBG_PRINTF("Set protocol");

}

