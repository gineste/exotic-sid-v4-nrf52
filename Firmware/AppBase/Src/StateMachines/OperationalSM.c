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

/* Config include */
#include "BoardConfig.h"

/* HAL Includes */
#include "HAL/HAL_GPIO.h"
#include "HAL/HAL_RTC.h"

/* BLE Includes */
#include "BLE/BLE_Application.h"

/* Libraries Includes */
#include <Libraries/ES_Protocol/Es_Protocol.h>

/* Application Includes */
#include "SensorManager.h"
#include "OperationalSM.h"
#include "ModeManagement.h"
#include "FrameBuilder.h"
#include "CC430_Interface.h"
#include "FrameBuilder.h"
#include "ES_OpCodes.h"
#include "ES_Commands.h"

#include "GlobalDefs.h"


/****************************************************************************************
 * Defines
 ****************************************************************************************/

/****************************************************************************************
 * Private type declarations
 ****************************************************************************************/
 
/****************************************************************************************
 * Private function declarations
 ****************************************************************************************/

/****************************************************************************************
 * Variable declarations
 ****************************************************************************************/
#if (EN_WAKEUP_ON_MOTION == 1)
   static uint8_t g_u8DeviceInMotion = 1u;
#endif

/****************************************************************************************
 * Public functions
 ****************************************************************************************/ 
/**@brief Entry point of Operational State.
 * @return None
 */
void vOperational_Entry(void)
{
//   e_FrameBuilder_Command_t l_eCfgCommand = FRAME_BLD_CMD_GENERIC;
   uint8_t l_au8Payload[20u] = { 0u };
   uint8_t l_u8Size = 0u;
   
//#if (ENABLE_BLE == 1)
//   /* Start Advertising in Fast Mode */
//   if(eBLE_AdvertiseStateGet() != BLE_ADV_EVT_IDLE)
//   {
//      vBLE_AdvertisingStop();
//   }
//   vBLE_AdvertisingStart(BLE_ADV_MODE_FAST);
//#endif
   
   /* Wake Up Sensors / Get Data / Shutdown Sensors */
   vSensorMngr_WakeUpSensor();   
   vSensorMngr_DataUpdate();
   vSensorMngr_ShutdownSensor();
   
#if (EN_WAKEUP_ON_MOTION == 1)
   //g_u8DeviceInMotion = u8LSM6DSL_IsAwake();
#endif
   
   l_au8Payload[0u] = SID_TAG;   /* Only SID_TAG */
   l_au8Payload[1u] = 0u;

   if(eFrameBuilder_PayloadGet((uint8_t)FRAME_BLD_CMD_GENERIC, &l_au8Payload[2u], &l_u8Size) == FRAME_BLD_ERROR_NONE)
   {
      l_u8Size += 2u;
      (void)eCC430_ITF_SendFrame(CMD_RF_TRANSMISSION, ES_HOST_NRF, ES_HOST_CC430, l_au8Payload, (uint16_t)l_u8Size);
      
   #if (ENABLE_BLE == 1)
      /* Update Advertise data with corresponding command saved in flash */
      vBLE_AdvDataUpdate(&l_au8Payload[2u], (l_u8Size-2u));
   #endif
   }
}
/**@brief Main mode of operation, get data and send it over SigFox, advertise, etc ...
 * @return None
 */
void vOperational_Process(void)
{   
#if (EN_WAKEUP_ON_MOTION == 1)
   static uint8_t l_u8DeviceInMotion = 0u;
   e_FrameBuilder_Command_t l_eCfgCommand = FRAME_BLD_CMD_GENERIC;
   uint8_t l_au8Payload[20u] = { 0u };
   uint8_t l_u8Size = 0u;
#endif
   
   if(u8MSM_CyclicProcessCheck() == 1u)
   {
#if (EN_WAKEUP_ON_MOTION == 1)
      //l_u8DeviceInMotion = u8LSM6DSL_IsAwake();
      
      if(   (l_u8DeviceInMotion == 0u)
         && (g_u8DeviceInMotion == 1u) ) 
      {
         /* Wake Up Sensors / Get Data / Shutdown Sensors */
         vSensorMngr_WakeUpSensor();   
         vSensorMngr_DataUpdate();
         vSensorMngr_ShutdownSensor();
         
         /* Send Data to CC430 */
         vFrameBuilder_CfgGet(&l_eCfgCommand);
         
         l_au8Payload[0u] = SID_TAG;   /* Only SID_TAG */
         l_au8Payload[1u] = 0u;
         
         if(eFrameBuilder_PayloadGet((uint8_t)l_eCfgCommand, &l_au8Payload[2u], &l_u8Size) == FRAME_BLD_ERROR_NONE)
         {            
            l_u8Size += 2u;
            (void)eCC430_ITF_SendFrame(CMD_RF_TRANSMISSION, ES_HOST_NRF, ES_HOST_CC430, l_au8Payload, (uint16_t)l_u8Size);
            
            /* Update Advertise data with corresponding command saved in flash */
            vBLE_AdvDataUpdate(&l_au8Payload[2u], (l_u8Size-2u));            
         }         
   
         g_u8DeviceInMotion = l_u8DeviceInMotion;
      }
      else if( (l_u8DeviceInMotion == 1u)
            && (g_u8DeviceInMotion == 0u) ) 
      {
         g_u8DeviceInMotion = l_u8DeviceInMotion;
      }
#endif
      vMSM_CyclicProcessClear();
   }
}

/**@brief Function to check if device should go in Connected, Error or DeepSleep State.
 * @return None
 */
void vOperational_Check(void)
{
   /* Check Ble Event Status */
   e_Ble_Event_t p_eBleEvent;
   p_eBleEvent = eBLE_EventGet();
   if(p_eBleEvent == BLE_EVENT_CONNECTED)
   {
      vMSM_StateMachineSet(MSM_CONNECTED);
   }
   else if(p_eBleEvent == BLE_EVENT_ERROR)
   {
      vMSM_StateMachineSet(MSM_ERROR);
   }
   else if(u8ModeMngt_IsItTimeToSleep() == 1u)
   {   
      vMSM_StateMachineSet(MSM_DEEP_SLEEP);
   }      
   else
   {  /* Stay in these state */  }      
}

/**@brief Exit point of Operational State.
 * @return None
 */
void vOperational_Exit(void)
{

}

/****************************************************************************************
 * Private functions
 ****************************************************************************************/

/****************************************************************************************
 * End Of File
 ****************************************************************************************/
 

