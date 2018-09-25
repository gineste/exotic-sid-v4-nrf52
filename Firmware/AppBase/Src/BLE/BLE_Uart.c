/* 
 *  ____  _  _   __   ____   __    ___    ____  _  _  ____  ____  ____  _  _  ____
 * (  __)( \/ ) /  \ (_  _) (  )  / __)  / ___)( \/ )/ ___)(_  _)(  __)( \/ )/ ___)
 *  ) _)  )  ( (  O )  )(    )(  ( (__   \___ \ )  / \___ \  )(   ) _) / \/ \\___ \
 * (____)(_/\_) \__/  (__)  (__)  \___)  (____/(__/  (____/ (__) (____)\_)(_/(____/
 *
 * Copyright (c) 2018 EXOTIC SYSTEMS. All Rights Reserved.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 * Date:          16 01 2018 (dd MM YYYY)
 * Author:        Yoann Rebischung
 * Description:   BLE Uart for nRF5x Devices SDK14 with NUS. 
 *
 */
 
/************************************************************************
 * Include Files
 ************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "ble_types.h"

#include "app_error.h"
#include "Libraries/ES_Queue.h"
#include "BoardConfig.h"
#include "sdk_config.h"

#include "ble.h"
#include "nrf_sdh_ble.h"
#include "nrf_ble_gatt.h"

#include "BLE/BT_Interface.h"

#include "GlobalDefs.h"

#include "BLE_Uart.h"

/************************************************************************
 * Defines
 ************************************************************************/
#define BLE_UART_BLE_OBSERVER_PRIO           BLE_NUS_BLE_OBSERVER_PRIO

#define BLE_UUID_NUS_SERVICE                 0x0001            /**< The UUID of the Nordic UART Service. */
#define BLE_UUID_NUS_RX_CHARACTERISTIC 		 0x0003            /**< The UUID of the RX Characteristic. */
#define BLE_UUID_NUS_TX_CHARACTERISTIC 		 0x0002            /**< The UUID of the TX Characteristic. */

#define MAX_DATA_SIZE                        (NRF_SDH_BLE_GATT_MAX_MTU_SIZE - 3u) /* 3u = 2 for Header size + 1 for OpCode size */

#define BUFFER_MAX_LEN                       ((uint8_t)255)    /* Size max of Ble message to send */
#define BLE_TX_FIFO_DEPTH                    ((uint8_t)4)      /* Number of possible messages in queue */

#define DEFAULT_MTU_SEG_BUFFER_SIZE          ((uint8_t)20)     /* Default MTU effective size */

/************************************************************************
 * Private type declarations
 ************************************************************************/
typedef struct _BLE_UART_TXINFO_ {
   uint8_t u8Locked;                      
   uint8_t au8DataToSend[BUFFER_MAX_LEN];
   uint16_t u16Length;
   uint8_t u8Seg;
} s_Ble_Uart_TxInfo_t;

/************************************************************************
 * Private function declarations
 ************************************************************************/
static void vBleUart_OnConnect(ble_evt_t const * p_pcsBleEvt);
static void vBleUart_OnDisconnect(ble_evt_t const * p_pcsBleEvt);
static void vBleUart_OnWrite(ble_evt_t const * p_pcsBleEvt, s_Ble_Uart_Service_t * p_psUartService);
static void vBleUart_OnTxComplete(ble_evt_t const * p_pcsBleEvt);

static void vBleUart_onBleEvtHandler(ble_evt_t const * p_pcsBleEvt, void * p_pvContext);
static void vBleUart_ServiceEvtHandler(s_Ble_Service_Evt_t * p_psEvt);
static void vBleUart_Reset(void);

static uint32_t u32TxNotifySend(uint8_t * p_pu8DataOut, uint16_t p_u16Size);

/************************************************************************
 * Variable declarations
 ************************************************************************/
/**< Used vendor specific UUID (NUS). */
static const ble_uuid128_t BLE_UUID_UART_BASE = {{0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0, 0x93, 0xF3, 0xA3, 0xB5, 0x00, 0x00, 0x40, 0x6E}}; 

static s_Ble_Service_t g_sBle_Uart_Service;
static s_Ble_Service_Char_Data_t g_sBle_Uart_CharacteristicRx;
static s_Ble_Service_Char_Data_t g_sBle_Uart_CharacteristicTx;

static uint8_t g_au8Ble_Uart_RxBuffer[MAX_DATA_SIZE] = { 0u };
static uint8_t g_au8Ble_Uart_TxBuffer[MAX_DATA_SIZE] = { 0u }; 

