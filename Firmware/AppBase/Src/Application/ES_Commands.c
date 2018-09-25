/* 
 ____  _  _   __   ____   __    ___    ____  _  _  ____  ____  ____  _  _  ____
(  __)( \/ ) /  \ (_  _) (  )  / __)  / ___)( \/ )/ ___)(_  _)(  __)( \/ )/ ___)
 ) _)  )  ( (  O )  )(    )(  ( (__   \___ \ )  / \___ \  )(   ) _) / \/ \\___ \
(____)(_/\_) \__/  (__)  (__)  \___)  (____/(__/  (____/ (__) (____)\_)(_/(____/
 *
 * Copyright (c) 2017 EXOTIC SYSTEMS. All Rights Reserved.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 * Date:          18 07 2017 (dd MM YYYY)
 * Author:        Yoann Rebischung
 * Description:   ES Commands Interface 
 *
 */
 
/******************************************************************************
 * Include Files
 ******************************************************************************/
/* Proprietary includes */
#include <stdint.h>
#include <string.h>

/* Application includes */
#include "FrameBuilder.h"
#include "MainStateMachine.h"
#include "ModeManagement.h"
#include "SID_Storage.h"

/* HAL includes */

/* Driver sensors includes */

/* BLE includes */
#include "BLE/BT_Interface.h"
#include "BLE/BLE_Application.h"

/* Libraries includes */
#include "Libraries/ES_Protocol/ES_Interfaces.h"

#include "Version.h"

#include "GlobalDefs.h"

#include "ES_OpCodes.h"
#include "ES_SlaveMngt.h"
#include "ES_Commands.h"

/************************************************************************
 * Defines
 ************************************************************************/
#define REPLY_SIZE         ((uint16_t)256)
#define ACK_SIZE           ((uint8_t)2)
#define REPLY_START_IDX    ((uint8_t)ACK_SIZE)

/************************************************************************
 * Private type declarations
 ************************************************************************/
 
/************************************************************************
 * Private function declarations
 ************************************************************************/
static e_ES_AckCode_t eFirmwareVersionGet(uint32_t p_u32Recipient, uint8_t * p_pau8Data, uint8_t * p_pu8Size);

/************************************************************************
 * Variable declarations
 ************************************************************************/

/************************************************************************
 * Public functions
 ************************************************************************/  
/**@brief Function called by bt_interface when a message is received by 
 *        smartphone application.
 * @param[in]  p_u8OpCode : Op Code of the command received
 * @param[in]  p_u32Expdt : Source
 * @param[in]  p_u32Recpt : Destination
 * @param[in]  p_pu8Data : Buffer on data received
 * @param[in]  p_u16DataLen : Data length
 * @return None
 */
