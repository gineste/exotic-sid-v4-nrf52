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
#ifndef FRAME_BUILDER_H
#define FRAME_BUILDER_H

/****************************************************************************************
 * Include Files
 ****************************************************************************************/
 
/****************************************************************************************
 * Defines
 ****************************************************************************************/
#define MAX_FRAME_BUILD_SIZE     (uint8_t)127u

/****************************************************************************************
 * Type definitions
 ****************************************************************************************/
typedef enum _FRAME_MNGR_ERROR_ {
   FRAME_BLD_ERROR_NONE = 0u,
   FRAME_BLD_ERROR_PARAM,
   FRAME_BLD_ERROR_CONFIG
}e_FrameBuilder_Error_t;

typedef enum _FRAME_MNGR_CMD_ {
   FRAME_BLD_CMD_BLE = 0u,
   FRAME_BLD_CMD_GENERIC,
   FRAME_BLD_CMD_MOTION_DATA,
   FRAME_BLD_CMD_STATUS,
   FRAME_BLD_CMD_ANGLE,
   FRAME_BLD_CMD_MAX   
}e_FrameBuilder_Command_t;

/****************************************************************************************
 * Public function declarations
 ****************************************************************************************/
void vFrameBuilder_Init(void);
e_FrameBuilder_Error_t eFrameBuilder_CfgUpdate(e_FrameBuilder_Command_t p_eCmd);
void vFrameBuilder_ResetCfg(void);
void vFrameBuilder_CfgGet(e_FrameBuilder_Command_t * p_peCmd);
uint8_t u8FrameBuilder_IsCfgSet(void);
e_FrameBuilder_Error_t eFrameBuilder_PayloadGet(uint8_t p_u8Command, uint8_t * p_pu8Payload, uint8_t * p_pu8PayloadSize);

#endif /* FRAME_BUILDER_H */

