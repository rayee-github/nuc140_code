/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) 2009 Nuvoton Technology Corp. All rights reserved.                                         */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#include <string.h>
#include "DrvUSB.h"
#include "HIDSysIO.h"
#include "HID_API.h"


#define DBG_PRINTF(...)
//#define DBG_PRINTF printf

#define Maximum(a,b)		((a)>(b) ? (a) : (b))
#define Minimum(a,b)		((a)<(b) ? (a) : (b))

S_HID_DEVICE g_HID_sDevice;

/* Endpoint description */
S_DRVUSB_EP_CTRL sEpDescription[] = 
{
	{CTRL_EP_NUM    | EP_INPUT,  HID_MAX_PACKET_SIZE_EP0, NULL},
	{CTRL_EP_NUM    | EP_OUTPUT, HID_MAX_PACKET_SIZE_EP0, NULL},
	{HID_IN_EP_NUM  | EP_INPUT,  HID_MAX_PACKET_SIZE_EP1, NULL},
	{HID_OUT_EP_NUM | EP_OUTPUT, HID_MAX_PACKET_SIZE_EP2, NULL},
	{0x00,0,NULL}
};

/* bus event call back */
S_DRVUSB_EVENT_PROCESS g_sBusOps[6] = 
{
	{NULL, NULL},                               /* attach event callback */
	{NULL, NULL},                               /* detach event callback */
	{DrvUSB_BusResetCallback, &g_HID_sDevice},  /* bus reset event callback */
	{NULL, NULL},                               /* bus suspend event callback */
	{NULL, NULL},                               /* bus resume event callback */
	{DrvUSB_CtrlSetupAck, &g_HID_sDevice},      /* setup event callback */
};

/* USB event call back */
S_DRVUSB_EVENT_PROCESS g_sUsbOps[12] = 
{
	{DrvUSB_CtrlDataInAck   , &g_HID_sDevice},/* ctrl pipe0 (EP address 0) In ACK callback */
	{DrvUSB_CtrlDataOutAck  , &g_HID_sDevice},/* ctrl pipe0 (EP address 0) Out ACK callback */
	{HID_IntInCallback      , &g_HID_sDevice},/* EP address 1 In ACK callback */
	{NULL, NULL},							  /* EP address 1 Out ACK callback */
	{NULL, NULL},                             /* EP address 2 In ACK callback */  
	{HID_IntOutCallback		, &g_HID_sDevice},/* EP address 2 Out ACK callback */   
	{NULL, NULL},                             /* EP address 3 In ACK callback */  
	{NULL, NULL},                             /* EP address 3 Out ACK callback */  
	{NULL, NULL},                             /* EP address 4 In ACK callback */  
	{NULL, NULL},                             /* EP address 4 Out ACK callback */  
	{NULL, NULL},                             /* EP address 5 In ACK callback */  
	{NULL, NULL},                             /* EP address 5 Out ACK callback */  
};

