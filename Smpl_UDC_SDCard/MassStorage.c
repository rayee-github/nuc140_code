/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright (c) Nuvoton Technology Corp. All rights reserved.                                             */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>

#include "Driver\DrvUSB.h"
#include "UDC.h"
#include "DrvSDCARD.h"

#define VENDER_CMD  0x61

#define TEST_SPI
//#define TEST_RAM  /* Only for IC verification */

//#define SPI_STORAGE_SIZE    (0*1024)  /* Configure the SPI storage size */
#define MASS_BUFFER_SIZE    48          /* Mass Storage command buffer size */
#define STORAGE_BUFFER_SIZE 1024        /* Data transfer buffer size in 512 bytes alignment */
#define	UDC_SECTOR_SIZE		512         /* logic sector size */

#define VENDOR_ID   0x0416
#define PRODUCT_ID  0x501E


//int32_t gTotalSectors = 0;
int32_t g_spiTotalSectors = 0;

uint8_t volatile MassLUN=0;
uint8_t volatile F_SPI_LUN=0xFF;

/* SPI Flash Programming API */


/* USB flow control variables */
uint8_t g_u8UsbState;
uint8_t g_u8Address;
uint8_t g_u8Config;
uint8_t g_u8Flag;
uint8_t g_u8BulkState;
//uint8_t g_au8SenseKey[3];
uint8_t g_au8SenseKey[4];

uint32_t g_u32SpiStartAddr;
uint32_t g_u32Address;
uint32_t g_u32Length;
uint32_t g_u32LbaAddress;
uint32_t g_u32BytesInStorageBuf;
uint32_t g_u32OutToggle;	// for Error Detection
uint8_t preventflag=0;
uint8_t g_u8Size;
int32_t g_bCBWInvalid;
int32_t udcOnLine = 0;

/* CBW/CSW variables */
//struct CBW g_sCBW;
//struct CSW g_sCSW;
struct CBW g_sCBW;
struct CSW g_sCSW;

uint32_t MassBlock[MASS_BUFFER_SIZE / 4];
uint32_t Storage_Block[STORAGE_BUFFER_SIZE / 4];

#define MassCMD_BUF        ((uint32_t)&MassBlock[0])
#define STORAGE_DATA_BUF   ((uint32_t)&Storage_Block[0])

// code = 5Ah, Mode Sense
static uint8_t ModePage_01[12] = {
	0x01, 0x0A, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00,
	0x03, 0x00, 0x00, 0x00 };

static uint8_t ModePage_05[32] = {
	0x05, 0x1E, 0x03, 0xE8, 0x02, 0x3F, 0x02, 0x00,
	0x01, 0xF4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x05, 0x1E, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x01, 0x68, 0x00, 0x00 };

static uint8_t ModePage_1B[12] = {
	0x1B, 0x0A, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00 };

static uint8_t ModePage_1C[8] = {
	0x1C, 0x06, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00 };

//======================================================
const uint8_t MassDeviceDescriptor[LEN_DEVICE] =
{
	LEN_DEVICE,		    // bLength
	DESC_DEVICE,	    // bDescriptorType
	0x10, 0x01,		    // bcdUSB
	0x00,			    // bDeviceClass
	0x00,			    // bDeviceSubClass
	0x00,			    // bDeviceProtocol
	MAX_PACKET_SIZE0,	// bMaxPacketSize0
	(VENDOR_ID & 0xFF), 
	(VENDOR_ID >> 8),   // idVendor
	(PRODUCT_ID & 0xFF), 
	(PRODUCT_ID >> 8),  // idProduct
	0x00, 0x00,		    // bcdDevice
	0x01,			    // iManufacture
	0x02,			    // iProduct
	0x01,			    // iSerialNumber
	0x01			    // bNumConfigurations
};

const uint8_t MassConfigurationBlock[LEN_CONFIG+LEN_INTERFACE+LEN_ENDPOINT*2] =
{
	LEN_CONFIG,		                                        // bLength
	DESC_CONFIG,	                                        // bDescriptorType
	(LEN_CONFIG+LEN_INTERFACE+LEN_ENDPOINT*2), 0x00,		// wTotalLength
	0x01,			                                        // bNumInterfaces
	0x01,			                                        // bConfigurationValue
	0x00,			                                        // iConfiguration
	0xC0,			                                        // bmAttributes
	0x32,			                                        // MaxPower

/* const BYTE cbyInterfaceDescriptor[LEN_INTERFACE] = */
	LEN_INTERFACE,	// bLength
	DESC_INTERFACE,	// bDescriptorType
	0x00,			// bInterfaceNumber
	0x00,			// bAlternateSetting
	0x02,			// bNumEndpoints
	0x08,			// bInterfaceClass
	0x06,			// bInterfaceSubClass
	0x50,			// bInterfaceProtocol
	0x00,			// iInterface

/* const BYTE cbyEndpointDescriptor1[LEN_ENDPOINT] = */
	LEN_ENDPOINT,	        // bLength
	DESC_ENDPOINT,	        // bDescriptorType
	0x82,			        // bEndpointAddress
	EP_BULK,		        // bmAttributes
	MAX_PACKET_SIZE, 0x00,	// wMaxPacketSize
	0x00,			        // bInterval

/* const BYTE cbyEndpointDescriptor2[LEN_ENDPOINT] = */
	LEN_ENDPOINT,	        // bLength
	DESC_ENDPOINT,	        // bDescriptorType
	0x03,			        // bEndpointAddress
	EP_BULK,		        // bmAttributes
	MAX_PACKET_SIZE, 0x00,	// wMaxPacketSize
	0x00			        // bInterval
};

const uint8_t MassStringDescriptor0[4] =
{
	4,				// bLength
	DESC_STRING,	// bDescriptorType
	0x09, 0x04
};

const uint8_t MassStringDescriptor1[16] =
{
	16,				// bLength
	DESC_STRING,	// bDescriptorType
	'N', 0, 'u', 0, 'v', 0, 'o', 0, 't', 0, 'o', 0, 'n', 0
};

const uint8_t InquiryID[36] = 
{
	0x00,					// Peripheral Device Type
	0x80,					// RMB
	0x00,					// ISO/ECMA, ANSI Version
	0x00,					// Response Data Format
	0x1F, 0x00, 0x00, 0x00,	// Additional Length

	/* Vendor Identification */
	'N', 'u', 'v', 'o', 't', 'o', 'n', ' ',

	/* Product Identification */
	'1', '0', '0', ' ', 'M', 'a', 's', 's', ' ', 'S', 't', 'o', 'r', 'a', 'g', 'e',

	/* Product Revision */
	'1', '.', '0', '0'
};


static uint32_t get_be32(uint8_t *buf)
{
	return ((uint32_t) buf[0] << 24) | ((uint32_t) buf[1] << 16) |
			((uint32_t) buf[2] << 8) | ((uint32_t) buf[3]);
}

