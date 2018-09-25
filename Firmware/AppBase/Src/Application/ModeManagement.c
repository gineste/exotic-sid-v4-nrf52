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
 * Date:          05/03/2018 (dd MM YYYY)
 * Author:        Yoann Rebischung
 * Description:   Mode management (WakeUp/ShutDown) 
 *
 */
 
/****************************************************************************************
 * Include Files
 ****************************************************************************************/
#include <stdint.h>
#include <string.h>

#include "BoardConfig.h"
#include "GlobalDefs.h"

#include "MainStateMachine.h"
#include "FrameBuilder.h"
#include "CC430_Interface.h"
#include "ES_OpCodes.h"
#include "ES_Commands.h"
#include "SensorManager.h"
#include "ES_SlaveMngt.h"

#include "BLE/BLE_Application.h"

#include "HAL/HAL_RTC.h"

/* Self include */
#include "ModeManagement.h"

/****************************************************************************************
 * Defines
 ****************************************************************************************/
#define FRAME_SIZE_MAX              (uint8_t)22

/****************************************************************************************
 * Private type declarations
 ****************************************************************************************/
typedef union _CFG_BYTE_0_ {
   struct {
      uint8_t bCmdNb:4;
      uint8_t bTransmissionMode:1;
      uint8_t bWindowsNb:3;
   }sByte0;
   uint8_t u8Cfg0;
}u_CfgByte0_t;

typedef union _CFG_BYTE_1_ {
   struct {
      uint8_t bTimeMSB:4;
      uint8_t bDimming:4;
   }sByte1;
   uint8_t u8Cfg1;
}u_CfgByte1_t;

typedef union _CFG_BYTE_2_ {
   struct {
      uint8_t bTimeLSB:8;
   }sByte2;
   uint8_t u8Cfg2;
}u_CfgByte2_t;

typedef struct _FRAME_WINDOWS_CFG_ {
   u_CfgByte0_t uByte0;
   u_CfgByte1_t uByte1;
   u_CfgByte2_t uByte2;
}s_WindowsFrameCfg_t;

typedef struct _FRAME_CFG_CC430_ {
   uint8_t u8WindowsNb;
   uint8_t u8Active;
   uint8_t u8Mode;
   uint8_t u8Cmd;
   int8_t s8Dimming;
   uint8_t u8TagType;
   uint16_t u16Time;
   uint8_t u8TimeToSend;
   uint16_t u16TimeToSend;
}s_CC430FrameCfg_t;


/****************************************************************************************
 * Private function declarations
 ****************************************************************************************/

/****************************************************************************************
 * Variable declarations
 ****************************************************************************************/
s_CC430FrameCfg_t g_asCfgFrame[FRAME_CFG_NUMBER] = { 0u };

/****************************************************************************************
 * Public functions
 ****************************************************************************************/ 
void vModeMngt_MotionDetected(uint8_t p_u8Level)
{
#if (EN_WAKEUP_ON_MOTION == 1)
   if(p_u8Level == 1u)
   {
      if(eMSM_GetState() == MSM_OPERATIONAL)         
      {
         /* Restart Advertising in Fast Mode */
         if(eBLE_AdvertiseStateGet() != BLE_ADV_EVT_IDLE)
         {
            vBLE_AdvertisingStop();
         }
         vBLE_AdvertisingStart(BLE_ADV_MODE_FAST);
      }
      else
      {  /* Nothing to do */  }
   }
#endif
}

