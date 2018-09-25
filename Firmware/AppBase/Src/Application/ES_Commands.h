/* 
 *  ____  _  _   __   ____   __    ___    ____  _  _  ____  ____  ____  _  _  ____
 * (  __)( \/ ) /  \ (_  _) (  )  / __)  / ___)( \/ )/ ___)(_  _)(  __)( \/ )/ ___)
 *  ) _)  )  ( (  O )  )(    )(  ( (__   \___ \ )  / \___ \  )(   ) _) / \/ \\___ \
 * (____)(_/\_) \__/  (__)  (__)  \___)  (____/(__/  (____/ (__) (____)\_)(_/(____/
 *
 * Copyright (c) 2017 EXOTIC SYSTEMS. All Rights Reserved.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */
#ifndef ES_COMMANDS_H
#define ES_COMMANDS_H

/************************************************************************
 * Include Files
 ************************************************************************/
#include <stdint.h>

/************************************************************************
 * Defines
 ************************************************************************/

/************************************************************************
 * Type definitions
 ************************************************************************/
typedef enum _ES_EXP_RECPT_CODE_ {
   ES_MASTER = 0u,
   ES_HOST_NRF,
   ES_HOST_CC430,
   ES_MAX_EXP_RECPT
} e_ES_ExpRecpt_t;

/************************************************************************
 * Public function declarations
 ************************************************************************/
void vES_CommandHandler(uint8_t p_u8OpCode,	uint32_t p_u32Expdt,	uint32_t p_u32Recpt,	uint8_t *p_pu8Data, uint16_t p_u16DataLen);
void vES_ReplySendCommand(uint8_t p_u8OpCode, uint8_t p_u8Acknowledge, uint8_t p_u8Expt, uint8_t *p_pu8Data, uint16_t p_u16DataLen);
void vES_ReplyFromSlave(uint8_t p_u8Expt, uint8_t *p_pu8Data, uint16_t p_u16DataLen);

#endif /* ES_COMMANDS_H */

