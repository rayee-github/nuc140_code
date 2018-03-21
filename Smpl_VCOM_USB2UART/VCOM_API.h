/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) 2009 Nuvoton Technology Corp. All rights reserved.                                         */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/

#ifndef _VCOM_API_H
#define _VCOM_API_H

#include "DrvUSB.h"

#define	VCOM_CTRL_MXPLD  64  /* Maximum packet size of control */ 
#define	BULK_EP_MXPLD    64  /* Maximum packet size of BULK endpoint */
#define INT_EP_MXPLD     8   /* Maximum packet size of INT endpoint  */

#define SEND_ENCAPSULATED_COMMAND   0x00
#define GET_ENCAPSULATED_RESPONSE   0x01
#define SET_COMM_FEATURE            0x02
#define GET_COMM_FEATURE            0x03
#define CLEAR_COMM_FEATURE          0x04
#define SET_LINE_CODE               0x20
#define GET_LINE_CODE               0x21
#define SET_CONTROL_LINE_STATE      0x22
#define SEND_BREAK                  0x23

#define CTRL_EP_NUM         0
#define INT_IN_EP_NUM       3   
#define BULK_OUT_EP_NUM     2
#define BULK_IN_EP_NUM      1

typedef struct
{
	uint8_t *au8DeviceDescriptor;
	uint8_t *au8ConfigDescriptor;
	uint8_t *au8ProductStringDescriptor;
	uint8_t *sVendorStringDescriptor;
	uint8_t *sProductStringDescriptor;
	void* *device;
	
} S_VCOM_DEVICE;


void SysTimerDelay(uint32_t us);
void VCOM_MainProcess(void);

void VCOM_CtrlSetupGetDescriptor(void* pVoid);
void VCOM_CtrlSetupSetConfiguration(void* pVoid);
void VCOM_SetLineCodingDataOut(void* pVoid);
void VCOM_SetLineCoding(void* pVoid);
void VCOM_GetLineCoding(void* pVoid);
void VCOM_BulkOutAckCallback(void* pVoid);
void VCOM_BulkInAckCallback(void* pVoid);
void VCOM_IntInAckCallback(void* pVoid);
void VCOM_CtrlLineState(void* pVoid);


#endif