static s_Ble_Uart_TxInfo_t g_sBle_Uart_TxInfo = { 0u };

static uint16_t g_u16SegSize = DEFAULT_MTU_SEG_BUFFER_SIZE;

/* Buffer with data to send */
static uint8_t g_au8BleUartOutputQueueBufferData[BLE_TX_FIFO_DEPTH * BUFFER_MAX_LEN];

/* Buffer manager */
static ES_Queue_Buffer_t g_sBleUartOutputQueueBuffer = {
   .size = BLE_TX_FIFO_DEPTH* BUFFER_MAX_LEN,
   .data = g_au8BleUartOutputQueueBufferData
};

/* Queue manager */
static ES_Queue_t g_sBleUartOutputQueue =  {
   .index_write = 0u,
   .index_read = 0u,
   .element_counter = 0u,
   .overlap_counter = 0u,
   .buffer = &g_sBleUartOutputQueueBuffer
};

/************************************************************************
 * Public functions
 ************************************************************************/
/**@brief  Initialize the ble_uart.
 * @param  p_sEventHandler handler.
 * @return None.
 */
void vBLE_Uart_ServiceInit(fpv_BleUart_EvtHandler_t p_sEventHandler)
{
   uint32_t l_u32ErrCode;
   /* NUS UUID */
   ble_uuid128_t l_sUart_Base_Uuid = BLE_UUID_UART_BASE;	

   /* Init service object */
   memset(&g_sBle_Uart_Service, 0u, sizeof(g_sBle_Uart_Service));
   g_sBle_Uart_Service.sBle_Uuid.uuid = BLE_UUID_NUS_SERVICE;
   if(p_sEventHandler != NULL)
   {
      g_sBle_Uart_Service.sEvtHandler = p_sEventHandler;
   }
   else
   {
      g_sBle_Uart_Service.sEvtHandler = vBleUart_ServiceEvtHandler;
   }
   
   /***** Init RX charateristic data struct *****/
   memset(&g_sBle_Uart_CharacteristicRx, 0u, sizeof(g_sBle_Uart_CharacteristicRx));
   g_sBle_Uart_CharacteristicRx.sBle_Uuid.uuid  = BLE_UUID_NUS_RX_CHARACTERISTIC;
   g_sBle_Uart_CharacteristicRx.pu8Desc         = NULL;
   g_sBle_Uart_CharacteristicRx.eRights         = BLE_SERVICE_CH_NOTIFY;
   g_sBle_Uart_CharacteristicRx.sData.u16Length = sizeof(g_au8Ble_Uart_RxBuffer);
   g_sBle_Uart_CharacteristicRx.sData.pu8Data   = g_au8Ble_Uart_RxBuffer;
   
   /***** Init TX charateristic data struct *****/
   memset(&g_sBle_Uart_CharacteristicTx, 0u, sizeof(g_sBle_Uart_CharacteristicTx));
   g_sBle_Uart_CharacteristicTx.sBle_Uuid.uuid  = BLE_UUID_NUS_TX_CHARACTERISTIC;
   g_sBle_Uart_CharacteristicTx.pu8Desc         = NULL;
   g_sBle_Uart_CharacteristicTx.eRights         = (e_Ble_Service_CharRights_t)(BLE_SERVICE_CH_WRITE | BLE_SERVICE_CH_WRITE_WO_RESPONSE);
   g_sBle_Uart_CharacteristicTx.sData.u16Length = sizeof(g_au8Ble_Uart_TxBuffer);
   g_sBle_Uart_CharacteristicTx.sData.pu8Data   = g_au8Ble_Uart_TxBuffer;
   
   /********************* Uart Service *********************/
   /* Add new service */
   l_u32ErrCode = u32Ble_Service_Init_Proprietary(&g_sBle_Uart_Service, l_sUart_Base_Uuid);
   APP_ERROR_CHECK(l_u32ErrCode);
   
   /* Add RX charateristic to service */
   l_u32ErrCode = u32Ble_Service_AddCharacteristic(&g_sBle_Uart_Service, &g_sBle_Uart_CharacteristicRx);
   APP_ERROR_CHECK(l_u32ErrCode);
   
   /* Add TX charateristic to service */
   l_u32ErrCode = u32Ble_Service_AddCharacteristic(&g_sBle_Uart_Service, &g_sBle_Uart_CharacteristicTx);
   APP_ERROR_CHECK(l_u32ErrCode);
   
   /* Initialize ES Queue */
   ES_Queue_Init(&g_sBleUartOutputQueue, &g_sBleUartOutputQueueBuffer, 0);   
   
   /* Add BleUart to Ble Observer/Notifier */
   NRF_SDH_BLE_OBSERVER(g_BLEUart_Observer, BLE_UART_BLE_OBSERVER_PRIO, vBleUart_onBleEvtHandler, NULL);
}