void RoughDelay(uint32_t t)
{
    volatile int32_t delay;

    delay = t;

    while(delay-- >= 0);
}

void my_memcpy(void *dest, void *src, int32_t size)
{
    int32_t i;

    for(i=0;i<size;i++)
       *((uint8_t *)dest + i) = *((uint8_t *)src + i);
}


void ModeSenseCommand(void)
{
	uint8_t i,j;
	uint8_t NumHead,NumSector;
	uint16_t NumCyl;

    /* Clear the command buffer */
    *((uint32_t *)MassCMD_BUF  ) = 0;
    *((uint32_t *)MassCMD_BUF+1) = 0;

	switch (g_sCBW.au8Data[0])
	{
		case 0x01:
		{
			outpb(MassCMD_BUF, 19);
			for (j = 0,i = 8 ; j<12; j++, i++)
				outpb(MassCMD_BUF+i, ModePage_01[j]);
			break;
		}
		case 0x05:
		{
			outpb(MassCMD_BUF, 39);
			
			for (j = 0,i=8; j<32; j++, i++)
				outpb(MassCMD_BUF+i, ModePage_05[j]);
			
			NumHead = 2;
			NumSector = 64;

#ifdef TEST_RAM
			NumCyl = gTotalSectors/128;
#endif

#ifdef TEST_SPI
			if (g_sCBW.bCBWLUN==F_SPI_LUN)
				NumCyl = g_spiTotalSectors/128;
#endif

			outpb(MassCMD_BUF+12, NumHead);
			outpb(MassCMD_BUF+13, NumSector);
			outpb(MassCMD_BUF+16, (uint8_t)(NumCyl >> 8));
			outpb(MassCMD_BUF+17, (uint8_t)(NumCyl & 0x00ff));
			break;
		}
		case 0x1B:
		{
			outpb(MassCMD_BUF, 19);
			
			for (j = 0,i = 8; j<12; j++, i++)
				outpb(MassCMD_BUF+i, ModePage_1B[j]);
			break;
		}
		case 0x1C:
		{
			outpb(MassCMD_BUF, 15);
			
			for (j = 0,i = 8; j<8; j++, i++)
				outpb(MassCMD_BUF+i, ModePage_1C[j]);
			break;
		}
		case 0x3F:
		{
			outpb(MassCMD_BUF, 0x47);
			i = 8;
			for (j = 0; j<12; j++, i++)
				outpb(MassCMD_BUF+i, ModePage_01[j]);
			for (j = 0; j<32; j++, i++)
				outpb(MassCMD_BUF+i, ModePage_05[j]);
			for (j = 0; j<12; j++, i++)
				outpb(MassCMD_BUF+i, ModePage_1B[j]);
			for (j = 0; j<8; j++, i++)
				outpb(MassCMD_BUF+i, ModePage_1C[j]);
			
			NumHead = 2;
			NumSector = 64;

#ifdef TEST_RAM
			NumCyl = gTotalSectors/128;
#endif

#ifdef TEST_SPI
			if (g_sCBW.bCBWLUN==F_SPI_LUN)
				NumCyl = g_spiTotalSectors/128;
#endif
			outpb(MassCMD_BUF+24, NumHead);
			outpb(MassCMD_BUF+25, NumSector);
			outpb(MassCMD_BUF+28, (uint8_t)(NumCyl >> 8));
			outpb(MassCMD_BUF+29, (uint8_t)(NumCyl & 0x00ff));
			break;
		}
		default:
			g_au8SenseKey[0] = 0x05;
			g_au8SenseKey[1] = 0x24;
			g_au8SenseKey[2] = 0x00;
	}
}


void ReqSenCommand(void)
{
    uint8_t tmp[20];
    
    if (preventflag==1)
    {
        preventflag=0;
		tmp[0] = 0x70;
    }
    else
		tmp[0] = 0xf0;

	tmp[1] = 0;
	tmp[3] = 0;
	tmp[4] = 0;
	tmp[5] = 0;
	tmp[6] = 0;
	tmp[7] = 0x0A;
	tmp[8] = 0;
	tmp[9] = 0;
	tmp[10] = 0;
	tmp[11] = 0;
	tmp[14] = 0;
	tmp[15] = 0;
	tmp[16] = 0;
	tmp[17] = 0;

	tmp[2] = g_au8SenseKey[0];
	tmp[12] = g_au8SenseKey[1];
	tmp[13] = g_au8SenseKey[2];

    my_memcpy(g_au8UsbBulk1, tmp, 20);
	// Sense Key
	g_au8SenseKey[0] = 0x00;
	g_au8SenseKey[1] = 0x00;
	g_au8SenseKey[2] = 0x00;
}


void RdFmtCapCommand(void)
{
    uint32_t i;
    uint32_t TotalSectors = 0;

	for (i = 0; i < 36; i++)
		outpb(MassCMD_BUF+i, 0);

	outpb(MassCMD_BUF+3, 0x10);
#ifdef TEST_RAM
	TotalSectors = gTotalSectors;
#endif

#ifdef TEST_SPI
    if (g_sCBW.bCBWLUN==F_SPI_LUN)
		TotalSectors = g_spiTotalSectors;
#endif

	outpb(MassCMD_BUF+4, *((uint8_t *)&TotalSectors+3));
	outpb(MassCMD_BUF+5, *((uint8_t *)&TotalSectors+2));
	outpb(MassCMD_BUF+6, *((uint8_t *)&TotalSectors+1));
	outpb(MassCMD_BUF+7, *((uint8_t *)&TotalSectors+0));
	outpb(MassCMD_BUF+8, 0x02);
	outpb(MassCMD_BUF+10, 0x02);
	outpb(MassCMD_BUF+12, *((uint8_t *)&TotalSectors+3));
	outpb(MassCMD_BUF+13, *((uint8_t *)&TotalSectors+2));
	outpb(MassCMD_BUF+14, *((uint8_t *)&TotalSectors+1));
	outpb(MassCMD_BUF+15, *((uint8_t *)&TotalSectors+0));
	outpb(MassCMD_BUF+18, 0x02);
}


void RdCurCapCommand(void)
{
    int i;
    uint32_t temp;

	for (i = 0; i < 36; i++)
		outpb(MassCMD_BUF+i, 0);
#ifdef TEST_RAM 
	temp = gTotalSectors - 1;
#endif
#ifdef TEST_SPI
    if (g_sCBW.bCBWLUN==F_SPI_LUN)
        temp = g_spiTotalSectors - 1;
#endif
	outpb(MassCMD_BUF+0, *((uint8_t *)&temp+3));
	outpb(MassCMD_BUF+1, *((uint8_t *)&temp+2));
	outpb(MassCMD_BUF+2, *((uint8_t *)&temp+1));
	outpb(MassCMD_BUF+3, *((uint8_t *)&temp+0));
	outpb(MassCMD_BUF+6, 0x02);
}


