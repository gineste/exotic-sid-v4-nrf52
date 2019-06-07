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
 * Description:   Interface of CC430 
 *
 */
 
/****************************************************************************************
 * Include Files
 ****************************************************************************************/
#include <stdint.h>
#include <string.h>
#include <nrf_delay.h>
/* Config */
#include "board.h"
#include "BoardConfig.h"
#include "GlobalDefs.h"

/* Library include */
#include <Libraries/ES_Protocol/ES_Interfaces.h>
#include <Libraries/ES_Protocol/ES_Protocol.h>
#include <Libraries/ES_Queue.h>

#include "ES_SlaveMngt.h"
#include "ES_OpCodes.h"

/* HAL include */
#include "HAL/HAL_GPIO.h"
#include "HAL/HAL_I2C.h"

/* Self include */
#include "CC430_Interface.h"

/****************************************************************************************
 * Defines
 ****************************************************************************************/
#define CC430_I2C_ADDRESS              (uint8_t)0x4D

#define CC430_ESI_MAX_FRAME_SIZE       (255u)
#define CC430_ESI_BUFFER_SIZE		      (4u * CC430_ESI_MAX_FRAME_SIZE)

#define ES_PROTOCOL_SIZE               (uint8_t)8

#define DELAY_WR_N_RD                  (uint32_t)10
#define RETRY_DELAY_NB                 (uint8_t)120   /* Wait max 600ms to get a reply from CC430, 
                                                         depends on transmission retry and delay btw retry */

/****************************************************************************************
 * Private type declarations
 ****************************************************************************************/
typedef enum _CC_ITF_ESI_SIGNALS_ {
	CC430_ESI_NO_SIG			      = 0x0000u,
	CC430_ESI_SIG_INBOX_READY	   = 0x0001u,
	CC430_ESI_SIG_OUTBOX_READY	   = 0x0002u,
} eCC430_ESI_Signals_t;

/****************************************************************************************
 * Private function declarations
 ****************************************************************************************/
static uint8_t	u8CC430_ESI_SendBuffer(uint8_t * p_pu8Buffer, uint16_t p_u16Length);

static uint8_t u8CC430_ESI_CheckFlags(eCC430_ESI_Signals_t p_eFlags);
static void    vCC430_ESI_ClearFlags(eCC430_ESI_Signals_t p_eFlags);
static void    vCC430_ESI_SetFlags(eCC430_ESI_Signals_t p_eFlags);

static uint8_t u8CC430_ESI_OnInboxWrite(uint8_t * p_pu8Buffer, uint16_t p_u16Length);
static uint8_t u8CC430_ESI_OnInboxWriteFinish(void);
static uint8_t u8CC430_ESI_OnInboxRead(uint8_t * p_pu8Buffer, uint16_t * p_pu16Length);
static uint8_t u8CC430_ESI_OnInboxReadFinish(void);
static uint8_t u8CC430_ESI_OnOutboxWrite(uint8_t * p_pu8Buffer, uint16_t p_u16Length);
static uint8_t u8CC430_ESI_OnOutboxWriteFinish(void);
static uint8_t u8CC430_ESI_OnOutboxRead(uint8_t * p_pu8Buffer, uint16_t * p_pu16Length);
static uint8_t u8CC430_ESI_OnOutboxReadFinish(void);

static uint8_t u8ReplySizeGet(uint8_t p_u8OpCode);
static void vIsReadyToRead(void * p_vContext);
   
/****************************************************************************************
 * Variable declarations
 ****************************************************************************************/
static ESI_Interface_t g_sCC430EsiItf;
static ES_Queue_t g_sCC430EsiInboxQueue;
static ES_Queue_t g_sCC430EsiOutboxQueue;
volatile uint32_t g_u32CC430EsiFlags = CC430_ESI_NO_SIG;
volatile uint8_t g_au8CC430InBuffer[CC430_ESI_MAX_FRAME_SIZE] = { 0u };
volatile uint8_t g_u8CC430InBufferSize = 0u;

/****************************************************************************************
 * Public functions
 ****************************************************************************************/ 
/**@brief Initialize Interface of CC430 (Queue,etc,...)
 * @return None
 */