uint8_t u8ModeMngt_IsItTimeToSleep(void)
{
   uint8_t l_u8ItsTime = 0u;
   
   /* Operational Time Over (No more Advertise) */
   l_u8ItsTime = (   (eMSM_GetState() == MSM_OPERATIONAL) 
                  && (eBLE_AdvertiseStateGet() == BLE_ADV_EVT_IDLE) 
   #if (EN_WAKEUP_ON_MOTION == 1)
   /* No activity on LSM6DSL */
   //               && u8LSM6DSL_IsAwake() == 0u ) ? 1u:0u;
   #else
                  ) ? 1u:0u;
   #endif
   
   return l_u8ItsTime;
}
uint8_t u8ModeMngt_BleFrameCfg(uint8_t * p_pau8Data, uint8_t p_u8Size)
{
   uint8_t l_u8Result = 0u;
   uint8_t l_u8Idx = 0u;
   uint8_t l_u8WinIdx = 0u;
   uint8_t l_u8FrameCfgNb = 0u;
   u_CfgByte0_t l_uFirstByte;
   s_WindowsFrameCfg_t l_asWinCfgFrame = { 0u };
   
   if((p_u8Size % FRAME_CFG_WINDOWS_BLE_SIZE) != 0u)
   {
      l_u8Result = 0u;
   }
   else
   {
      l_uFirstByte.u8Cfg0 = 0u;
      l_u8FrameCfgNb = p_u8Size / FRAME_CFG_WINDOWS_BLE_SIZE;
      if(l_u8FrameCfgNb > FRAME_CFG_NUMBER)
      {
         l_u8Result = 0u;
      }
      else
      {
         for(l_u8Idx = 0u; l_u8Idx < l_u8FrameCfgNb; l_u8Idx++)
         {
            l_uFirstByte.u8Cfg0 = p_pau8Data[FRAME_CFG_WINDOWS_BLE_SIZE*l_u8Idx];
            
            /* Get Windows Index in Global variable */
            l_u8WinIdx = l_uFirstByte.sByte0.bWindowsNb;
            
            if(l_u8WinIdx < FRAME_CFG_NUMBER)
            {
               /* Get data from buffer */
               l_asWinCfgFrame.uByte0.u8Cfg0 = p_pau8Data[(FRAME_CFG_WINDOWS_BLE_SIZE*l_u8Idx)];
               
               l_asWinCfgFrame.uByte1.u8Cfg1 = p_pau8Data[(FRAME_CFG_WINDOWS_BLE_SIZE*l_u8Idx) + 1u];
               l_asWinCfgFrame.uByte2.u8Cfg2 = p_pau8Data[(FRAME_CFG_WINDOWS_BLE_SIZE*l_u8Idx) + 2u];
               
               /* Make data readable */
               g_asCfgFrame[l_u8WinIdx].u8WindowsNb   = l_asWinCfgFrame.uByte0.sByte0.bWindowsNb;
               g_asCfgFrame[l_u8WinIdx].u8Mode        = l_asWinCfgFrame.uByte0.sByte0.bTransmissionMode;
               g_asCfgFrame[l_u8WinIdx].u8Cmd         = l_asWinCfgFrame.uByte0.sByte0.bCmdNb;
               g_asCfgFrame[l_u8WinIdx].s8Dimming     = (int8_t)p_pau8Data[(FRAME_CFG_WINDOWS_BLE_SIZE*l_u8Idx) + 1u];
               g_asCfgFrame[l_u8WinIdx].u8TagType     = p_pau8Data[(FRAME_CFG_WINDOWS_BLE_SIZE*l_u8Idx) + 2u];
               g_asCfgFrame[l_u8WinIdx].u16Time       = U8_TO_U16(p_pau8Data[(FRAME_CFG_WINDOWS_BLE_SIZE*l_u8Idx) + 3u], 
                                                               p_pau8Data[(FRAME_CFG_WINDOWS_BLE_SIZE*l_u8Idx) + 4u]);
               g_asCfgFrame[l_u8WinIdx].u16TimeToSend = 0u;
               
               if(g_asCfgFrame[l_u8WinIdx].u8Cmd == 0u)
               {
                  g_asCfgFrame[l_u8WinIdx].u8Active = 0u;
               }
               else
               {
                  g_asCfgFrame[l_u8WinIdx].u8Active = 1u;
               }
            }
         }
         
         l_u8Result = 1u;         
      }      
   }
   
   return l_u8Result;
}

uint8_t u8ModeMngt_FrameCfg(uint8_t * p_pau8Data, uint8_t p_u8Size)
{
   uint8_t l_u8Result = 0u;
   uint8_t l_u8Idx = 0u;
   uint8_t l_u8WinIdx = 0u;
   uint8_t l_u8FrameCfgNb = 0u;
   u_CfgByte0_t l_uFirstByte;
   s_WindowsFrameCfg_t l_asWinCfgFrame = { 0u };
   
   if((p_u8Size % FRAME_CFG_WINDOWS_SIZE) != 0u)
   {
      l_u8Result = 0u;
   }
   else
   {
      l_uFirstByte.u8Cfg0 = 0u;
      l_u8FrameCfgNb = p_u8Size / FRAME_CFG_WINDOWS_SIZE;
      if(l_u8FrameCfgNb > FRAME_CFG_NUMBER)
      {
         l_u8Result = 0u;
      }
      else
      {
         for(l_u8Idx = 0u; l_u8Idx < l_u8FrameCfgNb; l_u8Idx++)
         {
            l_uFirstByte.u8Cfg0 = p_pau8Data[FRAME_CFG_WINDOWS_SIZE*l_u8Idx];
            
            /* Get Windows Index in Global variable */
            l_u8WinIdx = l_uFirstByte.sByte0.bWindowsNb;
            
            if(l_u8WinIdx < FRAME_CFG_NUMBER)
            {
               /* Get data from buffer */
               l_asWinCfgFrame.uByte0.u8Cfg0 = p_pau8Data[(FRAME_CFG_WINDOWS_SIZE*l_u8Idx)];
               l_asWinCfgFrame.uByte1.u8Cfg1 = p_pau8Data[(FRAME_CFG_WINDOWS_SIZE*l_u8Idx) + 1u];
               l_asWinCfgFrame.uByte2.u8Cfg2 = p_pau8Data[(FRAME_CFG_WINDOWS_SIZE*l_u8Idx) + 2u];
               
               /* Make data readable */
               g_asCfgFrame[l_u8WinIdx].u8WindowsNb   = l_asWinCfgFrame.uByte0.sByte0.bWindowsNb;
               g_asCfgFrame[l_u8WinIdx].u8Mode        = l_asWinCfgFrame.uByte0.sByte0.bTransmissionMode;
               g_asCfgFrame[l_u8WinIdx].u8Cmd         = l_asWinCfgFrame.uByte0.sByte0.bCmdNb;
               g_asCfgFrame[l_u8WinIdx].s8Dimming     = (int8_t)((int8_t)(l_asWinCfgFrame.uByte1.sByte1.bDimming) << 1);
               g_asCfgFrame[l_u8WinIdx].u16Time       = U8_TO_U16(l_asWinCfgFrame.uByte1.sByte1.bTimeMSB, 
                                                               l_asWinCfgFrame.uByte2.sByte2.bTimeLSB);
               g_asCfgFrame[l_u8WinIdx].u16TimeToSend = 0u;
               
               if(g_asCfgFrame[l_u8WinIdx].u8Cmd == 0u)
               {
                  g_asCfgFrame[l_u8WinIdx].u8Active = 0u;
               }
               else
               {
                  g_asCfgFrame[l_u8WinIdx].u8Active = 1u;
               }
            }
         }
         
         l_u8Result = 1u;         
      }      
   }
   
   return l_u8Result;
}


