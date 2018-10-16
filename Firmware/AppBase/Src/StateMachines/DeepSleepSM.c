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

#include "board.h"
#include "BoardConfig.h"
#include "GlobalDefs.h"

#include "app_util_platform.h"

/* BLE Includes */
#include "BLE/BLE_Application.h"

/* HAL Includes */
#include "HAL/HAL_GPIO.h"
#include "HAL/HAL_Timer.h"

/* Application Includes */
#include "SensorManager.h"
#include "ModeManagement.h"

/* Drivers Includes */

/* Libraries Includes */
//#include "Libraries/NFC.h"

/* Self + MainStateMachine Includes */
#include "MainStateMachine.h"
#include "DeepSleepSM.h"

/****************************************************************************************
 * Defines
 ****************************************************************************************/

/****************************************************************************************
 * Private type declarations
 ****************************************************************************************/
 
/****************************************************************************************
 * Private function declarations
 ****************************************************************************************/
static void vCallBackWakeUpMode(void * p_pvContext);

/****************************************************************************************
 * Variable declarations
 ****************************************************************************************/
HAL_TIMER_DEF(g_DeepSleepWakeUpTimerIdx);
static volatile uint8_t g_u8TimeoutWakeUp = 0u;

/****************************************************************************************
 * Public functions
 ****************************************************************************************/ 
/**@brief Entry point of DeepSleep State.
 * @return None
 */
void vDeepSleep_Entry(void)
{
   static uint8_t l_u8TimerCreated = 0u;
   /* Stop Advertising (if still advertise)*/         
   if(eBLE_AdvertiseStateGet() != BLE_ADV_EVT_IDLE)
   {
      vBLE_AdvertisingStop();
   }
   
   if((l_u8TimerCreated == 0u) && (WAKE_ADV_TIMEOUT_BLE_MS != 0u))
   {
      (void)eHal_Timer_Create(&g_DeepSleepWakeUpTimerIdx, HAL_TIMER_MODE_SINGLE_SHOT, &vCallBackWakeUpMode);
      l_u8TimerCreated = 1u;
   }
   if(WAKE_ADV_TIMEOUT_BLE_MS != 0u)
   {
      if(eHal_Timer_Start(g_DeepSleepWakeUpTimerIdx, WAKE_ADV_TIMEOUT_BLE_MS) != HAL_TIMER_ERROR_NONE)
      {
         g_u8TimeoutWakeUp = 1u;
      }
   }
   PRINT_DEBUG("Wake Up with Motion, RFID or Timeout(%d ms)\n",WAKE_ADV_TIMEOUT_BLE_MS);
   
   vSensorMngr_WakeUpSensor();   
   vSensorMngr_ShutdownSensor();


#if (EN_NFC == 1)
   (void)u8NFC_IsFieldDetected();
//   vNFC_StartEmulation();
#endif
}

/**@brief Lowest Power Mode, do absolutely nothing.
 * @return None
 */
void vDeepSleep_Process(void)
{   
   if(u8MSM_CyclicProcessCheck() == 1u)
   {
      vMSM_CyclicProcessClear();
   }
}

/**@brief Lowest Power Mode, do absolutely nothing.
 * @return None
 */
void vDeepSleep_Check(void)
{
   uint8_t l_u8Motion = 0u;
   uint8_t l_u8RFID = 0u;
   uint8_t l_u8NFC = 0u;
   
#if (EN_WAKEUP_ON_MOTION == 1)
  // l_u8Motion = u8LSM6DSL_IsAwake();
#endif
   
#if (EN_RFID == 1)
   l_u8RFID = u8AS3933_WakeUpDetected();
#endif
   
#if (EN_NFC == 1)
   l_u8NFC = u8NFC_IsFieldDetected();
#endif
   
   if(   (l_u8Motion == 1u)
      || (g_u8TimeoutWakeUp == 1u) 
      || (l_u8RFID == 1u )
      || (l_u8NFC == 1u) )
   {
      if(g_u8TimeoutWakeUp == 1u)
      {
         PRINT_DEBUG("Wake Up by Timeout %s","\n");
      }
      else if(l_u8RFID == 1u)
      {            
      #if (EN_RFID == 1)
         vAS3933_ClearWakeUp();
      #endif 
         PRINT_DEBUG("Wake Up by RFID %s","\n");
         /* Start Advertise Fast */
         vBLE_AdvertisingStart(BLE_ADV_MODE_FAST);
      }
      else if(l_u8NFC == 1u)
      {
         PRINT_DEBUG("Wake Up by NFC %s","\n");
         /* Start Advertise Fast */
         vBLE_AdvertisingStart(BLE_ADV_MODE_FAST);
      }
      else if(l_u8Motion == 1u)
      {
         PRINT_DEBUG("Wake Up by Motion %s","\n");
      }
      else
      {
         PRINT_DEBUG("Unknow Wake Up condition %s","\n");         
      }
      
      vMSM_StateMachineSet(MSM_OPERATIONAL);
   }
   else
   {  /* Nothing todo */  }
}

/**@brief Exit point of DeepSleep State.
 * @return None
 */
void vDeepSleep_Exit(void)
{
   (void)eHal_Timer_Stop(g_DeepSleepWakeUpTimerIdx);

#if (EN_NFC == 1)
//   vNFC_StopEmulation();
#endif
   
   g_u8TimeoutWakeUp = 0u;
}

/****************************************************************************************
 * Private functions
 ****************************************************************************************/
static void vCallBackWakeUpMode(void * p_pvContext)
{
   g_u8TimeoutWakeUp = 1u;
}

/****************************************************************************************
 * End Of File
 ****************************************************************************************/
 

