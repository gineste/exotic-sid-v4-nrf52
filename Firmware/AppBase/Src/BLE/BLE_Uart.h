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
 */
#ifndef BLE_UART_H
#define BLE_UART_H

/************************************************************************
 * Include Files
 ************************************************************************/
#include <stdint.h>
#include <stdbool.h>

#include "ble.h"
#include "nrf_ble_gatt.h"

#include "Ble_Service.h"

/************************************************************************
 * Defines
 ************************************************************************/

/************************************************************************
 * Type definitions
 ************************************************************************/

/**@brief Service event type. */
typedef enum _BLE_UART_EVT_TYPE_ {
    BLE_UART_EVT_DATA_RECEIVED,    		/**< Write Data event. */
    BLE_UART_EVT_DATA_SENT,            /**< Data Written event. */
} e_Ble_Uart_EventType_t;

/**@brief Service event handler type. */
typedef void (*fpv_BleUart_EvtHandler_t) (s_Ble_Service_Evt_t * p_psEvt);

/**@brief   UART Service structure.
 * @details This structure contains status information related to the service.
 */
typedef struct _BLE_UART_SERVICE_ {
   uint16_t                 u16ConnHandle;            /**< Handle of the current connection (as provided by the SoftDevice). BLE_CONN_HANDLE_INVALID if not in a connection. */
   uint16_t                 u16ServiceHandle;         /**< Handle of Nordic UART Service (as provided by the SoftDevice). */
   uint8_t                  u8UuidType;               /**< UUID type for Nordic UART Service Base UUID. */
   bool                     bBusy;                    /**< busy flag, indicates that the hvx function returned busy and that there are still data to be transfered. */
   bool                     bIsNotificationEnabled;   /**< Variable to indicate if the peer has enabled notification of the RX characteristic.*/
} s_Ble_Uart_Service_t;

/************************************************************************
 * Public function declarations
 ************************************************************************/
void vBLE_Uart_ServiceInit(fpv_BleUart_EvtHandler_t p_sEventHandler);
uint32_t u32BLE_Uart_SendBuffer(uint8_t * p_pu8Buffer, uint16_t p_u16Length);
void vBLE_Uart_onGattEvt(nrf_ble_gatt_evt_t const * p_evt);

#endif /* BLE_UART_H */