void vCC430_ITF_Init(void)
{
   /* ES Interface settings */
   static uint8_t l_au8InQueueData[CC430_ESI_BUFFER_SIZE] = { 0u };
   static uint8_t l_au8OutQueueData[CC430_ESI_BUFFER_SIZE] = { 0u };
   
   /* Initialization transmit buffer */
   static uint8_t l_au8OutWriteBuffer[CC430_ESI_MAX_FRAME_SIZE] = { 0u };
   
   /* Initialization queue data */
   static uint8_t l_au8InReadBuffer[CC430_ESI_MAX_FRAME_SIZE] = { 0u };
   static ES_Queue_Buffer_t l_sInQueueBuffer =
   {
      .size = CC430_ESI_BUFFER_SIZE,
      .data = l_au8InQueueData
   };

   /* Initialization queue data */
   static uint8_t l_au8OutReadBuffer[CC430_ESI_MAX_FRAME_SIZE] = { 0u };
   static ES_Queue_Buffer_t l_sOutQueueBuffer = {
      .size = CC430_ESI_BUFFER_SIZE,
      .data = l_au8OutQueueData
   };

   /* Initialization frame box used with frame received */
   static ESI_FrameBox_t l_sInbox = {
      .read = 			      &u8CC430_ESI_OnInboxRead,
      .write = 			   &u8CC430_ESI_OnInboxWrite,
      .on_read_finish = 	&u8CC430_ESI_OnInboxReadFinish,
      .on_write_finish = 	&u8CC430_ESI_OnInboxWriteFinish,
      .buffer_read = {
         .data = l_au8InReadBuffer,
         .len = 0u,
         .size = CC430_ESI_MAX_FRAME_SIZE
      },
      .buffer_write = { /* No write buffer */
         .data = (void*) 0u,
         .len = 0u,
         .size = 0u
      }
   };

   /* Initialization frame box used with frame received */
   static ESI_FrameBox_t l_sOutbox = {
      .read = &u8CC430_ESI_OnOutboxRead,
      .write = &u8CC430_ESI_OnOutboxWrite,
      .on_read_finish = &u8CC430_ESI_OnOutboxReadFinish,
      .on_write_finish = &u8CC430_ESI_OnOutboxWriteFinish,
      .buffer_read = {
         .data = l_au8OutReadBuffer,
         .len = 0u,
         .size = CC430_ESI_MAX_FRAME_SIZE
      },
      .buffer_write = {
         .data = l_au8OutWriteBuffer,
         .len = 0u,
         .size = CC430_ESI_MAX_FRAME_SIZE
      },
   };

   /* Create frame handlers */
   static ESI_FrameHandler_t l_sCC_ESI_FrameHandler;
   static ESI_FrameSender_t l_sCC_ESI_FrameSender;
   
   /* Initialization frame handler */
   l_sCC_ESI_FrameHandler.handle = &vES_Slave_CommandHandler;
   l_sCC_ESI_FrameSender.send = &u8CC430_ESI_SendBuffer;

   /* Initialization queue in Rx and Tx */
   ES_Queue_Init(&g_sCC430EsiInboxQueue, &l_sInQueueBuffer, NULL);
   ES_Queue_Init(&g_sCC430EsiOutboxQueue, &l_sOutQueueBuffer, NULL);

   /* Initialization interface */
   ESI_Init(&g_sCC430EsiItf, &l_sInbox, &l_sOutbox, &l_sCC_ESI_FrameHandler, &l_sCC_ESI_FrameSender);
}

/**@brief Function to send data to the corresponding interface.
 * @param[in] p_u8OpCode
 * @param[in] p_u32Expdt
 * @param[in] p_u32Recpt
 * @param[in] p_cpu8Payload
 * @param[in] p_u16Length
 * @return Error Code
 */
ESI_ReturnCode_t eCC430_ITF_SendFrame(uint8_t p_u8OpCode, uint32_t p_u32Expdt, uint32_t p_u32Recpt, const uint8_t * p_cpu8Payload, uint16_t p_u16Length)
{
   return ESI_NewOutgoingFrame(&g_sCC430EsiItf, p_u8OpCode, p_u32Expdt, p_u32Recpt, p_cpu8Payload, p_u16Length);
}