//======================================================
// USB Configuration
//======================================================
void UsbCfg(void)
{
    /* Init the USB device address to 0x0 */
	_DRVUSB_SET_FADDR(0x00);

    /* Init control end point */
	USBD->STBUFSEG.STBUFSEG = 0x1F;						 		/* Buffer for setup packet */
    _DRVUSB_SET_CFG_EP0(EPT_CFGP);					  		/* Clear EP0 CTRL IN Ready Flag */
    _DRVUSB_SET_CFG(0, CFG0_SETTING);				   		/* EP0 ==> control IN end point, address 0 */
    _DRVUSB_SET_EP_BUF(0, USB_SRAM_BASE + BUF0_SETTING);	/* Buffer for EP0 */
    _DRVUSB_SET_CFG_EP1(EPT_CFGP);						   	/* Clear EP1 CTRL OUT Ready Flag */
    _DRVUSB_SET_CFG(1, CFG1_SETTING);					  	/* EP1 ==> control OUT end point, address 0 */
    _DRVUSB_SET_EP_BUF(1, USB_SRAM_BASE + BUF1_SETTING);  	/* Buffer for EP1 */
    
    /* Init Bulk IN end point */
	_DRVUSB_SET_CFG_EP2(EPT_CFGP);				 			/* Clear EP2 BULK IN Ready Flag */
	_DRVUSB_SET_CFG(2, CFG2_SETTING);			  			/* EP2 ==> Bulk IN end point, address 2 */
	_DRVUSB_SET_EP_BUF(2, USB_SRAM_BASE + BUF_BULK1); 		/* Buffer for EP2 (Bulk IN) */	
	
	/* Init Bulk OUT end point */
	_DRVUSB_SET_CFG_EP3(EPT_CFGP);			  				/* Clear EP3 BULK OUT Ready Flag */
	_DRVUSB_SET_CFG(3, CFG3_SETTING);		  				/* EP3 ==> Bulk OUT end point, address 3 */
	_DRVUSB_SET_EP_BUF(3, USB_SRAM_BASE + BUF_BULK0);		/* Buffer for EP3 (Bulk OUT) */
	
	g_u8Address = 0;    /* Device address */
	g_u8Config = 0;
	g_u8Flag = 0;       /* Flag to record the USB request */
	
	g_u8BulkState = BULK_CBW;
	g_au8SenseKey[0] = 0;
	g_au8SenseKey[1] = 0;
	g_au8SenseKey[2] = 0;
	g_bCBWInvalid = FALSE;

	g_u32Address = 0;
	g_u32Length = 0;
	g_u8Size = 0;

	g_sCSW.dCSWSignature = CSW_SIGNATURE;

	udcOnLine = 0;
	g_u32OutToggle = 0;	// for Error Detection
}

//======================================================
// USB Floating Detect
//======================================================
void UsbFdt(void)
{
	uint8_t u8FLDET = USBD->FLDET.FLDET;

	_DRVUSB_SET_EVENT_FLAG(INTSTS_FLDET);
	
	if (u8FLDET)
	{
	    /* USB Plug In */
	
		if(g_u8UsbState & USB_STATE_ATTACHED)
		{
		    /* Do nothing if it is already attached */
			return;
		}
		
		// Update USB state */
		g_u8UsbState = USB_STATE_ATTACHED;
		
		/* Init the end points */
		UsbCfg();
		
	    /* Enable USB and enable PHY */
        _DRVUSB_ENABLE_USB();
	}
	else
	{
	    /* USB Un-plug */
	
	    /* Update USB state */
		g_u8UsbState = USB_STATE_DETACHED;
		
		udcOnLine = 0;
		
	    /* Disable USB, Disable remote weak up, and disable PHY */
        _DRVUSB_DISABLE_USB();
	}
}


//======================================================
// USB Bus Event
//======================================================
void UsbBus(void)
{
	uint8_t u8ATTR = *((__IO uint32_t *)(&USBD->ATTR)) & 0xFF;
	
    /* Clear event flag */
	_DRVUSB_SET_EVENT_FLAG(INTSTS_BUS);
	
	/* Just return if USB detached. */
	if (g_u8UsbState == USB_STATE_DETACHED)
		return;

    /* Process USB bus command if USB is attached. */
	if (u8ATTR & ATTR_USBRST)
	{	
	    /* Enable USB and enable PHY */
        _DRVUSB_ENABLE_USB();		
		
		/* Enter default state */
		g_u8UsbState = USB_STATE_DEFAULT;
		
		/* Configure the end-points */
		UsbCfg();
	}
	else if (u8ATTR & ATTR_SUSPEND)
	{	
	    /* Enable USB but disable PHY */
        _DRVUSB_DISABLE_PHY();
		
		/* Update the USB state */
		g_u8UsbState |= USB_STATE_FLAG_SUSPENDED;
	}
	else if (u8ATTR & ATTR_RESUME)
	{
	    /* Enable USB and enable PHY */
		_DRVUSB_ENABLE_USB();
		
		/* Clear the suspend state */
		g_u8UsbState &= ~USB_STATE_FLAG_SUSPENDED;
	}
}