/**@brief  Send buffer over BLE.
 * @param  p_pu8Buffer.
 * @param  p_u16Length.
 * @return l_u32ErrCode.
 */
uint32_t u32BLE_Uart_SendBuffer(uint8_t * p_pu8Buffer, uint16_t p_u16Length)
{   
   uint32_t l_u32ErrCode = NRF_SUCCESS;
   uint16_t l_u16DataSent = 0u;
   
	/* Check if notification is enabled */
   if(   (g_sBle_Uart_Service.u16ConnHandle == BLE_CONN_HANDLE_INVALID) 
      || (g_sBle_Uart_CharacteristicRx.bIsNotificationEnabled == false) )
   {
      return NRF_ERROR_INVALID_STATE;
   }
      
   if(g_sBle_Uart_TxInfo.u8Locked == 0u)
   {
      /* Store data*/
      memcpy(g_sBle_Uart_TxInfo.au8DataToSend, p_pu8Buffer, p_u16Length);
      g_sBle_Uart_TxInfo.u16Length = p_u16Length;
      g_sBle_Uart_TxInfo.u8Seg = 0u;
      
   #if (LOG_BLE_TX == 1)
      PRINT_T2("Ble Wr Send %d \n",p_u16Length);
   #endif
   
      while(   (l_u32ErrCode == NRF_SUCCESS) 
            && (((uint16_t)g_sBle_Uart_TxInfo.u8Seg * g_u16SegSize) < g_sBle_Uart_TxInfo.u16Length) )
      {
         /* Compute new segment to send according to MTU size negotiated with master */
         l_u16DataSent = g_sBle_Uart_TxInfo.u16Length - (g_sBle_Uart_TxInfo.u8Seg * g_u16SegSize);
         l_u16DataSent = (l_u16DataSent > g_u16SegSize) ? g_u16SegSize : l_u16DataSent;
         
         /* Notify Master with part of message */
         l_u32ErrCode = u32TxNotifySend(&(g_sBle_Uart_TxInfo.au8DataToSend[g_sBle_Uart_TxInfo.u8Seg * g_u16SegSize]),
                                       l_u16DataSent);
                                       
         if(l_u32ErrCode == NRF_SUCCESS)
         {
            #if (LOG_BLE_TX == 1) 
               PRINT_T2_ARRAY("%02X", &(g_sBle_Uart_TxInfo.au8DataToSend[g_sBle_Uart_TxInfo.u8Seg * g_u16SegSize]), l_u16DataSent);
               PRINT_T2("%s","\n");
            #endif
            /* Increase segment in order to overflow u16Length (size of the current message) and find when msg in complete */
            g_sBle_Uart_TxInfo.u8Seg++;
         }
         else if (l_u32ErrCode == NRF_ERROR_RESOURCES)
         {  /* Wait for BLE_GATTS_EVT_HVN_TX_COMPLETE. */
            g_sBle_Uart_TxInfo.u8Locked = 1u;
            break;
         }
      }
   #if (LOG_BLE_TX == 1)
      PRINT_T2("%s","\n");
   #endif
      
   }
   else
   {
      ES_Queue_Write(&g_sBleUartOutputQueue, p_pu8Buffer, p_u16Length);
      l_u32ErrCode = NRF_SUCCESS;
   }
   
   return l_u32ErrCode;
}

void vBLE_Uart_onGattEvt(nrf_ble_gatt_evt_t const * p_evt)
{
   g_u16SegSize = p_evt->params.att_mtu_effective - 3u;  /* OPCODE_LENGTH - HANDLE_LENGTH */

   #if (LOG_BLE_RX == 1)
      PRINT_T2("MTU Size %d\n", g_u16SegSize);
   #endif   
}

/************************************************************************
 * Private functions
 ************************************************************************/
/**@brief Callback on Connect.
 * @param[in]  p_psBleEvent  Event handler.
 * @return None.
 */
static void vBleUart_OnConnect(ble_evt_t const * p_pcsBleEvt)
{
   g_sBle_Uart_Service.u16ConnHandle = p_pcsBleEvt->evt.gap_evt.conn_handle;
}

