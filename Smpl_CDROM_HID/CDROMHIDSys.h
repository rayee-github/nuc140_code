/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) 2009 Nuvoton Technology Corp. All rights reserved.                                         */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#ifndef __CDROMSYS_H__
#define __CDROMSYS_H__


#ifdef  __cplusplus
extern "C"
{
#endif


#define CDROM_MAJOR_NUM	    1
#define CDROM_MINOR_NUM     0
#define CDROM_BUILD_NUM	    1

#define E_HID_UNDEFINE                       _SYSINFRA_ERRCODE(TRUE, MODULE_ID_HID, 0)
#define E_HID_NULL_POINTER                   _SYSINFRA_ERRCODE(TRUE, MODULE_ID_HID, 1)
#define E_HID_BUFFER_OVERRUN                 _SYSINFRA_ERRCODE(TRUE, MODULE_ID_HID, 2)
#define E_HID_INVALID_EP_NUM                 _SYSINFRA_ERRCODE(TRUE, MODULE_ID_HID, 3)
#define E_HID_MUST_LESS_THAN_PACKET0_SIZE    _SYSINFRA_ERRCODE(TRUE, MODULE_ID_HID, 4)
#define E_HID_EXCEED_INT_IN_PACKET_SIZE      _SYSINFRA_ERRCODE(TRUE, MODULE_ID_HID, 5)
#define E_HID_CTRL_REG_TAB_FULL              _SYSINFRA_ERRCODE(TRUE, MODULE_ID_HID, 6)
#define E_HID_INVALID_REG_NUM                _SYSINFRA_ERRCODE(TRUE, MODULE_ID_HID, 7)

#define	CDROM_MAX_PACKET_SIZE_EP0	    64
#define	BULK_EP_MXPLD   			    64 	/* Maximum packet size of BULK endpoint */
#define	INT_EP_MXPLD   			    	64 	/* Maximum packet size of BULK endpoint */

#define	BULK_ONLY_MASS_STORAGE_RESET	0xFF
#define	GET_MAX_LUN						0xFE

/* Bulk In/Out State */
#define	BULK_CBW		0x00
#define	BULK_IN			0x01
#define	BULK_OUT		0x02
#define	BULK_CSW		0x04
#define BULK_NORMAL		0xFF

/* UFI Command */
#define	UFI_TEST_UNIT_READY				    0x00
#define	UFI_REQUEST_SENSE					0x03
#define	UFI_INQUIRY							0x12
#define	UFI_MODE_SELECT_6					0x15
#define	UFI_MODE_SENSE_6					0x1A
#define	UFI_START_STOP						0x1B
#define	UFI_PREVENT_ALLOW_MEDIUM_REMOVAL	0x1E
#define	UFI_READ_FORMAT_CAPACITY			0x23
#define	UFI_READ_CAPACITY					0x25
#define	UFI_READ_10							0x28
#define	UFI_WRITE_10						0x2A
#define	UFI_VERIFY_10						0x2F
#define	UFI_MODE_SELECT_10					0x55
#define	UFI_MODE_SENSE_10					0x5A


#define	CBW_SIGNATURE	0x43425355
#define	CSW_SIGNATURE	0x53425355


//***************************************************
// 		HID Class REQUEST
//***************************************************
#define GET_REPORT          0x01
#define GET_IDLE            0x02
#define GET_PROTOCOL        0x03
#define SET_REPORT          0x09
#define SET_IDLE            0x0A
#define SET_PROTOCOL        0x0B

#define HID_RPT_TYPE_INPUT		0x01
#define HID_RPT_TYPE_OUTPUT		0x02
#define HID_RPT_TYPE_FEATURE	0x03

    static void _HID_CLR_CTRL_READY_AND_TRIG_STALL()
    {
        _DRVUSB_CLEAR_EP_READY_AND_TRIG_STALL(0);
        _DRVUSB_CLEAR_EP_READY_AND_TRIG_STALL(1);
    }

    static void _HID_CLR_CTRL_READY()
    {
        _DRVUSB_CLEAR_EP_READY(0);
        _DRVUSB_CLEAR_EP_READY(1);
    }

typedef struct
{
	const uint8_t *au8DeviceDescriptor;
	const uint8_t *au8ConfigDescriptor;
	const uint8_t *au8ProductStringDescriptor;
	const uint8_t *sVendorStringDescriptor;

	/* HID */
	const uint8_t *pu8ReportDescriptor;
	uint32_t u32ReportDescriptorSize;

	uint8_t *pu8Report;
	uint32_t u32ReportSize;

	const uint8_t *pu8HIDDescriptor;
	const uint8_t *pu8IntInEPDescriptor;

	void* *device;
	
} S_CDROM_DEVICE;


/* For Mass Storage */
struct CBW
{
	uint32_t dCBWSignature;
	uint32_t dCBWTag;
	uint32_t dCBWDataTransferLength;
	uint8_t	 bmCBWFlags;
	uint8_t	 bCBWLUN;
	uint8_t	 bCBWCBLength;
	uint8_t	 u8OPCode;
	uint8_t	 u8LUN;
	uint8_t	 au8Data[14];
};

struct CSW
{
	uint32_t dCSWSignature;
	uint32_t dCSWTag;
	uint32_t dCSWDataResidue;
	uint8_t	 bCSWStatus;
};


extern S_CDROM_DEVICE gsCdromDevice;


int32_t CDROM_Open(uint32_t inReportSize, void *pfGetInReport, void *pfSetOutReport);
//int32_t CDROM_Open(void);
void CDROM_Close(void);
void CDROM_Start(S_CDROM_DEVICE *psDevice);
void CDROM_CtrlReset(S_CDROM_DEVICE *psDevice);
void CDROM_CtrlSetupSetConfiguration(void * pVoid);
void CDROM_BulkOutAckCallback(void * pVoid);
void CDROM_BulkInAckCallback(void * pVoid);
void CDROM_CtrlSetupGetDescriptor(void * pVoid);
void CDROM_CtrlDataInGetDescriptor(void * pVoid);
void CDROM_CtrlGetDescriptorOut(void * pVoid);
void CDROM_CtrlGetDescriptorIn(void * pVoid);
static int32_t CDROM_IsConfigureValue(uint8_t u8ConfigureValue);
void CDROM_GetMaxLUN(void * pVoid);
void UsbBulkInAck(void);
int32_t UsbBulkOutAck(void);

    void HID_CtrlSetupSetReport(void * pVoid);
    void HID_CtrlSetupGetReport(void * pVoid);
    void HID_CtrlSetupSetIdle(void * pVoid);
    void HID_CtrlSetupSetProtocol(void * pVoid);

#ifdef  __cplusplus
}
#endif

#endif // __CDROMSYS_H__