//======================================================
int32_t UsbStdReq(void)
{
	int volatile len, idx;
    uint8_t tmp[8];

    *((__IO uint32_t *)tmp) = *((__IO uint32_t *)g_au8UsbSetup);
    *((__IO uint32_t *)tmp+1) = *((__IO uint32_t *)g_au8UsbSetup+1);

	/* check if after DEFAULT state (RESET) */
	if ((g_u8UsbState & USB_STATE_FLAG_DEFAULT) == 0x00)
		return FALSE;

	/* parse the request in setup packet */	
    switch (tmp[1])
	{
		case GET_DESCRIPTOR:
		{
			//len = g_au8UsbSetup[6] + (g_au8UsbSetup[7] << 8);
			len = tmp[6] + ((uint32_t)tmp[7] << 8);
            
            //switch (g_au8UsbSetup[3])
			switch (tmp[3])
            {
				// Get Device Descriptor
				case DESC_DEVICE:
				{
					udcOnLine = 1;

					if (len > LEN_DEVICE)
					    len = LEN_DEVICE;

					//for (idx = 0; idx < len; idx++)
					//	g_au8UsbCtrl[idx] = MassDeviceDescriptor[idx];
                    my_memcpy(g_au8UsbCtrl, (void *)MassDeviceDescriptor, len);
					break;
				}
				// Get Configuration Descriptor
				case DESC_CONFIG:
				{
					if (len > MassConfigurationBlock[2])
					    len = MassConfigurationBlock[2];
					
					//for (idx = 0; idx < len; idx++)
					//	g_au8UsbCtrl[idx] = MassConfigurationBlock[idx];
					my_memcpy(g_au8UsbCtrl, (void *)MassConfigurationBlock, len);                   
                    break;
				}
				// Get String Descriptor
				case DESC_STRING:
				{
					// Get Language
					//if (g_au8UsbSetup[4] == 0)
					if(tmp[4] == 0)
                    {
						if(len > MassStringDescriptor0[0])
						    len = MassStringDescriptor0[0];
						
						//for (idx = 0; idx < len; idx++)
						//	g_au8UsbCtrl[idx] = MassStringDescriptor0[idx];
						my_memcpy(g_au8UsbCtrl, (void *)MassStringDescriptor0, len);
                        
                        break;
					}
					// Get String Descriptor
					if(len > MassStringDescriptor1[0])
					    len = MassStringDescriptor1[0];
					
					
					//for (idx = 0; idx < len; idx++)
					//	g_au8UsbCtrl[idx] = MassStringDescriptor1[idx];
					my_memcpy(g_au8UsbCtrl, (void *)MassStringDescriptor1, len);
                    
                    break;
				}
				default:
					return FALSE;
			}
			_DRVUSB_SET_CFG(0, DATA1 (CFG0_SETTING));
			_DRVUSB_TRIG_EP(0, len);
			g_u8Flag = FLAG_OUT_ACK;
			return TRUE;
		}
		case SET_ADDRESS:
		{
		    /* Accept set address command only before USB configured */
			if((g_u8UsbState & USB_STATE_FLAG_CONFIGURED) == 0)
			{
				/* Get the device address */
				//g_u8Address = g_au8UsbSetup[2];
				g_u8Address = tmp[2];
                
				_DRVUSB_SET_CFG(0, DATA1 (CFG0_SETTING));
				_DRVUSB_TRIG_EP(0, 0);
				g_u8Flag = FLAG_SET_ADDRESS;
				return TRUE;
			}
			return FALSE;
		}
		case SET_CONFIGURATION:
		{
			//g_u8Config = g_au8UsbSetup[2];
			g_u8Config = tmp[2];
            
			_DRVUSB_SET_CFG(0, DATA1 (CFG0_SETTING));
			_DRVUSB_TRIG_EP(0, 0);
			g_u8Flag = FLAG_SET_CONFIG;
			return TRUE;
		}
		case CLEAR_FEATURE:
		case SET_FEATURE:
		{
			if (g_bCBWInvalid)
			{
		        /* Invalid CBW */
			}
			//else if ((g_au8UsbSetup[0] == 2) && (g_au8UsbSetup[2] == FEATURE_ENDPOINT_HALT))
			else if((tmp[0] == 2) && (tmp[2] == FEATURE_ENDPOINT_HALT))
            {
				//UINT32 u32CfgAddr = ((g_au8UsbSetup[4] & 0xF) << 4) + CFGP0;
				uint32_t u32CfgAddr;
                uint8_t  u8Cfg;

                u32CfgAddr = ((tmp[4] & 0xF) << 4) + USBD_CFGP0;
                
                //u8Cfg = inp32(u32CfgAddr) & 0xff;
				u8Cfg = *((__IO uint32_t *) (u32CfgAddr)) & 0xFF;
				
				//if (g_au8UsbSetup[1] == CLEAR_FEATURE)
				if(tmp[1] == CLEAR_FEATURE)
                	u8Cfg &= ~EPT_stall;
				else
					u8Cfg |= EPT_stall;
				*((__IO uint32_t *) (u32CfgAddr)) = u8Cfg;
			}
			else
				return FALSE;
				
			_DRVUSB_SET_CFG(0, DATA1 (CFG0_SETTING));
			_DRVUSB_TRIG_EP(0, 0);
			return TRUE;
		}
		case GET_CONFIGURATION:
		{
			g_au8UsbCtrl[0] = g_u8Config;
			//ByteAssign(g_au8UsbCtrl, 0, g_u8Config);
            
			_DRVUSB_SET_CFG(0, DATA1(CFG0_SETTING));
			_DRVUSB_TRIG_EP(0, 1);
			g_u8Flag = FLAG_OUT_ACK;
			return TRUE;
		}
		case GET_STATUS:
		{
            uint8_t u8Data;

            u8Data = tmp[0];
			// Device
			//if (g_au8UsbSetup[0] == 0x80)
			if(u8Data == 0x80)
            	g_au8UsbCtrl[0] = 1;	// Self-Powered
                //ByteAssign(g_au8UsbCtrl, 0, 1);
			// Interface
			//else if (g_au8UsbSetup[0] == 0x81)
			else if(u8Data == 0x81)
            	g_au8UsbCtrl[0] = 0;
                //ByteAssign(g_au8UsbCtrl, 0, 0);
			// Endpoint
			//else if (g_au8UsbSetup[0] == 0x82)
			else if(u8Data == 0x82)
            {
				//UINT32 u32CfgAddr = ((g_au8UsbSetup[4] & 0xF) << 4) + CFGP0;
                uint32_t u32CfgAddr;
                u8Data = tmp[4];
                u32CfgAddr = ((u8Data & 0xF) << 4) + USBD_CFGP0;

				g_au8UsbCtrl[0] = (inp32(u32CfgAddr) >> 1) & 1;
                //ByteAssign(g_au8UsbCtrl, 0, (inp32(u32CfgAddr) >> 1) & 1);
			}
			else
				return FALSE;
   			
            g_au8UsbCtrl[1] = 0;
            //ByteAssign(g_au8UsbCtrl, 1, 0);

			_DRVUSB_SET_CFG(0, DATA1 (CFG0_SETTING));
			_DRVUSB_TRIG_EP(0, 1);
			g_u8Flag = FLAG_OUT_ACK;
			return TRUE;
		}
		case GET_INTERFACE:
		{
			g_au8UsbCtrl[0] = 0;
            //ByteAssign(g_au8UsbCtrl, 0, 0);

			_DRVUSB_SET_CFG(0, DATA1(CFG0_SETTING));
			_DRVUSB_TRIG_EP(0, 1);
			g_u8Flag = FLAG_OUT_ACK;
			return TRUE;
		}
		case SET_INTERFACE:
		{
			g_u8Flag = FLAG_SET_INTERFACE;

			_DRVUSB_SET_CFG(0, DATA1(CFG0_SETTING));
			_DRVUSB_TRIG_EP(0, 0);
			return TRUE;
		}
		default:
			return FALSE;
	}
}


