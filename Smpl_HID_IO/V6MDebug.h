#ifndef INC__V6MDEBUG_H__
#define INC__V6MDEBUG_H__
#ifdef __cplusplus
extern "C"
{
#endif

    /*---------------------------------------------------------------------------------------------------------*/
    /*                                                                                                         */
    /* Copyright (c) Nuvoton Technology Corp. All rights reserved.                                             */
    /*                                                                                                         */
    /*---------------------------------------------------------------------------------------------------------*/

#define V6M_CMD_RESET_CMD_SRIAL	0xFFFFFFFFUL
#define V6M_CMD_LED5_ON		0xB5UL
#define V6M_CMD_LED6_ON		0xB6UL
#define V6M_CMD_LED7_ON		0xB7UL
#define V6M_CMD_LED8_ON		0xB8UL
#define V6M_CMD_LED_ON		0xBFUL

#define V6M_CMD_LED5_OFF	0xC5UL
#define V6M_CMD_LED6_OFF	0xC6UL
#define V6M_CMD_LED7_OFF	0xC7UL
#define V6M_CMD_LED8_OFF	0xC8UL
#define V6M_CMD_LED_OFF		0xCFUL

#define V6M_MAX_COMMAND_LENGTH	(HID_MAX_PACKET_SIZE_EP0-2)

uint32_t LED_on(const uint8_t *pu8Buffer, uint32_t u32Len, uint8_t u8LedNum);
uint32_t LED_off(const uint8_t *pu8Buffer, uint32_t u32Len, uint8_t u8LedNum);
void V6M_ProcessCommand(const uint8_t *pu8Buffer, uint32_t u32Len);


#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
