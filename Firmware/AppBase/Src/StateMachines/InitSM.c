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
/* Proprietary includes */
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

/* Config include */
#include "BoardConfig.h"

/* HAL includes */

/* BLE include */
#include "BLE/BT_Interface.h"
#include "BLE/BLE_Application.h"

/* Application includes */
#include "SensorManager.h"
#include "ModeManagement.h"
#include "SID_Storage.h"
#include "ES_SlaveMngt.h"

/* Libraries includes */
//#include "Libraries/NFC.h"

/* Driver includes */
#if (EN_ST25DV == 1)
   #include "ST25DV/ST25DV.h"
#endif

/* Self include + State Machines */
#include "MainStateMachine.h"
#include "InitSM.h"

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
static uint8_t g_u8IsInitialized = 0u;
static uint8_t g_u8Error = 0u;

/****************************************************************************************
 * Public functions
 ****************************************************************************************/ 
/**@brief Entry point of Init State.
 * @return None
 */
void vInit_Entry(void)
{      
   /* BLE (ESI, BT_Itf, Ble_Frog */
   vBT_ITF_Init();
      
   /* Sensor Init */
   vSensorMngr_Init();
   
   /* Flash Storage */
   if(eStorage_Init() != SID_STORAGE_ERROR_NONE)
   {
      g_u8Error = 1u;
   }
   else
   {
      vES_Slave_Init();
   }
#if (EN_NFC == 1)
   vNFC_Init();
#endif
   
   g_u8IsInitialized = 0u;
}

/**@brief Function to initialize State Machine/Sensors/SigFox/etc...
 * @return None
 */
void vInit_Process(void)
{   
   /* Can switch to Operational now */
   g_u8IsInitialized = 1u;    
}

/**@brief Function to check if the system must go in Error or Operational
 * @return None
 */
void vInit_Check(void)
{
   if(g_u8Error == 1u)
   {  /* Something wrong happened go to Error */
      vMSM_StateMachineSet(MSM_ERROR);
   }
   else 
      if(g_u8IsInitialized == 1u)
   {  /* Initialization OK then go to Operational */
      vMSM_StateMachineSet(MSM_OPERATIONAL);
   }
   else
   {  /* Stay in these state */  }
}

/**@brief Exit point of Init State.
 * @return None
 */
void vInit_Exit(void)
{
#if (ENABLE_BLE == 1)
   if(g_u8IsInitialized == 1u)
   {
      /* Start Advertising in Fast Mode */
      if(eBLE_AdvertiseStateGet() != BLE_ADV_EVT_IDLE)
      {
         vBLE_AdvertisingStop();
      }
      vBLE_AdvertisingStart(BLE_ADV_MODE_FAST);
   }
#endif
#if (EN_NFC == 1)
   vST25DV_Initialization();
#endif
}

/****************************************************************************************
 * Private functions
 ****************************************************************************************/

/****************************************************************************************
 * End Of File
 ****************************************************************************************/
 