//======================================================
int32_t UsbClassReq(void)
{
    uint8_t tmp[8];

    *((__IO uint32_t *)tmp) = *((__IO uint32_t *)g_au8UsbSetup);
    *((__IO uint32_t *)tmp+1) = *((__IO uint32_t *)g_au8UsbSetup+1);

	//if ((g_au8UsbSetup[2] != 0) || (g_au8UsbSetup[3] != 0) || (g_au8UsbSetup[4] != 0) || (g_au8UsbSetup[5] != 0))
	if((tmp[2] != 0) || (tmp[3] != 0) || (tmp[4] != 0) || (tmp[5] != 0))
    	return FALSE;

	//switch (g_au8UsbSetup[1])
	switch(tmp[1])
    {
		// Bulk-Only Mass Storage Reset
		case BULK_ONLY_MASS_STORAGE_RESET:
		{
			//if ((g_au8UsbSetup[6] != 0) || (g_au8UsbSetup[7] != 0))
			if((tmp[6] != 0) || (tmp[7] != 0))
            	return FALSE;
			g_bCBWInvalid = FALSE;

			_DRVUSB_SET_CFG(0, DATA1 (CFG0_SETTING));
			_DRVUSB_TRIG_EP(0, 0);
			_DRVUSB_SET_CFG(2, DATA0 (CFG2_SETTING));
			
			g_u8BulkState = BULK_CBW;
			//outp32(USBD_MXPLD3, 31);
			_DRVUSB_TRIG_EP(3, 31);
			return TRUE;
		}
		// Get Max LUN
		case GET_MAX_LUN:
		{
			//if ((g_au8UsbSetup[6] != 1) || (g_au8UsbSetup[7] != 0))
            if((tmp[6] != 1) || (tmp[7] != 0))
				return FALSE;
			g_au8UsbCtrl[0] = MassLUN - 1;
            if(MassLUN != 1)
                while(1);
			//ByteAssign(g_au8UsbCtrl, 0, MassLUN - 1);
            
			_DRVUSB_SET_CFG(0, DATA1 (CFG0_SETTING));
			_DRVUSB_SET_CFG(3, DATA0 (CFG3_SETTING));
			_DRVUSB_TRIG_EP(0, 1);
			
			g_u8Flag = FLAG_OUT_ACK;
			return TRUE;
		}
	}
	return FALSE;
}


//======================================================
int32_t UsbSetup(void)
{
    /* Clear the data IN/OUT ready flag of control end-points */
	_DRVUSB_SET_CFG_EP0(CFGP_CLRRDY);	
	_DRVUSB_SET_CFG_EP1(CFGP_CLRRDY);
	
	/* USB device request in setup packet: offset 0, D[6..5]: 0=Stardard, 1=Class, 2=Vendor, 3=Reserved */
	switch (g_au8UsbSetup[0] & 0x60)
	{
		case REQ_STANDARD:
			return UsbStdReq();
		case REQ_CLASS:
			return UsbClassReq();
		default:
			return FALSE;
	}
}


//======================================================
void UsbInAck(void)
{
	switch (g_u8Flag)
	{
		// Out ACK
		case FLAG_OUT_ACK:
			_DRVUSB_TRIG_EP(1, 0);
			break;
		// Set Address
		case FLAG_SET_ADDRESS:
		{
			if (g_u8Address == 0)
				g_u8UsbState = USB_STATE_DEFAULT;
			else
				g_u8UsbState = USB_STATE_ADDRESS;
			_DRVUSB_SET_FADDR(g_u8Address);
			break;
		}
		// Set Interface
		case FLAG_SET_INTERFACE:
		// Set Configuration
		case FLAG_SET_CONFIG:
		{
			if (g_u8Config == 0)
				g_u8UsbState = USB_STATE_ADDRESS;
			else
			{
				g_u8UsbState = USB_STATE_CONFIGURED;
				// ---------------------------------
				_DRVUSB_SET_CFG(2, DATA0 (CFG2_SETTING));
				_DRVUSB_TRIG_EP(3, 31);
				// ---------------------------------
			}
			break;
		}
		// Get Max. LUN
		case FLAG_GET_MAX_LUN:
			_DRVUSB_TRIG_EP(1, 0);
			return;
		default:
			return;
	}
	g_u8Flag = 0;
}

//======================================================
void UsbOutAck(void)
{
	switch (g_u8Flag)
	{
		// Get Max. LUN
		case FLAG_GET_MAX_LUN:
		{
			// ---------------------------------
			_DRVUSB_SET_CFG(2, DATA0 (CFG2_SETTING));
			_DRVUSB_TRIG_EP(3, 31);
			// ---------------------------------
			break;
		}
		default:
			return;
	}
	g_u8Flag = 0;
}


//======================================================
void UsbRead(void)
{
    
	/* DATA0/DATA1 Toggle */
	if (*((__IO uint32_t *)(&USBD->EP[2].BUFSEG)) == BUF_BULK1)
		_DRVUSB_SET_EP_BUF(2, USB_SRAM_BASE + BUF_BULK0);
	else
		_DRVUSB_SET_EP_BUF(2, USB_SRAM_BASE + BUF_BULK1);
		
	/* Trigger to send out the data packet */	
	_DRVUSB_TRIG_EP(2, g_u8Size);
	
	g_u32Length -= g_u8Size;
	g_u32BytesInStorageBuf -= g_u8Size;

}

//======================================================
void UsbReadTrig(void)
{
    uint32_t u32Len;
    
	
    if (g_u32Length)
    {
        if (g_u32BytesInStorageBuf)
        {
        	/* Prepare next data packet */
        	g_u8Size = MAX_PACKET_SIZE;
        	if (g_u8Size > g_u32Length)
        	    g_u8Size = g_u32Length;

        	//if (inp32(USBD_BUFSEG2) == BUF_BULK1)
        	if (*((__IO uint32_t *)(&USBD->EP[2].BUFSEG)) == BUF_BULK1)
        	{
        		my_memcpy(g_au8UsbBulk0, (int8_t *)g_u32Address, g_u8Size);
        	}
        	else
        	{
        		my_memcpy(g_au8UsbBulk1, (int8_t *)g_u32Address, g_u8Size);
        	}
        	g_u32Address += g_u8Size;
    	}
    	else
    	{
			u32Len = g_u32Length;
			if (u32Len > STORAGE_BUFFER_SIZE)
				u32Len = STORAGE_BUFFER_SIZE;

			SpiRead(g_u32LbaAddress, u32Len, (uint8_t*)STORAGE_DATA_BUF);
			g_u32BytesInStorageBuf = u32Len;
			g_u32LbaAddress += (u32Len/UDC_SECTOR_SIZE);
			g_u32Address = STORAGE_DATA_BUF;		    
		    

        	/* Prepare next data packet */
        	g_u8Size = MAX_PACKET_SIZE;
        	if (g_u8Size > g_u32Length)
        	    g_u8Size = g_u32Length;

        	//if(inp32(USBD_BUFSEG2) == BUF_BULK1)
        	if (*((__IO uint32_t *)(&USBD->EP[2].BUFSEG)) == BUF_BULK1)
        	{
        		my_memcpy(g_au8UsbBulk0, (int8_t *)g_u32Address, g_u8Size);
        	}
        	else
        	{
        		my_memcpy(g_au8UsbBulk1, (int8_t *)g_u32Address, g_u8Size);
        	}
        	g_u32Address += g_u8Size;		    
    	}
    	/* DATA0/DATA1 Toggle */
		if (*((__IO uint32_t *)(&USBD->EP[2].BUFSEG)) == BUF_BULK1)
			_DRVUSB_SET_EP_BUF(2, USB_SRAM_BASE + BUF_BULK0);
		else
			_DRVUSB_SET_EP_BUF(2, USB_SRAM_BASE + BUF_BULK1);
		
		/* Trigger to send out the data packet */	
		_DRVUSB_TRIG_EP(2, g_u8Size);
		
		g_u32Length -= g_u8Size;
		g_u32BytesInStorageBuf -= g_u8Size;
		//while(_DRVUSB_GET_EVF() & EVF_USB==0);
	}
	else
		_DRVUSB_TRIG_EP(2, 0);

}