/*ctrl pipe call back.                                                                  */
/*it will be call by DrvUSB_CtrlSetupAck, DrvUSB_CtrlDataInAck and DrvUSB_CtrlDataOutAck*/
/*if in ack handler and out ack handler is 0, default handler will be called            */
S_DRVUSB_CTRL_CALLBACK_ENTRY g_asCtrlCallbackEntry[] =
{   //request type,command     ,setup ack handler         , in ack handler      ,out ack handler,  parameter
	{REQ_STANDARD, SET_ADDRESS, DrvUSB_CtrlSetupSetAddress, DrvUSB_CtrlDataInSetAddress, 0, &g_HID_sDevice},
	{REQ_STANDARD, CLEAR_FEATURE, DrvUSB_CtrlSetupClearSetFeature, 0, 0, &g_HID_sDevice},
	{REQ_STANDARD, SET_FEATURE, DrvUSB_CtrlSetupClearSetFeature, 0, 0, &g_HID_sDevice},
	{REQ_STANDARD, GET_CONFIGURATION, DrvUSB_CtrlSetupGetConfiguration, 0, 0, &g_HID_sDevice},
	{REQ_STANDARD, GET_STATUS, DrvUSB_CtrlSetupGetStatus, 0, 0, &g_HID_sDevice},
	{REQ_STANDARD, GET_INTERFACE, DrvUSB_CtrlSetupGetInterface, 0, 0, &g_HID_sDevice},
	{REQ_STANDARD, SET_INTERFACE, DrvUSB_CtrlSetupSetInterface, 0, 0, &g_HID_sDevice},
	{REQ_STANDARD, GET_DESCRIPTOR, HID_CtrlSetupGetDescriptor, HID_CtrlGetDescriptorIn, HID_CtrlGetDescriptorOut, &g_HID_sDevice},
	{REQ_STANDARD, SET_CONFIGURATION, DrvUSB_CtrlSetupSetConfiguration, 0, 0, &g_HID_sDevice},
	/* To support boot protocol */
	{REQ_CLASS, GET_REPORT, HID_CtrlSetupGetReport, 0, 0, &g_HID_sDevice},
	//{REQ_CLASS, SET_REPORT, HID_CtrlSetupSetReport, 0, HID_CtrlSetupDataOutReport, &g_HID_sDevice},
	{REQ_CLASS, SET_REPORT, HID_CtrlSetupSetReport, 0, 0, &g_HID_sDevice},
	{REQ_CLASS, SET_IDLE, HID_CtrlSetupSetIdle, 0, 0, &g_HID_sDevice},
	{REQ_CLASS, SET_PROTOCOL, HID_CtrlSetupSetProtocol, 0, 0, &g_HID_sDevice},
};

/* The callback is called when Host issues the interrupt IN transfer to get report */
void (*pfHID_GetInReport)(uint8_t *buf) = NULL;

/* The callback is called when Host issues the interrupt OUT transfer to set report */
void (*pfHID_SetOutReport)(uint8_t *buf) = NULL;

//register to USB driver
S_DRVUSB_CLASS sHidUsbClass = 
{
	(void *)&g_HID_sDevice, 
	HID_UsbStartCallBack,
	HID_IsConfigureValue,
    0
};


const uint8_t * gpu8UsbBuf = 0;
uint32_t gu32BytesInUsbBuf = 0;
int8_t gIsOverRequest = 0;

uint32_t HID_GetVersion(void)
{
	return HID_VERSION_NUM;
}

/*************************************************************************/
/*                                                                       */
/* DESCRIPTION                                                           */
/*      To initial the descriptors and install the handlers.             */
/*                                                                       */
/* INPUTS                                                                */
/*      pVoid     not used now                                           */
/*                                                                       */
/* OUTPUTS                                                               */
/*      None                                                             */
/*                                                                       */
/* RETURN                                                                */
/*      0           Success                                              */
/*		Otherwise	error												 */
/*                                                                       */
/*************************************************************************/
int32_t HID_Open(uint32_t inReportSize, void *pfGetInReport, void *pfSetOutReport)
{
	int32_t i32Ret = E_SUCCESS;

    if((uint32_t)pfGetInReport != NULL)
        pfHID_GetInReport = (void(*)(uint8_t *))pfGetInReport;
        
    if((uint32_t)pfSetOutReport != NULL)
        pfHID_SetOutReport = (void(*)(uint8_t *))pfSetOutReport;

	g_HID_sDevice.device = (void *)DrvUSB_InstallClassDevice(&sHidUsbClass);
	
	g_HID_sDevice.au8DeviceDescriptor = gau8DeviceDescriptor;
	g_HID_sDevice.au8ConfigDescriptor = gau8ConfigDescriptor;
	g_HID_sDevice.pu8HIDDescriptor = g_HID_sDevice.au8ConfigDescriptor + LEN_CONFIG + LEN_INTERFACE;	
	g_HID_sDevice.pu8IntInEPDescriptor = g_HID_sDevice.au8ConfigDescriptor + LEN_CONFIG + LEN_INTERFACE + LEN_HID;
	
	g_HID_sDevice.u32ReportSize = inReportSize;

	
	i32Ret = DrvUSB_InstallCtrlHandler(g_HID_sDevice.device, g_asCtrlCallbackEntry, 
					sizeof(g_asCtrlCallbackEntry) / sizeof(g_asCtrlCallbackEntry[0]));
	return i32Ret;
}

void HID_Close(void)
{
}

static void HID_UsbStartCallBack(void * pVoid)
{
	HID_Reset((S_HID_DEVICE *)pVoid);
	HID_Start((S_HID_DEVICE *)pVoid);
}

