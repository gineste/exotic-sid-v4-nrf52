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
 * Date:          26/03/2018 (dd MM YYYY)
 * Author:        Yoann Rebischung
 * Description:   Command Handler for Slave CC430 
 *
 */
 
/****************************************************************************************
 * Include Files
 ****************************************************************************************/
#include <stdint.h>
#include <string.h>

#include "ES_OpCodes.h"
#include "ES_Commands.h"
#include "CC430_Interface.h"
#include "SID_Storage.h"
#include "ModeManagement.h"

#include "GlobalDefs.h"

/* Self include */
#include "ES_SlaveMngt.h"

/****************************************************************************************
 * Defines
 ****************************************************************************************/
#define ACK_SIZE           ((uint8_t)2u)

#define ACK_IDX            ((uint8_t)0u)
#define OP_CODE_IDX        (uint8_t)(ACK_IDX + 1u)
#define DATA_IDX           (uint8_t)(OP_CODE_IDX + 1u)
#define ACK_OP_CODE_IDX    (uint8_t)(ACK_IDX + 2u)

/****************************************************************************************
 * Private type declarations
 ****************************************************************************************/
 
/****************************************************************************************
 * Private function declarations
 ****************************************************************************************/

/****************************************************************************************
 * Variable declarations
 ****************************************************************************************/
static uint16_t g_u16PeriodicTxRFCnter = 0u;
static uint8_t g_u8OnRequestTxRFCnter = 0u;

/****************************************************************************************
 * Public functions
 ****************************************************************************************/ 
/**@brief Function called by CC430 Interface when a message is received/Ack.
 * @param[in]  p_u8OpCode : Op Code of the command received
 * @param[in]  p_u32Expdt : Source
 * @param[in]  p_u32Recpt : Destination
 * @param[in]  p_pu8Data : Buffer on data received
 * @param[in]  p_u16DataLen : Data length
 * @return None
 */
void vES_Slave_CommandHandler(uint8_t p_u8OpCode,	uint32_t p_u32Expdt,	uint32_t p_u32Recpt,	uint8_t *p_pu8Data, uint16_t p_u16DataLen)
{
   if(p_u32Recpt == ES_HOST_CC430)
   {
      switch((e_ES_OpCode_t)p_u8OpCode)
      {
      /* Generic OpCode Cmd 0x00 - 0x1F */
         case CMD_RESET :                          /* Reset recipient */
         case CMD_GET_FIRMWARE_VERSION :           /* Get Firmware Version(Commit hash available if Build in Debug) */  
         case CMD_GET_COMPILATION_DATA :           /* Get Compilation Data (Build Date/Time) */     
            (void)eCC430_ITF_SendFrame(p_u8OpCode, p_u32Expdt, p_u32Recpt, p_pu8Data, p_u16DataLen);        
            break;
         
      /* User OpCode Cmd 0x20 - 0xEF */
         case CMD_RF_POWER_SET :                   /* Select Radio Tx Power between 10 and -63 */
         case CMD_RF_OOK_CARRIER_SET :             /* Cmd to set CC430 RF OOK carrier state */
         case CMD_RF_MOD_CARRIER_SET :             /* Cmd to set CC430 RF Modulation carrier state */
         case CMD_RF_SETTINGS_GET :                /* Cmd to get CC430 RF settings */
         case CMD_RF_SETTINGS_SET :                /* Cmd to set CC430 RF settings */
         case CMD_RF_RETRY_SET :                   /* Configure retry and delay of each type of tag */
            (void)eCC430_ITF_SendFrame(p_u8OpCode, p_u32Expdt, p_u32Recpt, p_pu8Data, p_u16DataLen);
            break;
         
      /* Gen Ack-Diag OpCode Cmd 0xF0 - 0xFF */
         case CMD_ACKNOWLEDGE:
            PRINT_UART_ARRAY("%02X", p_pu8Data, p_u16DataLen);
            PRINT_UART("%s","\n");            
            break;
         
         default:     
            (void)eCC430_ITF_SendFrame(p_u8OpCode, p_u32Expdt, p_u32Recpt, p_pu8Data, p_u16DataLen);       
            break;
      }
   }
   else if(p_u32Recpt == ES_MASTER)
   {
       vES_ReplyFromSlave(ES_HOST_CC430, p_pu8Data, p_u16DataLen);
   }
   else
   {
      
   }
}

void vES_Slave_Init(void)
{
   uint8_t l_au8CfgFrame[FRAME_IN_MEMORY_SIZE] = { 0u };
   uint8_t l_u8Size = 0u;   
   e_SIDStorage_Error_t l_eStorageErr;
   
   l_eStorageErr = eStorage_FrameConfigRead(l_au8CfgFrame, &l_u8Size);
   
   if(l_eStorageErr == SID_STORAGE_ERROR_NONE) 
   {
      l_u8Size /= FRAME_CFG_WINDOWS_BLE_SIZE;
      l_u8Size *= FRAME_CFG_WINDOWS_BLE_SIZE;
      if(u8ModeMngt_BleFrameCfg(l_au8CfgFrame, l_u8Size) == 1u)
      {
         __nop();
      }
   }
}

void vES_Slave_RFFrameSend(uint32_t p_u32Expdt, uint8_t * p_pau8Data, uint16_t p_u16Size)
{
   (void)eCC430_ITF_SendFrame(CMD_RF_TRANSMISSION, p_u32Expdt, ES_HOST_CC430, p_pau8Data, p_u16Size);
   
   if(p_u32Expdt == ES_HOST_NRF)
   {
      g_u16PeriodicTxRFCnter++;
   }
   else
   {
      g_u8OnRequestTxRFCnter++;
   }
}

uint8_t u8ES_Slave_TxOnRequestCounterGet(void)
{
   return g_u8OnRequestTxRFCnter;
}
uint16_t u16ES_Slave_TxPeriodicCounterGet(void)
{
   return g_u16PeriodicTxRFCnter;
}

/****************************************************************************************
 * Private functions
 ****************************************************************************************/

/****************************************************************************************
 * End Of File
 ****************************************************************************************/


