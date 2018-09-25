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
 * Date:          02/03/2018 (dd MM YYYY)
 * Author:        Yoann Rebischung
 * Description:   Frame Builder for BLE/CC430
 *
 */

/****************************************************************************************
 * Include Files
 ****************************************************************************************/
#include <stdint.h>
#include <string.h>

#include <ble_gap.h>

#include "GlobalDefs.h"
#include "BoardConfig.h"

#include "ES_SlaveMngt.h"
#include "SensorManager.h"

/* Self include */
#include "FrameBuilder.h"

/****************************************************************************************
 * Defines
 ****************************************************************************************/
#define MAX_FRAME_SIZE  MAX_FRAME_BUILD_SIZE

#define OFFSET_PRESSURE (uint16_t)300u
#define OFFSET_TEMP     (int16_t)-400

#define TEMPERATURE_MIN -400                 /* in 0.1°C */
#define TEMPERATURE_MAX 850                  /* in 0.1°C */
#define PRESSURE_MIN    3000u                /* in 1/10 hPa */
#define PRESSURE_MAX    11000u               /* in 1/10 hPa */

/****************************************************************************************
 * Private type declarations
 ****************************************************************************************/
typedef void (*fpvSensorDataGet)(uint8_t * p_u8Data, uint8_t * p_pu8Size);

typedef enum _DATA_SENSORS_IDX_ {
   DATA_BATTERY_IDX = 0u,
   DATA_TEMP_IDX,
   DATA_PRESSURE_IDX,
   DATA_HUMIDITY_IDX,
   DATA_ACCEL_IDX,
   DATA_GYRO_IDX,
   DATA_MAG_IDX,
   DATA_IDX_MAX
}e_DataSensorsIdx_t;

/****************************************************************************************
 * Private function declarations
 ****************************************************************************************/
static uint8_t u8Frame_BuildCmdBLE(uint8_t * p_pu8Buffer);
static uint8_t u8Frame_BuildCmdOne(uint8_t * p_pu8Buffer);
static uint8_t u8Frame_BuildCmdTwo(uint8_t * p_pu8Buffer);
static uint8_t u8Frame_BuildCmdThree(uint8_t * p_pu8Buffer);

static void vGetMACAddress(uint8_t * p_pu8Data, uint8_t * p_pu8Size);
static void vGetSensorData(e_DataSensorsIdx_t p_eDataSensorID, uint8_t * p_pu8Data, uint8_t * p_pu8Size);
   
/****************************************************************************************
 * Variable declarations
 ****************************************************************************************/
static fpvSensorDataGet g_cafpvSensorsDataGet[DATA_IDX_MAX] = {     
   vSensorMngr_BatteryGet,             /*    DATA_BATTERY_IDX = 0u,  */
   vSensorMngr_TemperatureGet,         /*    DATA_TEMP_IDX,          */
   vSensorMngr_PressureGet,            /*    DATA_PRESSURE_IDX,      */
   vSensorMngr_HumidityGet,            /*    DATA_HUMIDITY_IDX,      */
   vSensorMngr_AccelerometerGet,       /*    DATA_ACCEL_IDX,         */                             
   vSensorMngr_GyrometerGet,           /*    DATA_GYRO_IDX,          */                                
   vSensorMngr_MagnetomerGet           /*    DATA_MAG_IDX,           */     
};

static e_FrameBuilder_Command_t g_eDataStreamCmd = FRAME_BLD_CMD_BLE;

#if (AUTOMATIC_STREAM_DATA == 0)
   static uint8_t g_u8CfgUpdated = 0u;
#endif

/****************************************************************************************
 * Public functions
 ****************************************************************************************/ 
/**@brief Function to initialize structure of frame
 * @return None
 */
void vFrameBuilder_Init(void)
{

}

