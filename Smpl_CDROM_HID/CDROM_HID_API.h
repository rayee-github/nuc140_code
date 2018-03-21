/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) 2009 Nuvoton Technology Corp. All rights reserved.                                         */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/

#ifndef _CDROM_API_H
#define _CDROM_API_H


#define CTRL_EP_NUM         0
#define BULK_IN_EP_NUM      3
#define BULK_OUT_EP_NUM     4
#define HID_IN_EP_NUM    	1
#define HID_OUT_EP_NUM   	2

extern const uint8_t gau8VendorStringDescriptor[];
extern const uint8_t gau8StringLang[];
extern const uint8_t gau8VendorStringDescriptor[];
extern const uint8_t gau8ProductStringDescriptor[];
extern const uint8_t gau8StringSerial[];

extern const uint8_t g_CDROM_au8DeviceDescriptor[];
extern const uint8_t g_CDROM_au8ConfigDescriptor[];


void CDROM_BulkInAckCallback(void * pVoid);
void CDROM_BulkOutAckCallback(void * pVoid);
void CDROMHID_MainProcess(void);
void HID_IntInCallback(void * pVoid);
void HID_IntOutCallback(void * pVoid);
int32_t	HID_SetReportDescriptor(const uint8_t* pu8ReportDescriptor, uint32_t u32ReportDescriptorSize);
int32_t	HID_SetReportBuf(uint8_t* pu8Report, uint32_t u32ReportSize);


#endif

