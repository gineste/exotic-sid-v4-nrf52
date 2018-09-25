/* 
 *    ____  _  _   __   ____   __    ___    ____  _  _  ____  ____  ____  _  _  ____
 *   (  __)( \/ ) /  \ (_  _) (  )  / __)  / ___)( \/ )/ ___)(_  _)(  __)( \/ )/ ___)
 *    ) _)  )  ( (  O )  )(    )(  ( (__   \___ \ )  / \___ \  )(   ) _) / \/ \\___ \     
 *   (____)(_/\_) \__/  (__)  (__)  \___)  (____/(__/  (____/ (__) (____)\_)(_/(____/
 *
 * Copyright (c) 2018 EXOTIC SYSTEMS. All Rights Reserved.
 *
 * Licensees are granted free, non-transferable use of the information. NO WARRANTY 
 * of ANY KIND is provided. This heading must NOT be removed from the file.
 *
 */
#ifndef ES_SLAVEMNGT_H
#define ES_SLAVEMNGT_H

/****************************************************************************************
 * Include Files
 ****************************************************************************************/
 
/****************************************************************************************
 * Defines
 ****************************************************************************************/

/****************************************************************************************
 * Type definitions
 ****************************************************************************************/

/****************************************************************************************
 * Public function declarations
 ****************************************************************************************/
void vES_Slave_CommandHandler(uint8_t p_u8OpCode,	uint32_t p_u32Expdt,	uint32_t p_u32Recpt,	uint8_t *p_pu8Data, uint16_t p_u16DataLen);
void vES_Slave_Init(void);
void vES_Slave_RFFrameSend(uint32_t p_u32Expdt, uint8_t * p_pau8Data, uint16_t p_u16Size);

uint8_t u8ES_Slave_TxOnRequestCounterGet(void);
uint16_t u16ES_Slave_TxPeriodicCounterGet(void);
   
#endif /* ES_SLAVEMNGT_H */