e_FrameBuilder_Error_t eFrameBuilder_CfgUpdate(e_FrameBuilder_Command_t p_eCmd)
{
   e_FrameBuilder_Error_t l_eErrCode = FRAME_BLD_ERROR_PARAM;
   
   if(p_eCmd > FRAME_BLD_CMD_MAX)
   {
      l_eErrCode = FRAME_BLD_ERROR_PARAM;
   }
   else
   {
      g_eDataStreamCmd = p_eCmd;
   
   #if (AUTOMATIC_STREAM_DATA == 0)   
      g_u8CfgUpdated = 1u;
   #endif
      
      l_eErrCode = FRAME_BLD_ERROR_NONE;
   }

   return l_eErrCode;
}

void vFrameBuilder_ResetCfg(void)
{
#if (AUTOMATIC_STREAM_DATA == 0)  
   g_u8CfgUpdated = 0u;
#endif
}

void vFrameBuilder_CfgGet(e_FrameBuilder_Command_t * p_peCmd)
{
   (*p_peCmd) = g_eDataStreamCmd;
}

uint8_t u8FrameBuilder_IsCfgSet(void)
{
#if (AUTOMATIC_STREAM_DATA == 1)
   return 1u;
#else
   return g_u8CfgUpdated;
#endif
}

/**@brief   Function to get payload frame to send with desired protocol.
 * @param[in]  p_u8Command Index of command to build.
 * @param[out] p_pu8Payload Built frame to send
 * @param[out] p_pu8PayloadSize Size of frame
 * @return     Error Code
 */
e_FrameBuilder_Error_t eFrameBuilder_PayloadGet(uint8_t p_u8Command, uint8_t * p_pu8Payload, uint8_t * p_pu8PayloadSize)
{
   e_FrameBuilder_Error_t l_eErrCode = FRAME_BLD_ERROR_PARAM;
   uint8_t l_u8FrameSize = 0u;
   
   if( (p_pu8Payload != NULL) && (p_pu8PayloadSize != NULL) )// && (g_u8CfgUpdated == 1u) )
   {
      /* Build Frame */
      switch((e_FrameBuilder_Command_t)p_u8Command)
      {
         case FRAME_BLD_CMD_BLE:
            l_u8FrameSize = u8Frame_BuildCmdBLE(p_pu8Payload);
         break;
         case FRAME_BLD_CMD_GENERIC:
            l_u8FrameSize = u8Frame_BuildCmdOne(p_pu8Payload);
         break;
         case FRAME_BLD_CMD_MOTION_DATA:
            l_u8FrameSize = u8Frame_BuildCmdTwo(p_pu8Payload);
         break;
         case FRAME_BLD_CMD_STATUS:
            l_u8FrameSize = u8Frame_BuildCmdThree(p_pu8Payload);
         break;
         default:
            break;
      }
      
      (*p_pu8PayloadSize) = l_u8FrameSize;
      
      if(l_u8FrameSize == 0u)
      {
         l_eErrCode = FRAME_BLD_ERROR_PARAM;
      }
      else
      {
         l_eErrCode = FRAME_BLD_ERROR_NONE;
      }
   }
   
   return l_eErrCode;
}

/****************************************************************************************
 * Private functions
 ****************************************************************************************/
/**@brief   Function to build new frame to send with the corresponding 
 *          sensor configuration.
 * @param[out] p_pu8Buffer 
 * @return     Size of buffer, in case of error Size is equal to 0.
 */
static uint8_t u8Frame_BuildCmdBLE(uint8_t * p_pu8Buffer)
{
   uint8_t l_u8Idx = 0u;
   uint8_t l_u8Size = 0u;
   uint8_t l_u8DataLength = 0u;
   uint8_t l_au8Buffer[MAX_FRAME_SIZE] =  { 0u };
   
   if(p_pu8Buffer != NULL)
   {
      /* Serial MSB-LSB */
      vGetMACAddress(l_au8Buffer, &l_u8Size);
      
      /* Command number */
      l_au8Buffer[l_u8Size++] = FRAME_BLD_CMD_BLE;   
      
      memcpy(&p_pu8Buffer[0u], l_au8Buffer, l_u8Size);
      
      /* Build Frame with last data from activated Sensors */
      for(l_u8Idx = 0u;l_u8Idx < DATA_IDX_MAX; l_u8Idx++)
      {
         vGetSensorData((e_DataSensorsIdx_t)l_u8Idx, l_au8Buffer, &l_u8DataLength);
         if(l_u8DataLength != 0u)
         {
            memcpy(&p_pu8Buffer[l_u8Size], l_au8Buffer, l_u8DataLength);
         }
         l_u8Size += l_u8DataLength;
      }
      
      
   }
   
   return l_u8Size;
}