/**@brief Callback with data received by Tx characteristic 
 * @param[in] p_pu8Buffer[] Data received
 * @param[in] p_u16Length Len of data
 * @return None
 */
void vCC430_ITF_DataReceived(uint8_t * p_pu8Buffer, uint16_t p_u16Length)
{
	uint16_t l_u16Idx = 0u;
	
   for(l_u16Idx = 0u; l_u16Idx< p_u16Length; l_u16Idx++)
   {
      if(g_u8CC430InBufferSize < CC430_ESI_MAX_FRAME_SIZE)
      {
         g_au8CC430InBuffer[g_u8CC430InBufferSize] = p_pu8Buffer[l_u16Idx];
         ++g_u8CC430InBufferSize;
      }
   }
   
   if(ESP_Receiving != ESI_NewIncomingFrame(&g_sCC430EsiItf, (uint8_t*)g_au8CC430InBuffer, g_u8CC430InBufferSize))
   {
      g_u8CC430InBufferSize = 0u;
      memset((uint8_t*)g_au8CC430InBuffer, 0u, CC430_ESI_MAX_FRAME_SIZE);
   }
}

/**@brief Idle task of CC430 (waiting to data to send or data receive) 
 * @return None
 */
void vCC430_ITF_IdleTask(void)
{
   if (u8CC430_ESI_CheckFlags(CC430_ESI_SIG_INBOX_READY) == 1u)
   {
      if(ESI_Failed == ESI_ProcessIncomingFrame(&g_sCC430EsiItf))
      {
         vCC430_ESI_ClearFlags(CC430_ESI_SIG_INBOX_READY);
      }
      else{ /* Nothing to do */ }
   }

   if (u8CC430_ESI_CheckFlags(CC430_ESI_SIG_OUTBOX_READY) == 1u)
   {
      if(ESI_Failed == ESI_ProcessOutgoingFrame(&g_sCC430EsiItf))
      {
         vCC430_ESI_ClearFlags(CC430_ESI_SIG_OUTBOX_READY);
      }
      else{ /* Nothing to do */ }
   }   
}

/****************************************************************************************
 * Private functions
 ****************************************************************************************/
static uint8_t	u8CC430_ESI_SendBuffer(uint8_t * p_pu8Buffer, uint16_t p_u16Length)
{
   uint8_t l_u8ErrCode = (uint8_t)ESI_Failed;
   
#if (NO_CC430_ON_BOARD == 0)
   uint32_t l_u32ErrCode = 0u;
   uint8_t l_u8RdSize = 0u;
   uint8_t l_au8RdBuff[CC430_ESI_MAX_FRAME_SIZE] = { 0u };
   
   l_u8RdSize = u8ReplySizeGet(ESP_GetCommand(p_pu8Buffer));
   
	l_u32ErrCode = u32Hal_I2C_WrnRdCb(CC430_I2C_ADDRESS, p_pu8Buffer, (uint8_t)p_u16Length, l_au8RdBuff, l_u8RdSize, I2C_STOP, vIsReadyToRead);
   if(l_u32ErrCode == 0u)
   {
      #if (LOG_CC430 == 1)
         PRINT_UART_ARRAY("%02X", p_pu8Buffer, p_u16Length);
         PRINT_UART("%s","\n");
      
         PRINT_UART_ARRAY("%02X", l_au8RdBuff, l_u8RdSize);
         PRINT_UART("%s","\n");
      #endif
      
      /* Add received data to Incomming Frame */
      vCC430_ITF_DataReceived(l_au8RdBuff, l_u8RdSize);
      
      l_u8ErrCode = (uint8_t)ESI_Succeed;
   }
   else
   {
      PRINT_UART("Error I2C CC430 %d\n",l_u32ErrCode);
   }
#else
   l_u8ErrCode = (uint8_t)ESI_Succeed;
#endif
   
	return l_u8ErrCode;
}

static uint8_t u8CC430_ESI_CheckFlags(eCC430_ESI_Signals_t p_eFlags)
{
   return ((g_u32CC430EsiFlags & (p_eFlags)) == (p_eFlags)) ? 1u : 0u;
}