/**@brief Callback on Disconnect.
 * @param[in]  p_psBleEvent  Event handler.
 * @return None.
 */
static void vBleUart_OnDisconnect(ble_evt_t const * p_pcsBleEvt)
{
   vBleUart_Reset();
}

/**@brief Callback on Write.
 * @param[in]  p_psBleEvent  Event handler.
 * @return None.
 */
static void vBleUart_OnWrite(ble_evt_t const * p_pcsBleEvt, s_Ble_Uart_Service_t * p_psUartService)
{
   ble_gatts_evt_write_t const * l_pscEvtWrite = &p_pcsBleEvt->evt.gatts_evt.params.write;
   s_Ble_Service_Char_Data_t * l_psServCh = g_sBle_Uart_Service.psCharacteristics;
   s_Ble_Service_Evt_t l_sServiceEvt;
   
   while(l_psServCh)
   {      
      if(   (l_pscEvtWrite->handle == l_psServCh->sHandle.cccd_handle) 
         && (l_pscEvtWrite->len == 2u) )
      {
         if (ble_srv_is_notification_enabled(l_pscEvtWrite->data))
         {
            l_psServCh->bIsNotificationEnabled = true;
         }
         else
         {
            l_psServCh->bIsNotificationEnabled = false;
         }
         break;	/* EXIT */
      }
      else if(l_pscEvtWrite->handle == l_psServCh->sHandle.value_handle)
      {
         /* Something new */
         memcpy(l_psServCh->sData.pu8Data, l_pscEvtWrite->data, l_pscEvtWrite->len);
         l_psServCh->sData.u16Length = l_pscEvtWrite->len;
         if( g_sBle_Uart_Service.sEvtHandler != NULL )
         {
            l_sServiceEvt.psCh = l_psServCh;
            l_sServiceEvt.u16EvtType = BLE_UART_EVT_DATA_RECEIVED;
            g_sBle_Uart_Service.sEvtHandler(&l_sServiceEvt);
         }
         break;	/* EXIT */         
      }
      l_psServCh = l_psServCh->psNext;  /* Next characteristic */
   }
    
}

/**@brief Callback on Tx Complete.
 * @param[in]  p_psBleEvent  Event handler.
 * @return None.
 */
static void vBleUart_OnTxComplete(ble_evt_t const * p_pcsBleEvt)
{
   uint16_t	l_u8ExitRequested = 0u;
   uint16_t	l_u16DataSent = 0u;
   uint32_t	l_u32ErrCode = NRF_SUCCESS;
   
   if(g_sBle_Uart_TxInfo.u8Locked == 1u)
   {
//      /* Segment/Chunk higher than the length of the message, so we can take the new if any in queue */
//      if( (g_sBle_Uart_TxInfo.u8Seg * g_u16SegSize) >= g_sBle_Uart_TxInfo.u16Length )
//      {
//         if(ES_Queue_Read(&g_sBleUartOutputQueue, g_sBle_Uart_TxInfo.au8DataToSend, &g_sBle_Uart_TxInfo.u16Length) == ES_Queue_Failed)
//         {  /* No more msg in queue, unlock Tx */
//            g_sBle_Uart_TxInfo.u8Locked 	= 0u;
//         }
//         else
//         {  /* New message available in queue start from the beginning */
//            g_sBle_Uart_TxInfo.u8Seg = 0u;
//         }
//      }
   #if (LOG_BLE_TX == 1)      
      PRINT_T2("Ble Tx Send %d \n", g_sBle_Uart_TxInfo.u16Length);
   #endif
   
      while((l_u32ErrCode == NRF_SUCCESS) && (l_u8ExitRequested == 0u))
      {
         if((g_sBle_Uart_TxInfo.u8Seg * g_u16SegSize) < g_sBle_Uart_TxInfo.u16Length)
         {
            /* Compute new chunck to send according to MTU size negociated with master */
            l_u16DataSent = g_sBle_Uart_TxInfo.u16Length - (g_sBle_Uart_TxInfo.u8Seg * g_u16SegSize);
            l_u16DataSent = (l_u16DataSent > g_u16SegSize) ? g_u16SegSize : l_u16DataSent;

            /* Notify Master with part of message */
            l_u32ErrCode = u32TxNotifySend(&(g_sBle_Uart_TxInfo.au8DataToSend[g_sBle_Uart_TxInfo.u8Seg * g_u16SegSize]),
            l_u16DataSent);

            if(l_u32ErrCode == NRF_SUCCESS)
            {
               #if (LOG_BLE == 1) 
                  PRINT_T2_ARRAY("%02X", &(g_sBle_Uart_TxInfo.au8DataToSend[g_sBle_Uart_TxInfo.u8Seg * g_u16SegSize]), l_u16DataSent);
                  PRINT_T2("%s","\n");
               #endif
               /* Increase segment in order to overflow u16Length (size of the current message) and find when msg in complete */
               g_sBle_Uart_TxInfo.u8Seg++;
            } 
            else if (l_u32ErrCode == NRF_ERROR_RESOURCES)
            {   /* Wait for BLE_GATTS_EVT_HVN_TX_COMPLETE. */
               g_sBle_Uart_TxInfo.u8Locked = 1u;
               l_u8ExitRequested = 1u;
            }
         }
         else
         {
            if(ES_Queue_Read(&g_sBleUartOutputQueue, g_sBle_Uart_TxInfo.au8DataToSend, &g_sBle_Uart_TxInfo.u16Length) == ES_Queue_Failed)
            {  /* No more msg in queue, unlock Tx */
               g_sBle_Uart_TxInfo.u8Locked 	= 0u;
               l_u8ExitRequested = 1u;
            }
            else
            {  /* New message available in queue start from the beginning */
               g_sBle_Uart_TxInfo.u8Seg = 0u;
            }
         }
      }
   #if (LOG_BLE_TX == 1)
      PRINT_T2("%s","\n");
   #endif
   
   }
}

