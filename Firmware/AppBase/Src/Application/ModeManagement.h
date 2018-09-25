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
#ifndef MODE_MANAGEMENT_H
#define MODE_MANAGEMENT_H

/****************************************************************************************
 * Include Files
 ****************************************************************************************/
#include <stdint.h>

/****************************************************************************************
 * Defines
 ****************************************************************************************/
#define FRAME_CFG_NUMBER            (uint8_t)5u
#define FRAME_CFG_WINDOWS_SIZE      (uint8_t)3u
#define FRAME_CFG_WINDOWS_BLE_SIZE  (uint8_t)5u

#define FRAME_IN_MEMORY_SIZE        (uint8_t)(FRAME_CFG_NUMBER*FRAME_CFG_WINDOWS_BLE_SIZE)

/****************************************************************************************
 * Type definitions
 ****************************************************************************************/
typedef enum _MODE_MNGT_WAKEUP_SOURCE_ {
   MODE_MNGT_WAKEUP_SOURCE_TIMER = 0u,
   MODE_MNGT_WAKEUP_SOURCE_ACCEL,
   MODE_MNGT_WAKEUP_SOURCE_TOUCH,
   MODE_MNGT_WAKEUP_SOURCE_NFC,
   MODE_MNGT_WAKEUP_SOURCE_RFID,
   MODE_MNGT_WAKEUP_SOURCE_LAST   
}e_ModeMngt_WakeUpSource_t;


typedef enum _TAG_TYPE_
{
    MEMS_TAG     = ((uint8_t)0x00u),
    LDL_TAG      = ((uint8_t)0x01u),
    AK_TAG       = ((uint8_t)0x02u),
    SCHRADER_TAG = ((uint8_t)0x03u),
    SID_TAG      = ((uint8_t)0x04u),
    REDI_TAG     = ((uint8_t)0x05u),
    ASK_TAG      = ((uint8_t)0x06u),
    HUF_TAG      = ((uint8_t)0x07u),
    MAX_TAG      = ((uint8_t)0x08u), /* Must be after all tags identifiers and before NO_TAG */
    NO_TAG       = ((uint8_t)0xFFu)
} TagTypes_t;

/****************************************************************************************
 * Public function declarations
 ****************************************************************************************/
void vModeMngt_MotionDetected(uint8_t p_u8Level);
uint8_t u8ModeMngt_IsItTimeToSleep(void);
uint8_t u8ModeMngt_BleFrameCfg(uint8_t * p_pau8Data, uint8_t p_u8Size);
uint8_t u8ModeMngt_FrameCfg(uint8_t * p_pau8Data, uint8_t p_u8Size);
void vModeMngt_FrameProcess(void);
uint8_t u8Mode_Mngt_FrameEventNumberGet(void);

#endif /* MODE_MANAGEMENT_H */