static void vCC430_ESI_ClearFlags(eCC430_ESI_Signals_t p_eFlags)
{
   g_u32CC430EsiFlags &= (eCC430_ESI_Signals_t) ~(p_eFlags);
}

static void vCC430_ESI_SetFlags(eCC430_ESI_Signals_t p_eFlags)
{
   g_u32CC430EsiFlags |= (p_eFlags);
}


static uint8_t u8CC430_ESI_OnInboxWrite(uint8_t * p_pu8Buffer, uint16_t p_u16Length)
{
   ES_Queue_Write(&g_sCC430EsiInboxQueue, p_pu8Buffer, p_u16Length);
   return (uint8_t)ESI_Succeed;
}

static uint8_t u8CC430_ESI_OnInboxWriteFinish(void)
{
   vCC430_ESI_SetFlags(CC430_ESI_SIG_INBOX_READY);

   return (uint8_t)ESI_Succeed;
}

static uint8_t u8CC430_ESI_OnInboxRead(uint8_t * p_pu8Buffer, uint16_t * p_pu16Length)
{
   uint8_t l_u8RetCode = (uint8_t)ESI_Failed;
   
   if(ES_Queue_Read(&g_sCC430EsiInboxQueue, p_pu8Buffer, p_pu16Length) == ES_Queue_Succeed)
   {
      l_u8RetCode = (uint8_t)ESI_Succeed;
   }
   else
   {
      l_u8RetCode = (uint8_t)ESI_Failed;
   }

   return l_u8RetCode;
}

static uint8_t u8CC430_ESI_OnInboxReadFinish(void)
{
   return (uint8_t)ESI_Succeed;
}

static uint8_t u8CC430_ESI_OnOutboxWrite(uint8_t * p_pu8Buffer, uint16_t p_u16Length)
{
   ES_Queue_Write(&g_sCC430EsiOutboxQueue, p_pu8Buffer, p_u16Length);

   vCC430_ESI_SetFlags(CC430_ESI_SIG_OUTBOX_READY);
   
   return (uint8_t)ESI_Succeed;
}

static uint8_t u8CC430_ESI_OnOutboxWriteFinish(void)
{
   return (uint8_t)ESI_Succeed;
}

static uint8_t u8CC430_ESI_OnOutboxRead(uint8_t * p_pu8Buffer, uint16_t * p_pu16Length)
{
   uint8_t l_u8RetCode = (uint8_t)ESI_Failed;

   if(ES_Queue_Failed != ES_Queue_Read(&g_sCC430EsiOutboxQueue, p_pu8Buffer, p_pu16Length))
   {
      l_u8RetCode = (uint8_t)ESI_Succeed;
   }
   else
   {
      (*p_pu16Length) = 0u;
      l_u8RetCode = (uint8_t)ESI_Failed;
   }

   return l_u8RetCode;
}

static uint8_t u8CC430_ESI_OnOutboxReadFinish(void)
{
   return (uint8_t)ESI_Succeed;
}

static uint8_t u8ReplySizeGet(uint8_t p_u8OpCode)
{
   uint8_t l_u8Size = ES_PROTOCOL_SIZE;
   
   switch((e_ES_OpCode_t)p_u8OpCode)
   {
      case CMD_GET_FIRMWARE_VERSION:
         l_u8Size += 5u;
         break;
      case CMD_GET_COMPILATION_DATA:
         l_u8Size += 23u; 
         break;
      case CMD_RF_SETTINGS_GET:
         l_u8Size += 44u;
         break;
      default:
         l_u8Size += 2u;
         break;      
   }
   
   return l_u8Size;
}

static void vIsReadyToRead(void * p_vContext)
{
   uint8_t l_u8Retry = RETRY_DELAY_NB;
   uint32_t l_u32PinState = 1u;
   
   do{
      nrf_delay_ms(DELAY_WR_N_RD);
      l_u32PinState = u32Hal_GPIO_Read(IRQ_CC);
      l_u8Retry--;
   }while((l_u32PinState == 1u) && (l_u8Retry > 0u));
   __nop();
}

/****************************************************************************************
 * End Of File
 ****************************************************************************************/