/**@brief   Function to build new frame to send with the corresponding 
 *          sensor configuration.
 * @param[out] p_pau8Buffer 
 * @return     Size of buffer, in case of error Size is equal to 0.
 */
static uint8_t u8Frame_BuildCmdOne(uint8_t * p_pau8Buffer)
{
   uint8_t l_u8Size = 0u;
   uint8_t l_u8GetSize = 0u;
   uint8_t l_au8Buffer[12u] =  { 0u };
   int16_t l_as16Temp[3u] = { 0u };
   uint16_t l_u16Temp = 0u;
   
   /* Serial MSB-LSB */
   vGetMACAddress(p_pau8Buffer, &l_u8Size);
   
   /* Command, Battery and Pressure */
   p_pau8Buffer[l_u8Size] = (uint8_t)((FRAME_BLD_CMD_GENERIC & 0x0F) << 4u);                   /* Command 1 */
   
   vGetSensorData(DATA_BATTERY_IDX, l_au8Buffer, &l_u8GetSize);
   l_au8Buffer[0u] = (l_au8Buffer[0u] / 25u);
   l_au8Buffer[0u] = (l_au8Buffer[0u] >= 4)?3u:l_au8Buffer[0u];
   p_pau8Buffer[l_u8Size] |= (uint8_t)((l_au8Buffer[0u] & 0x03) << 2u);    /* Battery Level Percent 2bits */
   
   vGetSensorData(DATA_PRESSURE_IDX, l_au8Buffer, &l_u8GetSize);
   l_u16Temp = U8_TO_U16(l_au8Buffer[0u], l_au8Buffer[1u]);
   if(PRESSURE_MIN > l_u16Temp)
   {
      l_u16Temp = PRESSURE_MIN;
   }
   else if(PRESSURE_MAX < l_u16Temp)
   {
      l_u16Temp = PRESSURE_MAX;
   }   
   l_u16Temp -= PRESSURE_MIN; /* 0 = 300.0hPa */
   l_u16Temp = (l_u16Temp + 5u) / 10u;
   p_pau8Buffer[l_u8Size++] |= (uint8_t)((l_u16Temp & 0x0300) >> 8u);   /* MSB Pressure */
   
   p_pau8Buffer[l_u8Size++] = (uint8_t)(l_u16Temp & 0x00FF);           /* LSB Pressure */
  
 
   vGetSensorData(DATA_TEMP_IDX, l_au8Buffer, &l_u8GetSize);
   l_as16Temp[0u] = (int16_t)U8_TO_U16(l_au8Buffer[0u], l_au8Buffer[1u]);
   if(TEMPERATURE_MIN > l_as16Temp[0u])
   {
      l_as16Temp[0u] = TEMPERATURE_MIN;
   }
   else if (TEMPERATURE_MAX < l_as16Temp[0u])
   {
      l_as16Temp[0u] = TEMPERATURE_MAX;
   }
   else { /* Nothing to do */ }
   
   l_as16Temp[0u] = ((((l_as16Temp[0u] - TEMPERATURE_MIN) << 1) + 5)/ 10);  /* 0 = -400d°C (TEMPERATURE_OFFSET), Temperature is retrieved in 0.1°C, thus the division by 10 */
                                                                  /* 125°C == 250 => x2 with X << 1u*/
   p_pau8Buffer[l_u8Size++] = (uint8_t)(l_as16Temp[0u] & 0x00FF);      /* Temperature */
   
   /* TODO Add Kind of wakeup User or Periodic */
   /* Byte 5 bit 7*/
   
   /* Byte 5 bit 6:0 */
   vGetSensorData(DATA_HUMIDITY_IDX, l_au8Buffer, &l_u8GetSize);
   p_pau8Buffer[l_u8Size++] = (uint8_t)(l_au8Buffer[0u] & 0x7F);     /* Humidity */
   
   
   /* TODO compute Pitch Roll Yaw */
   vGetSensorData(DATA_ACCEL_IDX, l_au8Buffer, &l_u8GetSize);
   vGetSensorData(DATA_GYRO_IDX, &l_au8Buffer[l_u8GetSize], &l_u8GetSize);
   
   return l_u8Size;
}

