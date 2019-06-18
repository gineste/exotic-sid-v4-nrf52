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
 * Date:          10 07 2017 (dd MM YYYY)
 * Author:        Yoann Rebischung
 * Description:   Main State Machine of nRF BLE Gravity
 *
 */

/************************************************************************
 * Include Files
 ************************************************************************/
/* Proprietary includes */
#include <stdint.h>
#include <string.h>

#include "BoardConfig.h"
#include "GlobalDefs.h"

/* HAL Includes */
#include "HAL/HAL_GPIO.h"
#include "HAL/HAL_Timer.h"
#include "HAL/HAL_WDT.h"

/* Libraries includes */

/* BLE includes */
#include "BLE/BLE_Application.h"
#include "BLE/BT_Interface.h"

/* Application include */
#include "ModeManagement.h"
#include "CC430_Interface.h"

/* Self include + State Machines */
#include "PowerUpSM.h"
#include "InitSM.h"
#include "OperationalSM.h"
#include "ConnectedSM.h"
#include "DeepSleepSM.h"
#include "ErrorSM.h"

#include "Version.h"

#include "MainStateMachine.h"

/************************************************************************
 * Defines
 ************************************************************************/
#define NO_TIMER                       ((uint32_t)0u)

#define INIT_CB_TIMER_MS               ((uint32_t)200u)
#define OPERATIONAL_CB_TIMER_MS        ((uint32_t)250u)
#define CONNECTED_CB_TIMER_MS          1000u
#define DEEPSLEEP_CB_TIMER_MS          NO_TIMER//MIN_TO_MS(1u)

/************************************************************************
 * Private type declarations
 ************************************************************************/
typedef struct _STATEMACHINE_ {
   void (*fp_vEntryPoint)(void);
   void (*fp_vIdleProcess)(void);
   void (*fp_vNextStateCheck)(void);
   void (*fp_vExitPoint)(void);
   uint32_t u32CallbackTimer;
}s_StateMachine_t;

/************************************************************************
 * Private function declarations
 ************************************************************************/
static void vBackgroundProcess(void);

#if (EN_WDT_PROTECTION == 1)
   static void vWDTProtectionHandler(void);
#endif

static void vCallbackProcess(void * p_pvContext);

/************************************************************************
 * Variable declarations
 ************************************************************************/
//static uint8_t g_u8Error = 0u;
e_MainStateMachine_t g_eMainState = MSM_POWER_UP;
HAL_TIMER_DEF(MSM_ProcessTimerIdx);

static uint8_t g_u8CyclicProcess = 1u;

static s_StateMachine_t g_sStateMachines[MSM_MAX_NUMBERS] = {
   /*Entry,             Process,                Check,               Exit                 CallbackTimer*/
   {NULL,               vPowerUp_Process,       vPowerUp_Check,      NULL,                INIT_CB_TIMER_MS},         /* MSM_POWER_UP */
   {vInit_Entry,        vInit_Process,          vInit_Check,         vInit_Exit,          INIT_CB_TIMER_MS},         /* MSM_INIT */
   {vOperational_Entry, vOperational_Process,   vOperational_Check,  vOperational_Exit,   OPERATIONAL_CB_TIMER_MS},  /* MSM_OPERATIONAL */
   {vConnected_Entry,   vConnected_Process,     vConnected_Check,    vConnected_Exit,     CONNECTED_CB_TIMER_MS},    /* MSM_CONNECTED */
   {vDeepSleep_Entry,   vDeepSleep_Process,     vDeepSleep_Check,    vDeepSleep_Exit,     DEEPSLEEP_CB_TIMER_MS},    /* MSM_DEEP_SLEEP */
   {vError_Entry,       vError_Process,         vError_Check,        vError_Exit,         NO_TIMER}                  /* MSM_ERROR */
};

#if (EN_LOG == 1)
   static uint8_t g_au8StateString[MSM_MAX_NUMBERS][12u] = 
   { "POWER UP\0", "INIT\0", "OPERATIONAL\0", "CONNECTED\0", "DEEP SLEEP\0", "ERROR\0" };
#endif

#ifdef DEBUG
   uint16_t g_u16ProcessCnt = 0u;
#endif

/************************************************************************
 * Public functions
 ************************************************************************/
/**@brief Function to generate an interruption in order to wakeup µC.
 * @return None
 */
void vMSM_Init(void)
{
   /* Init Timer First */
   vHal_Timer_Init();
   
   /* GPIO Init */
   vHal_GPIO_Init();
   
   CLEAR_TERMINAL();
   PRINT_INFO("%s","\n/****************************/\n");
   PRINT_INFO("%s","/*          SID v4          */\n");
   PRINT_INFO("%s","/*      nRF v ");
   PRINT_INFO("%02d.",FW_VERSION_MAJOR);
      PRINT_INFO("%02d.",FW_VERSION_MINOR);
         PRINT_INFO("%02d",FW_VERSION_REVISION);
   PRINT_INFO("%s","      */\n/*      Commit ");
      PRINT_INFO("%s      */\n",COMMIT_NUMBER);
   PRINT_INFO("/*   %s ",BUILD_TIME);PRINT_INFO("%s   */\n",BUILD_DATE);
   PRINT_INFO("%s","/****************************/\n\n");

#if (EN_WDT_PROTECTION == 1)
   /* Watchdog Init */
   (void)eHal_WDT_Init(WDT_TIMEOUT, vWDTProtectionHandler);
   (void)eHal_WDT_Start();
#endif

   if(eHal_Timer_Create(&MSM_ProcessTimerIdx, HAL_TIMER_MODE_REPEATED, &vCallbackProcess) == HAL_TIMER_ERROR_NONE)
   {
      if(eHal_Timer_Start(MSM_ProcessTimerIdx, g_sStateMachines[g_eMainState].u32CallbackTimer) != HAL_TIMER_ERROR_NONE)
      {
         PRINT_ERROR("%s","Start Process Timer Fail !");
      }
   }
   else
   {
      PRINT_ERROR("%s","Create Process Timer Fail !");
   }

}

