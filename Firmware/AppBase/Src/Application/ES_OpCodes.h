/* 
 *  ____  _  _   __   ____   __    ___    ____  _  _  ____  ____  ____  _  _  ____
 * (  __)( \/ ) /  \ (_  _) (  )  / __)  / ___)( \/ )/ ___)(_  _)(  __)( \/ )/ ___)
 *  ) _)  )  ( (  O )  )(    )(  ( (__   \___ \ )  / \___ \  )(   ) _) / \/ \\___ \
 * (____)(_/\_) \__/  (__)  (__)  \___)  (____/(__/  (____/ (__) (____)\_)(_/(____/
 *
 * Copyright (c) 2018 EXOTIC SYSTEMS. All Rights Reserved.
 * Licensees are granted free and non-transferable use of the information.
 * NO WARRANTY of ANY KIND is provided. This heading must NOT be 
 * removed from the file.
 *
 * GENERATED FILE DO NOT EDIT MANUALLY
 * Version 1.1
 * Generated 25/09/2018 11:23:25
 * By Yoann Rebischung
 */
#ifndef ES_OPCODES_H
#define ES_OPCODES_H

/************************************************************************
 * Include Files
 ************************************************************************/
#include <stdint.h>

/************************************************************************
 * Defines
 ************************************************************************/

/************************************************************************
 * Type definitions
 ************************************************************************/
typedef enum _ES_OPCODE_ {
   /* Generic OpCode Cmd 0x00 - 0x1F */
   CMD_RESET                       = 0x01u, /* Reset recipient */
   CMD_GET_BOOTLOADER_VERSION      = 0x02u, /* Get Bootloader Version */
   CMD_GET_FIRMWARE_VERSION        = 0x03u, /* Get Firmware Version(Commit hash available if Build in Debug) */
   CMD_GET_HARDWARE_VERSION        = 0x04u, /* Get Hardware Version */
   CMD_GET_PRODUCT_NAME            = 0x05u, /* Get Product Name */
   CMD_GET_SERIAL_NUMBER           = 0x06u, /* Get Serial Number */
   CMD_GET_COMPILATION_DATA        = 0x07u, /* Get Compilation Data (Build Date/Time) */


   /* User OpCode Cmd 0x20 - 0xEF */
   CMD_SENSOR_CYC_DATA_SUB         = 0x20u, /* Select desired sensor to stream at specific data rate */
   CMD_SENSOR_CYC_DATA_STREAM      = 0x21u, /* Cmd to send DataStrem every x ms once connected */
   CMD_SENSOR_CFG                  = 0x22u, /* Cmd receive when application want current sensor config */
   CMD_SENSOR_DEAD                 = 0x23u, /* Send current dead sensors on board */

   CMD_FRAME_CFG_SET               = 0x30u, /* Select frame refresh rate and command for CC430 */
   CMD_FRAME_CFG_GET               = 0x31u, /* Get current frame windows stored in memory */

   CMD_RF_POWER_SET                = 0x40u, /* Select Radio Tx Power between 10 and -63 */
   CMD_RF_OOK_CARRIER_SET          = 0x41u, /* Cmd to set CC430 RF OOK carrier state */
   CMD_RF_MOD_CARRIER_SET          = 0x42u, /* Cmd to set CC430 RF Modulation carrier state */
   CMD_RF_SETTINGS_GET             = 0x43u, /* Cmd to get CC430 RF settings */
   CMD_RF_SETTINGS_SET             = 0x44u, /* Cmd to set CC430 RF settings */
   CMD_RF_TRANSMISSION             = 0x45u, /* Send CC430 Frame over the air */
   CMD_RF_RETRY_SET                = 0x46u, /* Configure retry and delay of each type of tag */

   CMD_BLE_NAME                    = 0x50u, /* Set the advertise Ble Name */
   CMD_TIMESTAMP                   = 0x51u, /* Set current Time of Application into firmware */


   /* Gen Ack-Diag OpCode Cmd 0xF0 - 0xFF */
   CMD_ACKNOWLEDGE                 = 0xF0u, /* Byte 0 is for NACK or ACK, Byte 1 is for Prev Command, Byte 2-n specific */
   CMD_END_OF_TRANSACTION          = 0xF1u, /* When command require multiple replies this is the last cmd received */
   CMD_DIAG_PING                   = 0xF2u, /* Ping Pong Diag command */
   CMD_DIAG_INTERNAL_COMM          = 0xF3u, /* Internal communication test */

   CMD_RET_ERROR_CODE              = 0xFEu, /* Last Error from command */

} e_ES_OpCode_t;

typedef enum _ES_ACK_ {
   ES_ACK_NOK                      = 0x00u,
   ES_ACK_OK                       = 0x01u,
} e_ES_AckCode_t;

typedef enum _ES_ERROR_CODE_ {
   ERROR_SUCCESS                   = 0x0000u, /* No Error */
   ERROR_UNKNOWN_CMD               = 0x0001u, /* Command inconnue */
   ERROR_UNKNOWN_RECIPIENT         = 0x0002u, /* Destinataire non accessible */
   ERROR_UNKNOWN_SOURCE            = 0x0003u, /* Source inconnue */

} e_ES_ErrorCode_t;


typedef enum _ES_IDDATA_ {
   DATAID_TEMPERATURE              = 0x00u, /* Temperature in degree Celcius */
   DATAID_PRESSURE                 = 0x01u, /* Pressure in hPa */
   DATAID_HUMIDITY                 = 0x02u, /* Humidity in % */
   DATAID_LIGHT                    = 0x03u, /* Light in Lux or Lumens */
   DATAID_UV                       = 0x04u, /* UV in index or A/B */
   DATAID_COLOR                    = 0x05u, /* Color in RGBA */
   DATAID_MICROPHONE               = 0x06u, /* Microphone in dB */
   DATAID_PROXIMITY                = 0x07u, /* Proximity in cm */

   DATAID_VOLTAGE                  = 0x10u, /* Voltage in mV */
   DATAID_CURRENT                  = 0x11u, /* Current in mA */
   DATAID_BATTERY_PERCENT          = 0x12u, /* Battery in % */

   DATAID_ACCELEROMETER            = 0x20u, /* Accelerometer in mg */
   DATAID_LINEAR_ACCELERATION      = 0x21u, /* In mg */
   DATAID_GRAVITY                  = 0x22u, /* in g */
   DATAID_GYROSCOPE                = 0x23u, /* in dps */
   DATAID_ROTATION                 = 0x24u, /* in degree */
   DATAID_MAGNETOMETER             = 0x25u, /* in mG */
   DATAID_ORIENTATION              = 0x26u, /*  */

   DATAID_NMEA                     = 0x30u, /* Depends on what is used GGA/RMC/… */
   DATAID_LOCATION                 = 0x31u, /* Latitude/Longitude/Altitude */
   DATAID_TIMESTAMP                = 0x32u, /* 32 bits timestamp UTC */

} e_ES_DataIdx_t;

/************************************************************************
 * Public function declarations
 ************************************************************************/



#endif /* ES_OPCODES_H */