/**@brief   Function to build new frame to send with the corresponding 
 *          sensor configuration.
 * @param[out] p_pau8Buffer 
 * @return     Size of buffer, in case of error Size is equal to 0.
 */
static uint8_t u8Frame_BuildCmdTwo(uint8_t * p_pau8Buffer)
{
   uint8_t l_u8Size = 0u;
   uint8_t l_u8GetSize = 0u;
   uint8_t l_au8Buffer[8u] =  { 0u };
   int16_t l_as16Temp[3u] = { 0 };
   
   /* Serial MSB-LSB */
   vGetMACAddress(p_pau8Buffer, &l_u8Size);
   
   /* Command */
   p_pau8Buffer[l_u8Size] = (uint8_t)((FRAME_BLD_CMD_MOTION_DATA & 0x0F) << 4u);       /* Command 2 4bits*/
   
   vGetSensorData(DATA_ACCEL_IDX, l_au8Buffer, &l_u8GetSize);
   l_as16Temp[0u] = (int16_t)U8_TO_U16(l_au8Buffer[0u],l_au8Buffer[1u]);   /* X */
   l_as16Temp[1u] = (int16_t)U8_TO_U16(l_au8Buffer[2u],l_au8Buffer[3u]);   /* Y */
   l_as16Temp[2u] = (int16_t)U8_TO_U16(l_au8Buffer[4u],l_au8Buffer[5u]);   /* Z */   
   
   p_pau8Buffer[l_u8Size++] |= (uint8_t)((l_as16Temp[0u] & 0xFF00) >> 8u);       /* Accel X MSB */
   
   /* Accel X LSB Byte 3 */
   p_pau8Buffer[l_u8Size++] = (uint8_t)(l_as16Temp[0u] & 0x00FF);                /* Accel X LSB */
   
   /* Accel Y MSB Byte 4 */
   p_pau8Buffer[l_u8Size++] = (uint8_t)((l_as16Temp[1u] & 0xFF00) >> 8u);        /* Accel Y MSB */
   
   /* Accel Y LSB & Z MSB Byte 5 */
   p_pau8Buffer[l_u8Size] = (uint8_t)((l_as16Temp[1u] & 0x00FF) << 8u);          /* Accel Y LSB */
   p_pau8Buffer[l_u8Size++] |= (uint8_t)((l_as16Temp[2u] & 0xFF00) >> 8u);       /* Accel Z MSB */   
   
   /* Accel Z LSB Byte 6 */
   p_pau8Buffer[l_u8Size++] = (uint8_t)(l_as16Temp[2u] & 0x00FF);                /* Accel Z LSB */
   
   /* Check FS Range 125DPS */
   vGetSensorData(DATA_GYRO_IDX, l_au8Buffer, &l_u8GetSize);
   /* Gyro X Byte 7 */
   p_pau8Buffer[l_u8Size++] = (uint8_t)(l_au8Buffer[0u] & 0x00FF);                /* Gyro X */
   /* Gyro Y Byte 8 */
   p_pau8Buffer[l_u8Size++] = (uint8_t)(l_au8Buffer[2u] & 0x00FF);                /* Gyro Y */
   /* Gyro Z Byte 9 */
   p_pau8Buffer[l_u8Size++] = (uint8_t)(l_au8Buffer[4u] & 0x00FF);                /* Gyro Z */
   
   return l_u8Size;
}