/**@brief  BLE Uart Event Manager.
 * @param  p_pcsBleEvt.
 * @param  p_pvContext.
 * @return None.
 */
static void vBleUart_onBleEvtHandler(ble_evt_t const * p_pcsBleEvt, void * p_pvContext)
{   
   switch(p_pcsBleEvt->header.evt_id)
   {
      case BLE_GAP_EVT_CONNECTED:
         vBleUart_OnConnect(p_pcsBleEvt);
      break;
      
      case BLE_GAP_EVT_DISCONNECTED:
      case BLE_GAP_EVT_TIMEOUT:
         vBleUart_OnDisconnect(p_pcsBleEvt);
      break;
      
      case BLE_GATTS_EVT_WRITE:
         vBleUart_OnWrite(p_pcsBleEvt, NULL);
      break;
      
      case BLE_GATTS_EVT_HVN_TX_COMPLETE:
         vBleUart_OnTxComplete(p_pcsBleEvt);
      break;
      
      default:
      break;  
   }
}

static void vBleUart_Reset(void)
{
   g_sBle_Uart_CharacteristicRx.bIsNotificationEnabled  = false;
   g_sBle_Uart_Service.u16ConnHandle = BLE_CONN_HANDLE_INVALID;

   ES_Queue_ClearAll(&g_sBleUartOutputQueue);
   memset(&g_sBle_Uart_TxInfo, 0u, sizeof(g_sBle_Uart_TxInfo));
}

/**@brief Function for handling the Uart Service event.
 * @param[in]   p_psEvt   ble event.
 */
static void vBleUart_ServiceEvtHandler(s_Ble_Service_Evt_t * p_psEvt)
{
   switch ( p_psEvt->u16EvtType ) 
   {
      case BLE_UART_EVT_DATA_RECEIVED:
         vBT_ITF_DataReceived(p_psEvt->psCh->sData.pu8Data, p_psEvt->psCh->sData.u16Length);
         break;

      case BLE_UART_EVT_DATA_SENT:
         break;

      default:
         break;
   }
}

/**@brief Function for notifying master with a part of /or full message.
 * @param[in]   p_pu8DataOut   Start Pointer of the message to send.
 * @param[in]   p_u16Size   Size to send.
 * @return Error Code
 */
static uint32_t u32TxNotifySend(uint8_t * p_pu8DataOut, uint16_t p_u16Size)
{
   uint32_t l_u32ErrCode = NRF_SUCCESS;
   
   ble_gatts_hvx_params_t const l_scHVX_Param =
   {
      .type   = BLE_GATT_HVX_NOTIFICATION,
      .handle = g_sBle_Uart_CharacteristicRx.sHandle.value_handle,
      .p_data = p_pu8DataOut,
      .p_len  = &p_u16Size,
   };
   
   l_u32ErrCode = sd_ble_gatts_hvx(g_sBle_Uart_Service.u16ConnHandle, &l_scHVX_Param);
   
   return l_u32ErrCode;
}

/************************************************************************
 * End Of File
 ************************************************************************/