//======================================================
void UsbBulkInAck(void)
{
	int volatile idx;

	if (g_u8BulkState == BULK_CSW)
	{
	    /* Prepare to receive the CBW */
		g_u8BulkState = BULK_CBW;

		_DRVUSB_SET_EP_BUF(3, USB_SRAM_BASE + BUF_BULK0);
		_DRVUSB_TRIG_EP(3, 31);
	}
	else if (g_u8BulkState == BULK_IN)
	{
		switch (g_sCBW.u8OPCode)
		{
			case UFI_READ_FORMAT_CAPACITY:
			case UFI_READ_CAPACITY:
			case UFI_MODE_SENSE_10:
			case UFI_MODE_SENSE_6:
 			{
/*				if (g_u32Length > 0)
				{
					UsbRead();
					return;
				}
*/			}
			case UFI_READ_10:
			{
				if (g_u32Length > 0)
				{
					UsbReadTrig();
					//UsbRead();
					return;
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
			case UFI_VERIFY_10:
			case UFI_START_STOP:
			case UFI_WRITE_10:
			case VENDER_CMD:
			{
				int tmp;
				
				tmp = g_sCBW.dCBWDataTransferLength - STORAGE_BUFFER_SIZE;
				if (tmp < 0) tmp = 0;
				
				g_sCSW.dCSWDataResidue = tmp;
				g_sCSW.bCSWStatus = 0;
				break;
			}
			case UFI_TEST_UNIT_READY:
			{
				g_sCSW.dCSWDataResidue = 0;
				g_sCSW.bCSWStatus = 0;
				break;
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
		_DRVUSB_SET_EP_BUF(2, USB_SRAM_BASE + BUF_BULK1);

		//for (idx = 0; idx < 13; idx++)
		//	g_au8UsbBulk1[idx] = *((UINT8 *) (&g_sCSW.dCSWSignature) + idx);
        my_memcpy(g_au8UsbBulk1, &g_sCSW.dCSWSignature, 16);

		g_u8BulkState = BULK_CSW;
		_DRVUSB_TRIG_EP(2, 13);
	}
}

//======================================================
void UsbWrite(void)
{
#ifndef TEST_RAM
	uint32_t lba, len;
#endif

	if (g_u32Length > MAX_PACKET_SIZE)
	{
		if (*((__IO uint32_t *)(&USBD->EP[3].BUFSEG)) == BUF_BULK0)
		{
			my_memcpy((char *)g_u32Address, g_au8UsbBulk0, MAX_PACKET_SIZE);
			_DRVUSB_SET_EP_BUF(3, USB_SRAM_BASE + BUF_BULK1);
			_DRVUSB_TRIG_EP(3, MAX_PACKET_SIZE);
		}
		else
		{
			my_memcpy((char *)g_u32Address, g_au8UsbBulk1, MAX_PACKET_SIZE);
			_DRVUSB_SET_EP_BUF(3, USB_SRAM_BASE + BUF_BULK0);
			_DRVUSB_TRIG_EP(3, MAX_PACKET_SIZE);
		}
				
		g_u32Address += MAX_PACKET_SIZE;
		g_u32Length -= MAX_PACKET_SIZE;
		
#ifdef TEST_SPI		
		/* Buffer full. Writer it to storage first. */
		if (g_u32Address >= (STORAGE_DATA_BUF+STORAGE_BUFFER_SIZE))
		{
		    //DBG_PRINTF("Buffer Full. Write it by segment ... remind: %d\n", g_u32Length);
			if (g_sCBW.bCBWLUN==F_SPI_LUN)
				SpiWrite(g_u32SpiStartAddr, STORAGE_BUFFER_SIZE, (uint8_t*)STORAGE_DATA_BUF);
		    
		    g_u32Address = STORAGE_DATA_BUF;
		    g_u32SpiStartAddr += STORAGE_BUFFER_SIZE/UDC_SECTOR_SIZE;
		}
#endif		
	}
	else
	{
	    //DBG_PRINTF("> Bulk out end packet, size:%d\n", g_u32Length);

		//if (inp32(USBD_BUFSEG3) == BUF_BULK0)
		if (*((__IO uint32_t *)(&USBD->EP[3].BUFSEG)) == BUF_BULK0)
		{
			my_memcpy((char *)g_u32Address, g_au8UsbBulk0, g_u32Length);
		}
		else
		{
			my_memcpy((char *)g_u32Address, g_au8UsbBulk1, g_u32Length);
		}
		g_u32Address += g_u32Length;
		g_u32Length = 0;


#ifndef TEST_RAM
		if (g_sCBW.u8OPCode == UFI_WRITE_10)
		{
			lba = get_be32(&g_sCBW.au8Data[0]);
			len = g_sCBW.dCBWDataTransferLength;
#ifdef TEST_SPI
			if (g_sCBW.bCBWLUN==F_SPI_LUN)
			{
			    len =  g_sCBW.dCBWDataTransferLength - (g_u32SpiStartAddr - lba) * UDC_SECTOR_SIZE ;
			    if(len)
			    {
				    SpiWrite(g_u32SpiStartAddr, len, (uint8_t*)STORAGE_DATA_BUF);
				}
		    }
#endif
		}
#endif
		g_u8BulkState = BULK_IN;
		UsbBulkInAck ();
	}
}

//======================================================
int32_t UsbBulkOutAck(void)
{
	uint8_t u8Len;
	int32_t i;

	if (g_u8BulkState == BULK_CBW)
	{
		//u8Len = inp32(USBD_MXPLD3);
		u8Len = *((__IO uint32_t *)(&USBD->EP[3].MXPLD));
		
		/* Check Signature & length of CBW */
		if ((*(uint32_t *) g_au8UsbBulk0 != CBW_SIGNATURE) || (u8Len != 31))
		{
			return FALSE;
		}
		
		/* Get the CBW */
		for (i = 0; i < u8Len; i++)
			*((uint8_t *) (&g_sCBW.dCBWSignature) + i) = g_au8UsbBulk0[i];
			
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
				UsbBulkInAck ();
				return TRUE;
			}
			case UFI_VERIFY_10:
			case UFI_START_STOP:
			case UFI_TEST_UNIT_READY:
			{
				g_u8BulkState = BULK_IN;
				UsbBulkInAck ();
				return TRUE;
			}
			case UFI_REQUEST_SENSE:
			{
				ReqSenCommand();
				g_u8BulkState = BULK_IN;
				_DRVUSB_TRIG_EP(2, 18);
				return TRUE;
			}
			case UFI_READ_FORMAT_CAPACITY:
			{
				if (g_u32Length == 0)
				{
					g_u32Length = g_sCBW.dCBWDataTransferLength;
					g_u32Address = MassCMD_BUF;
				}
				RdFmtCapCommand();
				g_u8BulkState = BULK_IN;
				if (g_u32Length > 20)
					g_u32Length=20;
				if (g_u32Length > 0)
				{
					if (g_u32Length > MAX_PACKET_SIZE)
						g_u8Size = MAX_PACKET_SIZE;
					else
						g_u8Size = g_u32Length;

					my_memcpy(g_au8UsbBulk1, (char *)g_u32Address, g_u8Size);

					g_u32Address += g_u8Size;
					_DRVUSB_SET_EP_BUF(2, USB_SRAM_BASE + BUF_BULK0);
					UsbRead ();
				}
				else
					UsbBulkInAck ();
				return TRUE;
			}
			case UFI_READ_CAPACITY:
			{
				if (g_u32Length == 0)
				{
    				g_u32Length = g_sCBW.dCBWDataTransferLength;
					g_u32Address = MassCMD_BUF;
				}
				
				RdCurCapCommand();
				g_u8BulkState = BULK_IN;
				if (g_u32Length > 0)
				{
					if (g_u32Length > MAX_PACKET_SIZE)
						g_u8Size = MAX_PACKET_SIZE;
					else
						g_u8Size = g_u32Length;

					my_memcpy(g_au8UsbBulk1, (char *)g_u32Address, g_u8Size);

					g_u32Address += g_u8Size;
					_DRVUSB_SET_EP_BUF(2, USB_SRAM_BASE + BUF_BULK0);
					UsbRead ();
				}
				else
					UsbBulkInAck ();
				return TRUE;
			}
			case UFI_MODE_SELECT_10:
			{
				g_u32Length = g_sCBW.dCBWDataTransferLength;
				g_u32Address = MassCMD_BUF;

				if (g_u32Length > 0)
				{
					_DRVUSB_TRIG_EP(3, MAX_PACKET_SIZE);
					g_u8BulkState = BULK_OUT;
				}
				return TRUE;
			}
			case UFI_MODE_SENSE_10:
			case UFI_MODE_SENSE_6:
			{
				if (g_u32Length == 0)
				{
					g_u32Length = g_sCBW.dCBWDataTransferLength;
					g_u32Address = MassCMD_BUF;
				}
				// ();
				//g_u8BulkState = BULK_IN;
				*((uint32_t *)MassCMD_BUF  ) = 0;
				*((uint32_t *)MassCMD_BUF+1) = 0;	
				outpb(MassCMD_BUF, 3);
				g_u32Length=4;
				g_u8BulkState = BULK_IN;
				
				if (g_u32Length > 0)
				{
					if (g_u32Length > MAX_PACKET_SIZE)
						g_u8Size = MAX_PACKET_SIZE;
					else
						g_u8Size = g_u32Length;
					my_memcpy(g_au8UsbBulk1, (char *)g_u32Address, g_u8Size);

					g_u32Address += g_u8Size;

					_DRVUSB_SET_EP_BUF(2, USB_SRAM_BASE + BUF_BULK0);
					UsbRead ();
				}
				else
					UsbBulkInAck ();
				return TRUE;
			}
			case UFI_INQUIRY:
			{
				//for (i = 0; i < 36; i++)
				//	g_au8UsbBulk1[i] = InquiryID[i];
                my_memcpy(g_au8UsbBulk1, (void *)InquiryID, 36);

				g_u8BulkState = BULK_IN;

				_DRVUSB_TRIG_EP(2, 36);
				return TRUE;
			}
			case UFI_READ_10:
			{
			    /* Check if it is a new transfer */
				if(g_u32Length == 0)
				{
				    /* Prepare the data for Bulk IN transfer */
				
					/* Get LBA address */
					g_u32LbaAddress = get_be32(&g_sCBW.au8Data[0]);
					g_u32Length = g_sCBW.dCBWDataTransferLength;
					g_u32BytesInStorageBuf = g_u32Length;

#ifdef TEST_SPI
					if(g_sCBW.bCBWLUN==F_SPI_LUN)
					{
					    i = g_u32Length;
					    if(i > STORAGE_BUFFER_SIZE)
					        i = STORAGE_BUFFER_SIZE;
					   
						SpiRead(g_u32LbaAddress, i, (uint8_t*)STORAGE_DATA_BUF);
					    g_u32BytesInStorageBuf = i;
					    g_u32LbaAddress += (i/UDC_SECTOR_SIZE);
					}
#endif

				}

				g_u32Address = STORAGE_DATA_BUF;
                /* Indicate the next packet should be Bulk IN Data packet */
				g_u8BulkState = BULK_IN;
				
				if(g_u32BytesInStorageBuf > 0)
				{
				    /* Set the packet size */
					if (g_u32BytesInStorageBuf > MAX_PACKET_SIZE)
						g_u8Size = MAX_PACKET_SIZE;
					else
						g_u8Size = g_u32BytesInStorageBuf;
					
					/* Prepare the first data packet (DATA1) */
					my_memcpy(g_au8UsbBulk1, (char *)g_u32Address, g_u8Size);
					g_u32Address += g_u8Size;
					
					/* kick - start */
					_DRVUSB_SET_EP_BUF(2, USB_SRAM_BASE + BUF_BULK0);
					UsbRead();
				}
				else
				{
					UsbBulkInAck();
				}
					
				return TRUE;
			}
			case UFI_WRITE_10:
			{
				if (g_u32Length == 0)
				{
					g_u32Length = g_sCBW.dCBWDataTransferLength;
#ifdef TEST_RAM    
					g_u32Address = get_be32(&g_sCBW.au8Data[0]);
					g_u32Address = (g_u32Address * UDC_SECTOR_SIZE) + STORAGE_DATA_BUF;
#else
					g_u32Address = STORAGE_DATA_BUF;
					g_u32SpiStartAddr = get_be32(&g_sCBW.au8Data[0]);
#endif
				}

				if ((g_u32Length > 0))
				{
					_DRVUSB_TRIG_EP(3, MAX_PACKET_SIZE);
					g_u8BulkState = BULK_OUT;
				}
				    
				return TRUE;
			}
			case VENDER_CMD: /* Vender Command */
			{
#ifdef __GNUC__
				extern __attribute__ ((weak)) BOOL ProcessVenderCmd(UINT8 u8Cmd);
#else			
			    extern __weak int32_t ProcessVenderCmd(uint8_t u8Cmd);
#endif
			    if((uint32_t)&ProcessVenderCmd != NULL)
			        return ProcessVenderCmd(g_sCBW.u8LUN);
			}
			default:
			{
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
        				_DRVUSB_TRIG_EP(2, 0);
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
	    //DrvSIO_printf("BULK_OUT: opcode %d\n", g_sCBW.u8OPCode);
	
		switch (g_sCBW.u8OPCode)
		{
			case UFI_WRITE_10:
			case UFI_MODE_SELECT_10:
			{
				UsbWrite();
				return TRUE;
			}
			case VENDER_CMD:
			{
#ifdef __GNUC__
				extern __attribute__ ((weak)) void ComWrite(void);
#else			
			    extern __weak void ComWrite(void);
#endif
			    if((uint32_t)ComWrite != NULL)
			        ComWrite();
			    return TRUE;
			}
		}
	}
	return TRUE;
}


//======================================================
// USB USB Event
//======================================================
void UsbUsb(uint32_t u32INTSTS)
{
	uint32_t u32EPSTS = _DRVUSB_GET_EPSTS();

	if (u32INTSTS & INTSTS_SETUP)
	{	
	    /* Clear setup event */
		_DRVUSB_SET_EVENT_FLAG(INTSTS_SETUP);	
		if (!UsbSetup())
		{	
		    /* Setup error, stall the device */
			_DRVUSB_SET_CFGP(0, (CFGP_SSTALL | CFGP_CLRRDY));
			_DRVUSB_SET_CFGP(1, (CFGP_SSTALL | CFGP_CLRRDY));
		}
	}
	else if(u32INTSTS & INTSTS_EPTF0)
	{	
	    /* Ctrl In */
		_DRVUSB_SET_EVENT_FLAG(INTSTS_EPTF0);
		UsbInAck();
	}
	else if(u32INTSTS & INTSTS_EPTF1)
	{	
	    /* Ctrl Out */
		_DRVUSB_SET_EVENT_FLAG(INTSTS_EPTF1);
		UsbOutAck();
	}
	else if(u32INTSTS & INTSTS_EPTF2)
	{	
	    /* Bulk In */
		_DRVUSB_SET_EVENT_FLAG(INTSTS_EPTF2);
		g_u32OutToggle = u32EPSTS;
		UsbBulkInAck();
	}
	else if(u32INTSTS & INTSTS_EPTF3)
	{	
	    /* Bulk Out */
		_DRVUSB_SET_EVENT_FLAG(INTSTS_EPTF3);
		if (g_u32OutToggle == (u32EPSTS & EPSTS3))
		{
			_DRVUSB_TRIG_EP(3, MAX_PACKET_SIZE);
		}
		else
		{
			if (UsbBulkOutAck())
				g_u32OutToggle = u32EPSTS & EPSTS3;
			else
			{	
			    /* stall */
				g_bCBWInvalid = TRUE;
				_DRVUSB_SET_CFGP(2, (CFGP_SSTALL | CFGP_CLRRDY));
				_DRVUSB_SET_CFGP(3, (CFGP_SSTALL | CFGP_CLRRDY));
			}
		}
	}
	else
	{	/* Unknow event */
		_DRVUSB_SET_EVENT_FLAG(INTSTS_USB); 
	}
}

//======================================================
// USB ISR
//======================================================
void UsbIsr(void)
{
	uint32_t u32INTSTS = _DRVUSB_GET_EVENT_FLAG();
	if (u32INTSTS & INTSTS_FLDET)
	{
	    /* Handle the USB attached/detached event */
		UsbFdt();
	}
	else if(u32INTSTS & INTSTS_BUS)
	{
	    /* Handle the USB bus event: Reset, Suspend, and Resume */
		UsbBus();
	}
	else if(u32INTSTS & INTSTS_USB)
	{
	    /* Handle the USB Protocol/Clase event */
		UsbUsb(u32INTSTS);
    }
}

//======================================================
// USB Initialization
//======================================================
void udcInit(void)
{
    int32_t temp;

    /* Enable PHY to send bus reset event */
	//*((__IO uint32_t *)(&USBD->ATTR)) = 0x310;
    _DRVUSB_ENABLE_USB();


    _DRVUSB_ENABLE_SE0();
    RoughDelay(1000);
    _DRVUSB_DISABLE_SE0();
    RoughDelay(1000);

    /* Disable PHY */
	//*((__IO uint32_t *)(&USBD->ATTR)) = 0x000;
    _DRVUSB_DISABLE_USB();


    /* Enable USB device clock */
    SYSCLK->APBCLK.USBD_EN = 1;

    /* Reset IP */	
    SYS->IPRSTC2.USBD_RST = 1;
    SYS->IPRSTC2.USBD_RST = 0;	

    /* Select USB divider source */
    SYSCLK->CLKDIV.USB_N = 0;

     _DRVUSB_ENABLE_USB();
   _DRVUSB_ENABLE_SE0();
    temp = 0x100;
    while(temp--);   
	_DRVUSB_DISABLE_SE0();

	g_u8UsbState = USB_STATE_DETACHED;
	_DRVUSB_TRIG_EP(1, 0x08);
	UsbFdt();
}

//======================================================
// USB Initialization
//======================================================
void udcStop(void)
{


	SYS->IPRSTC1.CHIP_RST = 1;

 	

}
void udcMassBulk(void)
{
    /* Handler the USB ISR by polling */
	while(1)
	{
		UsbIsr();
		if((GPIOD->PIN&0x2000)!=0)
			break;
	}
}


uint8_t Flash_Identify(uint8_t tLUN)
{

#ifdef TEST_SPI
    if (tLUN==F_SPI_LUN)
    {
        //g_spiTotalSectors = GetPHYSICAL_SIZE() / UDC_SECTOR_SIZE;
	   	g_spiTotalSectors = GetLogicSector();
        if(g_spiTotalSectors < 0)
        {
            /* cannot read format */
			g_au8SenseKey[0] = 0x03;
			g_au8SenseKey[1] = 0x30;
			g_au8SenseKey[2] = 0x01;
            return FALSE;  
        }
    }
#endif

    return TRUE;
}


uint8_t udcFlashInit(void)
{
	GPIOD->PMD.PMD12=1;
	GPIOD->DOUT&=0xEFFF;

	while((GPIOD->PIN&0x2000)!=0);
	RoughDelay(100000);
	if(DrvSDCARD_Open() == E_SUCCESS)
	{		
		printf("SDCard Open success\n");
	}
	else
	{
		printf("SDCard Open failed\n");
	}

#ifdef TEST_RAM
    MassLUN++;
#endif    

#ifdef TEST_SPI
    F_SPI_LUN=MassLUN;
    MassLUN++;
    if (!Flash_Identify(F_SPI_LUN))
        return 0;
#endif


    return 1;
}

