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
#define V6M_CMD_PROGRAM		0xA0UL

#define V6M_MAX_COMMAND_LENGTH	(HID_MAX_PACKET_SIZE_EP0-2)

    void V6M_ProcessCommand(const uint8_t *pu8Buffer);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
