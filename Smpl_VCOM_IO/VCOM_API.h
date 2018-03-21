/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) 2009 Nuvoton Technology Corp. All rights reserved.                                         */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/

#ifndef _VCOM_API_H
#define _VCOM_API_H


#define CTRL_EP_NUM         0
#define INT_IN_EP_NUM       3   
#define BULK_OUT_EP_NUM     2
#define BULK_IN_EP_NUM      1

extern const uint8_t gau8DeviceDescriptor[];
extern const uint8_t gau8ConfigDescriptor[];
extern const uint8_t gau8VendorStringDescriptor[];
extern const uint8_t gau8ProductStringDescriptor[];

extern const uint8_t gau8StringLang[];
extern const uint8_t gau8StringSerial[];
extern const uint8_t gau8StringAudio[];

void VCOM_SendData(void);
void VCOM_IntInAckCallback(void* pVoid);
int32_t VCOM_MainProcess(void);
void LED_on(const uint8_t *pu8Buffer, uint8_t u8LedNum);
void LED_off(const uint8_t *pu8Buffer, uint8_t u8LedNum);

#endif