void vES_CommandHandler(uint8_t p_u8OpCode, uint32_t p_u32Expdt, uint32_t p_u32Recpt, uint8_t *p_pu8Data, uint16_t p_u16DataLen)
{   
   uint8_t l_au8BufferOut[64u] = { 0u };
   uint8_t l_u8SizeOut = 0u;
   uint8_t l_u8SendAck = 0u;
   e_ES_AckCode_t l_eAckRet = ES_ACK_NOK;
   e_ES_ExpRecpt_t l_eExpeditor = ES_HOST_NRF;
      
#if (LOG_BLE == 1)
   PRINT_T2("Ble Rcv : 0x%02x\n",p_u8OpCode);
#endif   /* LOG_BLE */
   if(p_u32Recpt == ES_HOST_NRF)
   {
      switch((e_ES_OpCode_t)p_u8OpCode)
      {
      /* Generic OpCode Cmd 0x00 - 0x1F */
         case CMD_RESET :                          /* Reset recipient */
            break;
         case CMD_GET_BOOTLOADER_VERSION :         /* Get Bootloader Version */
            break;
         case CMD_GET_FIRMWARE_VERSION :           /* Get Firmware Version(Commit hash available if Build in Debug) */                
            l_eAckRet = eFirmwareVersionGet(p_u32Recpt, l_au8BufferOut, &l_u8SizeOut);
            l_eExpeditor = (e_ES_ExpRecpt_t)p_u32Recpt;
            l_u8SendAck = 1u;  
            break;
         case CMD_GET_HARDWARE_VERSION :           /* Get Hardware Version */
            l_au8BufferOut[l_u8SizeOut++] = HW_VERSION_MAJOR;
            l_au8BufferOut[l_u8SizeOut++] = HW_VERSION_MINOR;
            l_au8BufferOut[l_u8SizeOut++] = HW_VERSION_REVISION;
            l_eAckRet = ES_ACK_OK;
            l_u8SendAck = 1u;  
            break;
         case CMD_GET_PRODUCT_NAME :               /* Get Product Name */
         case CMD_GET_SERIAL_NUMBER :              /* Get Serial Number */
            break;
         case CMD_GET_COMPILATION_DATA :           /* Get Compilation Data (Build Date/Time) */         
            l_au8BufferOut[l_u8SizeOut++] = strlen(BUILD_DATE) + strlen(BUILD_TIME) + 1u; /* Separate with ';' */
            memcpy(&l_au8BufferOut[1u], BUILD_DATE, strlen(BUILD_DATE));
            l_au8BufferOut[1u +  strlen(BUILD_DATE)] = ';';
            memcpy(&l_au8BufferOut[2u + strlen(BUILD_DATE)], BUILD_TIME, strlen(BUILD_TIME));
            l_eAckRet = ES_ACK_OK;
            l_u8SendAck = 1u;
            break;
         
      /* User OpCode Cmd 0x20 - 0xEF */
         case CMD_SENSOR_CYC_DATA_SUB:             /* Select desired cmd to stream at specific data rate */
            if(eFrameBuilder_CfgUpdate((e_FrameBuilder_Command_t)p_pu8Data[0u]) != FRAME_BLD_ERROR_NONE)
            {
               l_eAckRet = ES_ACK_NOK;            
            }
            else
            {
               vStateMachine_ChangeTime(U8_TO_U32(p_pu8Data[1u],p_pu8Data[2u],p_pu8Data[3u],p_pu8Data[4u]));
               l_eAckRet = ES_ACK_OK;
            }
            l_u8SendAck = 1u;  
            break;
            
         case CMD_FRAME_CFG_SET:                 /* Select frame  refresh rate and command for CC430 */
            if(u8ModeMngt_BleFrameCfg(p_pu8Data, (uint8_t)p_u16DataLen-1u) == 1u)
            {
               if(eStorage_FrameConfigWrite(p_pu8Data, (uint8_t)p_u16DataLen) == SID_STORAGE_ERROR_NONE)
               {
                  l_eAckRet = ES_ACK_OK;
               }
               else
               {
                  l_eAckRet = ES_ACK_NOK;
               }
            }
            else
            {
               l_eAckRet = ES_ACK_NOK;
            }
            l_u8SendAck = 1u;  
            break;   
         case CMD_FRAME_CFG_GET:                /* Get current frame windows stored in memory */
            if(eStorage_FrameConfigRead(l_au8BufferOut, &l_u8SizeOut) == SID_STORAGE_ERROR_NONE)
            {
               l_u8SizeOut = (l_u8SizeOut > (FRAME_IN_MEMORY_SIZE + 1u))? (FRAME_IN_MEMORY_SIZE + 1u):l_u8SizeOut;
               l_eAckRet = ES_ACK_OK;
            }
            else
            {
               l_eAckRet = ES_ACK_NOK;
            }
            l_u8SendAck = 1u;              
            break;
         
         case CMD_BLE_NAME:                     /* Set the advertise Ble Name */     
            if(eStorage_DeviceNameWrite((char*)p_pu8Data, p_u16DataLen) == SID_STORAGE_ERROR_NONE)
            {
               vBLE_UpdateName((char*)p_pu8Data, (uint8_t)p_u16DataLen);
               l_eAckRet = ES_ACK_OK;
            }
            else
            {
               l_eAckRet = ES_ACK_NOK;
            }
            l_u8SendAck = 1u;
            break;       
         case CMD_TIMESTAMP:                    /* Set current Time of Application into firmware */
            break;
         
      /* Gen Ack-Diag OpCode Cmd 0xF0 - 0xFF */
         case CMD_DIAG_PING:                    /* Ping Pong Diag command */
            break;
         case CMD_DIAG_INTERNAL_COMM:           /* Internal communication test */
            break;
         
         default:
            l_eAckRet = ES_ACK_NOK;
            l_u8SendAck = 1u;
            PRINT_WARNING("%s","BLE COMMAND UNKNOWN");
            break;         
      }      

      if(l_u8SendAck == 1u)
      {
         vES_ReplySendCommand(p_u8OpCode, l_eAckRet, (uint8_t)l_eExpeditor, l_au8BufferOut, l_u8SizeOut);
      }
   }
   else
   {  /* Forward command to next device */
      vES_Slave_CommandHandler(p_u8OpCode, p_u32Expdt, p_u32Recpt, p_pu8Data, p_u16DataLen);
   }
}

