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

extern const uint8_t gau8MouseReport[];
extern const uint8_t gau8MouseReportDescriptor[];
extern const uint32_t gu32MouseReportDescriptorSize;
extern const uint8_t gau8StringLang[];
extern const uint8_t gau8StringSerial[];
extern const uint8_t gau8DeviceDescriptor[];
extern const uint8_t gau8ConfigDescriptor[];
extern const uint8_t gau8VendorStringDescriptor[];
extern const uint8_t gau8ProductStringDescriptor[];
extern const uint8_t gau8StringSerial[];

#ifdef  __cplusplus
}
#endif

#endif // #ifndef _HID_API_H