static uint8_t u8Frame_BuildCmdThree(uint8_t * p_pau8Buffer)
{
   uint8_t l_u8Size = 0u;
   uint8_t l_u8GetSize = 0u;
   uint8_t l_au8Buffer[8u] =  { 0u };
   uint16_t l_u16TransmissionNb = u16ES_Slave_TxPeriodicCounterGet();
   uint8_t l_u8WakeUpNb = 0u;
   
   /* Serial MSB-LSB */
   vGetMACAddress(p_pau8Buffer, &l_u8Size);
   
   /* Command Byte 2 */
   p_pau8Buffer[l_u8Size] = (uint8_t)((FRAME_BLD_CMD_STATUS & 0x0F) << 4u);            /* Command 3 4bits*/
   p_pau8Buffer[l_u8Size] |= (uint8_t)((0 & 0x01) << 3u);                              /* Kind of transmission */
   /* For now it's always Periodic */
   
   /* Counter TX and Wake Up Byte 2 */
   p_pau8Buffer[l_u8Size++] |= (uint8_t)((l_u16TransmissionNb & 0x0500) >> 8u);        /* MSB transmission nb */
   
   /* Byte 3 */
   p_pau8Buffer[l_u8Size++] = (uint8_t)((l_u16TransmissionNb & 0x0FF0) >> 4u);         /* Mid transmission nb */
   
   /* Byte 4 */
   p_pau8Buffer[l_u8Size] = (uint8_t)((l_u16TransmissionNb & 0x000F) << 4u);           /* LSB transmission nb */
   p_pau8Buffer[l_u8Size++] |= (uint8_t)((l_u8WakeUpNb & 0xF0) >> 4u);                 /* MSB Wake up nb */
   
   /* Byte 5 */
   p_pau8Buffer[l_u8Size] = (uint8_t)((l_u8WakeUpNb & 0x0F) << 4u);                    /* LSB Wake up nb */

   /* Battery mV Byte 5 & 6*/
   vGetSensorData(DATA_BATTERY_IDX, l_au8Buffer, &l_u8GetSize);
   /* Start from byte 1 since byte 0 is battery percent */
   p_pau8Buffer[l_u8Size++] |= (uint8_t)((l_au8Buffer[1u] & 0x0F00) >> 4u);            /* MSB battery mV */
   p_pau8Buffer[l_u8Size++] = (uint8_t)(l_au8Buffer[2u]);                              /* LSB battery mV */
   
   return l_u8Size;
}

static void vGetMACAddress(uint8_t * p_pu8Data, uint8_t * p_pu8Size)
{
   ble_gap_addr_t l_sDeviceAddr;
   /* Do not clear p_pu8Size size it already think like that */ 
   /* Serial MSB-LSB */
   if(sd_ble_gap_addr_get(&l_sDeviceAddr) == 0u)
   {
      p_pu8Data[(*p_pu8Size)++] = l_sDeviceAddr.addr[1u];
      p_pu8Data[(*p_pu8Size)++] = l_sDeviceAddr.addr[0u];
   }
   else
   {
      p_pu8Data[(*p_pu8Size)++] = 0xFF;
      p_pu8Data[(*p_pu8Size)++] = 0xFF;
   }
}

static void vGetSensorData(e_DataSensorsIdx_t p_eDataSensorID, uint8_t * p_pu8Data, uint8_t * p_pu8Size)
{
   if(p_eDataSensorID < DATA_IDX_MAX) 
   {
      if((g_cafpvSensorsDataGet[(uint8_t)p_eDataSensorID] != NULL) && (p_pu8Data != NULL) && (p_pu8Size != NULL))
      {
         (*p_pu8Size) = 0u;   /* Just in case */
         (*g_cafpvSensorsDataGet[(uint8_t)p_eDataSensorID])(p_pu8Data, p_pu8Size);
      }
   }
}

/****************************************************************************************
 * End Of File
 ****************************************************************************************/


