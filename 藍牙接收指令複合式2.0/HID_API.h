/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright (c) Nuvoton Technology Corp. All rights reserved.                                             */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#ifndef _HID_API_H
#define _HID_API_H

#ifdef  __cplusplus
extern "C"
{
#endif

extern const uint8_t g_HID_au8MouseReport[];
extern const uint8_t g_HID_au8MouseReportDescriptor[];
extern const uint32_t g_HID_u32MouseReportDescriptorSize;
extern const uint8_t g_HID_au8KeyboardReport[];
extern const uint8_t g_HID_au8KeyboardReportDescriptor[];
extern const uint32_t g_HID_u32KeyboardReportDescriptorSize;
extern const uint8_t g_HID_au8StringLang[];
extern const uint8_t g_HID_au8StringSerial[];
extern const uint8_t g_HID_au8DeviceDescriptor[];
extern const uint8_t g_HID_au8ConfigDescriptor[];
extern const S_DRVUSB_STRING_DESC g_HID_sVendorStringDesc;
extern const S_DRVUSB_STRING_DESC g_HID_sProductStringDesc;
extern const uint8_t g_HID_au8StringSerial[];

void HID_SetInReport(void);
void HID_Init(void);


void DrvUSB_BusSuspendCallback(void * pVoid);
void DrvUSB_BusResumeCallback(void * pVoid);

#ifdef  __cplusplus
}
#endif

#endif // #ifndef _HID_API_H