void vModeMngt_FrameProcess(void)
{
   uint8_t l_u8Idx = 0u;
   uint8_t l_au8Data[FRAME_SIZE_MAX]= { 0u };
   uint8_t l_u8Size = 0u;
   uint32_t l_u32Time = u32Hal_RTC_TimeStampGet();
   static uint32_t l_u32PrevTime = 0u;   
   static uint32_t l_u32OldTime[FRAME_CFG_NUMBER] = { 0u };
   uint8_t l_u8SomethingToSend = 0u;
   e_MainStateMachine_t l_eMainState;
   
   if(l_u32Time != l_u32PrevTime)
   {
      l_u32PrevTime = l_u32Time;
      for(l_u8Idx = 0u; l_u8Idx < FRAME_CFG_NUMBER; l_u8Idx++)
      {
         if(g_asCfgFrame[l_u8Idx].u8Active == 1u)
         {
            if(g_asCfgFrame[l_u8Idx].u8Mode == 0u)
            { /* Timer Mode */
               g_asCfgFrame[l_u8Idx].u16TimeToSend = l_u32Time - l_u32OldTime[l_u8Idx];
               if(g_asCfgFrame[l_u8Idx].u16TimeToSend >= g_asCfgFrame[l_u8Idx].u16Time)
               {
                  l_u8SomethingToSend = 1u;
                  g_asCfgFrame[l_u8Idx].u8TimeToSend = 1u;
                  
                  /* Update Old Time */
                  l_u32OldTime[l_u8Idx] = l_u32Time;
               }
            }
            else
            {  /* Event Mode */
               
            }
         }      
      }
      
      if(l_u8SomethingToSend == 1u)
      {
         /* Get Data(depending on state) and Send them */
         l_eMainState = eMSM_GetState();
         switch(l_eMainState)
         {
            case MSM_DEEP_SLEEP:
            case MSM_OPERATIONAL:
               vSensorMngr_WakeUpSensor();   
               vSensorMngr_DataUpdate();
               vSensorMngr_ShutdownSensor();
               break;
            case MSM_CONNECTED:
               break;
            default:
               break;
         }

         for(l_u8Idx = 0u; l_u8Idx < FRAME_CFG_NUMBER; l_u8Idx++)
         {
            if(g_asCfgFrame[l_u8Idx].u8TimeToSend == 1u)
            {
               l_au8Data[0u] = g_asCfgFrame[l_u8Idx].u8TagType;   /* Only SID_TAG */
               l_au8Data[1u] = g_asCfgFrame[l_u8Idx].s8Dimming;
               if(eFrameBuilder_PayloadGet(g_asCfgFrame[l_u8Idx].u8Cmd, &l_au8Data[2u], &l_u8Size) == FRAME_BLD_ERROR_NONE)
               {
                  vES_Slave_RFFrameSend(ES_HOST_NRF, l_au8Data, (uint16_t)l_u8Size + 2u);
                  //(void)eCC430_ITF_SendFrame(CMD_RF_TRANSMISSION, ES_HOST_NRF, ES_HOST_CC430, l_au8Data, (uint16_t)l_u8Size);
                  
                  g_asCfgFrame[l_u8Idx].u8TimeToSend = 0u;
               }
            }
         }
      }
   }   
}


uint8_t u8Mode_Mngt_FrameEventNumberGet(void)
{
   uint8_t l_u8Event = 0u;
   uint8_t l_u8Idx = 0u;
   
   for(l_u8Idx = 0u; l_u8Idx < FRAME_CFG_NUMBER; l_u8Idx++)
   {
      if((g_asCfgFrame[l_u8Idx].u8Active == 1u) && (g_asCfgFrame[l_u8Idx].u8Mode == 1u))
      {
         l_u8Event++;
      }
   }
   
   return l_u8Event;
}

/****************************************************************************************
 * Private functions
 ****************************************************************************************/

/****************************************************************************************
 * End Of File
 ****************************************************************************************/