static void HID_Reset(S_HID_DEVICE *psDevice)
{	
	DrvUSB_Reset(HID_IN_EP_NUM);
	DrvUSB_Reset(HID_OUT_EP_NUM);
 }

void HID_Start(S_HID_DEVICE *psDevice)
{
	DrvUSB_DataIn(HID_IN_EP_NUM, psDevice->pu8Report, psDevice->u32ReportSize);
   	DrvUSB_DataOutTrigger(HID_OUT_EP_NUM, HID_MAX_PACKET_SIZE_EP2);
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
    S_HID_DEVICE* psDevice = (S_HID_DEVICE*) pVoid;

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

    DrvUSB_DataOutTrigger(HID_OUT_EP_NUM, HID_MAX_PACKET_SIZE_EP2);
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

	g_HID_sDevice.pu8ReportDescriptor = pu8ReportDescriptor;
	g_HID_sDevice.u32ReportDescriptorSize = u32ReportDescriptorSize;
	
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
	if (u32ReportSize > HID_MAX_PACKET_SIZE_EP1)
	{
		return (E_HID_EXCEED_INT_IN_PACKET_SIZE);
	}

	g_HID_sDevice.pu8Report = pu8Report;
	g_HID_sDevice.u32ReportSize = u32ReportSize;

	return E_SUCCESS;
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
static int32_t HID_IsConfigureValue(uint8_t u8ConfigureValue)
{
	return (u8ConfigureValue == gau8ConfigDescriptor[5]);
}


void HID_CtrlSetupGetReport(void * pVoid)
{
    S_DRVUSB_DEVICE *psDevice = (S_DRVUSB_DEVICE *)((S_HID_DEVICE *)pVoid)->device;
    
	DBG_PRINTF("HID - Get Report");
    if(psDevice->au8Setup[3] == 3)
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
 	S_DRVUSB_DEVICE *psDevice = (S_DRVUSB_DEVICE *)((S_HID_DEVICE *)pVoid)->device;

    DBG_PRINTF("HID - Set Report");
	if(psDevice->au8Setup[3] == HID_RPT_TYPE_INPUT)
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

        DrvUSB_DataOutTrigger(CTRL_EP_NUM, HID_MAX_PACKET_SIZE_EP0);

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
    
	_DRVUSB_SET_EP_TOG_BIT(0,FALSE);
	_DRVUSB_TRIG_EP(0,0x00);


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

	_DRVUSB_SET_EP_TOG_BIT(0,FALSE);
	_DRVUSB_TRIG_EP(0,0x00);

    DBG_PRINTF("Set protocol");

}


void HID_PrepareDescriptors(const uint8_t *pu8Descriptor, uint32_t u32DescriptorSize, uint32_t u32RequestSize, uint32_t u32MaxPacketSize)
{
    
    gu32BytesInUsbBuf = u32RequestSize;
    if(u32RequestSize > u32DescriptorSize)
    {
        gu32BytesInUsbBuf = u32DescriptorSize;
        gIsOverRequest = 1;
    }
    gpu8UsbBuf = pu8Descriptor;

    DBG_PRINTF("Get descriptor 0x%08x %d size.\n", pu8Descriptor, u32DescriptorSize);

	if(gu32BytesInUsbBuf < u32MaxPacketSize)
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


void HID_CtrlGetDescriptorOut(void * pVoid)
{
    gu32BytesInUsbBuf = 0;
    gpu8UsbBuf = 0;
    gIsOverRequest = 0;
}

void HID_CtrlGetDescriptorIn(void * pVoid)
{
    uint32_t u32Len;


    DBG_PRINTF(" >>> 0x%08x %d size.\n", gpu8UsbBuf, gu32BytesInUsbBuf);
	
    if(gpu8UsbBuf)
    {

        if(gu32BytesInUsbBuf == 0)
        {
            /* Zero packet */
    		DrvUSB_DataIn(0, gpu8UsbBuf, 0);
    		gpu8UsbBuf = 0;
        }
        else
        {
            u32Len = Minimum(gu32BytesInUsbBuf, HID_MAX_PACKET_SIZE_EP0);
    		DrvUSB_DataIn(0, gpu8UsbBuf, u32Len);
    		gpu8UsbBuf += u32Len;
    		gu32BytesInUsbBuf -= u32Len;
    		
    		if(gu32BytesInUsbBuf == 0)
    		{
                if(u32Len < HID_MAX_PACKET_SIZE_EP0)
                {
                    /* This should be last IN packet due to it is less than UAC_MAX_PACKET_SIZE_EP0 */
                    gpu8UsbBuf = 0;
                }
                else
                {
                    if(!gIsOverRequest)
                    {
    		            /* This should be the last IN packet because there is no more data to 
                           transfer and it is not over request transfer */
                        gpu8UsbBuf = 0;
                    }
                 }
            }
    		
        }
    }
    else
    {
  	    /* The EP id 1 should always be used as control (OUT) endpoint */
		_DRVUSB_TRIG_EP(1,0x00);
    }
}


/*************************************************************************/
/*                                                                       */
/* DESCRIPTION                                                           */
/*      setup ACK handler for Get descriptor command                     */
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
void HID_CtrlSetupGetDescriptor(void * pVoid)
{
	S_HID_DEVICE *psDevice = (S_HID_DEVICE *) pVoid;
	S_DRVUSB_DEVICE *pUsbDevice = (S_DRVUSB_DEVICE *)psDevice->device;
	uint16_t u16Len;

	u16Len = 0;
	u16Len = pUsbDevice->au8Setup[7];
	u16Len <<= 8;
	u16Len += pUsbDevice->au8Setup[6];
	
	gIsOverRequest = 0;
	gu32BytesInUsbBuf = 0;
	gpu8UsbBuf = 0;
	switch (pUsbDevice->au8Setup[3])
	{
		// Get Device Descriptor
	case DESC_DEVICE:
	{
        HID_PrepareDescriptors(g_HID_sDevice.au8DeviceDescriptor, LEN_DEVICE, u16Len, HID_MAX_PACKET_SIZE_EP0);

	    /* Prepare the OUT to avoid HOST stop data phase without all data transfered. */
		_DRVUSB_TRIG_EP(1,0x00);

		break;
	}

	// Get Configuration Descriptor
	case DESC_CONFIG:
	{	
        HID_PrepareDescriptors(g_HID_sDevice.au8ConfigDescriptor, gau8ConfigDescriptor[2], u16Len, HID_MAX_PACKET_SIZE_EP0);
		break;
    }
		// Get HID Descriptor
	case DESC_HID:
    {
        HID_PrepareDescriptors(g_HID_sDevice.pu8HIDDescriptor, LEN_HID, u16Len, HID_MAX_PACKET_SIZE_EP0);
		break;
    }
		// Get Report Descriptor
	case DESC_HID_RPT:
	{
        HID_PrepareDescriptors(g_HID_sDevice.pu8ReportDescriptor, g_HID_sDevice.u32ReportDescriptorSize, u16Len, HID_MAX_PACKET_SIZE_EP0);
		break;
    }
		// Get String Descriptor
	case DESC_STRING:
	{
		// Get Language
		if (pUsbDevice->au8Setup[2] == 0)
		{
            HID_PrepareDescriptors(gau8StringLang, 4, u16Len, HID_MAX_PACKET_SIZE_EP0);
		}
		else
		{
			// Get String Descriptor
			switch (pUsbDevice->au8Setup[2])
			{
			case 1:
                HID_PrepareDescriptors((const uint8_t *)gau8VendorStringDescriptor, gau8VendorStringDescriptor[0], u16Len, HID_MAX_PACKET_SIZE_EP0);
				break;

			case 2:
                HID_PrepareDescriptors((const uint8_t *)gau8ProductStringDescriptor, gau8ProductStringDescriptor[0], u16Len, HID_MAX_PACKET_SIZE_EP0);
               
 				break;

			case 3:
                HID_PrepareDescriptors(gau8StringSerial, gau8StringSerial[0], u16Len, HID_MAX_PACKET_SIZE_EP0);
				break;

			default:
				/* Not support. Reply STALL. */
				DrvUSB_ClrCtrlReadyAndTrigStall();
			}
		}

		break;
	}
	default:
		/* Not support. Reply STALL. */
		DrvUSB_ClrCtrlReadyAndTrigStall();
	}
}