/**@brief Process function of Main State Machine.
 * @return None
 */
void vMSM_Process(void)
{
   /* Run Idle Process */
   if(g_u8CyclicProcess == 1u)
   {
#ifdef DEBUG
   g_u16ProcessCnt = 0u;
#endif
      if(g_sStateMachines[g_eMainState].fp_vIdleProcess != NULL)
      {
         g_sStateMachines[g_eMainState].fp_vIdleProcess();
      }/* Else nothing to execute */

      /* Then run Check for next State Process */
      if(g_sStateMachines[g_eMainState].fp_vNextStateCheck != NULL)
      {
         g_sStateMachines[g_eMainState].fp_vNextStateCheck();
      }/* Else nothing to execute */
      
      g_u8CyclicProcess = 0u;
      /* Re execute process directly (should generate event) for WFE__ */
      if(g_sStateMachines[g_eMainState].u32CallbackTimer == NO_TIMER)
      {
         g_u8CyclicProcess = 1u;
      }
   }
   vBackgroundProcess();
}

/**@brief Function to switch between the different state machine.
 *
 * @param[in]  p_eState : The next state machine to run
 * @return     None
 */
void vMSM_StateMachineSet(e_MainStateMachine_t p_eState)
{   
   if(p_eState < MSM_MAX_NUMBERS)
   {
      /* Stop Timer */
      if(g_sStateMachines[g_eMainState].u32CallbackTimer != NO_TIMER)
      {
         (void)eHal_Timer_Stop(MSM_ProcessTimerIdx);
      }
      /* Execute Exit point of current */
      if(g_sStateMachines[g_eMainState].fp_vExitPoint != NULL)
      {
         g_sStateMachines[g_eMainState].fp_vExitPoint();
      }/* Else nothing to execute */
      
      /* Assign new State */
      g_eMainState = p_eState;
      
      /* Execute Entry point of new State */
      if(g_sStateMachines[g_eMainState].fp_vEntryPoint != NULL)
      {
         g_sStateMachines[g_eMainState].fp_vEntryPoint();
      }/* Else nothing to execute */
      
      /* Start new Timer */
      if(g_sStateMachines[g_eMainState].u32CallbackTimer != NO_TIMER)
      {
         (void)eHal_Timer_Start(MSM_ProcessTimerIdx,g_sStateMachines[g_eMainState].u32CallbackTimer);
      }
            
      PRINT_INFO("%s","Main StateMachine : ");
      PRINT_STATE("%s\n",g_au8StateString[g_eMainState]);
      
   } /* Else not a valid state machine */
}

/**@brief Get current state of Main State Machine.
 * @return State of Main State Machine
 */
e_MainStateMachine_t eMSM_GetState(void)
{
   return g_eMainState;
}

/**@brief Get Cyclic process run state of State Machine.
 * @return State of CyclicProcess variable.
 */
uint8_t u8MSM_CyclicProcessCheck(void)
{
   return g_u8CyclicProcess;
}

/**@brief Clear Cyclic Process variable.
 * @return None.
 */
void vMSM_CyclicProcessClear(void)
{
   g_u8CyclicProcess = 0u;
}
void vMSM_CyclicProcessSet(uint8_t p_u8Activate)
{
   g_u8CyclicProcess = (p_u8Activate == 1u)?1u:0u;
}

/**@brief Set refresh time of the current state of Main State Machine.
 * @param[in]  p_u32RefreshTime : New refresh rate of the current 
 *             state machine.
 * @return     None
 */
void vStateMachine_ChangeTime(uint32_t p_u32RefreshTime)
{
   /* Stop Timer */
   if(g_sStateMachines[g_eMainState].u32CallbackTimer != NO_TIMER)
   {
      (void)eHal_Timer_Stop(MSM_ProcessTimerIdx);
   }  
   /* Update only Connected State Time */   
   g_sStateMachines[MSM_CONNECTED].u32CallbackTimer = p_u32RefreshTime;
   /* Update Timer */
   if(g_sStateMachines[g_eMainState].u32CallbackTimer != NO_TIMER)
   {
      (void)eHal_Timer_Start(MSM_ProcessTimerIdx,g_sStateMachines[g_eMainState].u32CallbackTimer);
   }
}

/************************************************************************
 * Private functions
 ************************************************************************/
/**@brief Idle process function necessary for UART/Event/Sensor/...
 * @return None
 */
static void vBackgroundProcess(void)
{
#ifdef DEBUG
   g_u16ProcessCnt++;
#endif
   /* BLE Interface */
   vBT_ITF_IdleTask();
      
   /* CC430 Process */
   vCC430_ITF_IdleTask();
   
#if (NO_FRAME_CHECK == 0)
   /* Frame Checker Process */
   vModeMngt_FrameProcess();
#endif
   
#if (EN_WDT_PROTECTION == 1)
   /* Watchdog timer reload */
   (void)eHal_WDT_Reload();
#endif
}

#if (EN_WDT_PROTECTION == 1)
static void vWDTProtectionHandler(void)
{
   __nop();
}
#endif


/**********************************************
 * Private Functions linked to State Machines *
 **********************************************/
static void vCallbackProcess(void * p_pvContext)
{
   g_u8CyclicProcess = 1u;
}

/************************************************************************
 * End Of File
 ************************************************************************/