void vES_ReplySendCommand(uint8_t p_u8OpCode, uint8_t p_u8Acknowledge, uint8_t p_u8Expt, uint8_t *p_pu8Data, uint16_t p_u16DataLen)
{
   uint8_t l_au8ReplyBuffer[REPLY_SIZE] = { 0u };
   l_au8ReplyBuffer[0u] = p_u8Acknowledge;
   l_au8ReplyBuffer[1u] = p_u8OpCode;
  
   if(   (p_u16DataLen < (REPLY_SIZE - ACK_SIZE)) 
      && (p_u16DataLen > 0u) )
   {
      memcpy(&l_au8ReplyBuffer[REPLY_START_IDX], p_pu8Data, p_u16DataLen);
      (void)eBT_ITF_SendFrame((uint8_t)CMD_ACKNOWLEDGE, p_u8Expt, ES_MASTER, l_au8ReplyBuffer, (p_u16DataLen + ACK_SIZE));
   }
   else
   {
      (void)eBT_ITF_SendFrame((uint8_t)CMD_ACKNOWLEDGE, p_u8Expt, ES_MASTER, l_au8ReplyBuffer, ACK_SIZE);
   }
}

void vES_ReplyFromSlave(uint8_t p_u8Expt, uint8_t *p_pu8Data, uint16_t p_u16DataLen)
{
   uint8_t l_au8ReplyBuffer[REPLY_SIZE] = { 0u };
  
   if(   (p_u16DataLen < (REPLY_SIZE - ACK_SIZE)) 
      && (p_u16DataLen > 0u) )
   {
      memcpy(l_au8ReplyBuffer, p_pu8Data, p_u16DataLen);
      (void)eBT_ITF_SendFrame((uint8_t)CMD_ACKNOWLEDGE, p_u8Expt, ES_MASTER, l_au8ReplyBuffer, p_u16DataLen);
   }
   else
   {
      (void)eBT_ITF_SendFrame((uint8_t)CMD_ACKNOWLEDGE, p_u8Expt, ES_MASTER, l_au8ReplyBuffer, ACK_SIZE);
   }
}
/************************************************************************
 * Private functions
 ************************************************************************/
static e_ES_AckCode_t eFirmwareVersionGet(uint32_t p_u32Recipient, uint8_t * p_pau8Data, uint8_t * p_pu8Size)
{
   e_ES_AckCode_t l_eAckRet = ES_ACK_NOK;
   if(p_pau8Data != NULL)
   {   
      (*p_pu8Size) = 0u;
   
      if(p_u32Recipient == ES_HOST_NRF)
      {
         p_pau8Data[(*p_pu8Size)++] = FW_VERSION_MAJOR;
         p_pau8Data[(*p_pu8Size)++] = FW_VERSION_MINOR;
         p_pau8Data[(*p_pu8Size)++] = FW_VERSION_REVISION;
      #ifndef RELEASE   /* So in Debug */
         p_pau8Data[(*p_pu8Size)++] = '.';
         memcpy(&p_pau8Data[(*p_pu8Size)], COMMIT_NUMBER, strlen(COMMIT_NUMBER));
         (*p_pu8Size) += strlen(COMMIT_NUMBER);
      #endif         
         l_eAckRet = ES_ACK_OK;
      }
   #ifdef DATA_LOGGER_EN
      else if(p_u32Recipient == ES_HOST_CC430)
      {
         //vES_Log_VersionGet(&p_pau8Data[0]);
         (*p_pu8Size) = 3u;
         l_eAckRet = ES_ACK_OK;
      }
   #endif
      else
      {
         l_eAckRet = ES_ACK_NOK;
      }    
   }

   return l_eAckRet;
}

/************************************************************************
 * End Of File
 ************************************************************************/

