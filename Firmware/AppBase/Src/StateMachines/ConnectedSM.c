/* 
 *    ____  _  _   __   ____   __    ___    ____  _  _  ____  ____  ____  _  _  ____
 *   (  __)( \/ ) /  \ (_  _) (  )  / __)  / ___)( \/ )/ ___)(_  _)(  __)( \/ )/ ___)
 *    ) _)  )  ( (  O )  )(    )(  ( (__   \___ \ )  / \___ \  )(   ) _) / \/ \\___ \     
 *   (____)(_/\_) \__/  (__)  (__)  \___)  (____/(__/  (____/ (__) (____)\_)(_/(____/
 *
 * Copyright (c) 2017 EXOTIC SYSTEMS. All Rights Reserved.
 *
 * Licensees are granted free, non-transferable use of the information. NO WARRANTY 
 * of ANY KIND is provided. This heading must NOT be removed from the file.
 *
 * Date:          10/10/2017 (dd MM YYYY)
 * Author:        Yoann Rebischung
 * Description:   Sub State Machine Module
 *
 */
 
/****************************************************************************************
 * Include Files
 ****************************************************************************************/
#include <stdint.h>
#include <string.h>

#include "BoardConfig.h"
#include "GlobalDefs.h"

/* Application Includes */
#include "SensorManager.h"
#include "FrameBuilder.h"

#include "ES_OpCodes.h"
#include "ES_Commands.h"
#include "CC430_Interface.h"

/* BLE Includes */
#include "BLE/BLE_Application.h"
#include "BLE/BT_Interface.h"

/* Libraries Includes */
#include <Libraries/ES_Protocol/Es_Protocol.h>

/* Self + MainStateMachine Includes */
#include "MainStateMachine.h"
#include "ConnectedSM.h"

/****************************************************************************************
 * Defines
 ****************************************************************************************/
#define MAX_PAYLOAD_SIZE      MAX_FRAME_BUILD_SIZE

/****************************************************************************************
 * Private type declarations
 ****************************************************************************************/
 
/****************************************************************************************
 * Private function declarations
 ****************************************************************************************/

/****************************************************************************************
 * Variable declarations
 ****************************************************************************************/

/****************************************************************************************
 * Public functions
 ****************************************************************************************/ 
/**@brief Entry point of Connected State.
 * @return None
 */
void vConnected_Entry(void)
{
   vSensorMngr_WakeUpSensor();
}

/**@brief Once connected functionalities if different from operational.
 * @return None
 */
void vConnected_Process(void)
{
   e_FrameBuilder_Command_t l_eCfgCommand = FRAME_BLD_CMD_BLE;
   
#if (CC430_STREAM_DATA == 1)
//   uint8_t l_au8PayloadOut[25u] = { 0u };
//   uint16_t l_u16SizeOut = 0u;
#endif
   uint8_t l_au8PayloadIn[MAX_PAYLOAD_SIZE] = { 0u };
   uint8_t l_u8SizeIn = 0u;
      
   if(u8MSM_CyclicProcessCheck() == 1u)
   {
      /* Update Data */
      vSensorMngr_DataUpdate();

      if(u8FrameBuilder_IsCfgSet() == 1u)
      {
         /* Get Register Cmd number to send */
         vFrameBuilder_CfgGet(&l_eCfgCommand);
         
         /* Build corresponding command */
         if(eFrameBuilder_PayloadGet((uint8_t)l_eCfgCommand, l_au8PayloadIn, &l_u8SizeIn) == FRAME_BLD_ERROR_NONE)
         {
            /* Send it over BLE */
            (void)eBT_ITF_SendFrame(CMD_SENSOR_CYC_DATA_STREAM, ES_HOST_NRF, ES_MASTER, l_au8PayloadIn, l_u8SizeIn);         
         
//      #if (EN_LOG == 1)
//         PRINT_T2_ARRAY("%02X", l_au8PayloadIn, l_u8SizeIn);
//         PRINT_T2("%s", "\n");
//      #endif
            /* Send it over CC430 */
         #if (CC430_STREAM_DATA == 1) 
            #if (NO_CC430_ON_BOARD == 0)
//         l_u16SizeOut = ESP_BuildCommand(l_au8PayloadOut, ES_HOST_NRF, ES_HOST_CC430, CMD_SENSOR_CYC_DATA_STREAM, l_au8PayloadIn, l_u8SizeIn);
//         (void)u32CC430_Send(l_au8PayloadOut, l_u16SizeOut);
            (void)eCC430_ITF_SendFrame(CMD_SENSOR_CYC_DATA_STREAM, ES_HOST_NRF, ES_HOST_CC430, l_au8PayloadIn, (uint16_t)l_u8SizeIn);
            #endif
         #endif
         }
      }
      
      
      vMSM_CyclicProcessClear();
   }
}

/**@brief Function to check if device should go in Operational, Error or DeepSleep State.
 * @return None
 */
void vConnected_Check(void)
{
   /* Check Ble Event Status */
   e_Ble_Event_t p_eBleEvent;
   p_eBleEvent = eBLE_EventGet();
   if(   (p_eBleEvent == BLE_EVENT_DISCONNECTED)
      || (p_eBleEvent == BLE_EVENT_TIMEOUT) )
   {
      vMSM_StateMachineSet(MSM_OPERATIONAL);
   }
   else if(p_eBleEvent == BLE_EVENT_ERROR)
   {
      vMSM_StateMachineSet(MSM_ERROR);
   }
   else
   {  /* Stay in these state */  }
}

/**@brief Exit point of Connected State.
 * @return None
 */
void vConnected_Exit(void)
{
   vFrameBuilder_ResetCfg();
   vSensorMngr_ShutdownSensor();   
}

/****************************************************************************************
 * Private functions
 ****************************************************************************************/

/****************************************************************************************
 * End Of File
 ****************************************************************************************/
 

